/*
 * rfc1662.c
 *
 *  Created on: 09/01/2011
 *      Author: asafe
 */

#include "BA1474.h"
#include "rfc1662.h"
#include "crc8.h"

static int replaceFlagAndEscape(unsigned char *data, unsigned char *dest, int datalen)
{
	int datai=0;
	int desti=0;
	for(datai=0;datai<datalen;datai++)
	{
		if(data[datai] == CHAR_FLAG)
		{
			dest[desti++] = CHAR_ESCAPE;
			dest[desti++] = CHAR_FLAG_XORED;
		}
		else if(data[datai] == CHAR_ESCAPE)
		{
			dest[desti++] = CHAR_ESCAPE;
			dest[desti++] = CHAR_ESCAPE_XORED;
		}
		else
		{
			dest[desti++] = data[datai];
		}
	}
	return desti; //return the size of the new data string
}

static int restoreFlagAndEscape(unsigned char *data, unsigned char *dest, int datalen)
{
	int datai=0;
	int desti=0;
	for(datai=0;datai<datalen;datai++)
	{
		if(data[datai] == CHAR_ESCAPE)
		{
			datai++;
			if(data[datai] == CHAR_FLAG_XORED)
			{
				dest[desti++] = CHAR_FLAG;
			}
			else if(data[datai] == CHAR_ESCAPE_XORED)
			{
				dest[desti++] = CHAR_ESCAPE;
			}
			else
			{
				//count this as an error on the stream!
			}
		}
		else
		{
			dest[desti++] = data[datai];
		}
	}
	return desti; //return the size of the new data string
}

//this function encapsulate the data with all the information needed
//to be sent via the rfc1662 protocol (PPP with HDLC like framing)
//this function should be called for sending data through the rs422 link
//dataLen should be the datalength only!

int rfc1662_pack(unsigned char *data, int dataLen, unsigned char *dest)
{
	int changed_data_size = 0;
	int changed_crc_size = 0;
	//calc crc and add it to the end of the data
	uint8_t crc = crc8(data, dataLen);
	//replace escape and flag
	changed_data_size = replaceFlagAndEscape(data, &dest[1], dataLen);
	changed_crc_size = replaceFlagAndEscape(&crc, &dest[changed_data_size+1], sizeof(crc));
	//add the flags
	dest[0] = CHAR_FLAG;
	dest[changed_data_size+changed_crc_size+1] = CHAR_FLAG;

	// returns the size of dest
	return changed_data_size+changed_crc_size+2;
}


int rfc1662_unpack(unsigned char *data, int len, unsigned char *dest)
{
	int restored_data_size = 0;
	//the received data is received without the start and end FLAGS
	restored_data_size = restoreFlagAndEscape(data, dest, len);

	if (restored_data_size < 1)
		return -1;

	uint8_t calculated_crc = crc8(dest, restored_data_size - 1);
	uint8_t message_crc = dest[restored_data_size - 1];
	if (calculated_crc == message_crc) {
		//crc is correct
		//return the size of the new data
		return restored_data_size-1; //we get dsti+1 at the end and the 1 crc
	}
	
	return -1;
}
