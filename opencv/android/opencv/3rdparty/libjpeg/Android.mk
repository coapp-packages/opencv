LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := jpeg
MODULE_PATH := $(OpenCV_Root)/3rdparty/libjpeg

sources := $(wildcard ${MODULE_PATH}/*.c)
LOCAL_SRC_FILES := $(sources:%=../../%)

LOCAL_C_INCLUDES := $(OpenCV_Root)/3rdparty/include/ $(MODULE_PATH)

include $(BUILD_STATIC_LIBRARY)
