#include <cutils/memory.h>
#include <utils/Log.h>

#include <sys/mman.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <ion_4.12.h>
#define ION_FLAG_CACHED 1 
#include <ion/ion.h>

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;
	int ionMemFD;
  
  	int ionDeviceFD = ion_open();
  	
  	printf("The ionDeviceFD =%d\n", ionDeviceFD);
  	
  	const int iPageSize = sysconf(_SC_PAGESIZE);
  	  	
  	printf("The page size =%d\n", iPageSize);
  	
  	int err = ion_alloc_fd(ionDeviceFD, iPageSize*10000, iPageSize, 4, ION_FLAG_CACHED, &ionMemFD);
  	
  	printf("The ionMemFD =%d err=%d\n", ionMemFD, err);
  	
  	void * addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, ionMemFD, 0);
  	
  	printf("The addr =%p\n", addr);
    
    
    sleep(100);
    
    return 0;
}
