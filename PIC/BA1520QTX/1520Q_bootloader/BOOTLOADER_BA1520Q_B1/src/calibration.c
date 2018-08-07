#include "BA1474.h"

#include "calibration.h"

typedef struct calibration_table_t {
  uint16_t values[CALIBRATION_TABLE_INDEX_MAX];
} calibration_table_t;

typedef struct calibration_area_t {
  calibration_table_t tables[CALIBRATION_TABLE_MAX];
} calibration_area_t;

#ORG CALIBRATION_AREA_START, CALIBRATION_AREA_END
ROM calibration_area_t calibration_area = {};

void get_calibration_table_data(calibration_table_id_t table_id, void* table_data)
{
  read_program_memory(CALIBRATION_AREA_START + table_id * sizeof(calibration_table_t), table_data, sizeof(calibration_table_t));
}

void set_calibration_table_data(calibration_table_id_t table_id, void* table_data)
{
  safe_write_program_memory(CALIBRATION_AREA_START + table_id * sizeof(calibration_table_t), table_data, sizeof(calibration_table_t));
}

#INLINE
uint16_t get_calibration_param(calibration_table_id_t table_id, uint8_t index) {
  uint16_t value = 0;
  read_program_memory(CALIBRATION_AREA_START + table_id * sizeof(calibration_table_t) + index * sizeof(uint16_t), &value, sizeof(uint16_t));
  return (value == 0xffff) ? 0 : value;
}

void set_calibration_param(calibration_table_id_t table, uint8_t index, uint16_t value) {
  calibration_table_t temp_table;
  memcpy(&temp_table, &calibration_area.tables[table], sizeof(temp_table));
  temp_table.values[index] = value;

  safe_write_program_memory(&calibration_area.tables[table], &temp_table, sizeof(temp_table));
}