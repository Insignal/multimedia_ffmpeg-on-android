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

#define LOG_TAG "__ffmpeg_b2_fd__"
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

/*
#include "bb_dx_libavformat.h"
#include <libavutil/avstring.h>
#include <libavformat/avformat.h>
#include <fcntl.h>
#if defined(__DP_SYS_WIN32__)
#include <io.h>
#endif
//#include <libavformat/os_support.h>
#include <libavformat/url.h>
*/

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

namespace android {

/* standard file protocol */

static int hofile_read(URLContext *h, unsigned char *buf, int size)
{
    int fd = (intptr_t) h->priv_data;
    int r = read(fd, buf, size);
printf("fkdsflkdslfkslfks\n");
    return (-1 == r)?AVERROR(errno):r;
}

static int hofile_write(URLContext *h, const unsigned char *buf, int size)
{
    int fd = (intptr_t) h->priv_data;
    int r = write(fd, buf, size);
    return (-1 == r)?AVERROR(errno):r;
}

static int hofile_get_handle(URLContext *h)
{
    return (intptr_t) h->priv_data;
}

static int hofile_check(URLContext *h, int mask)
{
    struct stat st;
    int ret = stat(h->filename, &st);
    if (ret < 0)
        return AVERROR(errno);

    ret |= st.st_mode&S_IRUSR ? mask&AVIO_FLAG_READ  : 0;
    ret |= st.st_mode&S_IWUSR ? mask&AVIO_FLAG_WRITE : 0;

    return ret;
}

int __av_strstart(const char *str, const char *pfx, const char **ptr)
{
     while (*pfx && *pfx == *str) {
         pfx++;
         str++;
     }
     if (!*pfx && ptr)
         *ptr = str;
     return !*pfx;
}

static int hofile_open(URLContext *h, const char *filename, int flags)
{
    int access;
    int fd;

printf("fkdsflkdslfkslfks\n");
    __av_strstart(filename, "hofd://", &filename);

    sscanf(filename, "%d", &fd);

    if (fd == -1)
        return AVERROR(errno);
    h->priv_data = (void *) (intptr_t) fd;
lseek(fd, 0, SEEK_SET);

    return 0;
}

/* XXX: use llseek */
static int64_t hofile_seek(URLContext *h, int64_t pos, int whence)
{
    int fd = (intptr_t) h->priv_data;
    if (whence == AVSEEK_SIZE) {
        struct stat st;
        int ret = fstat(fd, &st);
        return ret < 0 ? AVERROR(errno) : st.st_size;
    }
    return lseek(fd, pos, whence);
}

static int hofile_close(URLContext *h)
{
    int fd = (intptr_t) h->priv_data;
uuprintf("hofile_close");

    //return close(fd);
    return 0;
}

URLProtocol hofd_protocol = {
    "hofd",
    hofile_open,
    0,
    hofile_read,
    hofile_write,
    hofile_seek,
    hofile_close,
    0,
    0,
    0,
    hofile_get_handle,
    0,
    0,
    0,
    0,
    hofile_check
};

}  // namespace android
