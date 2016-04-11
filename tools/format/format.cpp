#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

/********************  SETTINGS  ********************/
// SDIO Settings
#define BLOCK_SIZE              512   /* SD card block size in Bytes (512 for a normal SD card) */

// Logger settings
#define RX_BUFFER_NUM_BLOCKS    20 /* 20 blocks * 512 = 10 KB RAM required per buffer*/
/**************   END OF SETTINGS   *****************/

#define BUFF_SIZE  (BLOCK_SIZE*RX_BUFFER_NUM_BLOCKS)

const unsigned long disk_size = 128UL*1024UL*1024UL;

void format_disk_raw(char* volume_name)
{
    FILE *volume;
    int k = 0;
    char buf[BUFF_SIZE] = {0};
  
	int i;

	printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\nWe will block-format drive '%s' \nAre you absolutely sure? Then type 1324: ",volume_name);
	std::cin >> i;
	if (i != 1324)
	{
		printf("\n...\nAborting...\n");
		return;
	}


    volume = fopen(volume_name, "w+b");
    if(!volume)
    {
        printf("Cant open Drive '%s'\n", volume_name);
        return;
    }
    setbuf(volume, NULL);       // Disable buffering

	printf("\nFormatting entire SDCard '%s': \n...\r",volume_name);

	unsigned long addr = 0;
    // read what is in sector and put in buf //
	while (addr < disk_size)
	{
		printf(".");

		if(fseek(volume, addr, SEEK_SET) != 0)
		{
			printf("Cant move to sector\n");
			return;
		}
 
		fwrite(buf, 1, 1, volume);

		int f = feof(volume);
		int e = ferror(volume);

		if (f!=0)
		{
			printf("End of file system found at addr %X: \n",addr);
			break;
		}
		printf("Format succes until %ld: \n",addr);

		addr += BUFF_SIZE;
	}
 
    fclose(volume);
 
    return;

}

int main(int argc, char** argv)
{
	char outfilename[2048] = "";

    printf("FORMAT SDCARD:\n-------------\n");
	printf("highspeedloggerbinaryformat <DRIVE>     : eg. '/dev/sdb' or '\\\\.\\G:'  \n");

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

#ifdef WIN32
		format_disk_raw("\\\\.\\G:");
#else
		format_disk_raw("/dev/sdb");
#endif

		exit(0);
	}

	exit(-1);

}

