#include <iostream>
#include <fstream>
#include <stdio.h>
//#include <android/log.h>
#include <chrono>
#include "rknn_api.h"

//rknn_tensor_attr outputs_attr[2];
//const int output_index0 = 0;
//const int output_index1 = 1;
rknn_context ctx = 0;



rknn_tensor_attr a;

void print_attr(rknn_tensor_attr attr){
	printf("tensor name:%s\nindex:%d\t n_dims:%d\t",attr.name,attr.index,attr.n_dims);
	printf("Dims: ");
	for(int i=0;i<attr.n_dims;i++){
		printf("%d  ",attr.dims[i]);
	}
	printf("\nelements:%d \t size:%d \t type:%d \t qtype:%d \t scale:%f\n",attr.n_elems,attr.size,(int)attr.type,(int)attr.qnt_type,attr.scale);
}

void print_input(rknn_input input,int n){
	float* t=(float*)(input.buf);
	for(int i=0;i<n;i++){
			printf("%-4d:%-9.1f",i,t[i]);
			if(((i+1)%10)==0){
				printf("\n");
			}
	}
}

void print_output(rknn_output output,int n){
	float* t=(float*)(output.buf);
	for(int i=0;i<n;i++){
			printf("%-4d:%-12f",i,t[i]);
			if(((i+1)%10)==0){
				printf("\n");
			}
	}
}


int main(){

	std::chrono::time_point<std::chrono::high_resolution_clock> t1;
	std::chrono::time_point<std::chrono::high_resolution_clock> t2;
	double t=0;
	const char* mParamPath="/system/run_npu/model.rknn";
	FILE *fp = fopen(mParamPath, "rb");
	if(fp == NULL) {
		//LOGE("fopen %s fail!\n", mParamPath);
		printf("fopen %s fail!\n", mParamPath);
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	int model_len = ftell(fp);
	void *model = malloc(model_len);
	fseek(fp, 0, SEEK_SET);
	if(model_len != fread(model, 1, model_len, fp)) {
		//LOGE("fread %s fail!\n", mParamPath);
		printf("fread %s fail!\n", mParamPath);
		free(model);
		fclose(fp);
		return -1;
	}

	fclose(fp);

	// RKNN_FLAG_ASYNC_MASK: enable async mode to use NPU efficiently.
	//int ret = rknn_init(&ctx, model, model_len, RKNN_FLAG_PRIOR_MEDIUM|RKNN_FLAG_ASYNC_MASK);
	int ret = rknn_init(&ctx, model, model_len, RKNN_FLAG_PRIOR_MEDIUM);
	//int ret = rknn_init(&ctx, model, model_len, RKNN_FLAG_COLLECT_PERF_MASK);
	free(model);

	if(ret < 0) {
		//LOGE("rknn_init fail! ret=%d\n", ret);
		printf("rknn_init fail! ret=%d\n", ret);
		return -1;
	}
    	
	rknn_input_output_num io_num;
	ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
	if(ret < 0) {
		printf("rknn_query fail! ret=%d\n",ret);
		return -1;
	}


	rknn_tensor_attr output0_attr;
	output0_attr.index = 0;
	ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &output0_attr,
	sizeof(output0_attr));
	if(ret < 0) {
		printf("rknn_query fail! ret=%d\n",ret);
		return -1;
	}


	rknn_tensor_attr input0_attr;
	input0_attr.index = 0;
	ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &input0_attr,
	sizeof(input0_attr));
	if(ret < 0) {
		printf("rknn_query fail! ret=%d\n",ret);
		return -1;
	}

	

	printf("\n\n*************\n");
	print_attr(input0_attr);
	printf("\n\n*************\n");
	print_attr(output0_attr);

	float data[475];
	for(int i=0;i<475;i++){
		data[i]=i;
	}
	rknn_input inputs[1];
	inputs[0].index = 0;
	inputs[0].buf = data;
	inputs[0].size = sizeof(data);
	inputs[0].pass_through = false;
	//inputs[0].pass_through = true;
	//inputs[0].type = RKNN_TENSOR_UINT8;
	inputs[0].type = RKNN_TENSOR_FLOAT32;
	inputs[0].fmt = RKNN_TENSOR_NHWC;
	ret = rknn_inputs_set(ctx, 1, inputs);
	if(ret < 0) {
		printf("rknn_input_set fail! ret=%d\n", ret);
		return -1;
	}

	ret = rknn_run(ctx, NULL);
	if(ret < 0) {
		printf("rknn_run fail! ret=%d\n", ret);
		return -1;
	}

	ret = rknn_inputs_set(ctx, 1, inputs);
	if(ret < 0) {
		printf("rknn_input_set fail! ret=%d\n", ret);
		return -1;
	}
	
	rknn_output outputs[1];
	outputs[0].want_float = true;
	outputs[0].is_prealloc = false;




	t1=std::chrono::high_resolution_clock::now();
	ret = rknn_run(ctx, NULL);
	if(ret < 0) {
		printf("rknn_run fail! ret=%d\n", ret);
		return -1;
	}
	t2=std::chrono::high_resolution_clock::now();




	ret = rknn_outputs_get(ctx, 1, outputs, NULL);
	if(ret < 0) {
		printf("rknn_outputs_get fail! ret=%d\n", ret);
		return -1;
	}
	auto t3=std::chrono::high_resolution_clock::now();

	double run_latency=std::chrono::duration_cast<std::chrono::duration<double>>(t2-t1).count();
	double getout_latency=std::chrono::duration_cast<std::chrono::duration<double>>(t3-t2).count();
	printf("\nrun latency:%f \t get output latency:%f \n",1000.0*run_latency,1000.0*getout_latency);


	_rknn_perf_run run_time;
	ret = rknn_query(ctx, RKNN_QUERY_PERF_RUN, &run_time,sizeof(run_time));
	if(ret < 0) {
		printf("rknn_query fail! ret=%d\n",ret);
		return -1;
	}
	printf("run_time:%ld\n",run_time.run_duration);


	_rknn_sdk_version version;
	ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &version,sizeof(version));
	if(ret < 0) {
		printf("rknn_query fail! ret=%d\n",ret);
		return -1;
	}
	printf("api version:%s \t drive version:%s \n",version.api_version,version.drv_version);


	//print_output(outputs[0]);
	printf("\n\n*************\n");
	print_input(inputs[0],input0_attr.n_elems);
	printf("\n\n*************\n");
	print_output(outputs[0],output0_attr.n_elems);
	
	rknn_perf_detail perf_detail;
	ret = rknn_query(ctx, RKNN_QUERY_PERF_DETAIL, &perf_detail,sizeof(rknn_perf_detail));
	printf("%s", perf_detail.perf_data);

	

	rknn_outputs_release(ctx, 1, outputs);

	rknn_destroy(ctx);

	/*
	outputs_attr[0].index = output_index0;
	ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(outputs_attr[0]), sizeof(outputs_attr[0]));
	if(ret < 0) {
		//LOGI("rknn_query fail! ret=%d\n", ret);
		std::cerr<<"rknn_query fail!\n";
        return;
    }

	outputs_attr[1].index = output_index1;
	ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(outputs_attr[1]), sizeof(outputs_attr[1]));
	if(ret < 0) {
		LOGI("rknn_query fail! ret=%d\n", ret);
		return;
	}
 
	rknn_input inputs[1];
	inputs[0].index = input_index;
	inputs[0].buf = inData;
	inputs[0].size = img_width * img_height * img_channels;
	inputs[0].pass_through = false;
	inputs[0].type = RKNN_TENSOR_UINT8;
	inputs[0].fmt = RKNN_TENSOR_NHWC;
	int ret = rknn_inputs_set(ctx, 1, inputs);
	if(ret < 0) {
		LOGE("rknn_input_set fail! ret=%d\n", ret);
		return false;
	}

	ret = rknn_run(ctx, nullptr);
	if(ret < 0) {
		LOGE("rknn_run fail! ret=%d\n", ret);
		return false;
	}

	rknn_output outputs[2];
	#if 0
	outputs[0].want_float = true;
	outputs[0].is_prealloc = true;
	outputs[0].index = output_index0;
	outputs[0].buf = y0;
	outputs[0].size = output_size0;
	outputs[1].want_float = true;
	outputs[1].is_prealloc = true;
	outputs[1].index = output_index1;
	outputs[1].buf = y1;
	outputs[1].size = output_size1;
	#else  // for workround the wrong order issue of output index.
	outputs[0].want_float = true;
	outputs[0].is_prealloc = true;
	outputs[0].index = output_index0;
	outputs[0].buf = y1;
	outputs[0].size = output_size1;
	outputs[1].want_float = true;
	outputs[1].is_prealloc = true;
	outputs[1].index = output_index1;
	outputs[1].buf = y0;
	outputs[1].size = output_size0;
	#endif
	ret = rknn_outputs_get(ctx, 2, outputs, nullptr);
	if(ret < 0) {
		LOGE("rknn_outputs_get fail! ret=%d\n", ret);
		return false;
	}

	rknn_outputs_release(ctx, 2, outputs);
	*/
	return 0;

}
