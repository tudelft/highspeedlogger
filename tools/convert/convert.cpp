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

char shift_buffer[64];

inline void wordswap_function(uint8_t *p)
{
	uint8_t tmp = p[0];
	p[0] = p[1];
	p[1] = tmp;
	tmp = p[2];
	p[2] = p[3];
	p[3] = tmp;
}

void parse_wordswap(char *buff)
{
	int offset = 64;

	// First swap 
	for (int j=offset;j<(BUFF_SIZE);j+= 4)
	{
		wordswap_function((uint8_t*) buff+j);
	}
}

void parse_update_shift_buffer(char* buff)
{
	for (int k=0; k<64;k++)
	{
		buff[k] = shift_buffer[k];
		shift_buffer[k] = buff[BUFF_SIZE-64+k];
	}
}

bool parse_spi_normal(char *buff, FILE* out, int shifted, bool test)
{
	int offset = 64;
	int *count = (int*) &buff[offset-shifted];	

	int validate = 0;
	bool first_column_is_a_counter = true;

	for (int j=offset;j<(BUFF_SIZE);j+= 64)
	{
		for (int col=0;col<16;col++)
		{
			int c = *count;

			// First column = counter
			if (col == 0)
			{
				// First Line
				if (j<=(offset+64))
				{
					first_column_is_a_counter = true;
					validate = c;
				}
				else
				{
					//printf("%d,",c);
					validate++;
					if (c!= validate)
					{
						first_column_is_a_counter = false;
					}
				}
			}

			if (!test)
				fprintf(out,"%d,", c);
			//fprintf(out,"%08X,", c);

			for (int k=0;k<4;k++)
			{
			//	fprintf(out,"%02x ",*p++);
			}
			// fprintf(out,", ");

			count ++;
		}
		if (!test)
			fprintf(out,"\n");
	}

	//if (!first_column_is_a_counter)
	//	fprintf(out,"============================================================================================");

	return first_column_is_a_counter;
}


void parse_uart_normal(char *buff, FILE* out)
{
	static int serial_parser = 0;
	int offset = 64;
	uint8_t* p = (uint8_t*) &buff[offset];		
	uint8_t values[64];
	int* v = (int*) &values[0];

	for (int j=offset;j<(BUFF_SIZE);j+= 2)
	{
		if (serial_parser == 0)
		{
			if (p[0] != 0x24)
			{
				serial_parser = -1;
				printf("!");
			}
		}
		if (serial_parser >= 65)
		{		
			serial_parser = 0;
			fprintf(out,"\n");
		}

		if ((serial_parser > 0) && (serial_parser <= 64))
		{
			values[serial_parser -1] = p[0];
		}

		//fprintf(txt,"%02X ", p[0]);
		p++;
		p++;
		serial_parser++;
		if (serial_parser == 65)
		{
			for (int k=0;k<16;k++)
			{
				fprintf(out,"%d,",v[k]);
			}
		}
	}
}


int main(int argc, char** argv)
{
	char outfilename[2048] = "";
	int shift_value = 0;

    printf("PARSE LOGFILE:\n-------------\n");
	printf("highspeedloggerbinaryparser <sd_hs_log_00001.bin> <outputfile.txt>   \n______________________________________________________________________\n");

	if (argc >= 2)
	{
		if (argc > 2)
		{
			sprintf(outfilename, "%s", argv[2]);
		}
		else
		{
			sprintf(outfilename, "%s.csv", argv[1]);
		}
	}
	else
	{
		exit(0);
	}

	char buff[BUFF_SIZE+64];
	// Open the stream
	printf("\nOpening input file: '%s'\n",argv[1]);
	std::ifstream is(argv[1], std::ifstream::in | std::ifstream::binary);

	/*
	// Determine the file length
	is.seekg(0, std::ifstream::end);
	std::size_t size=is.tellg();
	is.seekg(0, std::ifstream::beg);
	// Create a vector to store the data
	std::vector<int> v(size/sizeof(int));
	// Load the data
	*/

    printf("Opening output file: '%s'\n",outfilename);
	FILE* txt = fopen(outfilename,"w");
	int logtype = 0;

	for (;;)
	{
		is.read( &buff[0], BUFF_SIZE);
		if (!is.good()) break;

		if (  ((unsigned char)buff[0] == 0x1a) && ((unsigned char)buff[1] == 0x1b) && ((unsigned char)buff[2] == 0x1c))
        {
            if ((unsigned char)buff[3] == 0xaa)
            {
				fprintf(txt, "New Log: ");
				for (int i=4;i<6;i++)
					fprintf(txt, "%02x ", (unsigned char) buff[i]);
				fprintf(txt, "\n");

				logtype = (int) ((uint8_t) buff[6]);

                printf("H[%02x]",logtype);
            }
            else if ((unsigned char)buff[3] == 0xbb)
            {
				// fprintf(txt,"-------- \n");
                printf("D");
			}
            else
            {
                printf("? (%x)",(unsigned char)buff[3]);
            }

			switch(logtype)
			{
				// SPI
			case 0:
			case 1:
				{
					static int cnt = 0;
					// first correct byte order due to saving
					parse_wordswap(buff);

					// then import/remember old data when data is shifted
					parse_update_shift_buffer(buff);

					// Finally parse
					bool good = parse_spi_normal(buff,txt, shift_value, false);

					if (! good)
					{
						printf("<nosync>");

						for (int f=64-11;f<64-10;f++)
						{
							good = parse_spi_normal(buff,stdout, f, true);
							if (good)
							{
								printf("Sync set to: %d, ",f);
								shift_value = f;
								break;
							}
						}

					}
					// return -1;

					/*
					cnt++;
					if (cnt > 5)
					{
						is.close();
						fclose(txt);
						return 0;
					}
					*/
				}
				break;
			case 2:
			case 3:
				// UART
				parse_uart_normal(buff,txt);
				break;
			default:
				break;
			}

        }
        else
        {
    		printf(".");
        }
	}

	is.close();

	fclose(txt);

}

