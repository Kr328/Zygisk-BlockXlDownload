LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := xldownload
LOCAL_SRC_FILES := module.cpp
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
