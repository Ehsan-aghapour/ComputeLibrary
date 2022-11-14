LOCAL_PATH:= $(call my-dir)

common_src_files := main.c vnn_pre_process.c vnn_post_process.c vnn_inceptionv3.c

common_c_includes := \
	$(LOCAL_PATH)/../android_sdk/include/service/ovx_inc \
	$(LOCAL_PATH)/../android_sdk/include/service/ovx_inc/HAL \
	$(LOCAL_PATH)/../android_sdk/include/service/ovx_inc/CL \
	$(LOCAL_PATH)/../android_sdk/include/applib/ovxinc/include \
	$(LOCAL_PATH)/../android_sdk/include/applib/ovxinc/include/utils \
	$(LOCAL_PATH)/../android_sdk/include/applib/ovxinc/include/client \
	$(LOCAL_PATH)/../android_sdk/include/applib/ovxinc/include \
	$(LOCAL_PATH)/../android_sdk/include/applib/ovxinc/include/ops \

	
#common_shared_libraries := \
	libsysutils \
	libbinder \
	libcutils \
	liblog \
	libdl
#LOCAL_SHARED_LIBRARIES := $(common_shared_libraries)


include $(CLEAR_VARS)


LOCAL_SRC_FILES := $(common_src_files)
LOCAL_C_INCLUDES := $(common_c_includes)

LOCAL_LDLIBS +=  -L/home/ehsan/UvA/DRLPM/Khadas/Large/test/android_sdk/lib -lovxlib -ljpeg

ifeq ($(shell test $(PLATFORM_SDK_VERSION) -ge 26 && echo OK),OK)
LOCAL_PROPRIETARY_MODULE := true
endif



LOCAL_MODULE := vnn_inceptionv3
LOCAL_MODULE_TAGS := eng tests optional
LOCAL_ARM_MODE := $(VIV_TARGET_ABI)
LOCAL_MULTILIB := $(VIV_MULTILIB)


#LOCAL_CFLAGS += -pie -fPIE
#LOCAL_LDFLAGS += -pie -fPIE
#include $(BUILD_EXECUTABLE)

include $(BUILD_SHARED_LIBRARY) 
