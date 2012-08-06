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

#include "ffmpeg_b2_queue.h"

namespace android {

int32_t pkt_clst_init(_PKT_CLST_ *qcp, int max)
{
    qcp->max = max;
    qcp->sz = 0;
    qcp->s = 0;
    qcp->e = 0;

    qcp->bp = (NR_FRAME *)malloc(sizeof(NR_FRAME)*max);
    memset((void *)(qcp->bp), 0, sizeof(NR_FRAME)*max);

    return 0;
}

int32_t pkt_clst_probe(_PKT_CLST_ *qcp)
{
    return (qcp->max - qcp->sz);
}

int32_t pkt_clst_add(_PKT_CLST_ *qcp, NR_FRAME *bp)
{
    int num;

//printf("pkt_clst_add %d\n", bp->track_no);

    if( qcp->sz >= qcp->max ) return 1;

	num = qcp->e;
    memcpy((void*)(&(qcp->bp[num])), (void *)bp, sizeof(NR_FRAME));

    num++;
    if( num == qcp->max ) num = 0;
    qcp->e = num;

    qcp->sz++;

    return 0;
}


int32_t pkt_clst_get(_PKT_CLST_ *qcp, NR_FRAME *bp)
{
	int32_t num;

    if( qcp->sz == 0 ) return 1;

	num = qcp->s;
    memcpy((void *)bp, (void*)(&(qcp->bp[num])), sizeof(NR_FRAME));
    memset((void *)(&(qcp->bp[num])), 0, sizeof(NR_FRAME));

    num++;
    if( num == qcp->max ) num = 0;
    qcp->s = num;

    qcp->sz--;

    return 0;

}

int32_t pkt_clst_clear(_PKT_CLST_ *qcp)
{
	NR_FRAME nf;

    while( 1 )
    {
        if( pkt_clst_get(qcp, &nf) ) return 0;
        if( nf.data_ptr ) free((void *)(nf.data_ptr));
    }

    return 1;
}

}
