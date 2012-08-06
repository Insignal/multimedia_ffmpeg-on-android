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

#ifndef FFMPEG_SOURCE_H_

#define FFMPEG_SOURCE_H_

#include <stdio.h>

#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaErrors.h>
#include <utils/threads.h>
#include <drm/DrmManagerClient.h>

namespace android {

class FFmpegSource : public DataSource {
public:
    FFmpegSource(const char *filename);
    FFmpegSource(int fd, int64_t offset, int64_t length);

    virtual status_t initCheck() const;

    virtual ssize_t readAt(off64_t offset, void *data, size_t size);

    virtual status_t getSize(off64_t *size);

    virtual sp<DecryptHandle> DrmInitialization();

    virtual void getDrmInfo(sp<DecryptHandle> &handle, DrmManagerClient **client);

protected:
    virtual ~FFmpegSource();

private:
    int mFd;
    int64_t mOffset;
    int64_t mLength;
    Mutex mLock;

    /*for DRM*/
    sp<DecryptHandle> mDecryptHandle;
    DrmManagerClient *mDrmManagerClient;
    int64_t mDrmBufOffset;
    int64_t mDrmBufSize;
    unsigned char *mDrmBuf;

    ssize_t readAtDRM(off64_t offset, void *data, size_t size);

    FFmpegSource(const FFmpegSource &);
    FFmpegSource &operator=(const FFmpegSource &);
};

}  // namespace android

#endif  // FFMPEG_SOURCE_H_
