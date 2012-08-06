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

#ifndef _FFMPGE_B2_QUEUE_H__
#define _FFMPGE_B2_QUEUE_H__

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ffmpeg.h"


typedef struct
{
    int32_t max;
    int32_t sz;
    int32_t s;
    int32_t e;
    NR_FRAME *bp;
} _PKT_CLST_;


namespace android {

#ifdef __cplusplus
extern          "C"     {
#endif

int32_t pkt_clst_init(_PKT_CLST_ *qcp, int max);
int32_t pkt_clst_probe(_PKT_CLST_ *qcp);
int32_t pkt_clst_add(_PKT_CLST_ *qcp, NR_FRAME *bp);
int32_t pkt_clst_get(_PKT_CLST_ *qcp, NR_FRAME *bp);
int32_t pkt_clst_clear(_PKT_CLST_ *qcp);

#ifdef __cplusplus
}
#endif

}
#endif
