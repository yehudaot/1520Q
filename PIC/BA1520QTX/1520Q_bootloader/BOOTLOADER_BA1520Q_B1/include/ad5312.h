/*
 * ad5312.h
 *
 *  Created on: Oct 13, 2016
 *      Author: asaf
 */

#ifndef AD5312_H_
#define AD5312_H_

enum {
	DAC_SELECT_BIT = 15,
	REF_BUF_BIT = 14,
	PD1_BIT = 13,
	PD2_BIT = 12,
	PD_ARR_START_BIT = PD2_BIT,
};

enum {
	DAC_A = 0,
	DAC_B = 1,
};

enum {
	REF_UNBUFFERED = 0,
	REF_BUFFERED = 1,
};

/* Power Down Modes */
enum {
	PD_NORMAL = 0,
	PD_1K_TO_GND = 1,
	PD_100K_TO_GND = 2,
	PD_HIGH_IMPEDANCE_OUTPUT = 3,
};

enum {
	VREF_FULL_SCALE = 5000, /* mV */
	DAC_PRECISION = 1024 , /* bits */
};

#INLINE
void ad5312_setVout(int dac, uint16_t mV);
void ad5312_init();
#INLINE
void ad5312_latch();

#endif /* AD5312_H_ */
