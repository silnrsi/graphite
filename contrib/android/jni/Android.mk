#   GRAPHITE2 LICENSING
# 
#   Copyright 2010, SIL International
#   All rights reserved.
# 
#   This library is free software; you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License as published
#   by the Free Software Foundation; either version 2.1 of License, or
#   (at your option) any later version.
# 
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
# 
#   You should also have received a copy of the GNU Lesser General Public
#   License along with this library in the file named "LICENSE".
#   If not, write to the Free Software Foundation, Inc., 59 Temple Place, 
#   Suite 330, Boston, MA 02111-1307, USA or visit their web page on the 
#   internet at http://www.fsf.org/licenses/lgpl.html.
# 
#   Alternatively, you may use this library under the terms of the Mozilla
#   Public License (http://mozilla.org/MPL) or under the GNU General Public
#   License, as published by the Free Sofware Foundation; either version
#   2 of the license or (at your option) any later version.

LOCAL_PATH := $(call my-dir)

MY_ANDROID_SRC := $(HOME)/Work/android/android-src
MY_ANDROID_LIBS := $(MY_ANDROID_SRC)/out/target/product/generic/symbols/system/lib
#MY_ANDROID_LIBS := $(HOME)/Work/android/android-sdk-linux_x86/platforms/android-8/symbols/system/lib
MY_SKIA := $(MY_ANDROID_SRC)/external/skia
#MY_SKIA := $(HOME)/Work/android/skia/8

include $(CLEAR_VARS)

ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS += -DANDROID_ARM_LINKER
else
  ifeq ($(TARGET_ARCH),x86)
    LOCAL_CFLAGS += -DANDROID_X86_LINKER
  else
    ifeq ($(TARGET_ARCH),sh)
      LOCAL_CFLAGS += -DANDROID_SH_LINKER
    else
      $(error Unsupported TARGET_ARCH $(TARGET_ARCH))
    endif
  endif
endif

# set arm for debug purposes so we can set breakpoints
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS += -mapcs
LOCAL_MODULE := load-graphite
LOCAL_SHARED_LIBRARIES := graphite2
LOCAL_LDLIBS := -L $(MY_ANDROID_LIBS) -lskia -lcutils -landroid_runtime -lutils
LOCAL_CPPFLAGS += -fno-rtti -mapcs -fno-inline
LOCAL_SRC_FILES := loadgr_jni.cpp load.cpp graphite_layer.cpp
LOCAL_C_INCLUDES := $(MY_SKIA)/include/core \
                    $(MY_SKIA)/src/core \
                    $(MY_ANDROID_SRC)/external/freetype/include \
                    $(MY_ANDROID_SRC)/frameworks/base/include \
                    $(MY_ANDROID_SRC)/system/core/include
include $(BUILD_SHARED_LIBRARY)

NDK_APP_GDBSETUP := gdb.setup

ifeq ($(DUMP_VAR),)
include $(APP_PROJECT_PATH)/jni/graphite/Android.mk
endif

