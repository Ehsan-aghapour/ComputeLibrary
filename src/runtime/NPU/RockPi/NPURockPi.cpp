/*
 * NPURockPi.cpp
 *
 *  Created on: Jul 7, 2023
 *      Author: ehsan
 */
//#include "rockx.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <algorithm>
#include <bitset>
//#include <android/log.h>
#include <chrono>

//#include "arm_compute/runtime/NPU/rknn_api.h"


#include "arm_compute/runtime/NPU/NPU.h"

#include "arm_compute/core/Types.h"
#include "arm_compute/core/Validate.h"
#include "utils/Utils.h"

namespace arm_compute
{
//class NPUTemplate;
//class NPURockPi;
//using NPUType=RockPi;
const NPUTypes NPUType = NPUTypes::RockPi;
template <>
NPU<NPUType>::NPU(int _id)
{
	id=_id;
	NPU_Data._NPU_Context=new rknn_context(id);
	std::cerr<<"creating a RockPi NPU node...\n";

}

template <>
NPU<NPUType>::NPU(NPU<NPUType> &&) = default;

template <>
NPU<NPUType> &NPU<NPUType>::operator=(NPU<NPUTypes::RockPi> &&) = default;

template <>
NPU<NPUType>::~NPU()                               = default;

template <>
void NPU<NPUType>::configure(std::string name, std::vector<arm_compute::ITensor *> _inputs, std::vector<arm_compute::ITensor *> _outputs)
{
	inputs=_inputs;
	outputs=_outputs;
	//std::cerr<<_inputs[0]->info()->total_size()<<std::endl;
	ARM_COMPUTE_ERROR_ON_NULLPTR(input);
    //configure(input, output);
	std::cerr<<"numb of inputs: "<<inputs.size()<<" and number of outputs: "<<outputs.size()<<std::endl;


	std::cerr<<"Reading model...\n";
	//std::string model_name="/data/data/com.termux/files/home/ARMCL-RockPi/graphs/Google_8_12.rknn";
	std::string model_name="/data/data/com.termux/files/home/ARMCL-RockPi/graphs/"+name+".rknn";
	std::cerr<<"model name: "<<model_name<<std::endl;
	FILE *fp = fopen(model_name.c_str(), "rb");
	if(fp == NULL) {
		//LOGE("fopen %s fail!\n", mParamPath);
		printf("fopen %s failed!\n", model_name.c_str());
		return ;
	}
	fseek(fp, 0, SEEK_END);
	int model_len = ftell(fp);
	void *model = malloc(model_len);
	fseek(fp, 0, SEEK_SET);
	if(model_len != fread(model, 1, model_len, fp)) {
		//LOGE("fread %s fail!\n", mParamPath);
		printf("fread %s fail!\n", model_name.c_str());
		free(model);
		fclose(fp);
		return ;
	}
	std::cerr<<"model reading done.\n";

	fclose(fp);

	// RKNN_FLAG_ASYNC_MASK: enable async mode to use NPU efficiently.
	//int ret = rknn_init(&ctx, model, model_len, RKNN_FLAG_PRIOR_MEDIUM|RKNN_FLAG_ASYNC_MASK);
	//rknn_context ctx = 0;
	int ret = rknn_init(NPU_Data._NPU_Context, model, model_len, RKNN_FLAG_PRIOR_MEDIUM);
	std::cerr<<"initialized\n";
	//int ret = rknn_init(&ctx, model, model_len, RKNN_FLAG_COLLECT_PERF_MASK);
	free(model);

	if(ret < 0) {
		//LOGE("rknn_init fail! ret=%d\n", ret);
		printf("rknn_init fail! ret=%d\n", ret);
		return ;
	}


	NPU_Data.inputs.resize(inputs.size());
	NPU_Data.outputs.resize(outputs.size());
	int i=0;
	for(auto in:_inputs){
		rknn_input input;
		input.index=0;
		input.pass_through = Pass;
		input.fmt = RKNN_TENSOR_NHWC;
		/*if(in->buffer()==nullptr){
			std::string sss;
			std::cerr<<"hh\n";
			std::cin>>sss;
		}*/
		//input.buf = static_cast<void*>(in->buffer());
		input.size = in->info()->total_size();
		input.type = RKNN_TENSOR_FLOAT32;
		//NPU_Data.inputs.emplace_back(input);
		NPU_Data.inputs[i]=input;
		i++;
	}
	std::cerr<<"inputs has been set\n";
	i=0;
	for(auto out:_outputs){
		rknn_output output;
		int ret = rknn_query(*NPU_Data._NPU_Context, RKNN_QUERY_OUTPUT_ATTR, &NPU_Data.output_attr, sizeof(NPU_Data.output_attr));
		if(ret < 0) {
			printf("rknn_query fail! ret=%d\n",ret);
			//return -1;
		}

		if(out->info()->tensor_shape().total_size()!=NPU_Data.output_attr.n_elems){
			std::cerr<<"Output size missmatch\n";
			std::cerr<<"Expected Output size: "<<out->info()->tensor_shape().total_size()<<" Model Output size: "<<NPU_Data.output_attr.n_elems<<std::endl;
			//Output_size=NPU_Data.output_attr.n_elems;
		}
		else{
			std::cerr<<"Output size match with model: "<<NPU_Data.output_attr.n_elems<<std::endl;
		}


		output.want_float = true;
		output.is_prealloc = false;
		output.index=0;
		///NPU_Data.outputs[i].buf=out->buffer();
		NPU_Data.outputs[i]=output;
		i++;
	}





}



template <>
Status NPU<NPUType>::validate(const ITensorInfo *input, const ITensorInfo *output)
{
	return Status{};
    //return opencl::ClActivation::validate(input, output, act_info);
}

template <>
void NPU<NPUType>::prepare()
{
	std::string t;
	std::cerr<<"prepare\n";

	return;
}

template <>
void NPU<NPUType>::run()
{
	std::cerr<<"running npu\n";
	int i=0;
	for(auto in:inputs){
		const auto   pointer  = reinterpret_cast<void *>(in->buffer() + in->info()->offset_first_element_in_bytes());
		NPU_Data.inputs[i++].buf=pointer;
	}
	//First set input of the model
	int ret=rknn_inputs_set(*(NPU_Data._NPU_Context), NPU_Data.inputs.size(), &NPU_Data.inputs[0]);
	if(ret < 0){
		printf("rknn_input_set fail! ret=%d\n", ret);
		return;
	}
	std::cerr<<"npu set input done\n";

	ret = rknn_run(*(NPU_Data._NPU_Context), NULL);
	if(ret<0){
		std::cerr<<"npu_run: Error "<<ret<<" running NPU part with id: "<<id<<std::endl;
	}
	std::cerr<<"run model done\n";
	/*i=0;
	for(auto out:outputs){
		if(out->buffer()==nullptr){
			std::cerr<<"haa\n";
			std::string gg;
			std::cin>>gg;
		}
		NPU_Data.outputs[i].buf=static_cast<void*>(out->buffer());
		i++;
	}*/
	ret = rknn_outputs_get(*NPU_Data._NPU_Context, NPU_Data.outputs.size(), &NPU_Data.outputs[0], NULL);
	if(ret < 0) {
		printf("NPU get output fail! ret=%d\n",ret);
		return;
	}
	std::cerr<<"npu set output done\n";

	i=0;
	for(auto out:outputs){
		auto Output_data=(float*)(NPU_Data.outputs[i].buf);
		//std::cerr<<"hereee\n";
		auto tstart=std::chrono::high_resolution_clock::now();
		bool _Transpose=true;
		if(_Transpose){
			utils::fill_tensor_array2<float,ITensor>(*out,(float*)(Output_data),out->info()->total_size());
		}
		else{
			utils::fill_tensor_array<float,ITensor>(*out,(float*)(Output_data),out->info()->total_size());
		}
	}
	std::cerr<<"npu done\n";
	//std::string t;
	//std::cin>>t;

}
} // namespace arm_compute
