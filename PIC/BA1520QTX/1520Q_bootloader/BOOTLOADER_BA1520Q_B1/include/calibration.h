#ifndef CALIBRATION_H_
#define CALIBRATION_H_

#include <stdint.h>
#include <stdbool.h>

#define CALIBRATION_AREA_START 0x4000
#define CALIBRATION_AREA_END 0x4FFF

typedef enum {
  CALIBRATION_TABLE_PARAMS = 0,
  CALIBRATION_TABLE_PA_GAINS_1 = 1,
  CALIBRATION_TABLE_PA_GAINS_2 = 2,
  CALIBRATION_TABLE_MAX = 3
} calibration_table_id_t;

typedef enum {
  PARAM_CAL_TBL_VERSION = 0,
  PARAM_SERIAL_NUM = 1,
  PARAM_TEMP_MULT = 2,
  PARAM_FWD_MULT = 3,
  PARAM_REV_TRESH = 4,
  PARAM_INP_PWR_MULT = 5,
  PARAM_PWR_CURRENT_MULT = 6,
  PARAM_PRE_AMP_MULT = 7,
  PARAM_ISENSE_PA1_MULT = 8,
  PARAM_ISENSE_PA2_MULT = 9,
  PARAM_BOOT_WAIT_TIME_USEC = 10,
  PARAM_TX_ON_TIMING_USEC = 11,
  PARAM_TX_OFF_TIMING_USEC = 12,
  PARAM_PA_ON_TIMING_USEC = 13,
  PARAM_PA_OFF_TIMING_USEC = 14,
  PARAM_ANT_SEL_TIMING_USEC = 15,
  PARAM_FWD_SAMP_TIMING_USEC = 16,
  PARAM_REV_SAMP_TIMING_USEC = 17,
  PARAM_INP_PWR_SAMP_TIMING_USEC = 18,
} calibration_param_t;

#define CALIBRATION_TABLE_INDEX_MAX 32

void get_calibration_table_data(calibration_table_id_t table_id, void* table_data);
void set_calibration_table_data(calibration_table_id_t table_id, void* table_data);
#INLINE
uint16_t get_calibration_param(calibration_table_id_t table, uint8_t index);
void set_calibration_param(calibration_table_id_t table, uint8_t index, uint16_t value);

#endif /* CALIBRATION_H_ */
