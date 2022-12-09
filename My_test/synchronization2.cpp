#include <condition_variable>
#include <iostream>
#include <thread>


class DestTensor{
public:
    void check(){
        {
            std::unique_lock<std::mutex> lck(mutex_);
            //condVar.wait(lck, [this]{ return *(get_ready()); });   // (4)
            condVar.wait(lck, [this]{ return get_r(); });   // (4)
            lck.unlock();
    	}
    }
    void set_ready(){
        //std::this_thread::sleep_for(std::chrono::milliseconds(5*1000));
        {
            std::lock_guard<std::mutex> lck(mutex_);
            *ready = true;
            r=true;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cerr << "\n\n\n\n\n\n\n================================================\nStart Running All Subgraphs ...\n"<<
                "====================================================\n\n\n"<< std::endl;
        condVar.notify_all();
    }
    void set_data(int _data){
        data=_data;
    }
    int get_data(){
        return data;
    }
    bool* get_ready(){
        return ready;
    }
    bool get_r(){
        return r;
    }
private:
    std::mutex mutex_;
    std::condition_variable condVar;
    bool* ready=new bool(false);
    bool r=false;
    int data = {0};
};

class SourceTensor{
public:
    void set_dest_tensor(DestTensor* d){
        {
            dest_tensor=d;
    	}
    }
    void check(){
        std::cerr<<"Before check dest_tensor data is "<<dest_tensor->get_data()<<std::endl;
        dest_tensor->check();
        std::cerr<<"After check dest_tensor data is "<<dest_tensor->get_data()<<std::endl;

    }
    DestTensor* get_dest(){
        return dest_tensor;
    }
private:
    DestTensor* dest_tensor = {nullptr};
};



DestTensor dst;
SourceTensor src;
void thread_func(){
    src.check();
}

int main(){
    
    src.set_dest_tensor(&dst);
    std::thread checker=std::thread(thread_func);
    int t;
    std::this_thread::sleep_for(std::chrono::milliseconds(1*1000));
    std::cerr<<"Enter to set dst tensor ready: ";
    std::cin>>t;
    dst.set_data(t);
    dst.set_ready();
    checker.join();
    return 0;
}