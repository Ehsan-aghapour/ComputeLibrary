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
	std::cerr<<"rockpi npu\n";
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
	_name=name;
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
	//int ret = rknn_init(NPU_Data._NPU_Context, model, model_len, (RKNN_FLAG_PRIOR_MEDIUM | RKNN_FLAG_COLLECT_PERF_MASK));

	//int ret = rknn_init(NPU_Data._NPU_Context, model, model_len, RKNN_FLAG_PRIOR_MEDIUM);

	//int ret = rknn_init(NPU_Data._NPU_Context, model, model_len, RKNN_FLAG_COLLECT_PERF_MASK);
	//| RKNN_FLAG_COLLECT_PERF_MASK

	bool enable_op_profiling=false;
	auto init_flag = RKNN_FLAG_PRIOR_HIGH |
                   (enable_op_profiling ? RKNN_FLAG_COLLECT_PERF_MASK : 0);
	int ret = rknn_init(NPU_Data._NPU_Context, model, model_len,init_flag);
	std::cerr<<"initialized\n";
	//int ret = rknn_init(&ctx, model, model_len, RKNN_FLAG_COLLECT_PERF_MASK);
	free(model);

	if(ret < 0) {
		//LOGE("rknn_init fail! ret=%d\n", ret);
		printf("rknn_init fail! ret=%d\n", ret);
		return ;
	}
	/*_rknn_sdk_version version;
	ret = rknn_query(*NPU_Data._NPU_Context, RKNN_QUERY_SDK_VERSION, &version,sizeof(version));
	if(ret < 0) {
		printf("rknn_query fail! ret=%d\n",ret);
	}
	printf("api version:%s \t drive version:%s \n",version.api_version,version.drv_version);*/


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
		std::cerr<<"Input size: "<<in->info()->tensor_shape().total_size()<<std::endl;
		std::cerr<<in->info()->total_size()<<std::endl;
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
			std::cerr<<out->info()->total_size()<<std::endl;
			//Output_size=NPU_Data.output_attr.n_elems;
		}
		else{
			std::cerr<<"Output size match with model: "<<NPU_Data.output_attr.n_elems<<std::endl;
			std::cerr<<out->info()->total_size()<<std::endl;
		}

		output.want_float = true;
		output.index=i;
		bool preallocate=false;
		if(preallocate){
			output.is_prealloc = true;
			output.buf=reinterpret_cast<void *>(out->buffer()+out->info()->offset_first_element_in_bytes());
			output.buf=(out->buffer()+out->info()->offset_first_element_in_bytes());
			output.buf=reinterpret_cast<float *>(out->buffer()+out->info()->offset_first_element_in_bytes());
			output.buf=reinterpret_cast<double *>(out->buffer()+out->info()->offset_first_element_in_bytes());
			output.buf=(void*)(out->buffer()+out->info()->offset_first_element_in_bytes());
			output.size=out->info()->total_size();
		}
		else{
			output.is_prealloc = false;
		}

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
	auto start=std::chrono::high_resolution_clock::now();
	std::cerr<<"running npu\n";
	int i=0;
	for(auto in:inputs){
		const auto   pointer  = reinterpret_cast<double *>(in->buffer() + in->info()->offset_first_element_in_bytes());
		NPU_Data.inputs[i].buf=pointer;
		auto t=(double*)(NPU_Data.inputs[i].buf);
		std::cerr<<"input "<<i<<" of NPU, total size: "<<NPU_Data.inputs[i].size<<" first values: "<<t[0]<<", "<<t[1]<<", "<<t[2]<<std::endl;

		const auto   output_net2  = reinterpret_cast<double *>(in->buffer() + in->info()->offset_first_element_in_bytes());
		std::cerr<<"input "<<i<<" of NPU, total size: "<<NPU_Data.inputs[i].size<<" first values: "<<output_net2[0]<<", "<<output_net2[1]<<", "<<output_net2[2]<<std::endl;
		i++;
	}
	//First set input of the model
	int ret=rknn_inputs_set(*(NPU_Data._NPU_Context), NPU_Data.inputs.size(), &NPU_Data.inputs[0]);
	if(ret < 0){
		printf("rknn_input_set fail! ret=%d\n", ret);
		return;
	}
	std::cerr<<"npu set input done\n";
	auto end=std::chrono::high_resolution_clock::now();

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
	auto end2=std::chrono::high_resolution_clock::now();
	i=0;
	for(auto out:outputs){
		auto Output_data=(float*)(NPU_Data.outputs[i].buf);
		std::cerr<<"output "<<i<<" of NPU with the size: "<<NPU_Data.outputs[i].size<<std::endl;
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
	auto end3=std::chrono::high_resolution_clock::now();
	num_run++;
	double t_input=1000*(std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count());
	double t_run=1000*(std::chrono::duration_cast<std::chrono::duration<double>>(end2 - end).count());
	double t_output=1000*(std::chrono::duration_cast<std::chrono::duration<double>>(end3 - end2).count());
	input_time+=t_input;
	run_time+=t_run;
	output_time+=t_output;
	std::cerr<<"Timing of NPU part "<<_name<<" Frame:"<<num_run<<"  input_time: "<<t_input<<", run_time: "<<t_run<<", output_time: "<<t_output<<"\n\n";
	//rknn_outputs_release(ctx, 1, outputs);
	//consider preallocate approach
	rknn_perf_detail perf_detail;
	ret = rknn_query(*NPU_Data._NPU_Context, RKNN_QUERY_PERF_DETAIL, &perf_detail,sizeof(rknn_perf_detail));
	if(ret < 0) {
		printf("rknn_query fail! ret=%d\n",ret);
	}
	printf("%s", perf_detail.perf_data);

	_rknn_perf_run run_time;
	ret = rknn_query(*NPU_Data._NPU_Context, RKNN_QUERY_PERF_RUN, &run_time,sizeof(run_time));
	if(ret < 0) {
		printf("rknn_query fail! ret=%d\n",ret);
	}
	printf("\nrun_time:%ld\n",run_time.run_duration);






	//rknn_outputs_release(*NPU_Data._NPU_Context, 1, &NPU_Data.outputs[0]);
	std::cerr<<"npu done\n";
	//std::string t;
	//std::cin>>t;

}
} // namespace arm_compute
