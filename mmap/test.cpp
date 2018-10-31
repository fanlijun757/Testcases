#include <cutils/memory.h>
#include <utils/Log.h>

#include <sys/mman.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;
	void *ret;
  
    printf("pid=%d\n", getpid());
    ret = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    memset(ret, 127, 4096);
    
    printf("mmap addr=%p\n", ret);
    ret = malloc(4096);
    printf("malloc addr=%p\n", ret);
    
    
    sleep(100);
    
    return 0;
}
