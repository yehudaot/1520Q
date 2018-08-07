/*
 * message_handlers.c
 *
 *  Created on: Mar 26, 2017
 *      Author: asaf
 */

#include "BA1474.h"

#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "message.h"
#include "message_handlers.h"
#include "comm.h"
#include "gpio.h"
#include "version.h"
#include "calibration.h"
#include "ad5312.h"

static uint16_t tti_counter;
static software_mode_t software_mode;

static void send_ack() {
  ack_response_t response = {};
  response.generic.opcode = OP_ACK_RESPONSE;
  comm_send_message(&response, sizeof(response));
}

control_message_payload_t prev_control_message;
control_message_payload_t last_control_message;

static sf_status_response_t final_sf_status_response;
static sf_status_response_t in_progress_sf_status_response;

static void set_tx_state(bool on) {
  gpio_set_value(GPIO_TX_RX_SEL, on);
  gpio_set_value(GPIO_TX_RX_ENV, on);
  gpio_set_value(GPIO_TX_RX_ENVM, !on);
}

static void set_pa_gain(uint8_t amplifier_operation_frequency, uint8_t gain) {
  uint16_t value = get_calibration_param(amplifier_operation_frequency != 0 ? CALIBRATION_TABLE_PA_GAINS_2 : CALIBRATION_TABLE_PA_GAINS_1, gain);
  if(value == 0xffff) {
    /* make sure that empty memory won't cause high external output */
    value = 0;
  }
  ad5312_setVout(DAC_A, value);

  value = get_calibration_param(amplifier_operation_frequency != 0 ? CALIBRATION_TABLE_PA_GAINS_2 : CALIBRATION_TABLE_PA_GAINS_1, (CALIBRATION_TABLE_INDEX_MAX / 2) + gain);
  if(value == 0xffff) {
    /* make sure that empty memory won't cause high external output */
    value = 0;
  }
  ad5312_setVout(DAC_B, value);
}

#INLINE
static void select_antenna(uint8_t antenna) {
  gpio_set_value(GPIO_ANT_SEL, antenna);
  gpio_set_value(GPIO_ANT_SELN, !antenna);
}

static void sample_fwd_power(int n) {
  in_progress_sf_status_response.fwd_power_values[n] = gpio_get_analog(GPIO_FFWR);// * get_calibration_param(CALIBRATION_TABLE_PARAMS, PARAM_FWD_MULT) / 1000;
}

static void sample_rev_power(void) {
uint16_t rev_pass_tresh = get_calibration_param(CALIBRATION_TABLE_PARAMS, PARAM_REV_TRESH);
in_progress_sf_status_response.reverse_power_status = 1;
  	if((in_progress_sf_status_response.fwd_power_values[0] - gpio_get_analog(GPIO_RREV)) < rev_pass_tresh)	//((forwoard power - reverse power) < general calibration rev tresh param) 0 - fail, 1 - pass  
     in_progress_sf_status_response.reverse_power_status = 0;
}

static void sample_input_power(int n) {
  in_progress_sf_status_response.input_power_values[n] = gpio_get_analog(GPIO_RF_INDET);// * get_calibration_param(CALIBRATION_TABLE_PARAMS, PARAM_INP_PWR_MULT) / 1000;
}

//static void sample_pre_amp_power(int n) {					//13.6.18 not used 
  //in_progress_sf_status_response.pre_amp_power_values[n] = gpio_get_analog(GPIO_PREAMP_DET);// * get_calibration_param(CALIBRATION_TABLE_PARAMS, PARAM_PRE_AMP_MULT) / 1000;  //yehuda 22.5.18 remove from control message 
//}

static void finalize_sf_status_request_response() {
  final_sf_status_response = in_progress_sf_status_response;
}

/* sample the 4 adc inputs with delay between them */
#INLINE
void sample_adc_inputs(int n) {
  delay_us(get_calibration_param(CALIBRATION_TABLE_PARAMS, PARAM_FWD_SAMP_TIMING_USEC));
  sample_fwd_power(n);
  delay_us(get_calibration_param(CALIBRATION_TABLE_PARAMS, PARAM_REV_SAMP_TIMING_USEC));
//  sample_rev_power(n);																			//yehuda 13.6.18 remove from control message
//  delay_us(get_calibration_param(CALIBRATION_TABLE_PARAMS, PARAM_INP_PWR_SAMP_TIMING_USEC));
  sample_input_power(n);
//  sample_pre_amp_power(n);																		//yehuda 13.6.18 remove from control message
}

void sample_adc_inputs_helper(void *arg) {
  sample_adc_inputs((int)arg);
}

static uint16_t ant_delay = 0;
static uint16_t pa_delay = 0;
static uint16_t delay_tx_time = 0;

#if 0
static uint16_t adc_samp_to_sample = 0;

void samp_if_needed() {
  if(adc_samp_to_sample != 0) {
    sample_adc_inputs(4-adc_samp_to_sample);
    adc_samp_to_sample--;
  }
}
#endif

#INLINE
void perform_last_control_message() {
  int i;

  if(!last_control_message.bits.dont_change) {
    /* set antenna (delay if needed) */
    if(last_control_message.bits.tx_ant != prev_control_message.bits.tx_ant) {
      //delay_us(ant_delay);
      select_antenna(last_control_message.bits.tx_ant); 
    }
  
    /* set pa gain (delay if needed) */
    bool is_pa_gain_changed = last_control_message.bits.pa_gain != prev_control_message.bits.pa_gain
      || last_control_message.bits.amplifier_operation_frequency != prev_control_message.bits.amplifier_operation_frequency;
  
    if(is_pa_gain_changed) {
      //delay_us(pa_delay);
      ad5312_latch();
    }
  
    /* set tx/rx (delay if needed) */
    if(last_control_message.bits.tx_on != prev_control_message.bits.tx_on) {
      //delay_us(delay_tx_time);
      set_tx_state(last_control_message.bits.tx_on); 
    }
  }

  /* save the last response */
  finalize_sf_status_request_response(); /* get the working copy and use it as final */

  /* sample all sample groups (with delays) */
  
  for(i=0; i<NUM_ADC_SAMPLES; i++) {
    sample_adc_inputs(i);
  }
  sample_rev_power();
  //in_progress_sf_status_response.temperature = gpio_get_analog(GPIO_TMP);   			//yehuda 8.5.18 change to read the temperature in cellceus degrees
  in_progress_sf_status_response.temperature = (uint16_t)((gpio_get_analog(GPIO_TMP) - 167) * 0.30303);
  uint16_t pa1_current = gpio_get_analog(GPIO_PA1_ISENSE);
  uint16_t pa2_current = gpio_get_analog(GPIO_PA2_ISENSE);
  in_progress_sf_status_response.power_amplifier_current = pa1_current + pa2_current;

  
  in_progress_sf_status_response.control_identifier = last_control_message.control_identifier;
  prev_control_message = last_control_message;
  enable_interrupts(INT_EXT);
}

static void handle_control_message(void* payload_buffer) {
  int i;
  bool is_pa_gain_changed = false;
  sf_status_response_t response;
  response = final_sf_status_response;
  control_message_payload_t* payload = (control_message_payload_t*)payload_buffer;
 
  if(!payload->bits.dont_change) {
    /* reset if needed */
    if (payload->bits.reset) {
      reset_cpu();
    }
  
    memcpy(&last_control_message, payload, sizeof(control_message_payload_t));
  
    if(last_control_message.bits.tx_on != prev_control_message.bits.tx_on) {
      if (last_control_message.bits.tx_on == 1) {
        delay_tx_time = get_calibration_param(CALIBRATION_TABLE_PARAMS, PARAM_TX_ON_TIMING_USEC);
      } else {
        delay_tx_time = get_calibration_param(CALIBRATION_TABLE_PARAMS, PARAM_TX_OFF_TIMING_USEC);
      }
    }
  
    /* set the PA, don't latch it yet */
    if(last_control_message.bits.pa_gain != prev_control_message.bits.pa_gain || 
        last_control_message.bits.amplifier_operation_frequency != prev_control_message.bits.amplifier_operation_frequency) {
    
      is_pa_gain_changed = true;
      bool is_pa_gain_changed_to_off = last_control_message.bits.pa_gain == 0;
      if(is_pa_gain_changed_to_off) {
        pa_delay = get_calibration_param(CALIBRATION_TABLE_PARAMS, PARAM_PA_OFF_TIMING_USEC);
      } else {
        pa_delay = get_calibration_param(CALIBRATION_TABLE_PARAMS, PARAM_PA_ON_TIMING_USEC);
      }
    }
  }

  if (software_mode == MODE_OPERATIONAL) {
    //do nothing, actions will be done on the TTI_SYNC interrupt
  }
  else if (software_mode == MODE_TECHNICIAN) {
    perform_last_control_message();
    response = final_sf_status_response; /* get the updated response */
  } else {
    return;//wrong/invalid mode, don't send response
  }

  response.generic.opcode = OP_SF_STATUS_RESPONSE;
  response.tti_counter = tti_counter;
  response.last_control_bits = prev_control_message.bits;
  comm_send_message(&response, sizeof(response));

  if(is_pa_gain_changed == true) {
    set_pa_gain(last_control_message.bits.amplifier_operation_frequency, last_control_message.bits.pa_gain); 
    if(software_mode == MODE_TECHNICIAN) {
      ad5312_latch();
    } 
  }
}

static void handle_bit_status_request_message(void* payload_buffer) {
  int i;
  bit_status_response_t response = {};
  response.generic.opcode = OP_BIT_STATUS_RESPONSE;
  response.tti_counter = tti_counter;
  response.last_control_bits = prev_control_message.bits;
  response.mode = software_mode;
  for(i=0;i<NUM_ADC_SAMPLES;i++)
	{
	response.pre_amp_power_values[i] = gpio_get_analog(GPIO_PREAMP_DET);
	response.reverse_power_values[i] = gpio_get_analog(GPIO_RREV);
    }
  comm_send_message(&response, sizeof(response));
}

static void handle_version_request_message(void* payload_buffer) {
  version_response_t response = {};
  response.generic.opcode = OP_VERSION_RESPONSE;
  response.day = VERSION_DAY;
  response.month = VERSION_MONTH;
  response.year = VERSION_YEAR;
  response.major = VERSION_MAJOR;
  response.minor = VERSION_MINOR;
  response.serial_number = get_calibration_param(CALIBRATION_TABLE_PARAMS, PARAM_SERIAL_NUM);
  comm_send_message(&response, sizeof(response));
}

static void handle_change_mode_message(void* payload_buffer) {
  change_mode_message_payload_t* payload = (change_mode_message_payload_t*)payload_buffer;
  switch (payload->mode) {
  case MODE_TECHNICIAN:
    disable_interrupts(INT_EXT);
  case MODE_OPERATIONAL:
    enable_interrupts(INT_EXT);
  case MODE_MAINTENANCE:
    software_mode = payload->mode;
    break;
  default:
    return;//unknown mode, don't send ack
  }
  send_ack();
}

static void handle_set_calibration_table_message(void* payload_buffer) {
  set_calibration_table_message_payload_t* payload = (set_calibration_table_message_payload_t*)payload_buffer;

  if (software_mode != MODE_MAINTENANCE)
    return;//wrong mode, don't send ack

  if (payload->table_id >= CALIBRATION_TABLE_MAX)
    return;//don't send ack

  set_calibration_table_data(payload->table_id, payload->params);

  send_ack();
}

static void handle_get_calibration_table_message(void* payload_buffer) {
  get_calibration_table_message_payload_t* payload = (get_calibration_table_message_payload_t*)payload_buffer;

  if (payload->table_id >= CALIBRATION_TABLE_MAX)
    return;//don't send response

  calibration_table_response_t response = {};
  response.generic.opcode = OP_GET_CALIBRATION_TABLE_RESPONSE;
  response.table_id = payload->table_id;
  get_calibration_table_data(payload->table_id, response.params);
  comm_send_message(&response, sizeof(response));
}

static void h_test_set_data(void* payload_buffer) {
  send_ack();
}

static void h_test_get_data(void* payload_buffer) {
  get_data_line_response_t response = {};
  response.generic.opcode = OP_GET_DATA_LINE_MESSAGE;
  response.address = ((get_data_line_payload_t *)payload_buffer)->address;
  comm_send_message(&response, sizeof(response));
}

typedef void (*cmd_func)(void* message_payload);

typedef struct cmd_entry {
  uint8_t opcode;
  cmd_func func;
} cmd_entry;

static cmd_entry commands[] = {
  { OP_CONTROL_MESSAGE, handle_control_message },
  { OP_BIT_STATUS_REQUEST_MESSAGE, handle_bit_status_request_message },
  { OP_VERSION_REQUEST_MESSAGE, handle_version_request_message },
  { OP_CHANGE_MODE_MESSAGE, handle_change_mode_message },
  { OP_SET_CALIBRATION_TABLE_MESSAGE, handle_set_calibration_table_message },
  { OP_GET_CALIBRATION_TABLE_MESSAGE, handle_get_calibration_table_message },
  { OP_SET_DATA_LINE_MESSAGE, h_test_set_data },
  { OP_GET_DATA_LINE_MESSAGE, h_test_get_data },
};

void process_message(uint8_t* message_buffer, uint16_t length) {
  message_t* message = (message_t*)message_buffer;
  /* find the correct func */
  int i;
  for(i=0;i<ARRAY_SIZE(commands); i++) {
    if(commands[i].opcode == message->opCode) {
      commands[i].func(message->payload);
      return;
    }
  }
}

void init_message_handlers() {
  tti_counter = 0;
  software_mode = MODE_OPERATIONAL;

  memset(&in_progress_sf_status_response, 0, sizeof(in_progress_sf_status_response));
  memset(&final_sf_status_response, 0, sizeof(final_sf_status_response));
  memset(&last_control_message, 0, sizeof(last_control_message));

  prev_control_message = last_control_message;
  /* set bogus data so that perform_last_control_message() will act on all parameters */
  prev_control_message.bits.pa_gain = 3;
  prev_control_message.bits.tx_ant = 1;
  prev_control_message.bits.tx_on = 1;
  perform_last_control_message();
  
  ext_int_edge(L_TO_H);
  enable_interrupts(INT_EXT); 
}

#int_ext
void ext_isr(void) //TTI_SYNC line
{
  perform_last_control_message();
  ++tti_counter;
}