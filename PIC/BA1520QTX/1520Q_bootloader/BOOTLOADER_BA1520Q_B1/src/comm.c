/*
 * comm.c
 *
 *  Created on: Mar 23, 2017
 *      Author: asaf
 */

#include "BA1474.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "comm.h"
#include "uart.h"
#include "message.h"
#include "rfc1662.h"
#include "timer.h"
#include "message_handlers.h"

static uint8_t rfcBuff[MSG_MAX_MESSAGE_LEN * 2 + 2];

void comm_init() {
  /* init the comm port */
  uart_init();
}

/* this function receives a message from the host
 * the function will block until a message arrives */
bool comm_receive_message(void* buffer, uint16_t* size) {
  bool handleMessage = false;
  int buffIdx = 0;

  do {
    uint8_t c;
    if(!uart_get_byte_nonblocking(&c)) {
//      samp_if_needed();
      continue;
    }
    rfcBuff[buffIdx] = c;
//    rfcBuff[buffIdx] = uart_get_byte_blocking();
    if(rfcBuff[buffIdx] != CHAR_FLAG) {
    	buffIdx++;
      if(buffIdx == MSG_MAX_MESSAGE_LEN * 2 + 2) {
        buffIdx = 0;
      }
  	} else {
  		if(buffIdx >= 2) {
  			handleMessage = true;
  		} else {
  			// discard small messages
  			buffIdx = 0;
  		}
  	}
  } while(!handleMessage);

  if(handleMessage) {
    int len = rfc1662_unpack(rfcBuff, buffIdx, buffer);
    if(len == -1)
    {
      /* the packet fcs was wrong don't accept it */
      return false;
  	}
    *size = len;
    return true;
  }

  return false;
}

void comm_send_message(void* buffer, uint16_t size) {
  uint16_t rfc_len = rfc1662_pack(buffer, size, rfcBuff);
  uart_send(rfcBuff, rfc_len);
}