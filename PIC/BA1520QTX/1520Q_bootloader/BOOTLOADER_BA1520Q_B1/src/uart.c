/*
 * uart.c
 *
 *  Created on: Mar 23, 2017
 *      Author: asaf
 */
#include "BA1474.h"
#include "PIC18F45K22_registers.h"

#include <stdint.h>

#define MAX_UART_SIZE 180

static uint8_t rbuf[MAX_UART_SIZE];
static uint8_t rbuf_wr_idx;
static uint8_t rbuf_rd_idx;

static uint8_t wbuf[MAX_UART_SIZE];
static uint8_t wbuf_wr_idx;
static uint8_t wbuf_rd_idx;

void uart_clear_errors();

void uart_init() {
  rbuf_wr_idx = 0;
  rbuf_rd_idx = 0;
  wbuf_wr_idx = 0;
  wbuf_rd_idx = 0;
  enable_interrupts(int_RDA);
}

void uart_disable() {
  disable_interrupts(int_RDA);
  disable_interrupts(int_TBE);
}

static uint8_t get_next_byte() {
  uint8_t c = rbuf[rbuf_rd_idx++];
  if(rbuf_rd_idx >= MAX_UART_SIZE) {
    rbuf_rd_idx = 0;
  }
  return c;
}

/* stupid implementation, overwrite old data... */
static void push_bytes(uint8_t *src, uint16_t len) {
    int d = MAX_UART_SIZE - wbuf_wr_idx;
    if(len > d) {
      memcpy(wbuf + wbuf_wr_idx, src, d);
      len -= d;
      memcpy(wbuf, src + d, len);
      wbuf_wr_idx = len;
    } else {
      memcpy(wbuf + wbuf_wr_idx, src, len);
      wbuf_wr_idx += len;
    }
    if(wbuf_wr_idx == MAX_UART_SIZE) {
		  wbuf_wr_idx = 0;    
	  } 
}

uint8_t uart_get_byte_blocking() {
  while(rbuf_wr_idx == rbuf_rd_idx) {
enable_interrupts(INT_EXT); 
};
  return get_next_byte();
}

uint8_t uart_get_byte_nonblocking(uint8_t *dst) {
  if(rbuf_wr_idx == rbuf_rd_idx) {
    return 0;
  }
  *dst = get_next_byte();
  return 1;
}

void uart_send(void* buffer, uint8_t size) {
  push_bytes(buffer, size);
  enable_interrupts(int_TBE);

  /* wait here until done */
  //while(wbuf_rd_idx != wbuf_wr_idx) {};
}

/* UART IRQs */

#int_RDA
void RDA_isr(void)
{
  /* uart rx buffer can hold 2.x input bytes */
  do { 
    rbuf[rbuf_wr_idx++] = RCREG1;
    if (rbuf_wr_idx == MAX_UART_SIZE) {
      rbuf_wr_idx = 0;
    }
    uart_clear_errors();
  } while (kbhit());
  enable_interrupts(int_RDA);
}

#int_TBE
void TBE_isr(void)
{
  if (wbuf_rd_idx != wbuf_wr_idx){
    TXREG1 = wbuf[wbuf_rd_idx++];
    if(wbuf_rd_idx == MAX_UART_SIZE) {
      wbuf_rd_idx = 0;
    }
  } else {
    disable_interrupts(int_TBE);
  }
}

#bit CREN=getenv("BIT:CREN") 
#BIT OERR = getenv("BIT:OERR")
void uart_clear_errors()
{
  if (OERR) {
    CREN = 0;
    CREN = 1;
  }
}