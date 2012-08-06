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

#define LOG_TAG "__ffmpeg_b2_connect__"
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
#include <dlfcn.h>
//#include <media/stagefright/ffmpeg_connect.h>
#include "ffmpeg.h"

namespace android {

typedef struct
{
    void (*__xx_av_register_all)(void);
    int (*__xx_ffurl_register_protocol)(URLProtocol *, int);
#ifdef __DP_FFMPEG_NEW__
    int (*__xx_avformat_open_input)(AVFormatContext **, const char *, AVInputFormat *, AVDictionary **);
#endif
#ifdef __DP_FFMPEG_OLD__
    int (*__xx_av_open_input_file)(AVFormatContext **, const char *, AVInputFormat *, int, AVFormatParameters *);
#endif
    void (*__xx_av_close_input_file)(AVFormatContext *);
    int (*__xx_av_find_stream_info)(AVFormatContext *);
    int (*__xx_av_read_frame)(AVFormatContext *, AVPacket *);
    int (*__xx_avformat_seek_file)(AVFormatContext *, int, int64_t, int64_t, int64_t, int);
    AVCodec *(*__xx_avcodec_find_decoder)(enum CodecID);
    int (*__xx_avcodec_close)(AVCodecContext *);
    void (*__xx_av_free_packet)(AVPacket *);
    void (*__xx_avcodec_flush_buffers)(AVCodecContext *);
#ifdef __DP_FFMPEG_NEW__
    int (*__xx_av_dict_set)(AVDictionary **, const char *, const char *, int);
    void (*__xx_av_dict_free)(AVDictionary **);
#endif
    void *(*__xx_av_malloc)(size_t);
    void (*__xx_av_free)(void *);
    int videoStream, audioStream;
} __BB_DX_LIBAVFORMAT__;

typedef struct
{
    uint32_t        i_ty;
    uint32_t        av_ty;
    uint32_t        enable;
    char            *name;
} __FORMAT_MATCH__;


#define __FORMAT_AV_TY_AV__     0
#define __FORMAT_AV_TY_V__      1
#define __FORMAT_AV_TY_A__      2

__FORMAT_MATCH__        __format_match__[] =
{
    {__ITY_AVI__,             __FORMAT_AV_TY_AV__,    1,	"avi"},
    {__ITY_MOV__,             __FORMAT_AV_TY_AV__,    1,	"mov,mp4,m4a,3gp,3g2,mj2"},
    {__ITY_MPEGPS__,          __FORMAT_AV_TY_AV__,    0,	"mpeg"},
    {__ITY_MPEGTS__,          __FORMAT_AV_TY_AV__,    0,	"mpegts"},
#ifdef __DP_FFMPEG_NEW__
    {__ITY_MKV__,             __FORMAT_AV_TY_AV__,    0,	"matroska,webm"},
#endif
#ifdef __DP_FFMPEG_OLD__
    {__ITY_MKV__,             __FORMAT_AV_TY_AV__,    0,	"matroska"},
#endif
    {__ITY_MP3__,             __FORMAT_AV_TY_A__,     1,	"mp3"},
    {__ITY_OGG__,             __FORMAT_AV_TY_A__,     0,	"ogg"},
    {__ITY_WAV__,             __FORMAT_AV_TY_A__,     0,	"wav"},
    {__ITY_ASF__,             __FORMAT_AV_TY_A__,     0,	"asf"},
};


__FORMAT_MATCH__        __codec_match__[] =
{
    {__CODEC_OMX_MPEG4__,     __FORMAT_AV_TY_V__,     1,	"mpeg4"},
	{__CODEC_OMX_H264__,      __FORMAT_AV_TY_V__,     1,	"h264"},
    {__CODEC_VC_DIVX__,       __FORMAT_AV_TY_V__,     0,	"bloked"},
    {__CODEC_VC_MPEG2__,      __FORMAT_AV_TY_V__,     0,	"mpeg2video"},
    {__CODEC_VC_VC1RCV__,     __FORMAT_AV_TY_V__,     0,	"wmv3"},
    {__CODEC_VC_FIMV3__,      __FORMAT_AV_TY_V__,     0,	"msmpeg4"},
    {__CODEC_AC_MP3__,        __FORMAT_AV_TY_A__,     0,	"mp3"},
    {__CODEC_AC_AAC_LOW__,    __FORMAT_AV_TY_A__,     0,	"aac"},
};


int __set_time(uint32_t i_ty, int mode, AVStream *avp, NR_TRACK *ntp, AVPacket *pkt, NR_FRAME *nfp, int64_t mode3_us, int64_t *pts);

int __bb_dx_init_libavformat(void *fnp, int *ret_malloc_sz)
{
	DEMUX_FUNCS *dmxp = (DEMUX_FUNCS *)fnp;

	dmxp->fnp_dx_load               = dx_libavformat_load;
	dmxp->fnp_dx_unload             = dx_libavformat_unload;
	dmxp->fnp_dx_file_read          = dx_libavformat_file_read_open;
	dmxp->fnp_dx_file_colse         = dx_libavformat_file_read_close;
	dmxp->fnp_dx_frame_read         = dx_libavformat_frame_read;
	dmxp->fnp_dx_frame_seek         = dx_libavformat_frame_seek;
	dmxp->fnp_dx_frame_alloc        = dx_libavformat_frame_alloc;
	dmxp->fnp_dx_frame_release      = dx_libavformat_frame_release;

	*ret_malloc_sz = sizeof(__BB_DX_LIBAVFORMAT__);

	return 0;
}

int dx_libavformat_load(void *r_data, NR_MEDIA *nmp)
{
	void* hohop;

    __BB_DX_LIBAVFORMAT__ *bbp = (__BB_DX_LIBAVFORMAT__ *)r_data;

    hohop = dlopen("libffmpeg.so", RTLD_NOW);
    if( hohop == 0 ) return 1;

    bbp->__xx_av_register_all = 0;
    bbp->__xx_av_register_all = (void (*)(void))dlsym(hohop, "av_register_all");
    if( bbp->__xx_av_register_all == 0 ) return 1;

    bbp->__xx_ffurl_register_protocol = 0;
    bbp->__xx_ffurl_register_protocol = (int (*)(URLProtocol *, int))dlsym(hohop, "ffurl_register_protocol");
    if( bbp->__xx_ffurl_register_protocol == 0 ) return 1;
#ifdef __DP_FFMPEG_NEW__
    bbp->__xx_avformat_open_input = 0;
    bbp->__xx_avformat_open_input = (int (*)(AVFormatContext **, const char *, AVInputFormat *, AVDictionary **))dlsym(hohop, "avformat_open_input");
    if( bbp->__xx_avformat_open_input == 0 ) return 1;
#endif
#ifdef __DP_FFMPEG_OLD__
    bbp->__xx_av_open_input_file =  0;
    bbp->__xx_av_open_input_file = (int (*)(AVFormatContext **, const char *, AVInputFormat *, int, AVFormatParameters *))dlsym(hohop, "av_open_input_file");
    if( bbp->__xx_av_open_input_file == 0 ) return 1;
#endif
    bbp->__xx_av_close_input_file = 0;
    bbp->__xx_av_close_input_file = (void (*)(AVFormatContext *))dlsym(hohop, "av_close_input_file");
    if( bbp->__xx_av_close_input_file == 0 ) return 1;

    bbp->__xx_av_find_stream_info = 0;
    bbp->__xx_av_find_stream_info = (int (*)(AVFormatContext *))dlsym(hohop, "av_find_stream_info");
    if( bbp->__xx_av_find_stream_info == 0 ) return 1;

    bbp->__xx_av_read_frame = 0;
    bbp->__xx_av_read_frame = (int (*)(AVFormatContext *, AVPacket *))dlsym(hohop, "av_read_frame");
    if( bbp->__xx_av_read_frame == 0 ) return 1;

    bbp->__xx_avformat_seek_file = 0;
    bbp->__xx_avformat_seek_file = (int (*)(AVFormatContext *, int, int64_t, int64_t, int64_t, int))dlsym(hohop, "avformat_seek_file");
    if( bbp->__xx_avformat_seek_file == 0 ) return 1;

    bbp->__xx_avcodec_find_decoder = 0;
    bbp->__xx_avcodec_find_decoder = (AVCodec *(*)(enum CodecID))dlsym(hohop, "avcodec_find_decoder");
    if( bbp->__xx_avcodec_find_decoder == 0 ) return 1;

    bbp->__xx_avcodec_close = 0;
    bbp->__xx_avcodec_close = (int (*)(AVCodecContext *))dlsym(hohop, "avcodec_close");
    if( bbp->__xx_avcodec_close == 0 ) return 1;

    bbp->__xx_av_free_packet = 0;
    bbp->__xx_av_free_packet = (void (*)(AVPacket *))dlsym(hohop, "av_free_packet");
    if( bbp->__xx_av_free_packet == 0 ) return 1;

    bbp->__xx_avcodec_flush_buffers = 0;
    bbp->__xx_avcodec_flush_buffers = (void (*)(AVCodecContext *))dlsym(hohop, "avcodec_flush_buffers");
    if( bbp->__xx_avcodec_flush_buffers == 0 ) return 1;

#ifdef __DP_FFMPEG_NEW__
    bbp->__xx_av_dict_set = 0;
    bbp->__xx_av_dict_set = (int (*)(AVDictionary **, const char *, const char *, int))dlsym(hohop, "av_dict_set");
    if( bbp->__xx_av_dict_set == 0 ) return 1;

    bbp->__xx_av_dict_free = 0;
    bbp->__xx_av_dict_free = (void (*)(AVDictionary **))dlsym(hohop, "av_dict_free");
    if( bbp->__xx_av_dict_free == 0 ) return 1;
#endif
    bbp->__xx_av_malloc = 0;
    bbp->__xx_av_malloc = (void *(*)(size_t))dlsym(hohop, "av_malloc");
    if( bbp->__xx_av_malloc == 0 ) return 1;

    bbp->__xx_av_free = 0;
    bbp->__xx_av_free = (void (*)(void *))dlsym(hohop, "av_free");
    if( bbp->__xx_av_free == 0 ) return 1;

    return 0;
}

int dx_libavformat_unload(void *r_data, NR_MEDIA *nmp)
{
	return 0;
}

extern URLProtocol hofd_protocol;

int dx_libavformat_file_read_open(void *r_data, char *filename, NR_MEDIA *nmp)
{
	FILE *fp;
	NR_TRACK *ntp;
	int tmp, i, j, av_ty, enable;
	int64_t t_calc_duration_v, t_calc_duration_a;
	AVCodecContext *pCodecCtx;
	AVCodec	*pCodec;
	AVFormatContext	*pFormatCtx = NULL;

	__BB_DX_LIBAVFORMAT__ *bbp = (__BB_DX_LIBAVFORMAT__ *)r_data;

	memset(nmp, 0, sizeof(NR_MEDIA));

	(*(bbp->__xx_av_register_all))();
	(*(bbp->__xx_ffurl_register_protocol))(&hofd_protocol, sizeof(hofd_protocol));
#ifdef __DP_FFMPEG_NEW__
	//bbp->iformat = NULL;
	//bbp->dict = NULL;
uuprintf("__xx_avformat_open_input %s\n", filename);
	if( (*(bbp->__xx_avformat_open_input))(&pFormatCtx, filename, NULL, NULL) != 0 ) return 1; // Couldn't open file
	//if( (*(bbp->__xx_avformat_open_input))(&pFormatCtx, filename, bbp->iformat, &(bbp->dict)) != 0 ) return 1; // Couldn't open file
#endif
#ifdef __DP_FFMPEG_OLD__
uuprintf("__xx_av_open_input_file\n");
	if( (*(bbp->__xx_av_open_input_file))(&pFormatCtx, filename, NULL, 0, NULL) != 0 ) return 1;
#endif
uuprintf("__xx_av_find_stream_info\n");
	if( (*(bbp->__xx_av_find_stream_info))(pFormatCtx) < 0 ) return 1; // Couldn't find stream information
uuprintf("find demuxer %s\n", pFormatCtx->iformat->name);

	nmp->fourcc_ex1 = 0;
	nmp->fourcc_ex2 = 0;
	nmp->fourcc_ex3 = 0;

	for( i=0, tmp=0 ; i<(int)(sizeof(__format_match__)/sizeof(__FORMAT_MATCH__)) ; i++ )
	{
		if( strcmp(pFormatCtx->iformat->name, __format_match__[i].name) == 0 )
		{
			tmp = __format_match__[i].i_ty;
			av_ty = __format_match__[i].av_ty;
			enable = __format_match__[i].enable;
			break;
		}
    }
uuprintf("find demuxer %d\n", tmp);

	if( tmp == 0 )
	{
		fprintf(stdout,	"Track malloc error\n");
		fflush(stdout);
		exit(1);
	}

	if( enable == 0 )
	{
		return 1;
	}

	nmp->fourcc_ex1 = tmp;
	//nmp->duration = pFormatCtx->duration;

	// Find	the first video	stream
	bbp->videoStream = -1;
	bbp->audioStream = -1;
	nmp->track_sz = 0;
	for( i=0; i<(int)(pFormatCtx->nb_streams); i++ ) {
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
			bbp->videoStream = i;
			nmp->track_sz++;
			break;
		}
	}

	for( i=0; i<(int)(pFormatCtx->nb_streams); i++ ) {
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
			bbp->audioStream = i;
			nmp->track_sz++;
			break;
		}
	}

	if( nmp->track_sz )	{
		nmp->track_ptr = (uint64_t)malloc(sizeof(NR_TRACK)*(nmp->track_sz));
		if( nmp->track_ptr == 0 ) {
			fprintf(stdout,	"Track malloc error\n");
			fflush(stdout);
			exit(1);
		}
	}

	nmp->video_track_no = -1;
	nmp->audio_track_no = -1;

	for( i=0 ; i<(int)(nmp->track_sz) ; i++ ) {
		ntp = (NR_TRACK *)((uint8_t*)(nmp->track_ptr)+sizeof(NR_TRACK)*i);
		memset((void*)ntp, 0, sizeof(NR_TRACK));

		if( i == bbp->videoStream ) {
			pCodecCtx = pFormatCtx->streams[bbp->videoStream]->codec;
			pCodec = (*(bbp->__xx_avcodec_find_decoder))(pCodecCtx->codec_id);

			ntp->m_ty                       = NR_MEDIA_TY_AVFORMAT;
			ntp->c_ty                       = NR_C_TY_VIDEO;
			ntp->fourcc                     = MK_FOURCC("nrv0");
			ntp->ffmpeg_avformat_ptr        = (uint64_t)pFormatCtx;
			ntp->ffmpeg_codeccontext_ptr    = (uint64_t)pCodecCtx;
			ntp->ffmpeg_codec_ptr           = (uint64_t)pCodec;
			ntp->ffmpeg_codec_id            = (uint64_t)pCodecCtx->codec_id;
			ntp->pix_fmt					= pCodecCtx->pix_fmt;
			ntp->v_f_rate_num				= pFormatCtx->streams[bbp->videoStream]->r_frame_rate.num;
			ntp->v_f_rate_den				= pFormatCtx->streams[bbp->videoStream]->r_frame_rate.den;
			ntp->v_s_res_x					= pCodecCtx->width;
			ntp->v_s_res_y					= pCodecCtx->height;
			ntp->v_s_ratio					= (float)(ntp->v_s_res_x) / (float)(ntp->v_s_res_y);
			ntp->v_d_res_x					= ntp->v_s_res_x;
			ntp->v_d_res_y					= ntp->v_s_res_y;
			ntp->v_d_ratio					= ntp->v_s_ratio;

			nmp->video_track_no = i;
			__set_time(nmp->fourcc_ex1, 1, pFormatCtx->streams[bbp->videoStream], ntp, NULL, NULL, 0, 0);
			t_calc_duration_v = ntp->t_calc_duration;

uuprintf("__codec_match__ %s\n", pCodec->name);
			for( j=0, tmp=0 ; j<(int)(sizeof(__codec_match__)/sizeof(__FORMAT_MATCH__)) ; j++ )	{
				if( strcmp(pCodec->name, __codec_match__[j].name) == 0 ) {
					tmp = __codec_match__[j].i_ty;
					break;
				}
			}

			if( tmp == __CODEC_OMX_H264__ )	{
				nmp->h264_nall_length = 1 + (((uint8_t *)(pCodecCtx->extradata))[4] & 3);
				nmp->h264_profile = pCodecCtx->profile;
			}

			nmp->fourcc_ex2 = tmp;
			nmp->video_track_enable = 1;
		}

		if( i == bbp->audioStream )	{
			int32_t channels, bits_per_coded_sample, sample_fmt;

			pCodecCtx = pFormatCtx->streams[bbp->audioStream]->codec;
			pCodec = (*(bbp->__xx_avcodec_find_decoder))(pCodecCtx->codec_id);

			if( pCodecCtx->channels > 2 ) {
				pCodecCtx->request_channels = 2;
				channels = 2;
			} else
				channels = pCodecCtx->channels;
			bits_per_coded_sample = pCodecCtx->bits_per_coded_sample;
			sample_fmt = pCodecCtx->sample_fmt;

			if( bits_per_coded_sample == 0 ) {
#ifdef __DP_FFMPEG_NEW__
				if( pCodecCtx->sample_fmt == AV_SAMPLE_FMT_S16 )
#endif
#ifdef __DP_FFMPEG_OLD__
				if( pCodecCtx->sample_fmt == SAMPLE_FMT_S16 )
#endif
				{
					bits_per_coded_sample = 16;
					channels = 2;
				}
			} else {
#ifdef __DP_FFMPEG_NEW__
				if( (bits_per_coded_sample = 16) && (channels == 2) )
					sample_fmt = AV_SAMPLE_FMT_S16;
#endif
#ifdef __DP_FFMPEG_OLD__
				if( (bits_per_coded_sample = 16) && (channels == 2) )
					sample_fmt = SAMPLE_FMT_S16;
#endif
			}

			ntp->m_ty                       = NR_MEDIA_TY_AVFORMAT;
			ntp->c_ty                       = NR_C_TY_AUDIO;
			ntp->fourcc                     = MK_FOURCC("nra0");
			ntp->ffmpeg_avformat_ptr        = (uint64_t)pFormatCtx;
			ntp->ffmpeg_codeccontext_ptr    = (uint64_t)pCodecCtx;
			ntp->ffmpeg_codec_ptr           = (uint64_t)pCodec;
			ntp->ffmpeg_codec_id            = (uint64_t)pCodecCtx->codec_id;
			ntp->a_chs						= channels;
			ntp->a_bit_fmt	                = sample_fmt;
			ntp->a_bits						= bits_per_coded_sample;
			ntp->a_s_rate					= pCodecCtx->sample_rate;
			ntp->a_frame_size				= pCodecCtx->frame_size;
			//ntp->a_block_size				= bits_per_coded_sample/8 * channels * ntp->a_frame_size;
			ntp->a_block_size				= 0;
			//ntp->a_start					= 1;

			nmp->audio_track_no = i;
			__set_time(nmp->fourcc_ex1, 1, pFormatCtx->streams[bbp->audioStream], ntp, NULL, NULL, 0, 0);
			t_calc_duration_a = ntp->t_calc_duration;

			for( j=0, tmp=0 ; j<(int)(sizeof(__codec_match__)/sizeof(__FORMAT_MATCH__)) ; j++ )	{
				if( strcmp(pCodec->name, __codec_match__[j].name) == 0 ) {
					tmp = __codec_match__[j].i_ty;
					break;
				}
			}

			nmp->fourcc_ex3 = tmp;
			nmp->audio_track_enable = 1;
		}
		ntp->t_cur_us = -1;
	}

	if( av_ty == __FORMAT_AV_TY_AV__ ) {
		if( bbp->videoStream >= 0 ) {
			nmp->master_track_no = bbp->videoStream;
			nmp->duration_ms = t_calc_duration_v / 1000;
		} else {
			nmp->master_track_no = bbp->audioStream;
			nmp->duration_ms = t_calc_duration_a / 1000;
		}
    }
	if( av_ty == __FORMAT_AV_TY_A__ ) {
		nmp->video_track_enable = 0;
		nmp->master_track_no = bbp->audioStream;
		nmp->duration_ms = t_calc_duration_a / 1000;
	}


	nmp->m_ty                               = NR_MEDIA_TY_AVFORMAT;
	nmp->m_ptr.ffmpeg_avformat_ptr          = (uint64_t)pFormatCtx;

	nmp->start_demux                        = -1;
	nmp->start_decode                       = -1;
	nmp->speed_def                          = 1000;
	nmp->speed_play                         = nmp->speed_def;
	nmp->speed_play_bb                      = 0;
	nmp->speed_play_ff                      = 0;
	nmp->play_mode                          = __PLAY_MODE_PAUSE__;
	nmp->ff_msec                            = -1;
	nmp->ff_state                           = 0;
	nmp->frame_drops                        = 0;

	return 0;
}

int dx_libavformat_file_read_close(void *r_data, NR_MEDIA *nmp)
{
	NR_TRACK *ntp;
	int i;
	__BB_DX_LIBAVFORMAT__ *bbp = (__BB_DX_LIBAVFORMAT__ *)r_data;

/* codec_unload
    for( i=0 ; i<(nmp->track_sz) ; i++ )
    {
		ntp = (NR_TRACK *)(nmp->track_ptr+sizeof(NR_TRACK)*i);
		(*(bbp->__xx_avcodec_close))((AVCodecContext *)(ntp->ffmpeg_codeccontext_ptr));
	}
*/

    if( nmp->track_ptr ) free((void*)(nmp->track_ptr));
    (*(bbp->__xx_av_close_input_file))((AVFormatContext *)(nmp->m_ptr.ffmpeg_avformat_ptr));

    return 0;
}

int __set_time(uint32_t i_ty, int mode, AVStream *avp, NR_TRACK *ntp, AVPacket *pkt, NR_FRAME *nfp, int64_t mode3_us, int64_t *pts)
{
	switch( i_ty ) {
    case __ITY_MKV__:
    case __ITY_AVI__:
    case __ITY_MP3__:
    case __ITY_MPEGPS__:
    case __ITY_MPEGTS__:
    case __ITY_MOV__:
    case __ITY_OGG__:
    case __ITY_WAV__:
    default:
		if( mode == 1 )	{
			ntp->ff_pts_start = avp->start_time;
			//ntp->t_calc_duration = ((AVFormatContext *)(ntp->ffmpeg_avformat_ptr))->duration;
			ntp->t_calc_duration = (int64_t)(avp->duration - avp->start_time) * 1000 * 1000
	                                                * avp->time_base.num
	                                                / avp->time_base.den;
		} else if( mode == 2 ) {
			nfp->decode_contiune = 0;
			if( pkt->dts <= 0 ) { // 60000/2002
				nfp->t_us_pos = 0;
				nfp->decode_contiune = 1;
			} else {
				//nfp->t_us_pos = ntp->t_calc_duration * (pkt->pts - ntp->ff_pts_start) / (ntp->ff_pts_end - ntp->ff_pts_start);
				//nfp->decode_contiune = 0;
				nfp->t_us_pos = (int64_t)(pkt->dts - ntp->ff_pts_start) * 1000 * 1000
						* ((AVFormatContext *)(ntp->ffmpeg_avformat_ptr))->streams[nfp->track_no]->time_base.num
						/ ((AVFormatContext *)(ntp->ffmpeg_avformat_ptr))->streams[nfp->track_no]->time_base.den;
				//printf("dx_pos, %d:%f\n", ntp->c_ty, (float)(nfp->t_us_pos)/1000000);fflush(stdout);
            }
		}
		break;
	}

	return 0;
}

int dx_libavformat_frame_read(void *r_data, NR_MEDIA *nmp, NR_FRAME *nfp)
{
	NR_TRACK *ntp = 0;
	uint64_t tmp_xx;
	AVPacket packet;
	uint32_t track_no = -1;
	int type;
	__BB_DX_LIBAVFORMAT__ *bbp = (__BB_DX_LIBAVFORMAT__ *)r_data;

	memset((void *)nfp, 0, sizeof(NR_FRAME));

//printf("dx_format --01\n");fflush(stdout);
	if( (*(bbp->__xx_av_read_frame))((AVFormatContext *)(nmp->m_ptr.ffmpeg_avformat_ptr), &packet) < 0 ) return 1;
//printf("dx_format --02\n");fflush(stdout);

	if( (packet.stream_index == bbp->videoStream) )
	{
		ntp = (NR_TRACK *)((uint8_t*)(nmp->track_ptr)+sizeof(NR_TRACK)*nmp->video_track_no);
		track_no = nmp->video_track_no;
		type = NR_FRAME_TY_VIDEO;
	}
	if( (packet.stream_index == bbp->audioStream) )
	{
		ntp = (NR_TRACK *)((uint8_t*)(nmp->track_ptr)+sizeof(NR_TRACK)*nmp->audio_track_no);
		track_no = nmp->audio_track_no;
		type = NR_FRAME_TY_AUDIO;
	}

//printf("dx_format --1, %d\n", track_no);fflush(stdout);
	if( track_no >= 0 )
	{
		if( packet.data )
		{
			nfp->type = type;
			nfp->track_no = track_no;
			nfp->data_size = packet.size;
//printf("packet size : %d\n", nfp->data_size);fflush(stdout);
			dx_libavformat_frame_alloc(r_data, nfp);
			/*
			nfp->data_ptr = 0;
			nfp->data_ptr = (uint64_t)(*(bbp->__xx_av_malloc))(nfp->data_size);
			if( nfp->data_ptr == 0 )
			{
				fprintf(stdout,	"Frame malloc error\n");
				fflush(stdout);
				exit(1);
			}
			*/
			memcpy((void *)(nfp->data_ptr), (void *)(packet.data), packet.size);
		}
/*
//if( track_no == 0 )
{
        uuprintf("dx_format track:%d,", track_no);
        uuprintf(" flags:%4d,", packet.flags);
        uuprintf(" pts:%10d,", packet.pts);
        uuprintf(" dts:%10d\n", packet.dts);
}
*/
		nfp->i_frame_and_flag = 0;
		if( ntp->c_ty == NR_C_TY_VIDEO )
		{
			if( packet.flags == 1 )
				nfp->i_frame_and_flag = 1;

			//tmp++;
		}

		__set_time(nmp->fourcc_ex1, 2, NULL, ntp, &packet, nfp, 0, 0);
	}

	if( packet.data ) (*(bbp->__xx_av_free_packet))(&packet);

	return 0;
}

int dx_libavformat_frame_seek(void *r_data, NR_MEDIA *nmp, int track_no, uint64_t msec, int hw_on)
{
	NR_TRACK *ntp;
	uint64_t i, ts;
	int ret_v;
	__BB_DX_LIBAVFORMAT__ *bbp = (__BB_DX_LIBAVFORMAT__ *)r_data;

	ts = msec * (((AVFormatContext*)(nmp->m_ptr.ffmpeg_avformat_ptr))->streams[track_no]->time_base.den)
                / ( (((AVFormatContext*)(nmp->m_ptr.ffmpeg_avformat_ptr))->streams[track_no]->time_base.num) * 1000 )
                + ((NR_TRACK *)((uint8_t*)(nmp->track_ptr)+sizeof(NR_TRACK)*track_no))->ff_pts_start;

uuprintf("dx_seek, %d:%f:%lld\n", track_no, (float)msec/1000, ts);
	if( msec > nmp->duration_ms ) return 1;

	ret_v = (*(bbp->__xx_avformat_seek_file))(((AVFormatContext*)(nmp->m_ptr.ffmpeg_avformat_ptr)), track_no, INT64_MIN, ts, INT64_MAX, 0);

	if( ret_v < 0 )
	{
		printf("dll Seek error\n");fflush(stdout);
		return 1;
		//error
	}

//uuprintf("dx_seek video stream %d\n", bbp->videoStream);

	if( hw_on == 0 )
	{
		if( bbp->videoStream >= 0 )
			(*(bbp->__xx_avcodec_flush_buffers))((((AVFormatContext*)(nmp->m_ptr.ffmpeg_avformat_ptr))->streams[bbp->videoStream]->codec));
	}

//        if( bbp->audioStream >= 0 ) // audio tick noise
//                (*(bbp->__xx_avcodec_flush_buffers))((((AVFormatContext*)(nmp->m_ptr.ffmpeg_avformat_ptr))->streams[bbp->audioStream]->codec));

	return 0;
}

int dx_libavformat_frame_alloc(void *r_data, NR_FRAME *nfp)
{
	__BB_DX_LIBAVFORMAT__ *bbp = (__BB_DX_LIBAVFORMAT__ *)r_data;

	nfp->data_ptr = 0;
	//nfp->data_ptr = (uint64_t)(*(bbp->__xx_av_malloc))(nfp->data_size);
	nfp->data_ptr = (uint64_t)malloc(nfp->data_size);
	if( nfp->data_ptr == 0 )
	{
		fprintf(stdout,	"dx_libavformat_frame_alloc malloc error\n");
		fflush(stdout);
		exit(1);
	}

//if( nfp->type == NR_FRAME_TY_RGB32 ) uuprintf("ct_alloc %d\n", ++bbp->ct_alloc);

	return 0;
}

int dx_libavformat_frame_release(void *r_data, NR_FRAME *nfp)
{
	__BB_DX_LIBAVFORMAT__ *bbp = (__BB_DX_LIBAVFORMAT__ *)r_data;

//if( nfp->type == NR_FRAME_TY_RGB32 ) uuprintf("ct_free %d\n", ++bbp->ct_free);

	//if( nfp->data_ptr ) (*(bbp->__xx_av_free))((void *)(nfp->data_ptr));
	if( nfp->data_ptr ) free((void *)(nfp->data_ptr));
	memset((void *)nfp, 0, sizeof(NR_FRAME));

	return 0;
}

}  // namespace android
