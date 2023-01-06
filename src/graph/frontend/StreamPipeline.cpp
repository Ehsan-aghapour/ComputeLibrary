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
#include "arm_compute/graph/frontend/StreamPipeline.h"

#include "arm_compute/graph/nodes/ReceiverNode.h"
#include "arm_compute/graph/nodes/SenderNode.h"

#include "arm_compute/graph/Utils.h"
#include "arm_compute/graph/frontend/ILayer.h"

namespace arm_compute
{
namespace graph
{
namespace frontend
{
void StreamPipeline::set_common_params(arm_compute::utils::CommonGraphParams  _common_params){
	common_params=_common_params;
}
StreamPipeline::StreamPipeline(size_t id, std::string _name)
    : _manager(),  num_graphs(0), name(std::move(_name)), current_layer(0)
{
	std::cerr<<"hey\n";
	graph_id=0;
}


cpu_set_t* StreamPipeline::set_cores(cpu_set_t *set,int _core, bool _one_master_core){
	CPU_ZERO(set);
	if(_one_master_core){
		CPU_SET(_core,set);
	}
	else{
		if(_core < common_params.little_cores){
			for(int i=0;i<common_params.little_cores;i++){
				CPU_SET(i,set);
			}
		}
		else{
			for(int i=common_params.little_cores;i<common_params.total_cores;i++){
				CPU_SET(i,set);
			}
		}
	}
	return set;
}
cpu_set_t* StreamPipeline::set_cores(cpu_set_t *set,char cluster){
	CPU_ZERO(set);
	if(cluster=='L'){
		for(int i=0;i<common_params.little_cores;i++){
			CPU_SET(i,set);
		}
	}
	if(cluster=='B'){
		for(int i=common_params.little_cores;i<common_params.total_cores;i++){
			CPU_SET(i,set);
		}
	}
	return set;
}

void StreamPipeline::finalize(Target target, const GraphConfig &_config, std::set<int> *b, int blocking)
{

	std::cerr<<"Finalizing all graph\n";
	_manager.set_num_graphs(num_graphs);
	std::vector<std::thread> threads;
	bool p=true;
	if(p){
		for(auto i=0;i<_gs.size();i++){

			_ctxs[i].set_config(configs[i]);
			threads.push_back(std::thread(&StreamPipeline::finalize_parallel,this,i,b,blocking));
		}
		for(auto i=0;i<_gs.size();i++){
			threads[i].join();
		}
	}
	else{
		for(auto i=0;i<_gs.size();i++){
			_ctxs[i].set_config(configs[i]);
			finalize_parallel(i,b,blocking);
		}
	}
	for (int c=0;c<_ctxs.size();c++){
		std::cerr<<"context:\n";
		for(const auto& elem : _ctxs[c].memory_managers())
		{
		   std::cout << std::to_string((int)elem.first) << " " << std::to_string((int)elem.second.target)  << "\n";
		}
	}
}

void StreamPipeline::finalize_parallel(int i,std::set<int> *b, int blocking)
{
	PassManager pm = create_default_pass_manager(all_hints[i].target_hint, configs[i]);
	cpu_set_t set;

	char cluster='B';
	if(PE[i]=='L')
		cluster='L';
	std::stringstream stream;
	stream<<"Thread "<<i<<" setting affinity to "<<cluster<<std::endl;
	std::cerr<<stream.str();
	stream.str(std::string());
	set_cores(&set,cluster);
	ARM_COMPUTE_EXIT_ON_MSG(sched_setaffinity(0, sizeof(set), &set), "Error setting thread affinity");
	//std::cerr<<"Thread "<<i<<" set: "<<set.__bits<<std::endl;
	//std::this_thread::sleep_for(std::chrono::milliseconds(20000));
	stream<<"Starting finalizing graph "<<i<<" target "<<std::to_string((int)(all_hints[i].target_hint))<<std::endl;
	std::cerr<<stream.str();
	stream.str(std::string());

	_manager.finalize_graph(*(_gs[i]), _ctxs[i], pm, all_hints[i].target_hint, b, blocking);
	stream<<"Finish finalizing graph "<<i<<std::endl;
	std::cerr<<stream.str();
	stream.str(std::string());
	return;
}

void StreamPipeline::run(int n)
{
	int method=1;
	if (method==0){
		auto t1=std::chrono::high_resolution_clock::now();
		warmup(n);
		auto t2=std::chrono::high_resolution_clock::now();
		reset_timings();
		double x1=std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
		std::cerr<<"warm up took "<<x1*1000<<"ms\n";
		std::cerr<<"start running graphs\n";
		std::vector<std::thread> threads;
		t1=std::chrono::high_resolution_clock::now();
		for(auto i=0;i<_gs.size();i++){
			threads.push_back(std::thread(&StreamPipeline::run_parallel,this,i,n));
		}
		t2=std::chrono::high_resolution_clock::now();
		//double x1=std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();//157ms for 3 threads
		//std::cerr<<x1<<" cost of creating threads for running\n";
		for(auto i=0;i<_gs.size();i++){
			threads[i].join();
		}
	}
	if(method==1){
		std::cerr<<"start running graphs\n";
		std::vector<std::thread> threads;
		for(auto i=0;i<_gs.size();i++){
			threads.push_back(std::thread(&StreamPipeline::run_w_parallel,this,i,n));
		}
		for(auto i=0;i<_gs.size();i++){
			threads[i].join();
		}
	}
}
void StreamPipeline::warmup(int nn)
{
	std::cerr<<"start  warming up...\n";
	std::vector<std::thread> threads;
	auto t1=std::chrono::high_resolution_clock::now();
	for(auto i=0;i<_gs.size();i++){
		threads.push_back(std::thread(&StreamPipeline::run_parallel,this,i,nn));
	}
	auto t2=std::chrono::high_resolution_clock::now();
	//double x1=std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();//157ms for 3 threads
	//std::cerr<<x1<<" cost of creating threads for running\n";
	for(auto i=0;i<_gs.size();i++){
		threads[i].join();
	}
}

void StreamPipeline::run_parallel(int i, int n)
{
	std::stringstream stream;
	stream<<"runing graph "<<i<<std::endl;
	std::cerr<<stream.str();
    _manager.execute_graph(*_gs[i],n);
    //_manager.execute_graph(_g,n);
}

void StreamPipeline::run_w_parallel(int i, int n)
{
	std::stringstream stream;
	stream<<"runing graph "<<i<<std::endl;
	std::cerr<<stream.str();
    _manager.warmup_and_execute_graph(*_gs[i],n);
    //_manager.execute_graph(_g,n);
}


void StreamPipeline::reset_timings(){
	for(int i=0;i<num_graphs;i++){
		_manager.reset_timing(i);
	}
}
/*
StreamPipeline::StreamPipeline(size_t id, std::string name)
    : _ctx(), _manager(), _g(id, std::move(name))
{
}
void StreamPipeline::finalize(Target target, const GraphConfig &config)
{
	_manager.set_num_graphs(1);
    PassManager pm = create_default_pass_manager(target, config);
    _ctx.set_config(config);
    _manager.finalize_graph(_g, _ctx, pm, target);
}

void StreamPipeline::run(int n)
{
    _manager.execute_graph(_g,n);
}
*/


void StreamPipeline::measure(int n)
{
	_manager.print_times(*_gs[graph_id], n);
	//_manager.print_times(_g, n);
}

void StreamPipeline::reset()
{
	_manager.reset(*_gs[graph_id]);
	//_manager.reset(_g);
}




/*void Stream::run(double &in,double &task, double &out)
{
    _manager.execute_graph(_g,in,task,out);
}*/



int StreamPipeline::target_graph(int layer){
	for(int i=0; i<start_layer.size(); i++){
		if(layer>=start_layer[i] && layer<=end_layer[i]){
			return i;
		}
	}
	return start_layer.size()-1;
	//return -1;
}

void StreamPipeline::add_layer(ILayer &layer)
{
    auto nid   = layer.create_layer(*this);
    std::cerr<<"(streampipeline) Adding layer "<<layer.name()<<" "<<_tail_node<<"->"<<nid<<std::endl;
    _tail_node=nid;
}

const Graph &StreamPipeline::graph() const
{
	//return _g;
    return *(_gs[graph_id]);
}

Graph &StreamPipeline::graph()
{
	//return _g;
    return *(_gs[graph_id]);
}

StreamPipeline & StreamPipeline::operator<<(ILayer &layer)
{
	std::cerr<<"(streampipeline) "<<current_layer<<" Add Layer:"<<layer.name()<<std::endl;
	layer.add_input_node(this->get_tail_p(),this->get_graph_id_p());
	next_layer(layer.get_input_nodes());
    add_layer(layer);
    std::cerr<<"*******************************\n";
    return *this;
}
StreamPipeline & StreamPipeline::operator<<(ILayer &&layer)
{

	std::cerr<<"(streampipeline) "<<current_layer<<" Add Layer:"<<layer.name()<<std::endl;
	layer.add_input_node(this->get_tail_p(),this->get_graph_id_p());
	next_layer(layer.get_input_nodes());
    add_layer(layer);
    std::cerr<<"*******************************\n";
    return *this;
}
/** Overloaded stream operator to provide a target hint to the graph
 *
 * @param[in, out] s           Stream to provide the hint to
 * @param[in]      target_hint Target hint to be considered
 *
 * @return Updated stream
 */
StreamPipeline & StreamPipeline::operator<<(Target target_hint)
{
    hints().target_hint = target_hint;
    return *this;
}
/** Overloaded stream operator to provide a convolution method hint to the graph
 *
 * @param[in, out] s                       Stream to provide the hint to
 * @param[in]      convolution_method_hint Convolution method hint to be considered
 *
 * @return Updated stream
 */
StreamPipeline & StreamPipeline::operator<<(ConvolutionMethod convolution_method_hint)
{
    hints().convolution_method_hint = convolution_method_hint;
    return *this;
}
/** Overloaded stream operator to provide a depthwise convolution method hint to the graph
 *
 * @param[in, out] s                                 Stream to provide the hint to
 * @param[in]      depthwise_convolution_method_hint Depthwise Convolution method hint to be considered
 *
 * @return Updated stream
 */
StreamPipeline & StreamPipeline::operator<<(DepthwiseConvolutionMethod depthwise_convolution_method_hint)
{
    hints().depthwise_convolution_method_hint = depthwise_convolution_method_hint;
    return *this;
}
/** Overloaded stream operator to provide a fast math hint to the graph
 *
 * @param[in, out] s              Stream to provide the hint to
 * @param[in]      fast_math_hint Convolution method hint to be considered
 *
 * @return Updated stream
 */
StreamPipeline & StreamPipeline::operator<<(FastMathHint fast_math_hint)
{
    hints().fast_math_hint = fast_math_hint;
    return *this;
}

/*NodeID StreamPipeline::tail_node(int target)
{
	return Tail_node[target];
}*/

NodeID StreamPipeline::tail_node()
{
	std::cerr<<"(streampipeline) tail_node()- Tail_node: "<<_tail_node<<std::endl;
	return _tail_node;

}
void StreamPipeline::add_graph(int start, int end, char _PE, char _Host_PE){
    	int id=num_graphs;
    	num_graphs++;
    	//_gs.push_back(std::make_unique<GraphPipeline>(id, name, _PE, _Host_PE, start, end));
    	_gs.emplace_back(new GraphPipeline(id, name, _PE, _Host_PE, start, end));
    	input_time.push_back(0);
    	task_time.push_back(0);
    	output_time.push_back(0);
    	cost.push_back(0);
    	PE.push_back(_PE);
    	Host_PE.push_back(_PE);
    	start_layer.push_back(start);
    	end_layer.push_back(end);
    	arm_compute::graph::Target       target_GPU{ arm_compute::graph::Target::CL };
		arm_compute::graph::Target       target_CPU{ arm_compute::graph::Target::NEON };
		arm_compute::graph::Target       target=(_PE=='G')?target_GPU:target_CPU;
		//*(_gs[graph_id])<< target;
		StreamHints hint;
		hint.target_hint = target;
		all_hints.emplace_back(hint);
		GraphConfig config;
		int num_threads=0;
		if(_PE=='B')
			num_threads=common_params.threads;
		if(_PE=='L')
			num_threads=common_params.threads2;
		config.use_tuner   = common_params.enable_tuner;
		config.tuner_mode  = common_params.tuner_mode;
		config.tuner_file  = common_params.tuner_file;
		config.mlgo_file   = common_params.mlgo_file;
		config.num_threads = num_threads;
		configs.emplace_back(config);
		/*GraphContext ctx;
		_ctxs.emplace_back(std::move(ctx));*/
		_ctxs.emplace_back(GraphContext());
    	std::cerr<<"Adding Graph"<<id<<" target "<<std::to_string((int)(target))<<" PE: "<<_PE<<
    			" Host PE: "<<_Host_PE<<" Layers: "<<start<<"-"<<end<<std::endl;
}
NodeID StreamPipeline::next_layer(std::vector<std::pair<NodeID*,int*>> input_nodes ){
	IStreamPipeline::_target_graph=target_graph(current_layer);
	if (current_layer==start_layer[IStreamPipeline::_target_graph]){
		_hints=all_hints[IStreamPipeline::_target_graph];
		std::cerr<<"Starting Graph "<<IStreamPipeline::_target_graph<<" containing layers "<<start_layer[IStreamPipeline::_target_graph]<<"-"<<end_layer[IStreamPipeline::_target_graph]<<std::endl;
	}
	if (current_layer==end_layer[IStreamPipeline::_target_graph]){
		std::cerr<<"Ending layer of Graph "<<graph_id<<" containing layers "<<start_layer[graph_id]<<"-"<<end_layer[graph_id]<<std::endl;
	}

	for(auto input_node:input_nodes){
		//If input node is in another graph
		std::cerr<<"input node id: "<<*(input_node.first)<<" graph: "<<*(input_node.second)<<std::endl;
		if((_gs[IStreamPipeline::_target_graph])->node(*(input_node.first))==nullptr && *(input_node.first)!=EmptyNodeID){
			//Find the node that is mapped to the input node
			std::cerr<<"The input node "<<*(input_node.first)<<"-"<<*(input_node.second)<<" is not in target graph: "<<IStreamPipeline::_target_graph<<std::endl;
			auto mapped_node=node_map.find(input_node, IStreamPipeline::_target_graph);
			//No node mapped to input node need to create a T node in origin graph and R node in _target_graph
			//and mapped them to the input_node (to track node mapping)
			if ((mapped_node.second)==-1){
				//create a T node and append to the node key.first in graph key.second (and add it to mapping: node_map.insert(key,std::make_pair(node_id,key.second))))
				//create a R node in new graph (and add it to mapping: node_map.insert(key,std::make_pair(node_id,new__target_graph)) )
				// Add Transmitter to the previous graph containing input node for this node
				std::cerr<<"Input node for the layer "<<current_layer<<" is in graph: "<<*(input_node.second)<<
						" Adding Transmitter to that graph\n";


				//ITensorAccessorUPtr _accessor=get_Sender_accessor(common_params);
				//GraphBuilder::add_sender_node(*(_gs[i]), common_params_node, input, std::move(_accessor));
				int g_id=*(input_node.second);
				Graph& g=*(_gs[g_id].get());
				//NodeParams  common_params_node = { "Transmitter", hints().target_hint };

				std::string node_name=g.node(*(input_node.first))->name();
				NodeIdxPair input         = { *(input_node.first), 0 };
				NodeParams  common_params_node = { "Sender_"+node_name, all_hints[g_id].target_hint };
				common_params.labels="Sender";
				NodeID tail_sender=GraphBuilder::add_sender_node(g, common_params_node, input);
				SenderNode* s=dynamic_cast<SenderNode*>(g.node(tail_sender));
				node_map.insert(std::make_pair(*(input_node.first),*(input_node.second)),std::make_pair(tail_sender,g_id));
				//Add Receiver Node to the next graph
				common_params_node = { "Receiver_"+node_name, all_hints[IStreamPipeline::_target_graph].target_hint };
				NodeID tail_receiver = GraphBuilder::add_receiver_node(*(_gs[IStreamPipeline::_target_graph]), common_params_node, s->get_sender_tensor()->get_tensor()->desc());
				ReceiverNode* r=dynamic_cast<ReceiverNode*>(_gs[IStreamPipeline::_target_graph]->node(tail_receiver));
				node_map.insert(std::make_pair(*(input_node.first),*(input_node.second)),std::make_pair(tail_receiver,IStreamPipeline::_target_graph));
				*(input_node.first)=tail_receiver;
				*(input_node.second)=IStreamPipeline::_target_graph;
				s->get_sender_tensor()->add_receiver(r->get_receiver_tensor());
			}
			//There is a mapped node in the target graph for the input node which is originally from other graphs
			else if((mapped_node.second)==IStreamPipeline::_target_graph){
				//change the tail node from original graph to mapped node in target graph
				*(input_node.first)=mapped_node.first;
				*(input_node.second)=mapped_node.second;
			}
			else if((mapped_node.second)==*(input_node.second)){
				//create a R node in new graph (and add it to mapping: node_map.insert(key,std::make_pair(node_id,new_graph_id)) )
				//add R node into the T node(v.first) of origin graph graph[v.second].node(v.first).add_receiver(R);
				std::cerr<<"Input node for the layer "<<current_layer<<" is in graph: "<<*(input_node.second)<<
						"and there is a sender node (mapped) for that node\n";
				int g_id=*(input_node.second);
				Graph& g=*(_gs[g_id].get());
				SenderNode* s=dynamic_cast<SenderNode*>(g.node(mapped_node.first));
				//NodeParams  common_params_node = { "Receiver", hints().target_hint };
				std::string node_name=g.node(*(input_node.first))->name();
				NodeParams  common_params_node = { "Receiver_"+node_name, all_hints[IStreamPipeline::_target_graph].target_hint };
				common_params.labels="Receiver";
				//ITensorAccessorUPtr _accessor=get_Sender_accessor(common_params);
				//GraphBuilder::add_sender_node(*(_gs[i]), common_params_node, input, std::move(_accessor));
				//Add Receiver Node to the next graph

				NodeID tail_receiver = GraphBuilder::add_receiver_node(*(_gs[IStreamPipeline::_target_graph]), common_params_node, s->get_sender_tensor()->get_tensor()->desc());
				node_map.insert(std::make_pair(*(input_node.first),*(input_node.second)),std::make_pair(tail_receiver,IStreamPipeline::_target_graph));
				*(input_node.first)=tail_receiver;
				*(input_node.second)=IStreamPipeline::_target_graph;
				ReceiverNode* r=dynamic_cast<ReceiverNode*>(_gs[IStreamPipeline::_target_graph]->node(tail_receiver));
				s->get_sender_tensor()->add_receiver(r->get_receiver_tensor());
			}
			else{
				std::cerr<<"Error: Mapped Node\n";
			}
		}
	}

	std::cerr<<"Current layer: "<<current_layer<<std::endl;
	current_layer++;
	return 0;
}
void StreamPipeline::prnt(){
	std::cerr<<"hi\n";
	//std::cerr<<"num_graphs: "<<num_graphs<<" size: "<<_gs.size()<<std::endl;
	//std::cerr<<"Tail node: "<<Tail_node[_target_graph]<<std::endl;
}
void StreamPipeline::forward_tail(NodeID nid)
{
	//Tail_node[_target_graph] = (nid != NullTensorID) ? nid : Tail_node[_target_graph];
	//To update _tail_node(in IStream) also which is used by SubStream
	IStream::forward_tail(nid);
}


} // namespace frontend
} // namespace graph
} // namespace arm_compute
