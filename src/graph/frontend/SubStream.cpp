/*
 * Copyright (c) 2018 Arm Limited.
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
#include "arm_compute/graph/frontend/SubStream.h"

#include "arm_compute/graph/Graph.h"
#include "arm_compute/graph/frontend/ILayer.h"

namespace arm_compute
{
namespace graph
{
namespace frontend
{
SubStream::SubStream(IStream &s)
    : _s(s)
{
    _hints     = s.hints();
    //std::cerr<<"new sub stream with tail node: "<<s.tail_node()<<" and tail graph: "<<s.get_tail_graph_id()<<std::endl;
    _tail_node = s.tail_node();
    tail_graph_id=s.get_tail_graph_id();
    //tail_graph_id=IStreamPipeline::_target_graph;
}

void SubStream::add_layer(ILayer &layer)
{


	//std::cerr<<"(substream) "<<" Add Layer:"<<current_layer<<" : "<<layer.name()<<" On graph: "<<tail_graph_id<<"("<<IStreamPipeline::_target_graph<<") tail_node: "<<tail_node()<<" with "<< graph().nodes().size()<<" nodes\n";
    auto nid   = layer.create_layer(*this);
    //std::cerr<<"Graph:"<<IStreamPipeline::_target_graph<<"  "<<_tail_node<<"->"<<nid<<std::endl;
    _tail_node = nid;
    tail_graph_id=IStreamPipeline::_target_graph;

    /*if(layer.name()=="conv2d_18/BatchNorm"){
    	std::string dd;
    	std::cerr<<_tail_node<<std::endl;
    	int nodid=8;
    	auto nn=graph().node(nodid);
		std::cerr<<"name:"<<nn->name()<<std::endl;
		int in_nn=nn->num_inputs();
		std::cerr<<"num inputs:"<<in_nn<<std::endl;
		int inedges=nn->input_edges().size();
		for(int i=0;i<inedges;i++){
			std::cerr<<i<<" "<<nn->input_edge(i)->producer()->name()<<std::endl;
			std::cerr<<nn->input(i)->desc().shape[0]<<", id: "<<nn->input(i)->id()<<std::endl;
		}
		int out_nn=nn->num_outputs();
		std::cerr<<"num outputs:"<<out_nn<<std::endl;
		//int outedges=nn->output_edges().size();
		std::cerr<<nn->output(0)->desc().shape[0]<<", id: "<<nn->output(0)->id()<<std::endl;

    	std::cin>>dd;
    }*/
}

const Graph &SubStream::graph() const
{
    return _s.graph();
}

Graph &SubStream::graph()
{
    return _s.graph();
}

//Ehsan

SubStream & SubStream::operator<<(ILayer &layer)
{
	////IStreamPipeline::_target_graph=target_graph(current_layer);
	//std::cerr<<"(substream) "<<" << operator, layer: "<<current_layer<<" : "<<layer.name()<<" On graph: "<<tail_graph_id<<"("<<IStreamPipeline::_target_graph<<") tail_node: "<<tail_node()<<" with "<< graph().nodes().size()<<" nodes\n";
	layer.add_input_node(_tail_node,tail_graph_id);

	std::string formatPattern = ".*_g\\d*|.*relu.*";
	std::regex pattern(formatPattern, std::regex_constants::icase);
	if (regex_search(layer.name(), pattern)) {
		std::cerr << "Skipping layer: "<<layer.name() << std::endl;
	} else {
		_s.next_layer(layer.get_input_nodes(), _tail_node, tail_graph_id);
	}



    add_layer(layer);
    //std::cerr<<"*******************************\n";
    return *this;
}
SubStream & SubStream::operator<<(ILayer &&layer)
{
	//IStreamPipeline::_target_graph=target_graph(current_layer);
	//std::cerr<<"(substream) "<<" << operator, layer: "<<current_layer<<" : "<<layer.name()<<" On graph: "<<tail_graph_id<<"("<<IStreamPipeline::_target_graph<<") tail_node: "<<tail_node()<<" with "<< graph().nodes().size()<<" nodes\n";
	layer.add_input_node(_tail_node,tail_graph_id);

	std::string formatPattern = ".*_g\\d*|.*relu.*";
	std::regex pattern(formatPattern, std::regex_constants::icase);
	if (regex_search(layer.name(), pattern)) {
		std::cerr << "Skipping layer: "<<layer.name() << std::endl;
	} else {
		_s.next_layer(layer.get_input_nodes(), _tail_node, tail_graph_id);
	}



	add_layer(layer);
	//std::cerr<<"*******************************\n";
    return *this;
}

} // namespace frontend
} // namespace graph
} // namespace arm_compute
