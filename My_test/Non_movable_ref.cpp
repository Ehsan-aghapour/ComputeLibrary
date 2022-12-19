#include<vector>
#include<iostream>
#include<memory>

class Graph
{
public:
    Graph() = default;
    /** Constructor
     *
     * @param[in] id   Graph identification number. Can be used to differentiate between graphs. Default value 0
     * @param[in] name Graph name. Default value empty string
     */
    Graph(int id, std::string name){
        _id=id;
        _name=name;
    }
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    Graph(const Graph &) = delete;
    /** Prevent instances of this class from being copy assigned (As this class contains pointers) */
    Graph &operator=(const Graph &) = delete;
    /** Prevent instances of this class from being moved (As this class contains non movable objects) */
    Graph(Graph &&) = delete;
    /** Prevent instances of this class from being moved (As this class contains non movable objects) */
    Graph &operator=(Graph &&) = delete;
    int _id;
    std::string _name;
};

class Istream{
public:
    std::vector<Graph*> get_refs(){
        return ref_gs;
    }
    std::vector<Graph*> ref_gs;
};


class stream: public Istream
{
public:
    void add_graph(){
        Graph *g=new Graph(gs.size(),"grph");
        
        std::cout<<"Adding Graph: "<<gs.size()<<" name: "<<g->_name<<std::endl;
        gs.emplace_back(g);
        gs.emplace_back(new Graph(gs.size(),"grph2"));
        gs.push_back(std::make_unique<Graph>(gs.size(),"grph3"));
        //gs.push_back(new Graph(gs.size(),"grph"));
        for(int i=0;i<gs.size();i++){
            ref_gs.emplace_back((gs[i].get()));
        }
    }
    Graph& graph(int i){
        std::cout<<"Asking graph "<<i<<" within "<<gs.size()<<std::endl;
        return *(gs[i]);
    }
    std::vector<std::unique_ptr<Graph>> gs;
};



int main(){
    int B;
    std::cout<<"Hi\n";
    Graph g(2,"salam");
    std::cout<<g._name<<std::endl;
    std::vector<Graph*> gs;
    gs.push_back(&g);
    std::cout<<gs[0]->_name<<std::endl;
    std::cout<<"\n************************\n";
    
    stream s;
    s.add_graph();
    std::cout<<s.graph(0)._name<<std::endl;
    std::cout<<s.graph(1)._name<<std::endl;
    std::cout<<s.graph(2)._name<<std::endl;

    return 0;

}
