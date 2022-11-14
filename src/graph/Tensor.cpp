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
#include"annotate/streamline_annotate.h"
#include<chrono>


#include "arm_compute/graph/Tensor.h"

namespace arm_compute
{
namespace graph
{
Tensor::Tensor(TensorID id, TensorDescriptor desc)
    : _id(id), _desc(std::move(desc)), _handle(nullptr), _accessor(nullptr), _bound_edges()
{
}

TensorID Tensor::id() const
{
    return _id;
}

TensorDescriptor &Tensor::desc()
{
    return _desc;
}

const TensorDescriptor &Tensor::desc() const
{
    return _desc;
}

void Tensor::set_handle(std::unique_ptr<ITensorHandle> backend_tensor)
{
    _handle = std::move(backend_tensor);
}

ITensorHandle *Tensor::handle()
{
    return _handle.get();
}

void Tensor::set_accessor(std::unique_ptr<ITensorAccessor> accessor)
{
    _accessor = std::move(accessor);
}

ITensorAccessor *Tensor::accessor()
{
    return _accessor.get();
}

std::unique_ptr<ITensorAccessor> Tensor::extract_accessor()
{
    return std::move(_accessor);
}

bool Tensor::call_accessor()
{
    // Early exit guard
    if(!_accessor || !_handle)
    {
        return false;
    }

    // Map tensor
    _handle->map(true);

    // Return in case of null backend buffer
    if(_handle->tensor().buffer() == nullptr)
    {
        return false;
    }

    // Call accessor
    bool retval = _accessor->access_tensor(_handle->tensor());

    // Unmap tensor
    _handle->unmap();

    return retval;
}

//Ehsan
bool Tensor::my_call_accessor(int type)
{
	////ANNOTATE_MARKER_STR("input_output accessor start");
    // Early exit guard
	//std::cout<<"\n1\n";
    if(!_accessor || !_handle)
    {
        return false;
    }
    //std::cout<<"\n2\n";
    ////static int c=4;
    ///ANNOTATE_CHANNEL_COLOR(c,ANNOTATE_GREEN,"map");
    // Map tensor
    auto start=std::chrono::high_resolution_clock::now();
    _handle->map(true);
    auto finish=std::chrono::high_resolution_clock::now();
    mapping_time+=std::chrono::duration_cast<std::chrono::duration<double>>(finish - start).count();
    //std::cerr<<"mapping: "<<1000*(std::chrono::duration_cast<std::chrono::duration<double>>(finish - start).count())<<std::endl;
    //std::cerr<<"*******************************\n\n";
    ////ANNOTATE_CHANNEL_END(c++);
    // Return in case of null backend buffer
    if(_handle->tensor().buffer() == nullptr)
    {
        return false;
    }
    //std::cout<<"\n3\n";
    ////ANNOTATE_CHANNEL_COLOR(c,ANNOTATE_BLUE,"access");
    // Call accessor
    //std::string cc;
    //std::cout<<"salammm\n";
    ////start=std::chrono::high_resolution_clock::now();

    bool retval = _accessor->access_tensor(_handle->tensor());
    /*if(type==0){
    	copy_time+=dynamic_cast<ReceiverAccessor*>(_accessor)->get_trans_time();
    	std::cerr<<dynamic_cast<arm_compute::graph_utils::ReceiverAccessor*>(_accessor)->get_trans_time()<<std::endl;
    }
    if(type==1){
    	copy_time+=dynamic_cast<arm_compute::graph_utils::SenderAccessor*>(_accessor)->get_trans_time();
    	std::cerr<<dynamic_cast<arm_compute::graph_utils::SenderAccessor*>(_accessor)->get_trans_time()<<std::endl;
    }*/
    //std::cerr<<"copy time"<<trans_time<<std::endl;
    ////finish=std::chrono::high_resolution_clock::now();
    ////std::cerr<<"access: "<<(std::chrono::duration_cast<std::chrono::duration<double>>(finish - start).count())<<std::endl;
    ////ANNOTATE_CHANNEL_END(c++);
    ////ANNOTATE_CHANNEL_COLOR(c,ANNOTATE_RED,"unmap");
    // Unmap tensor
    start=std::chrono::high_resolution_clock::now();
    _handle->unmap();
    finish=std::chrono::high_resolution_clock::now();
    unmapping_time+=std::chrono::duration_cast<std::chrono::duration<double>>(finish - start).count();
    ////std::cerr<<"unmap: "<<(std::chrono::duration_cast<std::chrono::duration<double>>(finish - start).count())<<std::endl;
    ////ANNOTATE_CHANNEL_END(c++);
    return retval;
}

void Tensor::set_mapping_time(double t){
    mapping_time=t;
}

void Tensor::set_unmapping_time(double t){
	unmapping_time=t;
}

void Tensor::set_copy_time(double t){
	copy_time=t;
}

double Tensor::get_mapping_time(){
	return mapping_time;
}

double Tensor::get_unmapping_time(){
	return unmapping_time;
}

double Tensor::get_copy_time(){
	return copy_time;
}


void Tensor::bind_edge(EdgeID eid)
{
    _bound_edges.insert(eid);
}

void Tensor::unbind_edge(EdgeID eid)
{
    _bound_edges.erase(eid);
}

std::set<EdgeID> Tensor::bound_edges() const
{
    return _bound_edges;
}
} // namespace graph
} // namespace arm_compute
