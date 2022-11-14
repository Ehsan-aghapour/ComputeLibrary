/*
 * Copyright (c) 2016-2021 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "arm_compute/runtime/CPP/CPPScheduler.h"

#include "arm_compute/core/CPP/ICPPKernel.h"
#include "arm_compute/core/Error.h"
#include "arm_compute/core/Helpers.h"
#include "arm_compute/core/Utils.h"
#include "src/runtime/CPUUtils.h"
#include "support/Mutex.h"
//Ehsan
//#include "arm_compute/gl_vs.h"
#include<chrono>


#include <atomic>
#include <condition_variable>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <system_error>
#include <thread>

namespace arm_compute
{
namespace
{
class ThreadFeeder
{
public:
    /** Constructor
     *
     * @param[in] start First value that will be returned by the feeder
     * @param[in] end   End condition (The last value returned by get_next() will be end - 1)
     */
    explicit ThreadFeeder(unsigned int start = 0, unsigned int end = 0)
        : _atomic_counter(start), _end(end)
    {
    }
    /** Return the next element in the range if there is one.
     *
     * @param[out] next Will contain the next element if there is one.
     *
     * @return False if the end of the range has been reached and next wasn't set.
     */
    bool get_next(unsigned int &next)
    {
        next = atomic_fetch_add_explicit(&_atomic_counter, 1u, std::memory_order_relaxed);
        return next < _end;
    }

private:
    std::atomic_uint   _atomic_counter;
    const unsigned int _end;
};

/** Execute workloads[info.thread_id] first, then call the feeder to get the index of the next workload to run.
 *
 * Will run workloads until the feeder reaches the end of its range.
 *
 * @param[in]     workloads The array of workloads
 * @param[in,out] feeder    The feeder indicating which workload to execute next.
 * @param[in]     info      Threading and CPU info.
 */
void process_workloads(std::vector<IScheduler::Workload> &workloads, ThreadFeeder &feeder, const ThreadInfo &info)
{
    unsigned int workload_index = info.thread_id;
    do
    {
        ARM_COMPUTE_ERROR_ON(workload_index >= workloads.size());
        workloads[workload_index](info);
    }
    while(feeder.get_next(workload_index));
}

void set_thread_affinity(int core_id)
{
    if(core_id < 0)
    {
        return;
    }

#if !defined(__APPLE__)
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(core_id, &set);
    ARM_COMPUTE_EXIT_ON_MSG(sched_setaffinity(0, sizeof(set), &set), "Error setting thread affinity");
#endif /* !defined(__APPLE__) */
}

class Thread final
{
public:
    /** Start a new thread
     *
     * Thread will be pinned to a given core id if value is non-negative
     *
     * @param[in] core_pin Core id to pin the thread on. If negative no thread pinning will take place
     */
    explicit Thread(int core_pin = -1);

    Thread(const Thread &) = delete;
    Thread &operator=(const Thread &) = delete;
    Thread(Thread &&)                 = delete;
    Thread &operator=(Thread &&) = delete;

    /** Destructor. Make the thread join. */
    ~Thread();

    /** Request the worker thread to start executing workloads.
     *
     * The thread will start by executing workloads[info.thread_id] and will then call the feeder to
     * get the index of the following workload to run.
     *
     * @note This function will return as soon as the workloads have been sent to the worker thread.
     * wait() needs to be called to ensure the execution is complete.
     */
    void start(std::vector<IScheduler::Workload> *workloads, ThreadFeeder &feeder, const ThreadInfo &info);

    /** Wait for the current kernel execution to complete. */
    std::chrono::high_resolution_clock::time_point wait();

    /** Function ran by the worker thread. */
    void worker_thread();

    int thread_idd(){
    	return _info.thread_id;
    }

    std::chrono::high_resolution_clock::time_point get_ftime(){
    	return finish_time;
    }

    std::chrono::high_resolution_clock::time_point get_stime(){
        return start_time;
    }

private:
    std::thread                        _thread{};
    ThreadInfo                         _info{};
    std::vector<IScheduler::Workload> *_workloads{ nullptr };
    ThreadFeeder                      *_feeder{ nullptr };
    std::mutex                         _m{};
    std::condition_variable            _cv{};
    bool                               _wait_for_work{ false };
    bool                               _job_complete{ true };
    std::exception_ptr                 _current_exception{ nullptr };
    int                                _core_pin{ -1 };
    std::chrono::high_resolution_clock::time_point finish_time;
    std::chrono::high_resolution_clock::time_point start_time;
};

Thread::Thread(int core_pin)
    : _core_pin(core_pin)
{
    _thread = std::thread(&Thread::worker_thread, this);
}

Thread::~Thread()
{
    // Make sure worker thread has ended
    if(_thread.joinable())
    {
        ThreadFeeder feeder;
        start(nullptr, feeder, ThreadInfo());
        _thread.join();
    }
}

void Thread::start(std::vector<IScheduler::Workload> *workloads, ThreadFeeder &feeder, const ThreadInfo &info)
{
    _workloads = workloads;
    _feeder    = &feeder;
    _info      = info;
    start_time=std::chrono::high_resolution_clock::now();
    {
        std::lock_guard<std::mutex> lock(_m);
        _wait_for_work = true;
        _job_complete  = false;
    }
    _cv.notify_one();
}

std::chrono::high_resolution_clock::time_point Thread::wait()
{
    {
        std::unique_lock<std::mutex> lock(_m);
        _cv.wait(lock, [&] { return _job_complete; });
        std::chrono::high_resolution_clock::time_point t1=std::chrono::high_resolution_clock::now();
        //join_time=t1;
        return t1;
    }

    if(_current_exception)
    {
        std::rethrow_exception(_current_exception);
    }
}

void Thread::worker_thread()
{
    set_thread_affinity(_core_pin);
    //std::cerr<<"core pin worder thread:"<<_core_pin<<std::endl;
    while(true)
    {
        std::unique_lock<std::mutex> lock(_m);
        _cv.wait(lock, [&] { return _wait_for_work; });
        _wait_for_work = false;

        _current_exception = nullptr;
        //std::cerr<<"work thread processing "<<_core_pin<<std::endl;
        // Time to exit
        if(_workloads == nullptr)
        {
        	//std::cerr<<"\n\n\n\n\n\njjjjjjjjj\n\n\n\n\n\n";
            return;
        }

#ifndef ARM_COMPUTE_EXCEPTIONS_DISABLED
        try
        {
#endif /* ARM_COMPUTE_EXCEPTIONS_ENABLED */
            process_workloads(*_workloads, *_feeder, _info);

#ifndef ARM_COMPUTE_EXCEPTIONS_DISABLED
        }
        catch(...)
        {
            _current_exception = std::current_exception();
        }
#endif /* ARM_COMPUTE_EXCEPTIONS_DISABLED */
        _job_complete = true;
        finish_time=std::chrono::high_resolution_clock::now();
        //std::chrono::high_resolution_clock::time_point
        //auto tm=std::chrono::high_resolution_clock::now();
        //std::cerr<<"job complete thread:"<<_info.thread_id<<std::endl;
        lock.unlock();
        _cv.notify_one();
    }
}
} //namespace

struct CPPScheduler::Impl final
{
    explicit Impl(unsigned int thread_hint)
        : _num_threads(thread_hint), _threads(_num_threads - 1)
    {
    	std::cerr<<"inja num th:"<<_num_threads<<std::endl;
    }
    void set_num_threads(unsigned int num_threads, unsigned int thread_hint)
    {
        _num_threads = num_threads == 0 ? thread_hint : num_threads;
        _threads.resize(_num_threads - 1);
    }
    void set_num_threads_with_affinity(unsigned int num_threads, unsigned int thread_hint, arm_compute::graph::GraphConfig cfg, BindFunc func)
    {
    	//std::string tt;
    	//std::cerr<<"00\n";
    	//std::cin>>tt;
    	//std::cerr<<"000\n";
    	//std::cin>>tt;
        _num_threads = num_threads == 0 ? thread_hint : num_threads;

        // Set affinity on main thread

        //std::cerr<<"avaleshe\n";
        //std::cin>>tt;
        set_thread_affinity(func(0, thread_hint, cfg));
        //std::cerr<<"2\n";
          //      std::cin>>tt;

        // Set affinity on worked threads
        _threads.clear();
        std::cerr<<"the size is:"<<_threads.size()<<std::endl;
        //std::cerr<<"3\n";
          //      std::cin>>tt;
        for(auto i = 1U; i < _num_threads; ++i)
        {
            _threads.emplace_back(func(i, thread_hint, cfg));
            //std::cerr<<i+3<<"\n";
              //      std::cin>>tt;
        }
        //std::cerr<<"7\n";
          //      std::cin>>tt;
    }
    unsigned int num_threads() const
    {
        return _num_threads;
    }

    void run_workloads(std::vector<IScheduler::Workload> &workloads);

    unsigned int       _num_threads;
    std::list<Thread>  _threads;
    arm_compute::Mutex _run_workloads_mutex{};
};

/*
 * This singleton has been deprecated and will be removed in future releases
 */
CPPScheduler &CPPScheduler::get()
{
    static CPPScheduler scheduler;
    return scheduler;
}

CPPScheduler::CPPScheduler()
    : _impl(std::make_unique<Impl>(num_threads_hint()))
{
	//Ehsan
	//int mapped_core_index=0;
}

CPPScheduler::~CPPScheduler() = default;

void CPPScheduler::set_num_threads(unsigned int num_threads)
{
    // No changes in the number of threads while current workloads are running
    arm_compute::lock_guard<std::mutex> lock(_impl->_run_workloads_mutex);
    _impl->set_num_threads(num_threads, num_threads_hint());
}

void CPPScheduler::set_num_threads_with_affinity(unsigned int num_threads, arm_compute::graph::GraphConfig cfg, BindFunc func)
{
    // No changes in the number of threads while current workloads are running
    arm_compute::lock_guard<std::mutex> lock(_impl->_run_workloads_mutex);
    _impl->set_num_threads_with_affinity(num_threads, num_threads_hint(), cfg, func);
}

unsigned int CPPScheduler::num_threads() const
{
    return _impl->num_threads();
}

#ifndef DOXYGEN_SKIP_THIS
void CPPScheduler::run_workloads(std::vector<IScheduler::Workload> &workloads)
{
    // Mutex to ensure other threads won't interfere with the setup of the current thread's workloads
    // Other thread's workloads will be scheduled after the current thread's workloads have finished
    // This is not great because different threads workloads won't run in parallel but at least they
    // won't interfere each other and deadlock.

    arm_compute::lock_guard<std::mutex> lock(_impl->_run_workloads_mutex);
    const unsigned int                  num_threads = std::min(_impl->num_threads(), static_cast<unsigned int>(workloads.size()));
    //Ehsan
    std::chrono::high_resolution_clock::time_point tms[num_threads];
    //std::cout<<"\nnum threads:"<<num_threads<<std::endl;

    if(num_threads < 1)
    {
        return;
    }
    ThreadFeeder feeder(num_threads, workloads.size());
    ThreadInfo   info;
    info.cpu_info          = &_cpu_info;
    info.num_threads       = num_threads;
    unsigned int t         = 0;
    auto         thread_it = _impl->_threads.begin();
    for(; t < num_threads - 1; ++t, ++thread_it)
    {
        info.thread_id = t;
        thread_it->start(&workloads, feeder, info);
    }
    auto main_start_time=std::chrono::high_resolution_clock::now();
    info.thread_id = t;
    process_workloads(workloads, feeder, info);
    //tms[num_threads-1]=std::chrono::high_resolution_clock::now();
    auto main_finish_time=std::chrono::high_resolution_clock::now();
    //std::cerr<<"number of threads: "<<num_threads<<std::endl;
    //std::cerr<<"sizee:"<<_impl->_threads.size()<<std::endl;
#ifndef ARM_COMPUTE_EXCEPTIONS_DISABLED
    try
    {
#endif /* ARM_COMPUTE_EXCEPTIONS_DISABLED */
        for(auto &thread : _impl->_threads)
        {
            //tms[thread.thread_idd()]=thread.wait();
            auto join_time=thread.wait();
            //std::cerr<<"thread finished: "<<thread.thread_idd()<<"   time:"<<tms[thread.thread_idd()].time_since_epoch().count()<<std::endl;
           //replicate:task profiling:
            /* std::cerr<<"thread id: "<<thread.thread_idd()
            		<<"   start time: "<<thread.get_stime().time_since_epoch().count()
            		<<"   finish time: "<<thread.get_ftime().time_since_epoch().count()
					<<"   join time: "<<join_time.time_since_epoch().count()
					<<"   idle time: "<<1000000.0*std::chrono::duration_cast<std::chrono::duration<double>>(join_time-thread.get_ftime()).count()
					<<"   process time: "<<1000000.0*std::chrono::duration_cast<std::chrono::duration<double>>(thread.get_ftime()-thread.get_stime()).count()
					<<std::endl;*/
        }
        auto end=std::chrono::high_resolution_clock::now();
        //replicate:task profiling
        /*std::cerr<<"thread id: "<<num_threads-1
        		<<"   start time: "<<main_start_time.time_since_epoch().count()
        		<<"   finish time: "<<main_finish_time.time_since_epoch().count()
				<<"   join time: "<<end.time_since_epoch().count()
				<<"   idle time: "<<1000000.0*std::chrono::duration_cast<std::chrono::duration<double>>(end-main_finish_time).count()
				<<"   process time: "<<1000000.0*std::chrono::duration_cast<std::chrono::duration<double>>(main_finish_time-main_start_time).count()
				<<"\n\n";*/
#ifndef ARM_COMPUTE_EXCEPTIONS_DISABLED
    }
    catch(const std::system_error &e)
    {
        std::cerr << "Caught system_error with code " << e.code() << " meaning " << e.what() << '\n';
    }
#endif /* ARM_COMPUTE_EXCEPTIONS_DISABLED */
}
#endif /* DOXYGEN_SKIP_THIS */

void CPPScheduler::schedule_op(ICPPKernel *kernel, const Hints &hints, const Window &window, ITensorPack &tensors)
{
    schedule_common(kernel, hints, window, tensors);
}

void CPPScheduler::schedule(ICPPKernel *kernel, const Hints &hints)
{
    ITensorPack tensors;
    schedule_common(kernel, hints, kernel->window(), tensors);
}
} // namespace arm_compute
