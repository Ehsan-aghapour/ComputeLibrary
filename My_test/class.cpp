
#include <iostream>

class Tensor{
    public:
    
    Tensor(int _a){
        a=_a;
    }
    virtual ~Tensor() { };
    int a;
};
class TensorPipe: public Tensor{
    public:
    int b=0;
};



int main(){
    Tensor *t=new Tensor(4);
    TensorPipe* tp=dynamic_cast<TensorPipe*>(t);
    std::cerr<<tp->a<<" "<<tp->b<<std::endl;
    return 0;
}

