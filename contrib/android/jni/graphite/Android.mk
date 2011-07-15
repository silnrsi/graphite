# LOCAL_PATH := $(APP_PROJECT_PATH)/jni/graphite

include $(CLEAR_VARS)

_NS := GR2
GR2_BASE := ../../..
GR2_MACHINE := direct
#include /home/mhosken/Work/dev/Graphite/graphiteng/src/files.mk
include ../../src/files.mk

LOCAL_MODULE := graphite2
#LOCAL_SRC_FILES := $(foreach v,$(GR2_SOURCES),./$(v))
LOCAL_SRC_FILES := $(GR2_SOURCES)
LOCAL_C_INCLUDES := ../../include
LOCAL_EXPORT_C_INCLUDES := ../../include
#LOCAL_C_INCLUDES := /home/mhosken/Work/dev/Graphite/graphiteng/include
#LOCAL_EXPORT_C_INCLUDES := /home/mhosken/Work/dev/Graphite/graphiteng/include
LOCAL_CPPFLAGS := -mapcs -DNSEG_CACHE -DDISABLE_TRACING -DNDEBUG
include $(BUILD_SHARED_LIBRARY)

