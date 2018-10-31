#include <cutils/memory.h>
#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <ui/GraphicBufferMapper.h>
#include<sys/mman.h>
//#include </home/flj/O-Android/Android/frameworks/native/libs/nativewindow/include/android/native_window.h>

using namespace android;

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;
    // set up the thread-pool
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    // create a client to surfaceflinger
    sp<SurfaceComposerClient> client = new SurfaceComposerClient();
    
    sp<SurfaceControl> surfaceControl_p = client->createSurface(String8("red_layer_p"), 1920, 1080, 0x1, 0);
    
    SurfaceComposerClient::Transaction().setLayer(surfaceControl_p, 31005).apply();
	SurfaceComposerClient::Transaction().setPosition(surfaceControl_p,000.0f, 212.0f).apply();
	SurfaceComposerClient::Transaction().show(surfaceControl_p).apply();

    
    sp<Surface> surface = surfaceControl_p->getSurface();

    ANativeWindow* window = surface.get();
    
	native_window_api_connect(window, NATIVE_WINDOW_API_CPU);
    //native_window_set_buffers_transform(window, NATIVE_WINDOW_TRANSFORM_ROT_90);

	//int32_t err = native_window_set_buffers_sticky_transform(window, NATIVE_WINDOW_TRANSFORM_ROT_90);
	//
    //android_native_rect_t r={0,0,100,100};
	//native_window_set_buffers_user_dimensions(window, 3840, 2160);
	//native_window_set_scaling_mode(window, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
	//native_window_set_crop(window, &r);
	//native_window_set_usage(window, 0x00400000|0x10000000|0x00000030);
	//native_window_set_buffers_format(window, 0);
	//if (err)
	//{
//		ALOGE("huataol error");
//	}

    ANativeWindowBuffer * outBuffer;

    while(1)
    {
		char* vaddr;
		int status = window->dequeueBuffer_DEPRECATED(window, &outBuffer);
		GraphicBufferMapper::get().lock(outBuffer->handle, 0x00000030, Rect(0, 0, 100, 100), (void**)&vaddr);
			
		for(int i=0; i< 1080; i++) {
			for(int j=0; j<1920; j++) {
				vaddr[i*1920*4 + j*4 + 0] = 255;
				vaddr[i*1920*4 + j*4 + 1] = 0;
				vaddr[i*1920*4 + j*4 + 2] = 0;
				vaddr[i*1920*4 + j*4 + 3] = 255;
			}
		}
		GraphicBufferMapper::get().unlock(outBuffer->handle);
		printf("##### After set memory\n");
		status = window->queueBuffer_DEPRECATED(window, outBuffer);
  }

/*
     surface->lock(&outBuffer, NULL);
    
	//outBuffer.format = WINDOW_FORMAT_RGBA_8888;
    ALOGE("the format is %d", outBuffer.format);
    bpr = outBuffer.stride * bytesPerPixel(outBuffer.format);
    android_memset16((uint16_t*)outBuffer.bits, 0x0F10, bpr*outBuffer.height);
    surface->unlockAndPost();
  */  

    
    IPCThreadState::self()->joinThreadPool();
    
    return 0;
}
