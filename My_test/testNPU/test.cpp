#include <iostream>
#include <fstream>
#include <stdio.h>
//#include <android/log.h>
#include <chrono>
#include "rknn_api.h"

#include "test.h"

//rknn_tensor_attr outputs_attr[2];
//const int output_index0 = 0;
//const int output_index1 = 1;
rknn_context ctx = 0;






test::test(){};
void test::hi(){
	std::chrono::time_point<std::chrono::high_resolution_clock> t1;
	std::chrono::time_point<std::chrono::high_resolution_clock> t2;
	double t=0;
	const char* mParamPath="/system/run_npu/model.rknn";
	FILE *fp = fopen(mParamPath, "rb");
	if(fp == NULL) {
		//LOGE("fopen %s fail!\n", mParamPath);
		printf("fopen %s fail!\n", mParamPath);
		return;
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
		return;
	}

	fclose(fp);

	// RKNN_FLAG_ASYNC_MASK: enable async mode to use NPU efficiently.
	//int ret = rknn_init(&ctx, model, model_len, RKNN_FLAG_PRIOR_MEDIUM|RKNN_FLAG_ASYNC_MASK);
	int ret = rknn_init(&ctx, model, model_len, RKNN_FLAG_PRIOR_MEDIUM);
};

	
	