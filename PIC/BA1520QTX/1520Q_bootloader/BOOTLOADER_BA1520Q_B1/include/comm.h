/*
 * comm.h
 *
 *  Created on: Mar 23, 2017
 *      Author: asaf
 */

#ifndef COMM_H_
#define COMM_H_

#include <stdint.h>
#include <stdbool.h>

#define MSG_MAX_MESSAGE_LEN 80

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define MAX_INDEX_IN_ARRAY(x) (ARRAY_SIZE(x)-1)

void comm_init();
bool comm_receive_message(void* buffer, uint16_t* size);
void comm_send_message(void* buffer, uint16_t size);



#endif /* COMM_H_ */
