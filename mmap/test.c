#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
int main()
{
	int fd;
	char *mem_start;
	fd = open("/dev/mmap_dev", O_RDWR);
	if( ( mem_start = mmap( NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0))
			== MAP_FAILED)
	{
		printf("mmap failed.\n");
		exit(0);
	}
	printf("mem:%s\n", mem_start);
	return 0;
}
