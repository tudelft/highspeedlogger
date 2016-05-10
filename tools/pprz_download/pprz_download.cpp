#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>

#include <iostream>
#include <fstream>
#include <vector>

/********************  SETTINGS  ********************/
// SDIO Settings
#define BLOCK_SIZE              512   /* SD card block size in Bytes (512 for a normal SD card) */

// Logger settings
#define RX_BUFFER_NUM_BLOCKS    20 /* 20 blocks * 512 = 10 KB RAM required per buffer*/
/**************   END OF SETTINGS   *****************/

#define BUFF_SIZE  (BLOCK_SIZE*RX_BUFFER_NUM_BLOCKS)

void read_disk_raw(char* volume_name, unsigned long disk_size);

inline void wordswap(int32_t *i)
{
	uint8_t *p = (uint8_t*) i;
	uint8_t tmp = p[0];
	p[0] = p[1];
	p[1] = tmp;
	tmp = p[2];
	p[2] = p[3];
	p[3] = tmp;
}

int main(int argc, char** argv)
{
    printf("PARSE DISK:\n----------\n");
	printf("highspeedloggerbinaryparser <DRIVE>     : eg. '/dev/sdb' or '\\\\.\\G:'  \n");
	printf("highspeedloggerbinaryparser <DUMPFILE>  : eg. foo.diskdump \n");

	if (argc >= 2)
	{

		if (argc == 2)
		{
			read_disk_raw(argv[1], 128UL*1024UL*1024UL);
			exit(0);
		}
	}
	else
	{

#ifdef WIN32
		read_disk_raw("\\\\.\\G:", 2UL*1024UL*1024UL);
	    // read_disk_raw("d:\\cessnaLogFull.dd", 128UL*1024UL*1024UL);
#else
		read_disk_raw("/dev/sdb", 128UL*1024UL*1024UL);
#endif

		exit(0);
	}

	exit(-1);
}


static inline uint32_t big_endian(uint8_t* buf)
{
	return ((uint32_t) buf[0])*256UL*256UL*256UL + ((uint32_t) buf[1])*256UL*256UL + ((uint32_t) buf[2])*256UL + ((uint32_t) buf[3]);
}


// Read from sector //
void read_disk_raw(char* volume_name, unsigned long disk_size)
{
    FILE *volume;
    int k = 0;
    uint8_t buf[BUFF_SIZE] = {0};
    uint8_t bufheader[BUFF_SIZE] = {0};
  
    volume = fopen(volume_name, "r+b");
    if(!volume)
    {
        printf("Cant open Drive '%s'\n", volume_name);
		fclose(volume);
        return;
    }
    setbuf(volume, NULL);       // Disable buffering

	int log = -1;
	int cnt = 0;

	FILE* of = 0;

	printf("\nSearching for logfiles in '%s': \n...\r",volume_name);


	unsigned long addr = 0;

	addr= 0x2000 * BLOCK_SIZE;

	//printf("%u: ", addr);
	if(fseek(volume, addr, SEEK_SET) != 0)
	{
		printf("Cant move to sector\n");
		fclose(volume);
		return;
	}
 
	size_t r = fread(bufheader, 1, BLOCK_SIZE, volume);
	if (r <= 0)
	{
		int f = feof(volume);
		int e = ferror(volume);

		printf("Failed to read (read: %d) feof %d, ferror %d \n", r, f, e);

		fclose(volume);
		return;
	}

	printf("%X : ", addr);
	for (int i=0;i<BLOCK_SIZE;i++)
	{
		unsigned char b = bufheader[i];
		printf("%X, ", b);
	}
	printf("\n");


	unsigned long log_size = 0;

	uint32_t addr_log_end = big_endian(bufheader+0);
	uint8_t nr_of_logs = bufheader[4];



	if (addr_log_end < 0x4000) {
		printf("No logs found: %d %08X\n", nr_of_logs, addr_log_end);
		fclose(volume);
		return;
	} else {
		printf("Found %d logs:\n", nr_of_logs );
	}

	for (uint8_t log=0; log < nr_of_logs; log++) {
		uint32_t addr_log_start = big_endian( bufheader + 5 + 12*log );
		uint32_t log_size = big_endian( bufheader + 5 + 4 + 12*log );
		printf("Log %04d,\t0x%08X\t0x%08X\n", log, addr_log_start, log_size);

		addr = addr_log_start * BLOCK_SIZE;

		printf("FSeek to %08X\n",addr);

		int err = 0;
		if(fseek(volume, addr, SEEK_SET) != 0)
		{
			printf("Cant move to sector\n");
			fclose(volume);
			return;
		}
		err = errno;

		char filename[32];
		sprintf(filename, "sd_log_%05d.bin", log);
		of = fopen(filename, "w+b");

		// Read logfile
		while (log_size > 0)
		{
			uint32_t readbytes = log_size;
			if (readbytes > BLOCK_SIZE)
				readbytes = BLOCK_SIZE;

			size_t rdb = readbytes;
			size_t r = fread(buf, 1, BLOCK_SIZE, volume);

			printf("Read %08X %08X %d %d %d %d\r",addr, log_size, readbytes, r, err, errno);

			readbytes = r;
			fwrite(buf,1,readbytes,of);

			log_size --;
		}

		fclose(of);
		of = 0;
	}



    fclose(volume);
 
    return;
}

