LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
		    test.cpp
		    
LOCAL_C_INCLUDES := \
	external/skia/include/core \
	system/core/libion/ \
	system/core/libion/include/


LOCAL_SHARED_LIBRARIES := \
		    libcutils \
		    libutils \
		    libui \
		    libgui \
			libbinder \
			libion




            

LOCAL_MODULE:= secure_mmap_test

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)

