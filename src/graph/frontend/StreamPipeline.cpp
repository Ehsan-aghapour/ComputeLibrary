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
StreamPipeline::StreamPipeline(size_t id, std::string _name)
    : _manager(),  num_graphs(0), name(std::move(_name)), current_layer(0)
{
	graph_id=0;
}
void StreamPipeline::set_common_params(arm_compute::utils::CommonGraphParams  _common_params){
	common_params=_common_params;
}


void StreamPipeline::finalize(Target target, const GraphConfig &config, std::set<int> *b, int blocking)
{
    PassManager pm = create_default_pass_manager(target, config);
    //_ctxs[target_graph].set_config(config);
    //_manager.finalize_graph(*_gs[target_graph], _ctxs[target_graph], pm, target, b, blocking);
    _ctx.set_config(config);
    _manager.finalize_graph(*(_gs[graph_id]), _ctx, pm, target, b, blocking);
    //_manager.finalize_graph(_g, _ctx, pm, target, b, blocking);
}

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


void StreamPipeline::run(int n)
{
    _manager.execute_graph(*_gs[graph_id],n);
    //_manager.execute_graph(_g,n);
}

/*void Stream::run(double &in,double &task, double &out)
{
    _manager.execute_graph(_g,in,task,out);
}*/
void StreamPipeline::run(bool anotate, int nn)
{
	//start=std::chrono::high_resolution_clock::now();
    _manager.execute_graph(*(_gs[graph_id]), anotate, nn);
	//_manager.execute_graph(_g, anotate, nn);
    //finish=std::chrono::high_resolution_clock::now();
}




void StreamPipeline::add_layer(ILayer &layer)
{
    auto nid   = layer.create_layer(*this);
    std::cerr<<"(streampipeline) Adding layer "<<layer.name()<<" "<<_tail_node<<"->"<<nid<<std::endl;
    //std::cerr<<"(streampipeline) Adding layer "<<layer.name()<<" "<<Tail_node[target_graph]<<"->"<<nid<<std::endl;
    //Tail_node[target_graph] = nid;
    //also update _tail_node in IStream which is used by SubStream
    _tail_node=nid;
}

const Graph &StreamPipeline::graph() const
{
	//std::cerr<<"calling graph const\n";
    return *(_gs[graph_id]);
	//return _g;
}

Graph &StreamPipeline::graph()
{
	//std::cerr<<"calling grah\n";
	//std::cerr<<"target_graph: "<<target_graph<<" num graphs: "<<num_graphs<<" size: "<<_gs.size()<<std::endl;
    return *(_gs[graph_id]);
	//return _g;
}

StreamPipeline & StreamPipeline::operator<<(ILayer &layer)
{
	//prnt();
	//std::cerr<<"adding layer into streamlinepipline\n";
	std::cerr<<"(streampipeline) "<<current_layer++<<" Layer Name:"<<layer.name()<<std::endl;
	//if (layer.get_input_nodes().size()==0){
		layer.add_input_node(this->get_tail_p(),this->get_graph_id());
	//}
	//next_layer(layer.get_input_nodes());
    add_layer(layer);
    //prnt();
    std::cerr<<"*******************************\n";
    return *this;
}
StreamPipeline & StreamPipeline::operator<<(ILayer &&layer)
{
	//prnt();
	//std::cerr<<"__adding layer into streamlinepipline\n";
	//std::cerr<<"hey:"<<layer.name()<<std::endl;
	std::cerr<<"(streampipeline) "<<current_layer++<<" Layer Name:"<<layer.name()<<std::endl;
	//if (layer.get_input_nodes().size()==0){
		layer.add_input_node(this->get_tail_p(),this->get_graph_id());
	//}
	next_layer(layer.get_input_nodes());

    add_layer(layer);
    //prnt();
    std::cerr<<"*******************************\n";
    return *this;
}

/*NodeID StreamPipeline::tail_node(int target)
{
	return Tail_node[target];
}*/

NodeID StreamPipeline::tail_node()
{
	//std::cerr<<"(streampipeline) tail_node()- Tail_node: "<<Tail_node[target_graph]<<std::endl;
	//return Tail_node[target_graph];
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
    	//Tail_node.push_back(EmptyNodeID);

    	std::cerr<<"Adding Graph"<<id<<" PE: "<<_PE<<
    			" Host PE: "<<_Host_PE<<" Layers: "<<start<<"-"<<end<<std::endl;
    	//std::cerr<<"num_graphs: "<<num_graphs<<" size: "<<_gs.size()<<std::endl;
    	//prnt();
}
NodeID StreamPipeline::next_layer(std::vector<std::pair<NodeID*,int*>> input_nodes ){
	if (current_layer==end_layer[graph_id]){
		std::cerr<<"Graph "<<graph_id<<" was completed containing layers "<<start_layer[graph_id]<<"-"<<end_layer[graph_id]<<std::endl;
		graph_id++;
		arm_compute::graph::Target       target_GPU{ arm_compute::graph::Target::NEON };
		arm_compute::graph::Target       target_CPU{ arm_compute::graph::Target::CL };
		arm_compute::graph::Target       target=(PE[graph_id]=='G')?target_GPU:target_CPU;
		//*(_gs[graph_id])<< target;
		hints().target_hint = target;
	}
	current_layer++;

	//int k=0;
	for(auto input_node:input_nodes){
		//If input node is in another graph
		if((_gs[graph_id])->node(*(input_node.first))==nullptr){
			//Find the node that is mapped to the input node
			auto mapped_node=node_map.find(input_node, graph_id);
			//No node mapped to input node need to create a T node in origin graph and R node in graph_id
			//and mapped them to the input_node (to track node mapping)
			if ((mapped_node.second)==-1){
				//create a T node and append to the node key.first in graph key.second (and add it to mapping: node_map.insert(key,std::make_pair(node_id,key.second))))
				//create a R node in new graph (and add it to mapping: node_map.insert(key,std::make_pair(node_id,new_graph_id)) )
				// Add Transmitter to the previous graph containing input node for this node
				std::cerr<<"input node for the layer "<<current_layer<<" is in graph: "<<*(input_node.second)<<
						"Adding Transmitter to that graph\n";

				NodeParams  common_params_node = { "Transmitter", hints().target_hint };
				NodeIdxPair input         = { *(input_node.first), 0 };
				common_params.labels="transfer";
				//ITensorAccessorUPtr _accessor=get_Sender_accessor(common_params);
				//GraphBuilder::add_sender_node(*(_gs[i]), common_params_node, input, std::move(_accessor));
				int g_id=*(input_node.second);
				Graph& g=*(_gs[g_id].get());
				NodeID tail_sender=GraphBuilder::add_sender_node(g, common_params_node, input);
				SenderNode* s=dynamic_cast<SenderNode*>(g.node(tail_sender));
				node_map.insert(std::make_pair(*(input_node.first),*(input_node.second)),std::make_pair(tail_sender,g_id));
				//Add Receiver Node to the next graph
				NodeID tail_receiver = GraphBuilder::add_receiver_node(*(_gs[graph_id]), common_params_node, s->get_sender_tensor()->get_tensor()->desc());
				ReceiverNode* r=dynamic_cast<ReceiverNode*>(_gs[graph_id]->node(tail_receiver));
				node_map.insert(std::make_pair(*(input_node.first),*(input_node.second)),std::make_pair(tail_receiver,graph_id));
				s->get_sender_tensor()->add_receiver(r->get_receiver_tensor());

			}
			//
			else if((mapped_node.second)==graph_id){
				//change the tail node from key.first to v.first
				*(input_node.first)=mapped_node.first;
				*(input_node.second)=mapped_node.second;
			}
			else if((mapped_node.second)==*(input_node.second)){
				//create a R node in new graph (and add it to mapping: node_map.insert(key,std::make_pair(node_id,new_graph_id)) )
				//add R node into the T node(v.first) of origin graph graph[v.second].node(v.first).add_receiver(R);
				std::cerr<<"input node for the layer "<<current_layer<<" is in graph: "<<*(input_node.second)<<
						"and there is a sender node (mapped) for that node\n";
				int g_id=*(input_node.second);
				Graph& g=*(_gs[g_id].get());
				SenderNode* s=dynamic_cast<SenderNode*>(g.node(mapped_node.first));
				NodeParams  common_params_node = { "Transmitter", hints().target_hint };
				//NodeIdxPair input         = { *(input_node.first), 0 };
				common_params.labels="transfer";
				//ITensorAccessorUPtr _accessor=get_Sender_accessor(common_params);
				//GraphBuilder::add_sender_node(*(_gs[i]), common_params_node, input, std::move(_accessor));
				//Add Receiver Node to the next graph
				NodeID tail_receiver = GraphBuilder::add_receiver_node(*(_gs[graph_id]), common_params_node, s->get_sender_tensor()->get_tensor()->desc());
				node_map.insert(std::make_pair(*(input_node.first),*(input_node.second)),std::make_pair(tail_receiver,graph_id));
				ReceiverNode* r=dynamic_cast<ReceiverNode*>(_gs[graph_id]->node(tail_receiver));
				s->get_sender_tensor()->add_receiver(r->get_receiver_tensor());
			}
			else{
				std::cerr<<"Error: Mapped Node\n";
			}
		}
	}
	std::cerr<<"cur layer: "<<current_layer<<std::endl;
	return 2;
}
void StreamPipeline::prnt(){
	std::cerr<<"hi\n";
	//std::cerr<<"num_graphs: "<<num_graphs<<" size: "<<_gs.size()<<std::endl;
	//std::cerr<<"Tail node: "<<Tail_node[target_graph]<<std::endl;
}
void StreamPipeline::forward_tail(NodeID nid)
{
	//Tail_node[target_graph] = (nid != NullTensorID) ? nid : Tail_node[target_graph];
	//To update _tail_node(in IStream) also which is used by SubStream
	IStream::forward_tail(nid);
}


} // namespace frontend
} // namespace graph
} // namespace arm_compute
