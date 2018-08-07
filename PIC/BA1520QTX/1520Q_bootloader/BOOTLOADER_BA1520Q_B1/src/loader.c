#include "BA1474.h"

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include "comm.h"
#include "boot_version.h"
#include "main.h"
#include "timer.h"
#include "uart.h"
#include "message.h"
#include "calibration.h"

#define BOOT_WAIT_TIME_US 700

typedef struct {
  bool run_loader;
  bool is_first_page_dirty;
} bootloader_metadata_t;

#define ROM_METADATA_ADDRESS 0x1000
#ORG ROM_METADATA_ADDRESS,ROM_METADATA_ADDRESS + getenv("FLASH_ERASE_SIZE") {}

static void set_metadata(bootloader_metadata_t* value) {
  union {
    bootloader_metadata_t value;
    uint8_t padding[getenv("FLASH_ERASE_SIZE")];
  } data;

  memcpy(&data.value, value, sizeof(data.value));
  safe_write_program_memory(ROM_METADATA_ADDRESS, &data, sizeof(data));
}

bootloader_metadata_t *get_metadata() {
  static bootloader_metadata_t meta;
  read_program_memory(ROM_METADATA_ADDRESS, &meta, sizeof(bootloader_metadata_t));
  return &meta;
}

static void set_software_ready(bool value) {
  bootloader_metadata_t *temp_metadata = get_metadata();
  temp_metadata->run_loader = ~value;
  set_metadata(temp_metadata);
}

static void set_first_page_clean() {
  bootloader_metadata_t *temp_metadata = get_metadata();
  temp_metadata->is_first_page_dirty = false;
  set_metadata(temp_metadata);
}

static void set_first_page_dirty() {
  bootloader_metadata_t *temp_metadata = get_metadata();
  temp_metadata->is_first_page_dirty = true;
  set_metadata(temp_metadata);
}

#define BOOTLOADER_FIRST_PAGE_ADDRESS 0x1100
#org BOOTLOADER_FIRST_PAGE_ADDRESS, BOOTLOADER_FIRST_PAGE_ADDRESS + getenv("FLASH_ERASE_SIZE") {}


//the main program overwrites the interrupt handler in the first page, so we need to save/restore it.
static void fix_first_page() {
  uint8_t reset_vector[] = { 0x80, 0xEF, 0x00, 0xF0 };
  uint8_t interrupt_vector[] = { 0x04, 0x6E, 0xD8, 0xCf };
  uint8_t start_bytes[getenv("FLASH_ERASE_SIZE")];
  bool should_write = false;

  read_program_memory(0, start_bytes, sizeof(start_bytes));
  //check if the reset vector contains the correct jump, if not, overwrite it
  if (memcmp(start_bytes, reset_vector, sizeof(reset_vector)) != 0) {
    should_write = true;
    memcpy(start_bytes, reset_vector, sizeof(reset_vector));
  }

  //check if the interrupt handler contains the correct jump, if not, overwrite it
 if (memcmp(start_bytes+8, interrupt_vector, sizeof(interrupt_vector)) != 0) {
    should_write = true;
    memcpy(start_bytes+8, interrupt_vector, sizeof(interrupt_vector));
  }

  if(should_write) {
    safe_write_program_memory(0x0000, start_bytes, sizeof(start_bytes));
  }
}

static void handle_set_data_line_message(void* payload_buffer) {
  set_data_line_payload_t* payload = (set_data_line_payload_t*)payload_buffer;

  set_data_line_response_t response = {};
  response.generic.opcode = OP_SET_DATA_LINE_RESPONSE;
  response.address = payload->address;
  response.status = PROGRAMMING_STATUS_OK;

  if ((payload->address < CALIBRATION_AREA_START || payload->address > CALIBRATION_AREA_END)
      && payload->address > LOADER_END && payload->address < 65535) {
    if (!get_metadata()->run_loader) {
      set_software_ready(false);
    }

    safe_write_program_memory(payload->address, payload->data, sizeof(payload->data));
  }
  else {
    delay_us(1);
  }

  comm_send_message(&response, sizeof(response));
}

static void handle_get_data_line_message(void* payload_buffer) {
  get_data_line_payload_t* payload = (get_data_line_payload_t*)payload_buffer;
  get_data_line_response_t response = {};

  response.generic.opcode = OP_GET_DATA_LINE_RESPONSE;
  response.address = payload->address;
  read_program_memory(response.address, response.data, sizeof(response.data));

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
  response.serial_number = 0;
  comm_send_message(&response, sizeof(response));
}

static void handle_finish_update_process_message(void* payload_buffer) {
  uint16_t temp = 0xFFFF;
  ack_response_t response = {};
  set_software_ready(true);
  set_first_page_dirty();
  response.generic.opcode = OP_ACK_RESPONSE;
  write_program_memory(0x7F0, &temp, sizeof(temp));			//yehuda remove the bootloader flag to start the APP
  comm_send_message(&response, sizeof(response));
}

typedef void (*cmd_func)(void* message_payload);

typedef struct cmd_entry {
  uint8_t opcode;
  cmd_func func;
} cmd_entry;

static cmd_entry commands[] = {
  { OP_SET_DATA_LINE_MESSAGE, handle_set_data_line_message },
  { OP_GET_DATA_LINE_MESSAGE, handle_get_data_line_message },
  { OP_VERSION_REQUEST_MESSAGE, handle_version_request_message },
  { OP_FINISH_UPDATE_PROCESS_MESSAGE, handle_finish_update_process_message },
};

static uint8_t message_buffer[MSG_MAX_MESSAGE_LEN];

static void process_loader_message() {
  message_t* message;
  message = (message_t*)message_buffer;
  /* find the correct func */
  int i;
  for(i=0;i<ARRAY_SIZE(commands); i++) {
    if(commands[i].opcode == message->opcode) {
      //ugly hack to make p point to message->payload...
      uint16_t p = message;
      p += 1;
      commands[i].func((uint16_t*)p);
      return;
    }
  }
}

static void loader_main() {
  while (1) {
    uint16_t size;
    bool res = comm_receive_message(message_buffer, &size);
    if (res)
      process_loader_message();
  }
}

static void init_io_ports() {
  //we only care about B1, the PROG_EN pin
  output_high(PIN_B1);
  set_tris_b(0x02);//set all B pins to input mode
}

static void dummy_function() {
}


#org 0x100, 0x800
int main() {
  disable_interrupts(GLOBAL);		//probably unnecessary but just to be safe...
  uint16_t addr_7f0_flag = 0;
  bool start_loader = false;
  bootloader_metadata_t *metadata = get_metadata();		//returns data from program memory start address 0x1000
  if (!metadata->run_loader) { 							//otherwise we always want the bootloader
  	read_program_memory(0x7f0, &addr_7f0_flag, sizeof(addr_7f0_flag));
	if(addr_7f0_flag == 0x04D2)
		start_loader = true;
  }
  else
	start_loader = true;
	

  if (start_loader) {
    fix_first_page();
    comm_init();
    enable_interrupts(GLOBAL);
    loader_main();
    return 0;
  }
  else {
    void (*real_main)(void);
    real_main = dummy_function;//hack to overcome a compilation error
    real_main = (void*)MAIN_ADDRESS;
    real_main();
    return 0;
  }
}
