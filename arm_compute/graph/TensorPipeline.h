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
#include <thread>

#include <queue>
#include "arm_compute/runtime/Tensor.h"



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
	TensorPipelineReceiver();
	double send_data(Tensor* _tensor);
	double send_data(double* _npu_output);
	void wait_for_receiver();
	void signal_receiver();
	bool receive_data();

	bool get_receiver_ready();
	void set_receiver_ready();
	bool get_data_sent();
	void set_tensor(Tensor* t);
	Tensor* get_tensor();

	void set_name(std::string _name);

	void reset_timing();
	double get_transmition_time();
	double get_sender_wait_time();
	double get_receiver_wait_time();
	int get_graph_id();
	void set_graph_id(int g_id);

private:
	Tensor* tensor = nullptr;
	std::queue<std::unique_ptr<arm_compute::graph::Tensor>> buffer;
	std::queue<double*> NPU_buffer;
	std::mutex mutex_;
	std::condition_variable condVar;
	bool* receiver_ready=new bool(false);
	bool* data_sent=new bool (false);

	//For NPU
	bool  is_npu = false;
	unsigned int	Input_size=0;
	bool	_Transpose=true;


	//std::atomic<bool> receiver_ready;
	//std::atomic<bool> data_sent;
	std::string		name;
	int				graph_id;
	double	t_sender_wait;
	double	t_receiver_wait;
	double	t_transmition;
};




class TensorPipelineSender
{
public:
    /** Default constructor
     *
     * @param[in] id   Tensor ID
     * @param[in] desc Tensor information
     */
	void add_receiver(TensorPipelineReceiver* d);
	std::vector<TensorPipelineReceiver*> get_dest();
	bool send_data();

	void set_tensor(Tensor* t);
	Tensor* get_tensor();
	void set_name(std::string _name);
	int get_graph_id();
	void set_graph_id(int g_id);

private:
	Tensor* tensor = nullptr;
	//vector of receivers instead of one receiver
	std::vector<TensorPipelineReceiver*> receivers;
	std::string name;
	int graph_id;
	bool is_npu = false;
};


class TensorPipelineNPU : public Tensor {
public:
	bool my_call_accessor() override{
		return true;
	}
	virtual ~TensorPipelineNPU() {}


};

} // namespace graph
} // namespace arm_compute
#endif /* ARM_COMPUTE_GRAPH_TENSOR_Pipeline_H */