LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
		    test.cpp
		    
LOCAL_C_INCLUDES := \
	external/skia/include/core

LOCAL_SHARED_LIBRARIES := \
		    libcutils \
		    libutils \
		    libui \
		    libgui \
			libbinder
            

LOCAL_MODULE:= red_layer

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)

