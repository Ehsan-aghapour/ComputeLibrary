/*
 * Copyright (c) 2018-2019 Arm Limited.
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
#ifndef ARM_COMPUTE_GRAPH_TENSOR_Pipeline_H
#define ARM_COMPUTE_GRAPH_TENSOR_Pipeline_H

#include "arm_compute/graph/Tensor.h"
#include <condition_variable>
#include <thread>



namespace arm_compute
{
namespace graph
{
/** Tensor object **/

class TensorPipelineReceiver
{
public:
    /** Default constructor
     *
     * @param[in] id   Tensor ID
     * @param[in] desc Tensor information
     */
	TensorPipelineReceiver(){
		receiver_ready=false;
		data_sent=false;
	}
	double send_data(){
		{
			std::string s;
			auto tstart=std::chrono::high_resolution_clock::now();
			std::unique_lock<std::mutex> lck(mutex_);
			if(!get_receiver_ready()){
				s=name+" waiting for receiver to get ready\n";
				std::cerr<<s;
			}

			condVar.wait(lck, [this]{ return get_receiver_ready(); });
			auto tend1=std::chrono::high_resolution_clock::now();
			s=name+" transferring data\n";
			std::cerr<<s;
			receiver_ready=false;
			//Transfer data
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			data_sent=true;
			condVar.notify_all();
			s=name+" done\n";
			std::cerr<<s;
			lck.unlock();
			auto tend2=std::chrono::high_resolution_clock::now();
			t_sender_wait+=std::chrono::duration_cast<std::chrono::duration<double>>(tend1 - tstart).count();
			double t=std::chrono::duration_cast<std::chrono::duration<double>>(tend2 - tend1).count();
			t_transmition+=t;
			return t;
		}
	}
	void wait_for_receiver(){
		{
			std::unique_lock<std::mutex> lck(mutex_);
			condVar.wait(lck, [this]{ return get_receiver_ready(); });
			lck.unlock();
		}
		return;
	}
	void signal_receiver(){
		{
				std::unique_lock<std::mutex> lck(mutex_);
				data_sent=true;
				receiver_ready=false;
				condVar.notify_all();
				lck.unlock();
		}
	}
	bool receive_data(){
		{
			//std::lock_guard<std::mutex> lck(mutex_);
			std::string s;
			s=name+" setting ready for getting data\n";
			std::cerr<<s;
			auto tstart=std::chrono::high_resolution_clock::now();
			std::unique_lock<std::mutex> lck(mutex_);
			receiver_ready = true;
			condVar.notify_all();
			if(!get_data_sent()){
				s=name+" waiting for sender to send the data\n";
				std::cerr<<s;
			}
			condVar.wait(lck, [this]{ return get_data_sent(); });
			data_sent=false;
			s=name+" done\n";
			std::cerr<<s;
			lck.unlock();
			auto tend=std::chrono::high_resolution_clock::now();
			t_receiver_wait+=std::chrono::duration_cast<std::chrono::duration<double>>(tend - tstart).count();
		}
		return true;
	}

	bool get_receiver_ready(){
		return receiver_ready;
	}
	bool get_data_sent(){
		return data_sent;
	}
	void set_tensor(Tensor* t){
		tensor=t;
	}
	Tensor* get_tensor(){
		return tensor;
	}

	void set_name(std::string _name){
		name=std::string(_name);
	}

	void reset_timing(){
		t_sender_wait=0;
		t_receiver_wait=0;
		t_transmition=0;
	}
	double get_transmition_time(){
		return t_transmition;
	}
	double get_sender_wait_time(){
		return t_sender_wait;
	}
	double get_receiver_wait_time(){
		return t_receiver_wait;
	}

private:
	Tensor* tensor = nullptr;
	std::mutex mutex_;
	std::condition_variable condVar;
	bool receiver_ready=new bool(false);
	bool data_sent=new bool (false);
	std::string		name;
	double	t_sender_wait;
	double	t_receiver_wait;
	double	t_transmition;
};




class TensorPipelineSender
{
public:
    /** Default constructor
     *
     * @param[in] id   Tensor ID
     * @param[in] desc Tensor information
     */
	void add_receiver(TensorPipelineReceiver* d){
		receivers.emplace_back(d);
	}
	std::vector<TensorPipelineReceiver*> get_dest(){
		return receivers;
	}
	bool send_data(){
		std::string s;

		s=name+ " before check dest_tensor\n";
		std::cerr<<s;
		for(auto rec:receivers){
			rec->send_data();
		}
		s=name+" after check dest_tensor\n";
		std::cerr<<s;
		return true;

	}
	void set_tensor(Tensor* t){
		tensor=t;
	}
	Tensor* get_tensor(){
		return tensor;
	}
	void set_name(std::string _name){
		name=std::string(_name);
	}

private:
	Tensor* tensor = nullptr;
	//vector of receivers instead of one receiver
	std::vector<TensorPipelineReceiver*> receivers;
	std::string name;
};

} // namespace graph
} // namespace arm_compute
#endif /* ARM_COMPUTE_GRAPH_TENSOR_Pipeline_H */
