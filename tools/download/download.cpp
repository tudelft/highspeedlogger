#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>

#include <iostream>
#include <fstream>
#include <vector>

/********************  SETTINGS  ********************/
// SDIO Settings
#define BLOCK_SIZE              512ULL   /* SD card block size in Bytes (512 for a normal SD card) */

// Logger settings
#define RX_BUFFER_NUM_BLOCKS    20ULL /* 20 blocks * 512 = 10 KB RAM required per buffer*/


// SD Card size
#define SDCARD_SIZE	(8ULL*1024ULL*1024ULL*1024ULL)

/**************   END OF SETTINGS   *****************/




#define BUFF_SIZE  (BLOCK_SIZE*RX_BUFFER_NUM_BLOCKS)

void read_disk_raw(char* volume_name, uint64_t disk_size);

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
	printf("highspeedloggerbinaryparser <DUMPFILE>  : eg. foo.diskdump \n\n");

	if (argc >= 2)
	{

		if (argc == 2)
		{
			read_disk_raw(argv[1], SDCARD_SIZE);
			exit(0);
		}
	}
	else
	{

#ifdef WIN32
		read_disk_raw("\\\\.\\G:", SDCARD_SIZE);
	    // read_disk_raw("d:\\cessnaLogFull.dd", SDCARD_SIZE);
#else
		read_disk_raw("/dev/sdb", SDCARD_SIZE);
#endif

		exit(0);
	}

	exit(-1);
}


#if 0

#define WIN32_LEAN_AND_MEAN
#define BUFFER_SIZE 32

#include <windows.h>

void read_disk_raw(void)
{
	HANDLE hFile; 
	DWORD dwBytesRead = 0;
	char ReadBuffer[BUFFER_SIZE] = {0};

	hFile = CreateFile("\\\\.\\G:",GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	if (hFile == INVALID_HANDLE_VALUE) 
	{ 
		printf("Could not open file (error %d)\n", GetLastError());
		return; 
	}

	// Read one character less than the buffer size to save room for
	// the terminating NULL character.

	if( FALSE == ReadFile(hFile, ReadBuffer, BUFFER_SIZE-2, &dwBytesRead, NULL) )
	{
		printf("Could not read from file (error %d)\n", GetLastError());
		CloseHandle(hFile);
		return;
	}

	if (dwBytesRead > 0)
	{
		ReadBuffer[dwBytesRead+1]='\0'; // NULL character

		printf(TEXT("Text read from %s (%d bytes): \n"), dwBytesRead);
		printf("%s\n", ReadBuffer);
	}
	else
	{
		printf(TEXT("No data read from file %s\n"));
	}

	CloseHandle(hFile);

}

#endif


void scan_all_blocks(FILE* volume, uint64_t disk_size);

// Read from sector //
void read_disk_raw(char* volume_name, uint64_t disk_size)
{
    FILE *volume;
    int k = 0;
  
    volume = fopen(volume_name, "r+b");
    if(!volume)
    {
        printf("Cant open Drive '%s'\n", volume_name);
        return;
    }
    setbuf(volume, NULL);       // Disable buffering

#ifdef WIN32
	uint64_t sd = disk_size;
	sd /= 1024UL * 1024UL;
	printf("\nExpecting an SD-card of %lu MB.\n",sd);
#else
	long long sdcard_disk_size;
	ioctl(volume, BLKGETSIZE64, &sdcard_disk_size);
	sdcard_disk_size /= 1024ULL * 1024 ULL;
	printf("Found an SG card of %lld:",sdcard_disk_size);
#endif



	printf("\nSearching for logfiles in '%s': \n...\r",volume_name);

	scan_all_blocks(volume, disk_size);

    fclose(volume);
 }

void scan_all_blocks(FILE* volume, uint64_t disk_size)
{
    char buf[BUFF_SIZE] = {0};

	int log = -1;
	int cnt = 0;

	FILE* of = 0;

	uint64_t addr = 0;
	while (addr < disk_size)
	{
		if(_fseeki64(volume, addr, SEEK_SET) != 0)
		{
			printf("Cant move to sector\n");
			return;
		}
 
		// Read BLOCK
		size_t r = fread(buf, 1, BUFF_SIZE, volume);

		// Start of block must contain <CODE 1A 1B 1C>
		if (  ((unsigned char)buf[0] == 0x1a) && ((unsigned char)buf[1] == 0x1b) && ((unsigned char)buf[2] == 0x1c))
        {
			/////////////////////////////////////////////////
			// Start of new Logfile
            if ((unsigned char)buf[3] == 0xaa)
            {
				int tt = 0;
				int nr = 0;
				tt = (int) ((uint8_t)buf[6]);
				nr = (int)  ((uint8_t) buf[5] ) + ((uint16_t) buf[4] )*256;
				char filename[32];
				char logtype[16];
				log++;
				switch(tt)
				{
				case 0:
					sprintf(logtype, "SPI1");
					break;
				case 1:
					sprintf(logtype, "SPI2");
					break;
				case 2:
					sprintf(logtype, "UART2");
					break;
				case 3:
					sprintf(logtype, "UART3");
					break;
				default:
					sprintf(logtype, "UNKNOWN");
					break;
				}
				sprintf(filename, "sd_hs_log_%05d_%s.bin", log, logtype);
				if (of > 0)
				{
					fclose(of);
					of = 0;
				}
				of = fopen(filename, "w+b");
				fwrite(buf,1,BUFF_SIZE,of);
				if (log > 0)
					printf("%d x 10k\nLog %d [#%d]: type [%x=%s] addr: <%ld> ",cnt, log, nr, tt, logtype, addr/1024);
				else
	                printf("Log %d [#%d]: type [%x=%s] addr: <%ld> ",log, nr, tt, logtype, addr/1024);
				cnt = 0;
            }
			/////////////////////////////////////////////////
			// Continued Logfile
            else if ((unsigned char)buf[3] == 0xbb)
            {
				cnt++;
				if (of > 0)
					fwrite(buf,1,BUFF_SIZE,of);

            }
            else
            {
				if (of > 0)
				{
					fclose(of);
					of = 0;
				}
                printf("? (%x)",(unsigned char)buf[3]);
            }
        }
        else
        {
			if (of > 0)
			{
				fclose(of);
				of = 0;
			}
			if (cnt > 0)
			{
				printf("%d x 10k\nFound End <%ld>\n",cnt, addr/1024);
				cnt = 0;
				break;
				//addr = disk_size;
			}
			else
			{
				//printf("Cnt ==0\n");
			}
    		//printf(".");
        }

		int f = feof(volume);
		int e = ferror(volume);

		if (f!=0)
		{
			printf("%d x 10k\nEof <%ld>\n",cnt, addr/1024);
			cnt = 0;
			break;
		}

		addr += BUFF_SIZE;

		if (addr >= disk_size) {
			printf("End of disk: cnt=%d <%ld><%ld>\n",cnt,addr,disk_size);
		}
		
		//printf("\n %ld: %d %d %d ",addr, f, e, r);
	}
 
    // Print out what wiat in sector //
    // for(k=0;k<BUFFER_SIZE;k++)
    //    printf("%02x ", (unsigned char) buf[k]);
 
}

