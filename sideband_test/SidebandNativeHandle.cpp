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


#define LOG_TAG "SideBandNativeHandle"

#include <sys/mman.h>
#include <cutils/ashmem.h>
#include <utils/Log.h>
#include <string.h>
#include "SidebandNativeHandle.h"

static int global_index = 0;

namespace android {

SidebandNativeHandle::SidebandNativeHandle(bool inOwnsFd)
{
    ALOGV("%s:%i, create SidebandNativeHandle %p at client side,  inOwnsFd=%d",
            __FUNCTION__, __LINE__, this, inOwnsFd);

    // Set planeid with default value, use setVideoPlaneId() to update it.
    planeId = SIDEBAND_VIDEO_PLANE_MAIN;
    /* follow the native_handle's version */
    version = sizeof(native_handle);

    /*set numInts and numFds*/
    numInts = sNumInts;
    numFds = sNumFds;

    magic = sMagic;

    nativeHandle = NULL;
    clientTid = gettid();
    global_index++;

    ALOGV("%s:%i finish, this=%p, ID=(%d:%d), planeId:%d", __FUNCTION__,
          __LINE__, this,  clientTid, index, planeId);
}

SidebandNativeHandle::SidebandNativeHandle(const native_handle *handle, bool inOwnsFd)
{
    ALOGV("%s:%i, create SidebandNativeHandle %p at server side, handle=%p, inOwnsFd=%d",
         __FUNCTION__, __LINE__, this, handle, inOwnsFd);

    /* follow the native_handle's version */
    version = sizeof(native_handle);

    if ((handle) && !validate(handle)) {
        /*set numInts and numFds*/
        numInts = handle->numInts;
        numFds = handle->numFds;

        nativeHandle = handle;

        memcpy(&magic, handle->data+numFds, sizeof(int)*numInts);
    }
    else {
        ALOGE("%s:%i, this=%p, invalid native_handle %p to create sideband native handle",
              __FUNCTION__, __LINE__, this, handle);
        numInts = sNumInts;
        numFds = sNumFds;
        magic = sMagic;
        clientTid = index = -1;
        nativeHandle = NULL;
    }

    return;
}

SidebandNativeHandle::~SidebandNativeHandle()
{
}

status_t SidebandNativeHandle::setVideoPlaneId(int id)
{
    planeId = id;

    return NO_ERROR;
}
};  //namespace android
