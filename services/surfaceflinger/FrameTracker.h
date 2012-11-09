/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef ANDROID_FRAMETRACKER_H
#define ANDROID_FRAMETRACKER_H

#include <stddef.h>

#include <utils/Timers.h>
#include <utils/RefBase.h>

namespace android {

class String8;
class Fence;

// FrameTracker tracks information about the most recently rendered frames. It
// uses a circular buffer of frame records, and is *NOT* thread-safe -
// mutexing must be done at a higher level if multi-threaded access is
// possible.
//
// Some of the time values tracked may be set either as a specific timestamp
// or a fence.  When a non-NULL fence is set for a given time value, the
// signal time of that fence is used instead of the timestamp.
class FrameTracker {

public:
    // NUM_FRAME_RECORDS is the size of the circular buffer used to track the
    // frame time history.
    enum { NUM_FRAME_RECORDS = 128 };

    FrameTracker();

    // setDesiredPresentTime sets the time at which the current frame
    // should be presented to the user under ideal (i.e. zero latency)
    // conditions.
    void setDesiredPresentTime(nsecs_t desiredPresentTime);

    // setFrameReadyTime sets the time at which the current frame became ready
    // to be presented to the user.  For example, if the frame contents is
    // being written to memory by some asynchronous hardware, this would be
    // the time at which those writes completed.
    void setFrameReadyTime(nsecs_t readyTime);

    // setFrameReadyFence sets the fence that is used to get the time at which
    // the current frame became ready to be presented to the user.
    void setFrameReadyFence(const sp<Fence>& readyFence);

    // setActualPresentTime sets the timestamp at which the current frame became
    // visible to the user.
    void setActualPresentTime(nsecs_t displayTime);

    // setActualPresentFence sets the fence that is used to get the time
    // at which the current frame became visible to the user.
    void setActualPresentFence(const sp<Fence>& fence);

    // advanceFrame advances the frame tracker to the next frame.
    void advanceFrame();

    // clear resets all the tracked frame data to zero.
    void clear();

    // dump appends the current frame display time history to the result string.
    void dump(String8& result) const;

private:
    struct FrameRecord {
        FrameRecord() :
            desiredPresentTime(0),
            frameReadyTime(0),
            actualPresentTime(0) {}
        nsecs_t desiredPresentTime;
        nsecs_t frameReadyTime;
        nsecs_t actualPresentTime;
        sp<Fence> frameReadyFence;
        sp<Fence> actualPresentFence;
    };

    // processFences iterates over all the frame records that have a fence set
    // and replaces that fence with a timestamp if the fence has signaled.  If
    // the fence is not signaled the record's displayTime is set to INT64_MAX.
    //
    // This method is const because although it modifies the frame records it
    // does so in such a way that the information represented should not
    // change.  This allows it to be called from the dump method.
    void processFences() const;

    // mFrameRecords is the circular buffer storing the tracked data for each
    // frame.
    FrameRecord mFrameRecords[NUM_FRAME_RECORDS];

    // mOffset is the offset into mFrameRecords of the current frame.
    size_t mOffset;

    // mNumFences is the total number of fences set in the frame records.  It
    // is incremented each time a fence is added and decremented each time a
    // signaled fence is removed in processFences or if advanceFrame clobbers
    // a fence.
    //
    // The number of fences is tracked so that the run time of processFences
    // doesn't grow with NUM_FRAME_RECORDS.
    int mNumFences;
};

}

#endif // ANDROID_FRAMETRACKER_H
