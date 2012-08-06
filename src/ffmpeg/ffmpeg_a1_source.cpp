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

#define LOG_TAG "__ffmpeg_a1_source__"
#include <utils/Log.h>
	#define uuprintf(fmt, args...) LOGE("%s(%d): " fmt, __FUNCTION__, __LINE__, ##args)

#include <media/stagefright/FFmpegSource.h>
#include <media/stagefright/MediaDebug.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ffmpeg.h"

namespace android {

FFmpegSource::FFmpegSource(const char *filename)
    : mFd(-1),
      mOffset(0),
      mLength(-1),
      mDecryptHandle(NULL),
      mDrmManagerClient(NULL),
      mDrmBufOffset(0),
      mDrmBufSize(0),
      mDrmBuf(NULL){

    ff_ptr = 0;
    mFd = open(filename, O_LARGEFILE | O_RDONLY);
}

FFmpegSource::FFmpegSource(int fd, int64_t offset, int64_t length)
    : mFd(fd),
      mOffset(offset),
      mLength(length),
      mDecryptHandle(NULL),
      mDrmManagerClient(NULL),
      mDrmBufOffset(0),
      mDrmBufSize(0),
      mDrmBuf(NULL){
    CHECK(offset >= 0);
    CHECK(length >= 0);

    CUUKUUK *ckkp;
    int ret_sz;
    char ho_file[128];

    mFd = dup(fd);
uuprintf("dupdupdup %d %d", fd, mFd);
uuprintf("dupdupdup %d %d", fd, mFd);
uuprintf("dupdupdup %d %d", fd, mFd);
uuprintf("ffssss %p", ff_ptr);
    ff_ptr = new CUUKUUK;
uuprintf("ffssss %p", ff_ptr);
    ckkp = (CUUKUUK *)ff_ptr;

uuprintf("ffssss %p", ckkp);
    __bb_dx_init_libavformat(&(ckkp->dx_fns), &ret_sz);
    ckkp->dx_dptr = new uint8_t[ret_sz];
    ckkp->dx_fns.fnp_dx_load(ckkp->dx_dptr, &(ckkp->nrm));

    sprintf(ho_file, "hofd://%d", mFd);
uuprintf("ffssss %s", ho_file);

    if( ckkp->dx_fns.fnp_dx_file_read(ckkp->dx_dptr, ho_file, &(ckkp->nrm)) ) {
		delete (CUUKUUK *)ff_ptr;
		ff_ptr = 0;
    } else {
		ff_ex1 = ckkp->nrm.fourcc_ex1;
		ff_ex2 = ckkp->nrm.fourcc_ex2;
    }

/*
if( ff_ex1 != __ITY_WAV__ )
	{
		ckkp->dx_fns.fnp_dx_file_colse(ckkp->dx_dptr, &(ckkp->nrm));

		delete (ckkp->dx_dptr); ckkp->dx_dptr = 0;
		delete (ff_ptr); ff_ptr = 0;
	}
*/
}

FFmpegSource::~FFmpegSource() {
    CUUKUUK *ckkp = (CUUKUUK *)ff_ptr;
uuprintf("Class del %d ------", mFd);
uuprintf("Class del %d ------", mFd);
uuprintf("Class del %d ------", mFd);
uuprintf("Class del %d ------", mFd);

	if( ff_ptr ) {
		if( ckkp->dx_dptr ) delete [] (uint8_t *)ckkp->dx_dptr;
		delete (CUUKUUK *)ff_ptr;
	}

    if (mFd >= 0) {
        close(mFd);
        mFd = -1;
    }

    if (mDrmBuf != NULL) {
        delete[] mDrmBuf;
        mDrmBuf = NULL;
    }

    if (mDecryptHandle != NULL) {
        // To release mDecryptHandle
        CHECK(mDrmManagerClient);
        mDrmManagerClient->closeDecryptSession(mDecryptHandle);
        mDecryptHandle = NULL;
    }

    if (mDrmManagerClient != NULL) {
        delete mDrmManagerClient;
        mDrmManagerClient = NULL;
    }
}

status_t FFmpegSource::initCheck() const {
    return mFd >= 0 ? OK : NO_INIT;
}

ssize_t FFmpegSource::readAt(off64_t offset, void *data, size_t size) {
    if (mFd < 0) {
        return NO_INIT;
    }
    CUUKUUK *ckkp;

    ckkp = (CUUKUUK *)ff_ptr;
uuprintf("-----_____ read fd:%d sz:%d", mFd, size);
    Mutex::Autolock autoLock(mLock);

	if( ckkp->dx_fns.fnp_dx_frame_read(ckkp->dx_dptr, &(ckkp->nrm), (NR_FRAME *)data) == 0 ) {
uuprintf("FFmpegSource track_no %d\n", ((NR_FRAME *)data)->track_no);
uuprintf("FFmpegSource data_size %d\n", ((NR_FRAME *)data)->data_size);
uuprintf("FFmpegSource data_time %f\n", (float)(((NR_FRAME *)data)->t_us_pos)/1000000);
		return ((NR_FRAME *)data)->data_size;
	}

	return -1;

}

status_t FFmpegSource::getSize(off64_t *size) {
    Mutex::Autolock autoLock(mLock);

    if (mFd < 0)
        return NO_INIT;

    if (mLength >= 0) {
        *size = mLength;
        return OK;
    }

    *size = lseek64(mFd, 0, SEEK_END);

    return OK;
}

sp<DecryptHandle> FFmpegSource::DrmInitialization() {
    if (mDrmManagerClient == NULL)
        mDrmManagerClient = new DrmManagerClient();

    if (mDrmManagerClient == NULL)
        return NULL;

    if (mDecryptHandle == NULL) {
        mDecryptHandle = mDrmManagerClient->openDecryptSession(
                mFd, mOffset, mLength);
    }

    if (mDecryptHandle == NULL) {
        delete mDrmManagerClient;
        mDrmManagerClient = NULL;
    }

    return mDecryptHandle;
}

void FFmpegSource::getDrmInfo(sp<DecryptHandle> &handle, DrmManagerClient **client) {
    handle = mDecryptHandle;

    *client = mDrmManagerClient;
}

ssize_t FFmpegSource::readAtDRM(off64_t offset, void *data, size_t size) {
    size_t DRM_CACHE_SIZE = 1024;
    if (mDrmBuf == NULL) {
        mDrmBuf = new unsigned char[DRM_CACHE_SIZE];
    }

    if (mDrmBuf != NULL && mDrmBufSize > 0 && (offset + mOffset) >= mDrmBufOffset
            && (offset + mOffset + size) <= (mDrmBufOffset + mDrmBufSize)) {
        /* Use buffered data */
        memcpy(data, (void*)(mDrmBuf+(offset+mOffset-mDrmBufOffset)), size);
        return size;
    } else if (size <= DRM_CACHE_SIZE) {
        /* Buffer new data */
        mDrmBufOffset =  offset + mOffset;
        mDrmBufSize = mDrmManagerClient->pread(mDecryptHandle, mDrmBuf,
                DRM_CACHE_SIZE, offset + mOffset);
        if (mDrmBufSize > 0) {
            int64_t dataRead = 0;
            dataRead = size > mDrmBufSize ? mDrmBufSize : size;
            memcpy(data, (void*)mDrmBuf, dataRead);
            return dataRead;
        } else {
            return mDrmBufSize;
        }
    } else {
        /* Too big chunk to cache. Call DRM directly */
        return mDrmManagerClient->pread(mDecryptHandle, data, size, offset + mOffset);
    }
}
}  // namespace android
