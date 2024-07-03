/*
 * Copyright (c) 2017-2021 Arm Limited.
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
#include "arm_compute/runtime/NEON/functions/NEEarlyExitLayer.h"
#include "arm_compute/core/Validate.h"
#include "arm_compute/runtime/Tensor.h"

namespace arm_compute
{
NEEarlyExitLayer::NEEarlyExitLayer()
{
}

NEEarlyExitLayer::NEEarlyExitLayer(NEEarlyExitLayer &&) = default;
NEEarlyExitLayer &NEEarlyExitLayer::operator=(NEEarlyExitLayer &&) = default;
NEEarlyExitLayer::~NEEarlyExitLayer() = default;

void NEEarlyExitLayer::configure(ITensor *input)
{
	_input=input;
    ARM_COMPUTE_ERROR_ON_NULLPTR(input);
}



Status NEEarlyExitLayer::validate(const ITensorInfo *input)
{
    ARM_COMPUTE_RETURN_ERROR_ON_NULLPTR(input);
    return Status{};
}

void NEEarlyExitLayer::run()
{
	std::cerr<<"Now it is running early exit layer enter to continue...\n";


	const auto   output_prt    = reinterpret_cast<float *>(_input->buffer() + _input->info()->offset_first_element_in_bytes());
	auto num_elements=_input->info()->total_size()/_input->info()->element_size();
	std::cerr<<"Tensor shape is: "<<_input->info()->tensor_shape()<<std::endl;
	std::cerr<<"total size is: "<<_input->info()->total_size()<<std::endl;
	std::cerr<<"number of elements are: "<<num_elements<<std::endl;
	int cnt=0;
	int cnt2=0;
	//if 1000 100
	//check if max > th break
	//
	/*for(auto i=0; i < num_elements;i++){
		cnt2++;
		if(cnt2>1000)
			break;
		//std::cerr<<"i:"<<cnt<<" v:"<<output_prt[i]<<"   ";
		std::cerr << std::left << std::setw(3) << std::to_string(cnt)
		                  << std::left << std::setw(8) << "->" + std::to_string(output_prt[i])
		                  << "   ";
		cnt++;
		 if (cnt%8==0)
			 std::cerr<<std::endl;

	}*/

	/* Printing
	cnt=0;
	cnt2=0;
	for(size_t offset = 0; offset < _input->info()->total_size(); offset += _input->info()->element_size())
	{
		cnt2++;
		if(cnt2>1000)
			break;
		 const auto value = *reinterpret_cast<float *>(_input->buffer() + offset);
		 std::cerr << std::left << std::setw(3) << std::to_string(cnt)
		 		                  << std::left << std::setw(8) << "->" + std::to_string(value)
		 		                  << "   ";
		 cnt++;
		 if (cnt%8==0)
			 std::cerr<<std::endl;

	}*/
	std::string s;
	//std::cin>>s;
	//std::cerr<<"\ncontinue\n";
}


} // namespace arm_compute
