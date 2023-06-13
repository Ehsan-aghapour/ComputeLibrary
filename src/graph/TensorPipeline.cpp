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
//For printing shape of a tensor
#include "utils/TypePrinter.h"



#include "arm_compute/graph/TensorPipeline.h"

namespace arm_compute
{
namespace graph
{

double TensorPipelineReceiver::send_data(Tensor* _tensor){
		{
			std::string s;
			auto tstart=std::chrono::high_resolution_clock::now();
			std::unique_lock<std::mutex> lck(mutex_);
			if(!get_receiver_ready()){
				s="graph:" + std::to_string(graph_id) +name+" waiting for its receiver to get ready\n";
				std::cerr<<s;
			}
			condVar.wait(lck, [this]{ return get_receiver_ready(); });
			auto tend1=std::chrono::high_resolution_clock::now();
			s="graph:" + std::to_string(graph_id) + name+" transferring data\n";
			tensor->handle()->map(true);
			//_tensor->handle()->map(true);
			std::cerr<<s;
			receiver_ready=false;
			//Transfer data
			//const auto   output_net  = reinterpret_cast<double *>(_tensor->handle()->tensor().buffer() + _tensor->handle()->tensor().info()->offset_first_element_in_bytes());
			std::cerr<<"graph:" + std::to_string(graph_id) +name +" dfdf\n";
			std::cerr<<"graph:" + std::to_string(graph_id) +name+" _tensor desc: "<<_tensor->desc().shape<<std::endl;
			std::cerr<<"graph:" + std::to_string(graph_id) +name+" tensor desc: "<<tensor->desc().shape<<std::endl;
			tensor->handle()->tensor().copy_from(_tensor->handle()->tensor());
			tensor->handle()->unmap();
			data_sent=true;
			condVar.notify_all();
			s="graph:" + std::to_string(graph_id) +name+" done\n";
			std::cerr<<s;
			lck.unlock();
			auto tend2=std::chrono::high_resolution_clock::now();
			t_sender_wait+=std::chrono::duration_cast<std::chrono::duration<double>>(tend1 - tstart).count();
			double t=std::chrono::duration_cast<std::chrono::duration<double>>(tend2 - tend1).count();
			t_transmition+=t;
			return t;
		}
}
void TensorPipelineReceiver::wait_for_receiver(){
	{
		std::unique_lock<std::mutex> lck(mutex_);
		condVar.wait(lck, [this]{ return get_receiver_ready(); });
		lck.unlock();
	}
	return;
}
void TensorPipelineReceiver::signal_receiver(){
	{
			std::unique_lock<std::mutex> lck(mutex_);
			data_sent=true;
			receiver_ready=false;
			condVar.notify_all();
			lck.unlock();
	}
}
bool TensorPipelineReceiver::receive_data(){
	{
		//std::lock_guard<std::mutex> lck(mutex_);
		std::string s;
		s="graph:" + std::to_string(graph_id) +name+" setting ready for getting data\n";
		std::cerr<<s;
		auto tstart=std::chrono::high_resolution_clock::now();
		std::unique_lock<std::mutex> lck(mutex_);
		receiver_ready = true;
		condVar.notify_all();
		if(!get_data_sent()){
			s="graph:" + std::to_string(graph_id) + name+" waiting for sender to send the data\n";
			std::cerr<<s;
		}
		condVar.wait(lck, [this]{ return get_data_sent(); });
		data_sent=false;
		s="graph:" + std::to_string(graph_id) + name+" Receiver done\n";
		std::cerr<<s;
		lck.unlock();
		auto tend=std::chrono::high_resolution_clock::now();
		t_receiver_wait+=std::chrono::duration_cast<std::chrono::duration<double>>(tend - tstart).count();
	}
	return true;
}

void TensorPipelineReceiver::set_receiver_ready(){
	auto s="graph:" + std::to_string(graph_id) + name+" set receiver ready\n";
	std::cerr<<s;
	receiver_ready=true;
}









bool TensorPipelineSender::send_data(){
		std::string s;

		s="graph:" + std::to_string(graph_id) +name+ " before check dest_tensor\n";
		std::cerr<<s;
		tensor->handle()->map(true);
		for(auto rec:receivers){
			rec->send_data(tensor);
		}
		tensor->handle()->unmap();
		s="graph:" + std::to_string(graph_id) +name+" after check dest_tensor\n";
		std::cerr<<s;
		return true;

	}

} // namespace graph
} // namespace arm_compute
