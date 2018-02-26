#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <time.h>
#include <math.h>

#ifndef WIN32
#define _FILE_OFFSET_BITS 64
#define MYSEEK64 fseek
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#undef BLOCK_SIZE
#else
#define MYSEEK64 _fseeki64
#endif

/********************  SETTINGS  ********************/
// SDIO Settings
#define BLOCK_SIZE              512ULL   /* SD card block size in Bytes (512 for a normal SD card) */

// Logger settings
#define RX_BUFFER_NUM_BLOCKS    20ULL /* 20 blocks * 512 = 10 KB RAM required per buffer*/
/**************   END OF SETTINGS   *****************/

#define BUFF_SIZE  (BLOCK_SIZE*RX_BUFFER_NUM_BLOCKS)

// max card size supported is 8MB
uint64_t disk_size = 8ULL*1024ULL*1024ULL*1024ULL;

void format_disk_raw(char* volume_name)
{
    FILE *volume;
    int k = 0;
    char buf[BUFF_SIZE] = {0, 0, 0, 0, 0};
  
	int i;

	printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\nWe will block-format drive '%s' \nAre you absolutely sure? Then type 1324: ",volume_name);
	std::cin >> i;
	if (i != 1324)
	{
		printf("\n...\nAborting...\n");
		return;
	}

#ifdef WIN32
	uint64_t sd = disk_size;
	sd /= 1024ULL * 1024ULL;
	printf("\nExpecting an SD-card of %llu MB.\n",sd);
#else
	int fd = open(volume_name, O_RDONLY);

	uint64_t sdcard_disk_size;
	ioctl(fd, BLKGETSIZE64, &sdcard_disk_size);
	close(fd);
	uint64_t sd = sdcard_disk_size / 1024ULL / 1024ULL;
	
	if (sd != 0)
	{
		disk_size = sdcard_disk_size;
		printf("Found an SG card of %llu MB %llu\n",sd, disk_size);
	} else {
		sd = disk_size / (1024ULL * 1024ULL);
		printf("\nExpecting an SD-card of %llu MB.\n",sd);
	}
#endif

    volume = fopen(volume_name, "w+b");
    if(!volume)
    {
        printf("Cant open Drive '%s'\n", volume_name);
        return;
    }
    setbuf(volume, NULL);       // Disable buffering

	printf("\nFormatting entire SDCard '%s': \n...\r",volume_name);
	sleep(3);

	uint64_t addr = 0;

	addr= 0x2000 * BLOCK_SIZE;

	if(MYSEEK64(volume, addr, SEEK_SET) != 0)
	{
		printf("Can't move to sector\n");
		fclose(volume);
		return;
	}

	fwrite(buf, 5, 1, volume);

	addr = 0;
    // read what is in sector and put in buf //

	time_t time_start = time(0);
	float remaining = 0;
	while (addr < disk_size)
	{
		printf(".");

		if(MYSEEK64(volume, addr, SEEK_SET) != 0)
		{
			printf("Can't move to sector\n");
			break;
		}
 
		fwrite(buf, 1, 1, volume);

		int f = feof(volume);
		int e = ferror(volume);

		if (addr > 0)
			remaining = (time(0) - time_start) * (disk_size / addr - 1);

		if (f!=0)
		{
			printf("End of file system found at addr %lluX: \n",addr);
			break;
		}
		printf("Format success until %llu (%lluMB), Done: %llu%%, Time left: %02.0f:%02.0f:%02.0f\r", addr, addr/1000000, 100*addr/disk_size, remaining / 3600, fmod(remaining, 3600) / 60, fmod(remaining, 60.));

		addr += BUFF_SIZE;
	}
 
    fclose(volume);
 
    return;
}

int main(int argc, char** argv)
{
	char outfilename[2048] = "";

    printf("FORMAT SDCARD:\n-------------\n");

	if (argc >= 2)
	{
		if (argc == 2)
		{
			format_disk_raw(argv[1]);
			exit(0);
		}
	}
	else
	{
		printf("Please provide the disk drive you would like to format.\n");
		printf("highspeedloggerbinaryformat <DRIVE>     : eg. '/dev/sdb' or '\\\\.\\G:'  \n");
		exit(0);
	}

	exit(-1);

}

