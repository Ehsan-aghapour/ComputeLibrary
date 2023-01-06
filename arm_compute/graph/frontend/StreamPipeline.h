/*
 * Copyright (c) 2018-2020 Arm Limited.
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
#ifndef ARM_COMPUTE_GRAPH_STREAM_PIPELINE_H
#define ARM_COMPUTE_GRAPH_STREAM_PIPELINE_H

#include "arm_compute/graph/frontend/IStreamPipeline.h"
#include "arm_compute/graph/frontend/IStreamOperators.h"
#include "arm_compute/graph/frontend/Types.h"

//#include "arm_compute/graph/Graph.h"
#include "arm_compute/graph/GraphContext.h"
#include "arm_compute/graph/GraphManagerPipeline.h"


//#include "utils/CommonGraphOptions.h"
#include "arm_compute/graph/GraphPipeline.h"
#include "arm_compute/graph/frontend/Stream.h"

#include "arm_compute/graph/GraphBuilder.h"
#include "utils/CommonGraphOptions.h"

namespace arm_compute
{
namespace graph
{
namespace frontend
{
// Forward Declarations
class ILayer;


class NodeMap{
public:
    void insert(std::pair<NodeID,int> key, std::pair<NodeID,int> value ){
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
            std::vector<std::pair<NodeID,int>> vv;
            vv.push_back(value);
            mm.insert(std::make_pair(key,vv) );
        }
    }

    std::pair<int,int> find(std::pair<NodeID,int> key, int target_graph){
        std::pair<NodeID,int> r={0,-2};
        if(mm.find(key)==mm.end()){
            std::cerr<<"There is no mapping for node graph \n";
            r={0,-1};
            //create a T node and append to the node key.first in graph key.second (add it to mapping also)
            //create a R node in new graph (and add it to mapping)
        }
        else{
            auto maps=mm[key];
            bool exist=false;
            for(auto v:maps){
                if(v.second==target_graph){
                    std::cerr<<"There is a mapped node in this graph\n";
                    exist=true;
                    r=v;
                    //change the tail node from key.first to v.first
                    break;
                }
            }
            if(!exist){
                for(auto v:maps){
                    if(v.second==key.second){
                        std::cerr<<"The T node in that graph is node: "<<v.first<<"\n";
                        r=v;
                        //create a R node in new graph (and add it to mapping)
                        //add R node into the T node(v.first) of origin graph
                        break;
                    }
                }
            }
        }
        std::cerr<<"mappd node for node "<<key.first<<" in graph "<<key.second<<" is node "<<r.first<<" in graph "<<r.second<<std::endl;
        return r;
    }
    std::pair<int,int> find(std::pair<NodeID*,int*> key, int target_graph){
    	return find(std::make_pair(*(key.first), *(key.second)), target_graph);
    }


    void print(){
        for(auto entry:mm){
            std::cerr<<"\n\n\n"<<entry.first.first<<" in graph "<<entry.first.second<<std::endl;
            for(auto v: entry.second){
                std::cerr<<"equals to: "<<v.first<<" in graph "<<v.second<<std::endl;
            }
        }
    }


private:
    std::map< std::pair<NodeID,int> , std::vector< std::pair<NodeID,int>> > mm;
};


/** Stream frontend class to construct simple graphs in a stream fashion */
class StreamPipeline final : public IStreamPipeline
{
public:
    /** Constructor
     *
     * @param[in] id   Stream id
     * @param[in] name Stream name
     */
	StreamPipeline(size_t id, std::string name);
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    StreamPipeline(const StreamPipeline &) = delete;
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    StreamPipeline &operator=(const StreamPipeline &) = delete;
    /** Finalizes the stream for an execution target
     *
     * @param[in] target Execution target
     * @param[in] config (Optional) Graph configuration to use
     */

    void create_graphs();
    void finalize(Target target, const GraphConfig &config, std::set<int> *b=NULL, int blocking=0);
    void finalize_parallel(int i,std::set<int> *b, int blocking);
    /** Executes the stream **/
    //Ehsan
    void run(int nn=0);
    //void run(bool annotate, int nn=0);
    void run_parallel(int i, int n=0);
    void run_w_parallel(int i, int n=0);

    void warmup(int nn=0);
    void run_w(int nn=0);

    //test
    //void finalize(Target target, const GraphConfig &config);


    void measure(int n);
    void reset();
    // Inherited overridden methods
    void add_layer(ILayer &layer) override;

    Graph       &graph() override;
    const Graph &graph() const override;
    /*std::chrono::time_point<std::chrono::high_resolution_clock> get_start_time(){
    	return start;
    }
    std::chrono::time_point<std::chrono::high_resolution_clock> get_finish_time(){
    	return finish;
    }
    void set_start_time(std::chrono::time_point<std::chrono::high_resolution_clock> t){
    	start=t;
    }
    void set_finish_time(std::chrono::time_point<std::chrono::high_resolution_clock> t){
    	finish=t;
    }
    double get_time(){
    	return std::chrono::duration_cast<std::chrono::duration<double>>(finish - start).count();
    }*/

    void set_input_time(int target, double t){
    	_manager.set_input_time(target, t);
    }
    void set_task_time(int target, double t){
        _manager.set_task_time(target, t);
    }
    void set_output_time(int target, double t){
        _manager.set_output_time(target, t);
    }
    void set_cost_time(int target, double t){
    	cost[target]=t;
    }

    double get_input_time(int target){
    	return _manager.get_input_time(target);
    }
    double get_task_time(int target){
    	return _manager.get_task_time(target);
    }
    double get_output_time(int target){
    	return _manager.get_output_time(target);
    }
    double get_cost_time(int target){
        return cost[target];
    }
    StreamPipeline &operator<<(ILayer &layer);
    StreamPipeline &operator<<(ILayer &&layer);
    StreamPipeline &operator<<(Target target_hint);
    StreamPipeline &operator<<(ConvolutionMethod convolution_method_hint);
    StreamPipeline &operator<<(DepthwiseConvolutionMethod depthwise_convolution_method_hint);
    StreamPipeline &operator<<(FastMathHint fast_math_hint);

    int get_next_id(){
    	return num_graphs;
    }
    NodeID tail_node() override;
    //NodeID tail_node(int target);

    void add_graph(int start, int end, char _PE, char _Host_PE);
    NodeID next_layer(std::vector<std::pair<NodeID*,int*>>) override;
    void set_common_params(arm_compute::utils::CommonGraphParams);
    void prnt();
    void forward_tail(NodeID nid) override;
    int target_graph(int layer);
    /*StreamHints &hints() override{
    	std::cerr<<"calling hints in streampipeline\n";
    	return all_hints[graph_id];
    }*/
    cpu_set_t* set_cores(cpu_set_t *set,int _core, bool _one_master_core=false);
    cpu_set_t* set_cores(cpu_set_t *set,char cluster);

    void reset_timings();

    GraphManagerPipeline* manager(){
    	return &_manager;
    }

private:
    //Important: GraphContext must be declared *before* the GraphManager because the GraphManager
    //allocates resources from the context and therefore needs to be destroyed before the context during clean up.

    GraphManagerPipeline _manager; /**< Graph manager */
    //std::vector<GraphContext> _ctxs;     /**< Graph context to use */
    GraphContext _ctx;     /**< Graph context to use */
    std::vector<GraphContext> _ctxs;
    std::vector<std::unique_ptr<Graph>>        _gs;       /**< Internal graph representation of the stream */
    Graph        _g;       /**< Internal graph representation of the stream */

    //Ehsan
    //std::chrono::time_point<std::chrono::high_resolution_clock> start;
    //std::chrono::time_point<std::chrono::high_resolution_clock> finish;
    std::vector<double> input_time;
    std::vector<double> task_time;
    std::vector<double> output_time;
    std::vector<double> cost;
    int					num_graphs;

    std::string			name;
    std::vector<char>	PE;
    std::vector<int>	start_layer;
    std::vector<int>	end_layer;
    std::vector<char>	Host_PE;
    std::vector<StreamHints> all_hints;
    //std::vector<NodeID>	Tail_node;
    int					current_layer;
    arm_compute::utils::CommonGraphParams  common_params;
    std::vector<GraphConfig> 				configs;
    NodeMap				node_map;
    int					n_warmup=2;
};
} // namespace frontend
} // namespace graph
} // namespace arm_compute
#endif /* ARM_COMPUTE_GRAPH_STREAM_PIPELINE_H */
