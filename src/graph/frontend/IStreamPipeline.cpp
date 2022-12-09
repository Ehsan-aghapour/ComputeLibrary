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
#include "arm_compute/graph/frontend/IStreamPipeline.h"

#include "arm_compute/graph/Utils.h"
#include "arm_compute/graph/frontend/ILayer.h"

namespace arm_compute
{
namespace graph
{
namespace frontend
{






void IStreamPipeline::add_layer(ILayer &layer)
{
    auto nid   = layer.create_layer(*this);
    std::cerr<<"(IStreamPipeline) Adding layer "<<layer.name()<<" "<<_tail_node<<"->"<<nid<<std::endl;
    _tail_node=nid;
}
/*
const Graph &IStreamPipeline::graph() const
{
	//std::cerr<<"calling graph const\n";
	return this->graph();
	//return _g;
}

Graph &IStreamPipeline::graph()
{
    return this->graph();
	//return _g;
}

IStreamPipeline & IStreamPipeline::operator<<(ILayer &layer)
{

	std::cerr<<"(IStreamPipeline) "<<current_layer++<<" Layer Name:"<<layer.name()<<std::endl;
    add_layer(layer);
    std::cerr<<"*******************************\n";
    return *this;
}
IStreamPipeline & IStreamPipeline::operator<<(ILayer &&layer)
{

	std::cerr<<"(IStreamPipeline) "<<current_layer++<<" Layer Name:"<<layer.name()<<std::endl;
    add_layer(layer);
    std::cerr<<"*******************************\n";
    return *this;
}



NodeID IStreamPipeline::tail_node()
{
	//std::cerr<<"(IStreamPipeline) tail_node()- Tail_node: "<<Tail_node[target_graph]<<std::endl;
	//return Tail_node[target_graph];
	std::cerr<<"(IStreamPipeline) tail_node()- Tail_node: "<<_tail_node<<std::endl;
	return _tail_node;

}
*/


} // namespace frontend
} // namespace graph
} // namespace arm_compute
