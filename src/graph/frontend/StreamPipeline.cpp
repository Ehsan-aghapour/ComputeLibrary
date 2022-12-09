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

#include "arm_compute/graph/Utils.h"
#include "arm_compute/graph/frontend/ILayer.h"

namespace arm_compute
{
namespace graph
{
namespace frontend
{
StreamPipeline::StreamPipeline(size_t id, std::string _name)
    : _manager(), name(std::move(_name)), target_graph(0), num_graphs(0), current_layer(0)
{

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
    _manager.finalize_graph(*(_gs[target_graph]), _ctx, pm, target, b, blocking);
    //_manager.finalize_graph(_g, _ctx, pm, target, b, blocking);
}

void StreamPipeline::measure(int n)
{
	_manager.print_times(*_gs[target_graph], n);
	//_manager.print_times(_g, n);
}

void StreamPipeline::reset()
{
	_manager.reset(*_gs[target_graph]);
	//_manager.reset(_g);
}


void StreamPipeline::run(int n)
{
    _manager.execute_graph(*_gs[target_graph],n);
    //_manager.execute_graph(_g,n);
}

/*void Stream::run(double &in,double &task, double &out)
{
    _manager.execute_graph(_g,in,task,out);
}*/
void StreamPipeline::run(bool anotate, int nn)
{
	//start=std::chrono::high_resolution_clock::now();
    _manager.execute_graph(*(_gs[target_graph]), anotate, nn);
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
    return *(_gs[target_graph]);
	//return _g;
}

Graph &StreamPipeline::graph()
{
	//std::cerr<<"calling grah\n";
	//std::cerr<<"target_graph: "<<target_graph<<" num graphs: "<<num_graphs<<" size: "<<_gs.size()<<std::endl;
    return *(_gs[target_graph]);
	//return _g;
}

StreamPipeline & StreamPipeline::operator<<(ILayer &layer)
{
	//prnt();
	//std::cerr<<"adding layer into streamlinepipline\n";
	std::cerr<<"(streampipeline) "<<current_layer++<<" Layer Name:"<<layer.name()<<std::endl;
	if (layer.get_input_nodes().size()==0){
		layer.add_input_node(_tail_node);
	}
	next_layer(layer.get_input_nodes());
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
	if (layer.get_input_nodes().size()==0){
		layer.add_input_node(_tail_node);
	}
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
NodeID StreamPipeline::next_layer(std::vector<NodeID> input_nodes){
	if (current_layer==end_layer[target_graph]){
		std::cerr<<"Graph "<<target_graph<<" was completed containing layers "<<start_layer[target_graph]<<"-"<<end_layer[target_graph]<<std::endl;
	}
	current_layer++;
	target_graph++;

	arm_compute::graph::Target       target_GPU{ arm_compute::graph::Target::NEON };
	arm_compute::graph::Target       target_CPU{ arm_compute::graph::Target::CL };
	arm_compute::graph::Target       target=(PE=='G')?target_GPU:target_CPU;
	*(_gs[target_graph])<< target;

	for(auto input_node:input_nodes){
		if((_gs[target_graph])->node(input_node)==nullptr){
			int i = target_graph-1;
			while (i >= 0){
				if((_gs[i])->node(input_node)!=nullptr){
					// Add Transmitter to the previous graph containing input node for this node
					std::cerr<<"input node for the layer "<<current_layer<<" is in graph: "<<i<<
							"Adding Transmitter to that graph\n";

					NodeParams  common_params_node = { "Transmitter", hints().target_hint };
					NodeIdxPair input         = { input_node, 0 };
					common_params.labels="transfer";
					ITensorAccessorUPtr _accessor=get_Sender_accessor(common_params);
					GraphBuilder::add_sender_node(*(_gs[i]), common_params_node, input, std::move(_accessor));

					//Add Receiver Node to the next graph
					NodeID tail = GraphBuilder::add_receiver_node(*(_gs[i]), common_params_node, input, std::move(_accessor));

				}
			}
		}
	}
	//InputLayer(input_descriptor, get_input_accessor(common_params, std::move(preprocessor), false));

	std::cerr<<"cur layer: "<<current_layer<<std::endl;
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
