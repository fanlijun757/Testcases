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


#include <utils/Log.h>
#include <utils/NativeHandle.h>

#include <binder/IMemory.h>
#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/BufferItemConsumer.h>

#include <unistd.h>

#include "SidebandNativeHandle.h"

using namespace android;

#define HALF_RES_X         960
#define HALF_RES_Y         540
#define ANIMATION_COUNT     60
#define X_OFFSET            (HALF_RES_X/ANIMATION_COUNT)
#define Y_OFFSET            (HALF_RES_Y/ANIMATION_COUNT)
#define X_WINDOW_SIZE       480
#define Y_WINDOW_SIZE       270
#define X_RATIO             ((((float)HALF_RES_X/X_WINDOW_SIZE))/ANIMATION_COUNT)
#define Y_RATIO             ((((float)HALF_RES_Y/Y_WINDOW_SIZE))/ANIMATION_COUNT)

static pthread_t thread;
static int bFinished = 0;

sp<Surface> mSurface;
sp<SurfaceComposerClient> mComposerClient;
sp<SurfaceControl> mSurfaceControl;
SidebandNativeHandle *sb_nativeHandle = NULL;
sp<NativeHandle> nativeHandle;

static void punchHole(
    sp<Surface> &mSurface,
    sp<NativeHandle> &nativeHandle)
{
    mSurface->setSidebandStream(nativeHandle);

    return;
}

int main(int argc, char** argv)
{
    if (argc != 1) {
        printf("usage: %s\n", argv[0]);
        exit(0);
    }

    mComposerClient = new SurfaceComposerClient;
    if (mComposerClient->initCheck()) {
        printf("%s:%i, fail to create mComposerClient\n", __FUNCTION__, __LINE__);
        return 1;
    }

    mSurfaceControl = mComposerClient->createSurface(
                String8("Test Surface"), 500, 500, PIXEL_FORMAT_RGBX_8888, 0);

    if ((mSurfaceControl == NULL) || (!mSurfaceControl->isValid())) {
        printf("%s:%i, fail to create SurfaceControl\n", __FUNCTION__, __LINE__);
        return 1;
    }
    
     printf("%s:%i, Qunidaye !!!!! n", __FUNCTION__, __LINE__);

	SurfaceComposerClient::Transaction t;
	t.setLayer(mSurfaceControl, 21000+1).show(mSurfaceControl).apply();	
	t.setPosition(mSurfaceControl, 1000, 200).setSize(mSurfaceControl, 500, 500).apply();

    mSurface = mSurfaceControl->getSurface();
    if (mSurface == NULL) {
        printf("%s:%i, fail to getSurface\n", __FUNCTION__, __LINE__);
        goto error_exit;
    }

    sb_nativeHandle = new SidebandNativeHandle(true);

    
    if (!sb_nativeHandle) {
        printf("%s:%i, fail to getSurface\n", __FUNCTION__, __LINE__);
        goto error_exit;
    }
    
    sb_nativeHandle->planeId = 1;
        
    nativeHandle = NativeHandle::create(sb_nativeHandle, false);

    /* first step:
        punch a hole and wait the position changes */
    punchHole(mSurface, nativeHandle);

    //for (i = 0; i < ANIMATION_COUNT; i++)
    {
	
		

    }

    /*sleep(3);

    x_off = y_off = 0;
    x_ratio = y_ratio = 1.0;
    for (i = 0; i < ANIMATION_COUNT; i++)
    {
        usleep(33000);
        x_off += X_OFFSET;
        y_off += Y_OFFSET;
        x_ratio += X_RATIO;
        y_ratio += Y_RATIO;
        SurfaceComposerClient::openGlobalTransaction();
        mSurfaceControl->setPosition(x_off, y_off);
        //mSurfaceControl->setSize(x_ratio*X_WINDOW_SIZE, y_ratio*Y_WINDOW_SIZE);
        mSurfaceControl->setMatrix(x_ratio, 0.0, 0.0, y_ratio);
        SurfaceComposerClient::closeGlobalTransaction();
    }*/

    sleep(300000);
    mSurface->setSidebandStream(NULL);

    goto normal_exit;

error_exit:
    printf("%s:%i, found error, exit...\n", __FUNCTION__, __LINE__);
normal_exit:
    if (thread){
        //delete this thread
        bFinished = 1;
        pthread_join(thread, NULL);
    }

    if (sb_nativeHandle)
        delete sb_nativeHandle;

    nativeHandle = NULL;
    sb_nativeHandle = NULL;
    mSurface = NULL;
    mSurfaceControl = NULL;
    mComposerClient = NULL;

    printf("%s:%i, return\n", __FUNCTION__, __LINE__);
    return -1;
}
