#include "BA1474.h"

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include "comm.h"
#include "gpio.h"
#include "main.h"
#include "message.h"
#include "message_handlers.h"
#include "ad5312.h"
#include "timer.h"

#include "uart.h"

//allows the main program to run without a bootloader
static void fix_boot() {
  bool should_write = false;

  uint8_t start_bytes[getenv("FLASH_ERASE_SIZE")];
  read_program_memory(0, start_bytes, getenv("FLASH_ERASE_SIZE"));

  uint8_t clear_flash[] = { 0xFF, 0xFF, 0xFF, 0xFF };
  uint8_t jump_to_0x2700[] = { 0x80, 0xEF, 0x13, 0xF0 };

  if (memcmp(start_bytes, clear_flash, sizeof(clear_flash)) == 0) {
    should_write = true;
    //there's nothing at the reset vector let's put a jump to main() there.
    //this happens when running directly without a bootloader.
    //the debugger somehow knows to jump to the real main(), but after a reset it no longer works.
    memcpy(start_bytes, jump_to_0x2700, sizeof(jump_to_0x2700));
  }

  uint8_t jump_to_0x2008[] = { 0x04, 0xEF, 0x10, 0xF0 };
  //check if the interrupt handler contains the correct jump, if not, overwrite it
  if (memcmp(start_bytes + 0x0008, jump_to_0x2008, sizeof(jump_to_0x2008)) != 0) {
    should_write = true;
    memcpy(start_bytes + 0x0008, jump_to_0x2008, sizeof(jump_to_0x2008));
  }

  if (should_write)
    write_program_memory(0, start_bytes, getenv("FLASH_ERASE_SIZE"));
}

static uint8_t message_buffer[MSG_MAX_MESSAGE_LEN];
#org MAIN_ADDRESS, MAIN_ADDRESS+0x800
void main() {
  fix_boot();
  comm_init();
  gpio_init();
  init_message_handlers();
  enable_interrupts(GLOBAL);

  /* turn on the LED */
  gpio_set(GPIO_LED);

  /* enable 5V power */
  gpio_set(GPIO_5V_EN);
  ad5312_init();

  while (1) {
    uint16_t size;
    bool res = comm_receive_message(message_buffer, &size);
    if (res) {
      process_message(message_buffer, size);
	//gpio_set_value(GPIO_TX_RX_SEL, on);
	output_low(GPIO_TX_RX_SEL);
    }
  }
}
