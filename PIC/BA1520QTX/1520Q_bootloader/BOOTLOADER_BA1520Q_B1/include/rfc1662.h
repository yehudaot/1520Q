/*
 * rfc1662.h
 *
 *  Created on: 09/01/2011
 *      Author: asafe
 */

#ifndef __RFC1662_H__
#define __RFC1662_H__

#define CHAR_FLAG 0x7E
#define CHAR_FLAG_XORED 0x5E
#define CHAR_ESCAPE 0x7D
#define CHAR_ESCAPE_XORED 0x5D

int rfc1662_unpack(unsigned char *data, int len, unsigned char *dest);
int rfc1662_pack(unsigned char *data, int dataLen, unsigned char *dest);

#endif /* __RFC1662_H__ */
