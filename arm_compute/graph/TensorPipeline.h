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
#ifndef ARM_COMPUTE_GRAPH_TENSOR_Pipeline_H
#define ARM_COMPUTE_GRAPH_TENSOR_Pipeline_H

#include "arm_compute/graph/Tensor.h"
#include <condition_variable>



namespace arm_compute
{
namespace graph
{
/** Tensor object **/

class TensorPipelineReceiver
{
public:
    /** Default constructor
     *
     * @param[in] id   Tensor ID
     * @param[in] desc Tensor information
     */
	void check(){
		{
			std::unique_lock<std::mutex> lck(mutex_);
			condVar.wait(lck, [this]{ return *(get_ready()); });
			lck.unlock();
		}
	}
	void set_ready(){
		{
			std::lock_guard<std::mutex> lck(mutex_);
			*ready = true;
		}
		condVar.notify_all();
	}

	bool* get_ready(){
		return ready;
	}
	void set_tensor(Tensor* t){
		tensor=t;
	}
	Tensor* get_tensor(){
		return tensor;
	}


private:
	Tensor* tensor = nullptr;
	std::mutex mutex_;
	std::condition_variable condVar;
	bool* ready=new bool(false);
};




class TensorPipelineSender
{
public:
    /** Default constructor
     *
     * @param[in] id   Tensor ID
     * @param[in] desc Tensor information
     */
	void set_dest_tensor(TensorPipelineSender* d){
		{
			receiver=d;
		}
	}
	TensorPipelineSender* get_dest(){
		return receiver;
	}
	void check(){
		std::cerr<<"Before check dest_tensor "<<std::endl;
		receiver->check();
		std::cerr<<"After check dest_tensor "<<std::endl;

	}
	void set_tensor(Tensor* t){
		tensor=t;
	}
	Tensor* get_tensor(){
		return tensor;
	}


private:
	Tensor* tensor = nullptr;
	TensorPipelineReceiver* receiver;
};

} // namespace graph
} // namespace arm_compute
#endif /* ARM_COMPUTE_GRAPH_TENSOR_Pipeline_H */
