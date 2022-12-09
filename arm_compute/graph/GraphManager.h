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
#ifndef ARM_COMPUTE_GRAPH_GRAPH_MANAGER_H
#define ARM_COMPUTE_GRAPH_GRAPH_MANAGER_H

#include "arm_compute/graph/Types.h"
#include "arm_compute/graph/Workload.h"

#include <map>

namespace arm_compute
{
namespace graph
{
// Forward declaration
class Graph;
class GraphContext;
class PassManager;

/** Graph manager class
 *
 * Manages a list of graphs along with their resources
 */


class GraphManager
{
public:
    /** Default Constructor **/
    GraphManager();
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    GraphManager(const GraphManager &) = delete;
    /** Default move constructor */
    GraphManager(GraphManager &&) = default;
    /** Prevent instances of this class from being copied (As this class contains pointers) */
    GraphManager &operator=(const GraphManager &) = delete;
    /** Default move assignment operator */
    GraphManager &operator=(GraphManager &&) = default;
    /** Finalizes a given graph
     *
     * @warning At this given time finalize_graph will alter the passed graph,
     *          plan is to avoid by copying the graph structure,
     *          or provide another entry-point for this functionality as it will increase the memory requirements
     *
     * @param[in] graph  Graph to finalize
     * @param[in] ctx    Graph context
     * @param[in] pm     Pass manager to use for any optimization passes
     * @param[in] target Execution target (Single target execution is currently supported)
     */
    void finalize_graph(Graph &graph, GraphContext &ctx, PassManager &pm, Target target, std::set<int> *b=NULL, int blocking=0);
    /** Executes a graph
     *
     * @param[in] graph Graph to execute
     */
    //Ehsan
    //void execute_graph(Graph &graph);
    void execute_graph(Graph &graph, int nn=0);
    void execute_graph(Graph &graph, bool annotate, int nn=0);
    //void execute_graph(Graph &graph, double &in, double &task, double &out, int nn=0);
    //void execute_graph(Graph &graph, double &in, double &task, double &out, bool annotate, int nn=0);
    /** Invalidates the graph execution workload
     *
     * @param[in] graph Graph to invalidate
     */
    void invalidate_graph(Graph &graph);

    //Ehsan
    void print_times(Graph &graph, int n);
    void reset(Graph &graph);

    void set_input_time(double t){
    	input_time=t;
    }
    void set_task_time(double t){
        task_time=t;
    }
    void set_output_time(double t){
        output_time=t;
    }


    double get_input_time(){
    	return input_time;
    }
    double get_task_time(){
    	return task_time;
    }
    double get_output_time(){
    	return output_time;
    }


private:
    std::map<GraphID, ExecutionWorkload> _workloads = {}; /**< Graph workloads */
    double input_time=0;
    double task_time=0;
    double output_time=0;
};
} // namespace graph
} // namespace arm_compute
#endif /* ARM_COMPUTE_GRAPH_GRAPH_MANAGER_H */
