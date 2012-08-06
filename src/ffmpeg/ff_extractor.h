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

#ifndef FF_EXTRACTOR_H_

#define FF_EXTRACTOR_H_

#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/MediaSource.h>
#include <utils/Vector.h>
#include <utils/threads.h>
#include "ffmpeg.h"
#include "ffmpeg_b2_queue.h"

namespace android {

struct AMessage;
class DataSource;
class String8;

class FFExtractor : public MediaExtractor {
public:
    // Extractor assumes ownership of "source".
    FFExtractor(const sp<DataSource> &source);

    virtual size_t countTracks();
    virtual sp<MediaSource> getTrack(size_t index);
    virtual sp<MetaData> getTrackMetaData(size_t index, uint32_t flags);

    virtual sp<MetaData> getMetaData();

    _PKT_CLST_ aa_q;
    _PKT_CLST_ vv_q;
    int32_t vv_no, aa_no;
    int32_t vv_sz, aa_sz;
    Mutex aLock;
    int32_t interrupt;
    int32_t thread_mode;
    int32_t vv_enable, aa_enable;

protected:
    virtual ~FFExtractor();

private:
    struct Track {
        int32_t ff_track;
        sp<MetaData> mMeta;
        Track *next;
        bool skipTrack;
    };
    Track *mFirstTrack, *mLastTrack;
    int32_t track_sz;

    sp<DataSource> mDataSource;
    status_t mInitCheck;

    sp<MetaData> mMainMetaData;

    status_t init();
    pthread_t mThread;
    static void *ThreadWrapper(void *);
    status_t threadFunc();

    void track_check(int container);
    const char * GetMIMETypeForHandler(uint32_t handler);
    FFExtractor(const FFExtractor &);
    FFExtractor &operator=(const FFExtractor &);
};

bool SniffFFmpeg(
        const sp<DataSource> &source, String8 *mimeType, float *confidence,
        sp<AMessage> *);

}  // namespace android

#endif  // FF_EXTRACTOR_H_
