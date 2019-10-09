# Copyright 2005 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

LOCAL_MODULE:= libaction_test
LOCAL_SRC_FILES := action_test.cpp


LOCAL_CFLAGS   += -Wno-unused-parameter -fPIC # -rdynamic
LOCAL_CPPFLAGS   += -Wno-unused-parameter -fPIC # -rdynamic
LOCAL_C_INCLUDES += 

LOCAL_LDFLAGS := -Wl,--allow-shlib-undefined

# LOCAL_CPPFLAGS += -std=gnu++11
LOCAL_SHARED_LIBRARIES += libcutils libutils libdl liblog
#

# include external/stlport/libstlport.mk
include $(BUILD_SHARED_LIBRARY)


