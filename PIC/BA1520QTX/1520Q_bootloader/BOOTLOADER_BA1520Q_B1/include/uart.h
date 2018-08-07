/*
 * uart.h
 *
 *  Created on: Mar 23, 2017
 *      Author: asaf
 */

#ifndef UART_H_
#define UART_H_

#include <stdint.h>
#include <stddef.h>

void uart_init();
void uart_disable();
uint8_t uart_get_byte_blocking();
uint8_t uart_get_byte_nonblocking(uint8_t *dst);
void uart_send(void* buffer, uint8_t size);
void uart_clear_errors();

#endif /* UART_H_ */
