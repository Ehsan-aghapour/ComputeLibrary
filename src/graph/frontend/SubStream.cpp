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
    std::cerr<<"new substream with tail node: "<<s.tail_node()<<std::endl;
    _tail_node = s.tail_node();
    graph_id=*(s.get_graph_id());
}

void SubStream::add_layer(ILayer &layer)
{
    auto nid   = layer.create_layer(*this);
    std::cerr<<"(SubStream) Adding layer "<<layer.name()<<" "<<_tail_node<<"->"<<nid<<std::endl;
    _tail_node = nid;
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

	//if (layer.get_input_nodes().size()==0){
		layer.add_input_node(this->get_tail_p(),this->get_graph_id());
	//}
	_s.next_layer(layer.get_input_nodes());
	std::cerr<<"(SubStream) Layer Name:"<<layer.name()<<std::endl;
    add_layer(layer);
    std::cerr<<"*******************************\n";
    return *this;
}
SubStream & SubStream::operator<<(ILayer &&layer)
{
	layer.add_input_node(this->get_tail_p(),this->get_graph_id());
	_s.next_layer(layer.get_input_nodes());
	std::cerr<<"(SubStream) Layer Name:"<<layer.name()<<std::endl;
	add_layer(layer);
	std::cerr<<"*******************************\n";
    return *this;
}

} // namespace frontend
} // namespace graph
} // namespace arm_compute
