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
    /** Executes the stream **/
    //Ehsan
    void run(int nn=0);
    void run(bool annotate, int nn=0);

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

    int get_next_id(){
    	return num_graphs;
    }
    NodeID tail_node();
    //NodeID tail_node(int target);

    void add_graph(int start, int end, char _PE, char _Host_PE);
    NodeID next_layer(std::vector<NodeID>) override;
    void set_common_params(arm_compute::utils::CommonGraphParams);
    void prnt();
    void forward_tail(NodeID nid);

private:
    //Important: GraphContext must be declared *before* the GraphManager because the GraphManager
    //allocates resources from the context and therefore needs to be destroyed before the context during clean up.

    GraphManagerPipeline _manager; /**< Graph manager */
    //std::vector<GraphContext> _ctxs;     /**< Graph context to use */
    GraphContext _ctx;     /**< Graph context to use */
    std::vector<std::unique_ptr<Graph>>        _gs;       /**< Internal graph representation of the stream */
    //Graph        _g;       /**< Internal graph representation of the stream */

    //Ehsan
    //std::chrono::time_point<std::chrono::high_resolution_clock> start;
    //std::chrono::time_point<std::chrono::high_resolution_clock> finish;
    std::vector<double> input_time;
    std::vector<double> task_time;
    std::vector<double> output_time;
    std::vector<double> cost;
    int					num_graphs;
    int					target_graph;
    std::string			name;
    std::vector<char>	PE;
    std::vector<int>	start_layer;
    std::vector<int>	end_layer;
    std::vector<char>	Host_PE;
    //std::vector<NodeID>	Tail_node;
    int					current_layer;
    arm_compute::utils::CommonGraphParams  common_params;
};
} // namespace frontend
} // namespace graph
} // namespace arm_compute
#endif /* ARM_COMPUTE_GRAPH_STREAM_PIPELINE_H */
