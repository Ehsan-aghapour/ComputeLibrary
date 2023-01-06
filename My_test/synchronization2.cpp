#include <condition_variable>
#include <iostream>
#include <thread>


class DestTensor{
public:
    void send_data(){
        {
            std::unique_lock<std::mutex> lck(mutex_);
            //condVar.wait(lck, [this]{ return *(get_ready()); });   // (4)
            if(!get_r())
                std::cerr<<"Sender(Src) is waiting for receiver to be ready then send data\n";
            condVar.wait(lck, [this]{ return get_r(); });   // (4)
            *ready = false;
            r=false;
            std::cerr<<"sender is signaled from receiver to send data\n";
            set_data_ready(true);
            condVar.notify_all();
            std::cerr<<"sender sent data done\n";
            lck.unlock();
    	}
    }
    void wait_for_data(){
        //std::this_thread::sleep_for(std::chrono::milliseconds(5*1000));
        {
            //std::lock_guard<std::mutex> lck(mutex_);
            std::cerr<<"Receiver(dest) is ready for getting data\n";
            std::unique_lock<std::mutex> lck(mutex_);
            *ready = true;
            r=true;
        
            //std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            condVar.notify_all();
            if(!get_data_ready())
                std::cerr<<"Receiver is waiting for sender to transfer data\n";
            condVar.wait(lck,[this]{ return get_data_ready(); });
            set_data_ready(false);
            std::cerr<<"Receiver got data done\n";
            std::cerr << "\n\n\n\n\n\n\n================================================\nCommunication complete\n"<<
                "====================================================\n\n\n"<< std::endl;
            lck.unlock();
        }
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

    bool get_data_ready(){
        return *data_ready;
    }
    void set_data_ready(bool v){
        *data_ready=v;
        return;
    }
private:
    std::mutex mutex_;
    std::condition_variable condVar;
    bool* ready=new bool(false);
    bool* data_ready=new bool(false);
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
    void send_data(){
        //std::cerr<<"Before check dest_tensor data is "<<dest_tensor->get_data()<<std::endl;
        dest_tensor->send_data();
        //std::cerr<<"After check dest_tensor data is "<<dest_tensor->get_data()<<std::endl;

    }
    DestTensor* get_dest(){
        return dest_tensor;
    }
private:
    DestTensor* dest_tensor = {nullptr};
};



DestTensor dst;
SourceTensor src;
int range=50;
int lowest=0;
void send_func(){
    for(int i=0;i<100;i++){      
        int random_integer = lowest + rand() % range;
        std::string s=std::to_string(i) + " sender wait for "+std::to_string(random_integer)+'\n';
        std::cerr<<s;
        std::this_thread::sleep_for(std::chrono::milliseconds(random_integer*1));
        src.send_data();
    }
}

void rec_func(){
    for(int i=0;i<100;i++){
        int random_integer = lowest + rand() % range;
        std::string s=std::to_string(i)+" rec wait for "+std::to_string(random_integer)+'\n';
        std::cerr<<s;
        std::this_thread::sleep_for(std::chrono::milliseconds(random_integer*1));
        dst.wait_for_data();
    }
}

int main(){
    
    src.set_dest_tensor(&dst);
    std::thread sender=std::thread(send_func);
    std::thread receiver=std::thread(rec_func);
    /*int t;
    std::this_thread::sleep_for(std::chrono::milliseconds(1*1000));
    std::cerr<<"Enter to set dst tensor ready: ";
    std::cin>>t;
    //dst.set_data(t);
    //dst.set_ready();*/
    sender.join();
    receiver.join();
    return 0;
}