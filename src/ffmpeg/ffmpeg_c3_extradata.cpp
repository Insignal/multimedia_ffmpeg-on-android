/*
 * Copyright (C) 2012	The FFMPEG for Android Open Source Project
 *						InSignal Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#define LOG_TAG "__ffmpeg_c3_extradata__"
#include <utils/Log.h>
	#define uuprintf(fmt, args...) LOGE("%s(%d): " fmt, __FUNCTION__, __LINE__, ##args)


#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaDebug.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include <utils/String8.h>
//#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/Utils.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

#include "ffmpeg_c3_extradata.h"

namespace android {


void __hexdump__(char *name, const void *_data, size_t size)
{
    const uint8_t *data = (const uint8_t *)_data;

    int i, j, k, pos;
    uint8_t str[100];

    uuprintf("____DUMP____DUMP____NAME : %s\n", name);
    memset(str, 0, 100);
    uuprintf("Dec sz =  %4d| 000 001 002 003 004 005 006 007 008 009 00a 00b 00c 00d 00e 00f\n", size);
    for( i=0, j=0, k=0, pos=0 ; i<size ; i++, j++ )
    {
		if( j == 16 ) { j = 0; k++; pos=0; }
        if( j == 0 ) { sprintf((char*)&(str[pos]), "[%04d]", i); pos+=6; }
        sprintf((char*)&(str[pos]), " %3d", data[i]); pos+=4;
        if( j == 15 ) uuprintf("Dec Dump %s\n", str);
    }
	if( j < 16 ) uuprintf("Dec Dump %s\n", str);

    memset(str, 0, 100);
    uuprintf("Hex sz =  %4x| 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n", size);
    for( i=0, j=0, k=0, pos=0 ; i<size ; i++, j++ )
    {
        if( j == 16 ) { j = 0; k++; pos=0; }
        if( j == 0 ) { sprintf((char*)&(str[pos]), "[%04x]", i); pos+=6; }
        sprintf((char*)&(str[pos]), " %02x", data[i]); pos+=3;
        if( j == 15 ) uuprintf("Hex Dump %s\n", str);
    }
	if( j < 16 ) uuprintf("Hex Dump %s\n", str);
}

static size_t __GetSizeWidth(size_t x) {
    size_t n = 1;
    while (x > 127) {
        ++n;
        x >>= 7;
    }
    return n;
}

static uint8_t *__EncodeSize(uint8_t *dst, size_t x) {
    while (x > 127) {
        *dst++ = (x & 0x7f) | 0x80;
        x >>= 7;
    }
    *dst++ = x;
    return dst;
}

sp<ABuffer> ffmpeg_extradata_xvid(uint8_t *extradata, int32_t extradata_size) {
    size_t len1 = extradata_size + __GetSizeWidth(extradata_size) + 1;
    size_t len2 = len1 + __GetSizeWidth(len1) + 1 + 13;
    size_t len3 = len2 + __GetSizeWidth(len2) + 1 + 3;

uuprintf("xvid extra :::: GetSizeWidth(config->size()) = %d", __GetSizeWidth(extradata_size));
uuprintf("xvid extra :::: GetSizeWidth(config->size()) = %d", __GetSizeWidth(len1));
uuprintf("xvid extra :::: len1 = %d", len1);
uuprintf("xvid extra :::: len2 = %d", len2);
uuprintf("xvid extra :::: len3 = %d", len3);


    sp<ABuffer> csd = new ABuffer(len3);
    uint8_t *dst = csd->data();
    memset((void*)dst, 0, len3);
uuprintf("xvid extra :::: ptr_s = %p", dst);
    *dst++ = 0x03;
    dst = __EncodeSize(dst, len2 + 3);
uuprintf("xvid extra :::: ptr_s = %p", dst);
    *dst++ = 0x00;  // ES_ID
    *dst++ = 0x00;
    *dst++ = 0x00;  // streamDependenceFlag, URL_Flag, OCRstreamFlag

    *dst++ = 0x04;
    dst = __EncodeSize(dst, len1 + 13);
    *dst++ = 0x01;  // Video ISO/IEC 14496-2 Simple Profile
    for (size_t i = 0; i < 12; ++i) {
        *dst++ = 0x00;
    }

    *dst++ = 0x05;
uuprintf("xvid extra :::: ptr_s = %p", dst);
    dst = __EncodeSize(dst, extradata_size);
uuprintf("xvid extra :::: ptr_s = %p", dst);
    memcpy(dst, extradata, extradata_size);
uuprintf("xvid extra :::: config->size() = %d", extradata_size);
    dst += extradata_size;

    __hexdump__("xvid 1", csd->data(), csd->size());

    return csd;
}

static void __EncodeSize14(uint8_t **_ptr, size_t size) {
    CHECK_LE(size, 0x3fff);

    uint8_t *ptr = *_ptr;

    *ptr++ = 0x80 | (size >> 7);
    *ptr++ = size & 0x7f;

    *_ptr = ptr;
}

/* IN  /frameworks/base/media/libstagefright/Utils.cpp
uint16_t U16_AT(const uint8_t *ptr) {
    return ptr[0] << 8 | ptr[1];
}

uint32_t U32_AT(const uint8_t *ptr) {
    return ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];
}
*/

size_t h264_parseNALSize(const uint8_t *data, int32_t mNALLengthSize){
	switch (mNALLengthSize) {
		case 1:
			return *data;
		case 2:
			return U16_AT(data);
		case 3:
			return ((size_t)data[0] << 16) | U16_AT(&data[1]);
		case 4:
			return U32_AT(data);
	}

	// This cannot happen, mNALLengthSize springs to life by adding 1 to
	// a 2-bit integer.
	CHECK(!"Should not be here.");

	return 0;
}

sp<ABuffer> ffmpeg_esds_aac_low(uint8_t *extradata, int32_t extradata_size, int bit_rate) {
	CHECK(extradata_size + 23 < 128);

	int len = extradata_size + 25;
    sp<ABuffer> csd = new ABuffer(len);
    uint8_t *dst = csd->data();
    uint8_t buf[4];
    memset((void*)dst, 0, len);

//=================================
    *dst++ = 0x03;
    *dst++ = 23 + extradata_size;
    *dst++ = 0x00;  // ES_ID
    *dst++ = 0x00;
    *dst++ = 0x00;  // streamDependenceFlag, URL_Flag, OCRstreamFlag
//=================================
    *dst++ = 0x04;
    *dst++ = 15 + extradata_size;
    *dst++ = 0x67;
    *dst++ = 0x15;
    *dst++ = 0x00;
    *dst++ = 0x12;	//buf 0
    *dst++ = 0x34;	//buf 1
/*
    *dst++ = 0x00;
    *dst++ = 0x02;
    *dst++ = 0xfb;
    *dst++ = 0x78;
    *dst++ = 0x00;
    *dst++ = 0x02;
    *dst++ = 0xc7;
    *dst++ = 0xb0;
*/
    *(int *)buf = bit_rate;
    //*(int *)buf = 0x12345678;
    *dst++ = buf[0];
    *dst++ = buf[1];
    *dst++ = buf[2];
    *dst++ = buf[3];
    *(int *)buf = bit_rate * 2;
    *dst++ = buf[0];
    *dst++ = buf[1];
    *dst++ = buf[2];
    *dst++ = buf[3];
//=================================
    *dst++ = 0x05;
    dst = __EncodeSize(dst, extradata_size);
    memcpy(dst, extradata, extradata_size);
    dst += extradata_size;
//=================================
    *dst++ = 0x06;
    *dst++ = 0x01;
    *dst++ = 0x02;

    //__hexdump__("xvid 1", csd->data(), csd->size());

    return csd;
}

}  // namespace android
