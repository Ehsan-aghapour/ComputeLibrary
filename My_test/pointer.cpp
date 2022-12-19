
#include <iostream>
#include <map>
#include <vector>

class Tensor{
    public:
    
    Tensor(int _a){
        a=_a;
        //m[1]=[4];
        m.insert(std::pair<int,int>(1,4));
    }
    std::map<int ,int>* get_m(){
        return &m;
    }
    std::map<int ,int>& get_m2(){
        return m;
    }
    virtual ~Tensor() { };
    int a;
    std::map<int,int> m;
};
class TensorPipe: public Tensor{
    public:
    int b=0;
};

class NodeMap{
public:
    void insert(std::pair<int,int> key, std::pair<int,int> value ){
        if (mm.find(key)!=mm.end()){
            auto vv=mm[key];
            bool exist=false;
            for(auto v:vv){
                if (v==value){
                    std::cerr<<"Already exist!\n";
                    exist=true;
                }
            }
            if(!exist){
                mm[key].push_back(value);
            }
        }
        else{
            std::vector<std::pair<int,int>> vv;
            vv.push_back(value);
            mm.insert(std::make_pair(key,vv) );
        }
    }

    std::pair<int,int> find(std::pair<int,int> key, int target_graph){
        std::pair<int,int> r={-2,-2};
        if(mm.find(key)==mm.end()){
            std::cout<<"There is no mapping for node graph \n";
            r={-1,-1};
            //create a T node and append to the node key.first in graph key.second (add it to mapping also)
            //create a R node in new graph (and add it to mapping)
        }
        else{
            auto maps=mm[key];
            bool exist=false;
            for(auto v:maps){
                if(v.second==target_graph){
                    std::cout<<"There is a mapped node in this graph\n";
                    exist=true;
                    r=v;
                    //change the tail node from key.first to v.first
                    break;
                }
            }
            if(!exist){
                for(auto v:maps){
                    if(v.second==key.second){
                        std::cout<<"The T node in that graph is node: "<<v.first<<"\n";
                        r=v;
                        //create a R node in new graph (and add it to mapping)
                        //add R node into the T node(v.first) of origin graph
                        break;
                    }
                }
            }
        }
        std::cout<<"mappd node for node "<<key.first<<" in graph "<<key.second<<" is node "<<r.first<<" in graph "<<r.second<<std::endl;
        return r;
    }


    void print(){
        for(auto entry:mm){
            std::cout<<"\n\n\n"<<entry.first.first<<" in graph "<<entry.first.second<<std::endl;
            for(auto v: entry.second){
                std::cout<<"equals to: "<<v.first<<" in graph "<<v.second<<std::endl;
            }
        }
    }


private:
    std::map< std::pair<int,int> , std::vector< std::pair<int,int>> > mm;
};

class test{
public:
    std::vector<std::pair<int*,int*>> get_m(){
        return m;
    }
    void add(std::pair<int*,int*> p){
        m.push_back(p);
    }
    void print(){
        for (auto item:m){
            std::cout<<*(item.first)<<","<<*(item.second)<<std::endl;
        }
    }
private:
    std::vector<std::pair<int*,int*>> m;
};

int main(){
    
    test t;
    int *a=new int(2);
    int *b=new int(7);
    auto p=std::make_pair(a,b);
    t.add(p);
    auto v=t.get_m();
    for (auto item:v){
        std::cout<<*(item.first)<<","<<*(item.second)<<std::endl;
    }
    *a=1;
    *b=3;
    
    t.print();
    v[0]={a,b};
    t.print();
    return 0;
}

