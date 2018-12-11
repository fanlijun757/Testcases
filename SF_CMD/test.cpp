#include <cutils/memory.h>
#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <ui/GraphicBufferMapper.h>
#include <sys/mman.h>
#include <gui/ISurfaceComposer.h>
#include <gui/ISurfaceComposerClient.h>

using namespace android;
sp<IBinder> composer = NULL;

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;
	if(argc != 2) {
		printf("Please run as \"sf_cmd 1\" or \"sf_cmd 0\"\n");
		return 0;
	}
	int enable = !strcmp(argv[1], "1");
	
	const String16 name("SurfaceFlinger");
	composer = defaultServiceManager()->getService(name);

    assert(composer != NULL);    
    
    Parcel data;
    data.writeInterfaceToken(String16("android.ui.ISurfaceComposer"));
    if(enable) {
    	data.writeInt32(1);
    	printf("Enable HDR patch\n");
    } else {
    	data.writeInt32(0);
    	printf("Disable HDR patch\n");
    }
    composer->transact(2001, data, NULL, 0);
    
    return 0;
}
