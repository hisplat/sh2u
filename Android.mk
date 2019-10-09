# Copyright 2005 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= simple_httpd
LOCAL_SRC_FILES := main.cpp
LOCAL_SRC_FILES += client.cpp
LOCAL_SRC_FILES += http_header.cpp
LOCAL_SRC_FILES += httpd_config.cpp
LOCAL_SRC_FILES += request.cpp
LOCAL_SRC_FILES += response.cpp
LOCAL_SRC_FILES += tools/socket.cpp
LOCAL_SRC_FILES += tools/select.cpp
LOCAL_SRC_FILES += tools/string_util.cpp
LOCAL_SRC_FILES += tools/logging.cpp
LOCAL_SRC_FILES += tools/dump.cpp
LOCAL_SRC_FILES += tools/buffer.cpp
LOCAL_SRC_FILES += tools/serialize/archive.cpp


LOCAL_CFLAGS    += -Wno-unused-parameter -fPIC -fPIE -fpie -rdynamic
LOCAL_CPPFLAGS  += -Wno-unused-parameter -fPIC -fPIE -fpie -rdynamic
LOCAL_LDFLAGS  +=  -fPIC -rdynamic -pie
LOCAL_C_INCLUDES += 

# LOCAL_CPPFLAGS += -std=gnu++11
LOCAL_SHARED_LIBRARIES += libcutils libutils libdl liblog
#

# include external/stlport/libstlport.mk
include $(BUILD_EXECUTABLE)
include $(call all-makefiles-under,$(LOCAL_PATH))


