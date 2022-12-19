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
#ifndef ARM_COMPUTE_GRAPH_ILAYER_H
#define ARM_COMPUTE_GRAPH_ILAYER_H

//Ehsan
#include "arm_compute/graph/frontend/SubStream.h"

namespace arm_compute
{
namespace graph
{
namespace frontend
{
// Forward declarations
class IStream;

/** ILayer interface */
class ILayer
{
public:
    /** Default destructor */
    virtual ~ILayer() = default;
    /** Create layer and add to the given stream.
     *
     * @param[in] s Stream to add layer to.
     *
     * @return ID of the created node.
     */
    virtual NodeID create_layer(IStream &s) = 0;
    /** Sets the name of the layer
     *
     * @param[in] name Name of the layer
     *
     * @return The layer object
     */
    ILayer &set_name(std::string name)
    {
        _name = name;
        return *this;
    }
    /** Layer name accessor
     *
     * @return Returns the name of the layer
     */
    const std::string &name() const
    {
        return _name;
    }

    //Ehsan
    std::vector<std::pair<NodeID*,int*>> get_input_nodes(){return input_nodes;};
    void add_input_node(NodeID* node, int* graph_id){ input_nodes.push_back(std::make_pair(node,graph_id));};
    /*virtual std::vector<std::unique_ptr<SubStream>> get_sub_streams();

    void restore_tail_nodes(std::vector<std::unique_ptr<SubStream>> sub_streams){
    	if(input_nodes.size() < sub_streams.size()){
			std::cerr<<"ILayer: size of input_nodes is smaller than size of substreams\n";
			return;
		}
		for (unsigned int _i=0;_i<sub_streams.size();_i++){
			sub_streams[_i]->forward_tail(*(input_nodes[_i].first));
		}
    }
    virtual void restore_tail_nodes();
    virtual void set_tail_nodes(int,NodeID);*/

protected:
    std::string _name = {};

    //Ehsan
    std::vector<std::pair<NodeID*,int*>> input_nodes;
};
} // namespace frontend
} // namespace graph
} // namespace arm_compute
#endif /* ARM_COMPUTE_GRAPH_ILAYER_H */
