
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


int main(){
    NodeMap node_map;
    auto key=std::make_pair(0,0);
    auto value=std::make_pair(3,2);
    node_map.insert(key,value);
    value={4,1};
    node_map.insert(key,value);
    node_map.print();
    key={2,2};
    auto mappedNode=node_map.find(key,3);
    value={3,2};
    node_map.insert(key,value);
    value={7,3};
    node_map.insert(key,value);
    mappedNode=node_map.find(key,3);


    int target_graph=3;
    auto mapped_node=node_map.find(key,target_graph);  
    if (mapped_node.second==-1){
        //create a T node and append to the node key.first in graph key.second (and add it to mapping: node_map.insert(key,std::make_pair(node_id,key.second))))
        //create a R node in new graph (and add it to mapping: node_map.insert(key,std::make_pair(node_id,new_graph_id)) )
    }
    else if(mapped_node.second==target_graph){
        //change the tail node from key.first to v.first
    }
    else if(mapped_node.second==key.second){
        //create a R node in new graph (and add it to mapping: node_map.insert(key,std::make_pair(node_id,new_graph_id)) )
        //add R node into the T node(v.first) of origin graph graph[v.second].node(v.first).add_receiver(R);
    }
    
    return 0;
}

