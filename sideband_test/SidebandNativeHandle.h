/*
 * Copyright (C) 2018 Synaptics Incorporated. All rights reserved.
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


#ifndef ANDROID_SIDEBAND_NATIVE_HANDLE_H
#define ANDROID_SIDEBAND_NATIVE_HANDLE_H

#include <pthread.h>

#include <utils/Errors.h>
#include <cutils/native_handle.h>


// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------
struct SidebandNativeHandle : public native_handle {

public:
    /* ints */
    int     magic;          /* for handle validation */
    int     clientTid;      /* for debug usage */
    int     index;          /* for debug usage */
    int     planeId;        /* video plane id will be used by this sideband stream */

    const native_handle *nativeHandle;

    static const int sNumFds = 0;
    static const int sNumInts = 4;
    static const int sMagic = 0x53424e48; /*SBNH*/

    enum {
        SIDEBAND_VIDEO_PLANE_MAIN = 0x0,
        SIDEBAND_VIDEO_PLANE_PIP = 0x1,
    };

    SidebandNativeHandle(bool ownsFd = true);

    SidebandNativeHandle(const native_handle *handle, bool ownsFd = false);

    ~SidebandNativeHandle();

    static int validate(const native_handle* h)
    {
        const SidebandNativeHandle* hnd = (const SidebandNativeHandle*)h;
        if (!hnd || hnd->version != sizeof(native_handle) ||
                hnd->numInts != sNumInts || hnd->numFds != sNumFds ||
                hnd->magic != sMagic)
        {
            ALOGW("invalid sideband handle (at %p)", hnd);
            return -EINVAL;
        }
        return 0;
    }

    /*server use, set the video plane id to shared memory region */
    status_t setVideoPlaneId(int id);
};

// ----------------------------------------------------------------------------
}; // namespace android
#endif // ANDROID_SIDEBAND_NATIVE_HANDLE_H

