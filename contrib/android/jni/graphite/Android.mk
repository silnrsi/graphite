# LOCAL_PATH := $(APP_PROJECT_PATH)/jni/graphite

include $(CLEAR_VARS)

_NS := GR2
GR2_BASE := ../../..
GR2_MACHINE := direct
include $(GR2_BASE)/src/files.mk

LOCAL_MODULE := graphite2
LOCAL_SRC_FILES := $(foreach v,$(GR2_SOURCES),./$(v))
# LOCAL_SRC_FILES := $(GR2_SOURCES)
LOCAL_C_INCLUDES := $(GR2_BASE)/include
LOCAL_EXPORT_C_INCLUDES := $(GR2_BASE)/include
LOCAL_CPPFLAGS := -mapcs -DNSEG_CACHE -DDISABLE_TRACING -DNDEBUG
include $(BUILD_SHARED_LIBRARY)

