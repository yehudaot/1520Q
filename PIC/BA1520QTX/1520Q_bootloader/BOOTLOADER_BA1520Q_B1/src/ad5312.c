/*
 * ad5312.c
 *
 *  Created on: Oct 13, 2016
 *      Author: asaf
 */

#include "BA1474.h"

#include <stdint.h>

#include "ad5312.h"
#include "gpio.h"

#define DAC_MV_PER_BIT          ((float)DAC_PRECISION / VREF_FULL_SCALE)
#define DEFAULT_REF_BUF_MODE    ((int)REF_UNBUFFERED)
#define DEFAULT_POWER_MODE      ((int)PD_NORMAL)
#define DEFAULT_POWER_UP_OUTPUT 0

uint16_t ad5312_default_conf_word;

/* SPI parameters */
enum {
  AD5312_LATCH_GPIO = GPIO_DAC_LDAC,
};

static void writeData(int device, uint16_t value) {
	uint16_t data = ad5312_default_conf_word +
					((uint16_t)device << DAC_SELECT_BIT) +
					((value & 0x3ff) << 2);
  spi_xfer(data);  
}

#INLINE
void ad5312_latch() {
  int i;
	/*  Active Low, transfers the contents of the input registers to their respective DAC registers. */
  gpio_clear(AD5312_LATCH_GPIO);
  for(i=0;i<10;i++) {}
  gpio_set(AD5312_LATCH_GPIO);
}

#INLINE
void ad5312_setVout(int dac, uint16_t mV) {
	uint16_t vout = (uint16_t)(mV * DAC_MV_PER_BIT);
	if(vout >= DAC_PRECISION) {
		vout = DAC_PRECISION-1; /* cap output to maximum value */
	}
	writeData(dac, vout);
}

void ad5312_init() {
	/* set the default configuration word */
	ad5312_default_conf_word = (DEFAULT_REF_BUF_MODE << REF_BUF_BIT) +
									(DEFAULT_POWER_MODE << PD_ARR_START_BIT);

	/* set the latch gpio to '1' */
	gpio_set(AD5312_LATCH_GPIO);

	/* set the AD5312 to DEFAULT_POWER_UP_OUTPUT */
  ad5312_setVout(DAC_A, DEFAULT_POWER_UP_OUTPUT);
  ad5312_setVout(DAC_B, DEFAULT_POWER_UP_OUTPUT);
	ad5312_latch();
}
