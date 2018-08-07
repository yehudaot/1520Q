/*
 * message.h
 *
 *  Created on: Mar 23, 2017
 *      Author: asaf
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <stdint.h>
#include "calibration.h"

enum {
  NUM_ADC_SAMPLES = 4,
};

enum {
  OP_CONTROL_MESSAGE = 0x00,
  OP_VERSION_REQUEST_MESSAGE = 0x02,
  OP_CHANGE_MODE_MESSAGE = 0x03,
  OP_SET_CALIBRATION_TABLE_MESSAGE = 0x04,
  OP_GET_CALIBRATION_TABLE_MESSAGE = 0x05,
  OP_SET_DATA_LINE_MESSAGE = 0x06,
  OP_GET_DATA_LINE_MESSAGE = 0x07,
  OP_FINISH_UPDATE_PROCESS_MESSAGE = 0x08,
  OP_BIT_STATUS_REQUEST_MESSAGE = 0x09,
};

enum {
  OP_ACK_RESPONSE = 0x80,
  OP_SF_STATUS_RESPONSE = 0x81,
  OP_VERSION_RESPONSE = 0x82,
  OP_GET_CALIBRATION_TABLE_RESPONSE = 0x85,
  OP_SET_DATA_LINE_RESPONSE = 0x86,
  OP_GET_DATA_LINE_RESPONSE = 0x87,
  OP_BIT_STATUS_RESPONSE = 0x89,
};

typedef enum {
  MODE_OPERATIONAL = 0,
  MODE_TECHNICIAN = 1,
  MODE_MAINTENANCE = 2,
} software_mode_t;

enum {
  CALIBRATION_TABLE_TEMP = 0,
  CALIBRATION_TABLE_FWD = 1,
  CALIBRATION_TABLE_REV = 2,
  CALIBRATION_TABLE_INP = 3,
  CALIBRATION_TABLE_CURRENT = 4,
  CALIBRATION_TABLE_PA_BIAS = 5,
  CALIBRATION_TABLE_TIMING = 6,
  CALIBRATION_TABLE_GENERAL = 7,
};

enum {
  PROGRAMMING_STATUS_OK = 0,
  PROGRAMMING_STATUS_ERROR = 1,
};

typedef struct message_t {
  uint8_t opcode;
  uint8_t payload[]; /* this will include the payload and the crc */
} message_t;

typedef struct generic_response_t {
  uint8_t opcode;
} generic_response_t;

/* message payloads*/

typedef struct control_bits_t {
  uint8_t tx_on : 1;
  uint8_t pa_gain : 3;
  uint8_t tx_ant : 1;
  uint8_t amplifier_operation_frequency : 1;
  uint8_t reset : 1;
  uint8_t dont_change : 1;
} control_bits_t;

typedef struct control_message_payload_t {
  control_bits_t bits;
  uint16_t control_identifier;
} control_message_payload_t;

typedef struct change_mode_message_payload_t {
  uint8_t mode;
} change_mode_message_payload_t;


typedef struct set_calibration_table_message_payload_t {
  uint8_t table_id;
  uint16_t params[CALIBRATION_TABLE_INDEX_MAX];
} set_calibration_table_message_payload_t;

typedef struct get_calibration_table_message_payload_t {
  uint8_t table_id;
} get_calibration_table_message_payload_t;

typedef struct set_data_line_payload_t {
  uint32_t address;
  uint8_t data[64];
} set_data_line_payload_t;

typedef struct get_data_line_payload_t {
  uint32_t address;
} get_data_line_payload_t;

/* responses */

typedef struct ack_response_t {
  generic_response_t generic;
} ack_response_t;

typedef struct sf_status_response_t {
  generic_response_t generic;
  control_bits_t last_control_bits;
  uint16_t tti_counter;
  uint16_t control_identifier;
  uint16_t fwd_power_values[NUM_ADC_SAMPLES];
//  uint16_t reverse_power_values[NUM_ADC_SAMPLES];		//yehuda move rev power to bit status
  uint16_t input_power_values[NUM_ADC_SAMPLES];
  //uint16_t pre_amp_power_values[NUM_ADC_SAMPLES];		//yehuda move pre amp to bit status
  uint16_t temperature;
  uint16_t power_amplifier_current;
  uint8_t reverse_power_status;							//Difference between forword power and reverse power compared to general calibration rev tresh param
} sf_status_response_t;

typedef struct bit_status_response_t {
  generic_response_t generic;
  control_bits_t last_control_bits;
  uint16_t tti_counter;
  uint8_t mode;
  uint16_t pre_amp_power_values[NUM_ADC_SAMPLES]; 		//yehuda add pre amp to bit status
  uint16_t reverse_power_values[NUM_ADC_SAMPLES];		//yehuda add rev power to bit status
} bit_status_response_t;

typedef struct version_response_t {
  generic_response_t generic;
  uint8_t day;
  uint8_t month;
  uint16_t year;
  uint8_t major;
  uint8_t minor;
  uint16_t serial_number;
} version_response_t;

typedef struct calibration_table_response_t {
  generic_response_t generic;
  uint8_t table_id;
  uint16_t params[CALIBRATION_TABLE_INDEX_MAX];
} calibration_table_response_t;

typedef struct set_data_line_response_t {
  generic_response_t generic;
  uint32_t address;
  uint8_t status;
} set_data_line_response_t;

typedef struct get_data_line_response_t {
  generic_response_t generic;
  uint32_t address;
  uint8_t data[64];
} get_data_line_response_t;

#endif /* MESSAGE_H_ */
