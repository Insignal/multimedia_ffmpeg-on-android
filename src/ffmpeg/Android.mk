#
# Copyright (C) 2012	The FFMPEG for Android Open Source Project
#						InSignal Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

ifeq ($(strip$(BOARD_USES_FFMPEG)),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=					\
        ffmpeg_a1_source.cpp		\
        ffmpeg_b2_connect.cpp		\
        ffmpeg_b2_fd.cpp			\
        ffmpeg_b2_queue.cpp			\
        ffmpeg_c3_extradata.cpp		\
        ff_extractor.cpp			\

LOCAL_C_INCLUDES:=		\
	$(JNI_H_INCLUDE)	\
	$(TOP)/frameworks/base/include/media/stagefright/openmax \
        $(TOP)/frameworks/base/media/libstagefright \
        $(TOP)/external/ffmpeg-dev/include			\

LOCAL_MODULE:= libstagefright_ffmpeg

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

include $(BUILD_STATIC_LIBRARY)
endif
