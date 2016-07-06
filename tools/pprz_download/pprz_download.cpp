#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <stdint.h>

#include <iostream>
#include <fstream>
#include <vector>

/********************  SETTINGS  ********************/
// SDIO Settings
#define BLOCK_SIZE              512   /* SD card block size in Bytes (512 for a normal SD card) */

/**************   END OF SETTINGS   *****************/

#define BUFF_SIZE  (BLOCK_SIZE)

void read_disk_raw(char* volume_name, unsigned long disk_size);

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

#ifndef TRANSPORT_PAYLOAD_LEN
#define TRANSPORT_PAYLOAD_LEN 256
#endif

/** Generic reception transport header
 */
struct transport_rx {
  uint8_t payload[TRANSPORT_PAYLOAD_LEN]; ///< payload buffer
  volatile uint8_t payload_len;           ///< payload buffer length
  volatile bool msg_received;             ///< message received flag
  uint8_t ovrn, error;                    ///< overrun and error flags
};

/** Data type
 */
enum TransportDataType {
  DL_TYPE_ARRAY_LENGTH,
  DL_TYPE_CHAR,
  DL_TYPE_UINT8,
  DL_TYPE_INT8,
  DL_TYPE_UINT16,
  DL_TYPE_INT16,
  DL_TYPE_UINT32,
  DL_TYPE_INT32,
  DL_TYPE_UINT64,
  DL_TYPE_INT64,
  DL_TYPE_FLOAT,
  DL_TYPE_DOUBLE,
  DL_TYPE_TIMESTAMP
};

/** Data format (scalar or array)
 */
enum TransportDataFormat {
  DL_FORMAT_SCALAR,
  DL_FORMAT_ARRAY
};


/** Generic transmission transport header
 */
struct transport_tx {
  void *impl;                                     ///< pointer to parent implementation
};

struct pprz_transport {
  // generic reception interface
  struct transport_rx trans_rx;
  // specific pprz transport_rx variables
  uint8_t status;
  uint8_t payload_idx;
  uint8_t ck_a_rx, ck_b_rx;
  // generic transmission interface
  struct transport_tx trans_tx;
  // specific pprz transport_tx variables
  uint8_t ck_a_tx, ck_b_tx;
};



// Parsing function, only needed for modules doing their own parsing
// without using the pprz_check_and_parse function
extern bool parse_pprz(struct pprz_transport *t, uint8_t c);

// PPRZ parsing state machine
#define UNINIT      0
#define GOT_STX     1
#define GOT_LENGTH  2
#define GOT_PAYLOAD 3
#define GOT_CRC1    4

// Start byte
#define PPRZ_STX  0x99
	
// Parsing function
bool parse_pprz(struct pprz_transport *t, uint8_t c)
{
  switch (t->status) {
    case UNINIT:
      if (c == PPRZ_STX) {
        t->status++;
      }
      break;
    case GOT_STX:
      if (t->trans_rx.msg_received) {
        t->trans_rx.ovrn++;
        goto error;
      }
      t->trans_rx.payload_len = c - 4; /* Counting STX, LENGTH and CRC1 and CRC2 */
      t->ck_a_rx = t->ck_b_rx = c;
      t->status++;
      t->payload_idx = 0;
      break;
    case GOT_LENGTH:
      t->trans_rx.payload[t->payload_idx] = c;
      t->ck_a_rx += c; t->ck_b_rx += t->ck_a_rx;
      t->payload_idx++;
      if (t->payload_idx == t->trans_rx.payload_len) {
        t->status++;
      }
      break;
    case GOT_PAYLOAD:
      if (c != t->ck_a_rx) {
        goto error;
      }
      t->status++;
      break;
    case GOT_CRC1:
      if (c != t->ck_b_rx) {
        goto error;
      }
      t->trans_rx.msg_received = true;
      goto restart;
    default:
      goto error;
  }
  return true;
error:
  t->trans_rx.error++;
restart:
  t->status = UNINIT;
  return false;
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

	size_t r = 0;
	int cnti = 0;
	do {
		r = fread(bufheader, 1, BLOCK_SIZE, volume);
		printf("Trying to read .... \r");
		cnti++;
	} while ((cnti<20000) && (r <=0));

	if (r <= 0)
	{
		int f = feof(volume);
		int e = ferror(volume);

		printf("\nFailed to read (read: %lu) feof %d, ferror %d \n", r, f, e);

		fclose(volume);
		return;
	}

	printf("HEADER DATA @ %08lX : ", addr);
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
		printf("No logs found: %d %04X\n", nr_of_logs, addr_log_end);
		fclose(volume);
		return;
	} else {
		printf("Found %d logs:\n", nr_of_logs );
	}

	for (uint8_t log=0; log < nr_of_logs; log++) {
		uint32_t addr_log_start = big_endian( bufheader + 5 + 12*log );
		uint32_t log_size = big_endian( bufheader + 5 + 4 + 12*log );
		printf("Log %04d,\t0x%04X\t0x%04X\t0x%04X\n", log, addr_log_start, log_size, (addr_log_start+log_size));

		addr = addr_log_start * BLOCK_SIZE;

		printf("FSeek to %08lX\n",addr);

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

			printf("Read %08lX %04X %d %lu %d %d\r", addr, log_size, readbytes, r, err, errno);

			readbytes = r;
			fwrite(buf,1,readbytes,of);

			log_size --;
		}


		fclose(of);
		of = 0;
	}

	// After the last logfile: keep reading untill bad pprzlink parsing fails
	bool pprz_valid = true;
	struct pprz_transport pprz;

	char filename[32];
	sprintf(filename, "sd_log_extra_unclosed.bin", log);
	of = fopen(filename, "w+b");

	while (pprz_valid)
	{
		size_t r = fread(buf, 1, BLOCK_SIZE, volume);
		for (int i=0; pprz_valid && (i<BLOCK_SIZE); i++)
		{
			pprz_valid = parse_pprz(&pprz, buf[i]);
		}
		uint32_t readbytes = r;
		fwrite(buf,1,readbytes,of);

	}
	fclose(of);

    fclose(volume);
 
    return;
}

