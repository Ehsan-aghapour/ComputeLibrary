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

//Ehsan


//#include "Power.h"


#include "arm_compute/graph/GraphManagerPipeline.h"



namespace arm_compute
{
namespace graph
{
GraphManagerPipeline::GraphManagerPipeline()
    : GraphManager()
{
	set_num_graphs(1);
}


void GraphManagerPipeline::finalize_graph(Graph &graph, GraphContext &ctx, PassManager &pm, Target target, std::set<int> *blocking_set, int blocking)
{
    // Check if graph has been registered
	std::cerr<<"graph id: "<<graph.id()<<std::endl;
    if(_workloads.find(graph.id()) != std::end(_workloads))
    {
        ARM_COMPUTE_ERROR("Graph is already registered!");
    }
    //std::cout<<"graph id:"<<graph.id()<<std::endl;
    // Apply IR mutating passes
    //std::cerr<<"befor pass 1 graph "<<graph.id()<<std::endl;
    //print_times(graph,1);

    pm.run_type(graph, IGraphMutator::MutationType::IR);
    //std::cerr<<"0\n";
    // Force target to all graph construct
    // TODO (COMPMID-2014) : Support heterogeneous execution
    Target forced_target = target;
    if(!is_target_supported(target))
    {
	//Ehsan
	//std::cout<<"target is not supported."<<std::endl;

        forced_target = get_default_target();
        ARM_COMPUTE_LOG_GRAPH_INFO("Switching target from " << target << " to " << forced_target << std::endl);
    }
#if My_print > -1
    //Ehsan
    std::cout<<"*********force target is: "<<target<<std::endl;
#endif
    force_target_to_graph(graph, forced_target);

    //std::cerr<<"1\n";
    // Setup backend context
    // TODO (COMPMID-2014) : Setup all backends needed by the graph

    setup_requested_backend_context(ctx, forced_target);
    // Configure all tensors
    /*Ehsan:
     * set TensforHandle for all tensors which TensorInfo of TensorAllocator for each TensorHandle is set based on information of each tensor such as shape,datatype,
     * quantinfo and ...
     * strides in bytes for all dimensions also is set in tensorInfo
     */
    //std::cerr<<"2\n";
    detail::configure_all_tensors(graph);
    //std::cerr<<"3\n";
    // Apply backend mutating passes

    //std::cerr<<"befor pass 2 graph "<<graph.id()<<std::endl;
    //print_times(graph,1);
    pm.run_type(graph, IGraphMutator::MutationType::Backend);
    // Perform topological sort
    std::vector<NodeID> topological_sorted_nodes = dfs(graph);
    std::cerr<<"size of topological sorted nodes: "<<topological_sorted_nodes.size()<<std::endl;
    // Validate all nodes
    detail::validate_all_nodes(graph);
    //std::cerr<<"4\n";
    // Configure all nodes
    auto workload = detail::configure_all_nodes_pipeline(graph, ctx, topological_sorted_nodes);
    ARM_COMPUTE_ERROR_ON_MSG(workload.tasks.empty(), "Could not configure all nodes!");
#if My_print > 0
    //Ehsan
    std::cout<<"\nGraphManager, outputs size:"<<workload.outputs.size()<<std::endl;
#endif
    // Allocate const tensors and call accessors
    //std::cerr<<"5\n";
    detail::allocate_const_tensors_pipeline(graph);
    detail::call_all_const_node_accessors(graph);
    std::cerr<<"prepare:\n";
    // Prepare graph
    detail::prepare_all_tasks(workload);
    std::cerr<<workload.inputs.size()<<std::endl;
    //Ehsan
    //std::cerr<<"3"<<std::endl;
    int ii=0;
    //std::set<int> blocking_set1 {1, 2, 3, 4};
    //std::set<int> *blocking_set=&blocking_set1;
    //std::cerr<<"6\n";
    if(blocking_set!=NULL){
		for(auto &task : workload.tasks)
		{
			if(!task.task)
				continue;
			bool b=false;
			if(blocking_set->find(ii) != blocking_set->end()){
				  b=true;
				  task.ending=true;
			}
			if(blocking==1){
				if(blocking_set!=NULL and b && target==arm_compute::graph::Target ::CL)
					task.block=1;
			}
			if(blocking==2){
				if(blocking_set!=NULL && target==arm_compute::graph::Target ::CL){
					task.block=1;
				}
			}

			ii++;
		}
    }
    if(target==arm_compute::graph::Target ::CL){
    	workload.tasks[workload.tasks.size()-1].block=1;
    }
    //std::cerr<<"7\n";
    std::cerr<<"4"<<std::endl;
#if My_print > -1
    //Ehsan
        DotGraphPrinter p;
        p.print(graph,std::cout);
#endif
    std::cerr<<"5\n";
    // Setup tensor memory (Allocate all tensors or setup transition manager)
    std::cerr<<"Big cores: "<<ctx.config().big_cores<<std::endl;
    std::cerr<<ctx.config().use_transition_memory_manager<<std::endl;
    if(ctx.config().use_transition_memory_manager)
    {
#if My_print > -1
    	//Ehsan
    	std::cerr<<"transition memory mangaer is used\n";
#endif

        detail::configure_transition_manager(graph, ctx, workload);
    }
    else
    {
    	std::cerr<<"else\n";
        detail::allocate_all_tensors(graph);
    }
    std::cerr<<"8\n";
    // Finalize Graph context
    ctx.finalize();
    std::cerr<<"9\n";
    // Register graph
    std::lock_guard<std::mutex> lock(_mtx);
    _workloads.insert(std::make_pair(graph.id(), std::move(workload)));
    ARM_COMPUTE_LOG_GRAPH_VERBOSE("Created workload for graph with ID : " << graph.id() << std::endl);
    //std::cerr<<"after pass graph "<<graph.id()<<std::endl;
    //print_times(graph,1);
    //std::cerr<<"10\n";
}
void GraphManagerPipeline::reset_timing(int graph_id){
	input_time[graph_id]=0;
	task_time[graph_id]=0;
	output_time[graph_id]=0;
	transmition_time[graph_id]=0;
	auto it = _workloads.find(graph_id);
	detail::reset_transmit_timings(it->second);
}

void GraphManagerPipeline::execute_graph(Graph &graph, int nn)
{
    // Check if graph is finalized
	/*if(graph.id()==1){
		std::cerr<<"test:\n";
		print_times(graph,1);
	}*/
	std::stringstream stream;
	stream<<"start of execute graph "<<graph.id()<<" in graph manager\n";
	//stream<<"number of workloads: "<<_workloads.size()<<std::endl;
	std::cerr<<stream.str();
	stream.str(std::string());
    auto it = _workloads.find(graph.id());
    ARM_COMPUTE_ERROR_ON_MSG(it == std::end(_workloads), "Graph is not registered!");
    //Ehsan measure input, task and output timings:

    int n=4;
    for(int k=0; k<n;k++)
    {
    	if(measure_when_full){
    		if(k==num_graphs){
    			auto t1=std::chrono::high_resolution_clock::now();
    			reset_timing(graph.id());
    			auto t2=std::chrono::high_resolution_clock::now();
    			double x1=std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
    			std::string s="timing reset (for reseting measurement when pipeline is full) took "+std::to_string(x1*1000)+"ms\n";
    			std::cerr<<s;
    		}
    	}
    	// Call input accessors
		auto tstart=std::chrono::high_resolution_clock::now();
		//stream<<"graph_id:"<<graph.id()<<std::endl;
		//std::cerr<<stream.str();
		stream<<"graph_id:"<<graph.id()<<" calling inputs"<<std::endl;
		std::cerr<<stream.str();
		stream.str(std::string());
        detail::call_all_input_node_accessors(it->second);

        std::cerr<<"call all input called\n";
        auto tfinish=std::chrono::high_resolution_clock::now();
        double x1=std::chrono::duration_cast<std::chrono::duration<double>>(tfinish - tstart).count();
        std::cerr<<"size of input_time: "<<input_time.size()<<std::endl;

        input_time[graph.id()] +=x1;



        //Call All receivers
        tstart=std::chrono::high_resolution_clock::now();
        stream<<"graph_id:"<<graph.id()<<" calling receivers"<<std::endl;
        std::cerr<<stream.str();
        stream.str(std::string());
		detail::call_all_receivers(it->second);

		std::cerr<<"receivers called\n";
		tfinish=std::chrono::high_resolution_clock::now();
		x1=std::chrono::duration_cast<std::chrono::duration<double>>(tfinish - tstart).count();
		input_time[graph.id()] +=x1;





        // Run graph
		//std::cerr<<"\ntask:"<<task<<std::endl;
		stream<<"graph_id:"<<graph.id()<<" Calling all tasks"<<std::endl;
		std::cerr<<stream.str();
		stream.str(std::string());
		detail::call_all_tasks_pipeline(it->second,nn);
		tstart=std::chrono::high_resolution_clock::now();
		double x2=std::chrono::duration_cast<std::chrono::duration<double>>(tstart-tfinish).count();
		task_time[graph.id()] += x2;



		//Call All Senders
		tstart=std::chrono::high_resolution_clock::now();
		stream<<"graph_id:"<<graph.id()<<" Calling all senders"<<std::endl;
		std::cerr<<stream.str();
		stream.str(std::string());
		transmition_time[graph.id()]+=detail::call_all_senders(it->second);

		std::cerr<<"senders called\n";
		tfinish=std::chrono::high_resolution_clock::now();
		x1=std::chrono::duration_cast<std::chrono::duration<double>>(tfinish - tstart).count();
		output_time[graph.id()] +=x1;




        // Call output accessors
		double x3=0;
		stream<<"graph_id:"<<graph.id()<<" calling outputs"<<std::endl;
		std::cerr<<stream.str();
		stream.str(std::string());
        detail::call_all_output_node_accessors(it->second);
        tfinish=std::chrono::high_resolution_clock::now();
        x3=std::chrono::duration_cast<std::chrono::duration<double>>(tfinish - tstart).count();

        stream<<"Graph"<<graph.id()<<"   Input: "<<x1*1000<<"   Task: "<<x2*1000<<"   Out: "<<x3*1000<<"   Proc: "<<(x2+x3)*1000<<std::endl;
        std::cerr<<stream.str();
        stream.str(std::string());
        output_time[graph.id()] +=x3;
    }
}



void GraphManagerPipeline::warmup_and_execute_graph(Graph &graph, int nn)
{
    // Check if graph is finalized
	/*if(graph.id()==1){
		std::cerr<<"test:\n";
		print_times(graph,1);
	}*/
	std::stringstream stream;
	stream<<"start of execute graph "<<graph.id()<<" in graph manager\n";
	//stream<<"number of workloads: "<<_workloads.size()<<std::endl;
	std::cerr<<stream.str();
	stream.str(std::string());
    auto it = _workloads.find(graph.id());
    ARM_COMPUTE_ERROR_ON_MSG(it == std::end(_workloads), "Graph is not registered!");
    //Ehsan measure input, task and output timings:

    /*int cc=warmup_n+(num_graphs-1)-graph.id();
    for(int k=0; k<cc;k++)
        {
		// Call input accessors
		auto tstart=std::chrono::high_resolution_clock::now();
		//stream<<"graph_id:"<<graph.id()<<std::endl;
		//std::cerr<<stream.str();
		stream<<"graph_id:"<<graph.id()<<" calling inputs"<<std::endl;
		std::cerr<<stream.str();
		stream.str(std::string());
		detail::call_all_input_node_accessors(it->second);

		std::cerr<<"call all input called\n";
		auto tfinish=std::chrono::high_resolution_clock::now();
		double x1=std::chrono::duration_cast<std::chrono::duration<double>>(tfinish - tstart).count();
		std::cerr<<"size of input_time: "<<input_time.size()<<std::endl;

		input_time[graph.id()] +=x1;



		//Call All receivers
		tstart=std::chrono::high_resolution_clock::now();
		stream<<"graph_id:"<<graph.id()<<" calling receivers"<<std::endl;
		std::cerr<<stream.str();
		stream.str(std::string());
		detail::call_all_receivers(it->second);

		std::cerr<<"receivers called\n";
		tfinish=std::chrono::high_resolution_clock::now();
		x1=std::chrono::duration_cast<std::chrono::duration<double>>(tfinish - tstart).count();
		input_time[graph.id()] +=x1;





		// Run graph
		//std::cerr<<"\ntask:"<<task<<std::endl;
		stream<<"graph_id:"<<graph.id()<<" Calling all tasks"<<std::endl;
		std::cerr<<stream.str();
		stream.str(std::string());
		detail::call_all_tasks_pipeline(it->second,nn);
		tstart=std::chrono::high_resolution_clock::now();
		double x2=std::chrono::duration_cast<std::chrono::duration<double>>(tstart-tfinish).count();
		task_time[graph.id()] += x2;



		//Call All Senders
		tstart=std::chrono::high_resolution_clock::now();
		stream<<"graph_id:"<<graph.id()<<" Calling all senders"<<std::endl;
		std::cerr<<stream.str();
		stream.str(std::string());
		transmition_time[graph.id()]+=detail::call_all_senders(it->second);

		std::cerr<<"senders called\n";
		tfinish=std::chrono::high_resolution_clock::now();
		x1=std::chrono::duration_cast<std::chrono::duration<double>>(tfinish - tstart).count();
		output_time[graph.id()] +=x1;




		// Call output accessors
		double x3=0;
		stream<<"graph_id:"<<graph.id()<<" calling outputs"<<std::endl;
		std::cerr<<stream.str();
		stream.str(std::string());
		detail::call_all_output_node_accessors(it->second);
		tfinish=std::chrono::high_resolution_clock::now();
		x3=std::chrono::duration_cast<std::chrono::duration<double>>(tfinish - tstart).count();

		stream<<"Graph"<<graph.id()<<"   Input: "<<x1*1000<<"   Task: "<<x2*1000<<"   Out: "<<x3*1000<<"   Proc: "<<(x2+x3)*1000<<std::endl;
		std::cerr<<stream.str();
		stream.str(std::string());
		output_time[graph.id()] +=x3;
	}*/






    int cc=warmup_n+(num_graphs-1)-graph.id();
    int n=4;
    for(int k=0; k<n;k++)
    {

    	if(k==cc){
    		reset_timing(graph.id());
    		if(!parallel){
    			//start power measurement
    			std::cerr<<"non parallel or start with empty pipeline so just first stage synchronized\n";
    		}
    	}
    	// Call input accessors
		auto tstart=std::chrono::high_resolution_clock::now();
		//stream<<"graph_id:"<<graph.id()<<std::endl;
		//std::cerr<<stream.str();
		stream<<"graph_id:"<<graph.id()<<" calling inputs"<<std::endl;
		std::cerr<<stream.str();
		stream.str(std::string());
        detail::call_all_input_node_accessors(it->second);

        std::cerr<<"call all input called\n";
        auto tfinish=std::chrono::high_resolution_clock::now();
        double x1=std::chrono::duration_cast<std::chrono::duration<double>>(tfinish - tstart).count();
        std::cerr<<"size of input_time: "<<input_time.size()<<std::endl;

        input_time[graph.id()] +=x1;



        //Call All receivers
        tstart=std::chrono::high_resolution_clock::now();
        stream<<"graph_id:"<<graph.id()<<" calling receivers"<<std::endl;
        std::cerr<<stream.str();
        stream.str(std::string());
		detail::call_all_receivers(it->second);

		std::cerr<<"receivers called\n";
		tfinish=std::chrono::high_resolution_clock::now();
		x1=std::chrono::duration_cast<std::chrono::duration<double>>(tfinish - tstart).count();
		input_time[graph.id()] +=x1;


		if(k==cc){
			if(measure_when_full && parallel)
			{
				std::unique_lock<std::mutex> lck(_mtx);
				stream<<"\n\n\n\n\ngraph "<<graph.id()<<" wait after set frame "<<cc+1<<" in its input\n\n\n\n";
				std::cerr<<stream.str();
				stream.str(std::string());
				c=c+1;
				if(c==num_graphs){
					//start power measurement
					condVar.notify_all();
				}
				else{
					condVar.wait(lck, [this]{ return (c==num_graphs); });
				}
				lck.unlock();
			}
			/*else if(graph.id()==0){
				//start power measurement
				std::cerr<<"non parallel or start with empty pipeline so just first stage synchronized\n";
			}*/
		}


        // Run graph
		//std::cerr<<"\ntask:"<<task<<std::endl;
		stream<<"graph_id:"<<graph.id()<<" Calling all tasks"<<std::endl;
		std::cerr<<stream.str();
		stream.str(std::string());
		detail::call_all_tasks_pipeline(it->second,nn);
		tstart=std::chrono::high_resolution_clock::now();
		double x2=std::chrono::duration_cast<std::chrono::duration<double>>(tstart-tfinish).count();
		task_time[graph.id()] += x2;



		//Call All Senders
		tstart=std::chrono::high_resolution_clock::now();
		stream<<"graph_id:"<<graph.id()<<" Calling all senders"<<std::endl;
		std::cerr<<stream.str();
		stream.str(std::string());
		transmition_time[graph.id()]+=detail::call_all_senders(it->second);

		std::cerr<<"senders called\n";
		tfinish=std::chrono::high_resolution_clock::now();
		x1=std::chrono::duration_cast<std::chrono::duration<double>>(tfinish - tstart).count();
		output_time[graph.id()] +=x1;




        // Call output accessors
		double x3=0;
		stream<<"graph_id:"<<graph.id()<<" calling outputs"<<std::endl;
		std::cerr<<stream.str();
		stream.str(std::string());
        detail::call_all_output_node_accessors(it->second);
        tfinish=std::chrono::high_resolution_clock::now();
        x3=std::chrono::duration_cast<std::chrono::duration<double>>(tfinish - tstart).count();

        stream<<"Graph"<<graph.id()<<"   Input: "<<x1*1000<<"   Task: "<<x2*1000<<"   Out: "<<x3*1000<<"   Proc: "<<(x2+x3)*1000<<std::endl;
        std::cerr<<stream.str();
        stream.str(std::string());
        output_time[graph.id()] +=x3;
    }
}


//Ehsan

void GraphManagerPipeline::print_times(Graph &graph, int n)
{
	auto it = _workloads.find(graph.id());
	ExecutionWorkload *workload = &it->second;
	double sum=0;
	int c=0;
	int l=0;
	double tt=0;
	for(auto &task:workload->tasks){
		if(!task.task){
			std::cerr<<"nadareeeeeeeee\n";
			continue;
		}
		std::cout<<c++<<"\tLayer Name: "<<task.node->name()
				<<" \t Layer time: "<<task.time(n)
				<<" \t number of inputs: "<<task.node->num_inputs()
				<<" \t input shape: "<<task.node->input(0)->desc().shape
				<<" \t output shape: "<<task.node->output(0)->desc().shape<<std::endl;

		tt+=task.time(n);
		if(task.ending){
			std::cout<<"Layer Number: "<<l<<" \t time: "<<tt<<std::endl;
			tt=0;
			l++;
			std::cout<<"----------------------------\n";
		}
		sum+=task.time(n);
	}
	std::cout<<"\n Sum of Layers time: "<<sum<<std::endl;
}

void GraphManagerPipeline::reset(Graph &graph)
{
	auto it = _workloads.find(graph.id());
	ExecutionWorkload *workload = &it->second;
	for(auto &task:workload->tasks){
		task.reset();
	}
}


} // namespace graph
} // namespace arm_compute
