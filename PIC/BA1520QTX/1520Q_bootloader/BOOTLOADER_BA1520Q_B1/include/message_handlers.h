/*
 * message_handlers.h
 *
 *  Created on: Mar 26, 2017
 *      Author: asaf
 */

#ifndef MESSAGE_HANDLERS_H_
#define MESSAGE_HANDLERS_H_

void init_message_handlers();
void process_message(uint8_t* message, uint16_t length);
#INLINE
void perform_last_control_message();
void control_handle_timer();
void samp_if_needed();

#endif /* MESSAGE_HANDLERS_H_ */
