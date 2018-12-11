LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
		    SidebandNativeHandle.cpp \
		    SidebandNativehandle_test.cpp
		    

LOCAL_SHARED_LIBRARIES := \
		    libcutils \
		    libutils \
		    libui \
		    libgui \
			libbinder
            

LOCAL_MODULE:= red123_layer

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)

