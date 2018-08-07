/*
 * gpio.c
 *
 *  Created on: Mar 8, 2017
 *      Author: asafe
 */
 
#include "BA1474.h"

#include <stdint.h>
 
#include "gpio.h"
#include "comm.h"

/* note, analog inputs pin # is their anX number */
static const st_gpio gpios []  = {
      //{Pin #,     Direction,    Name}
      {PIN_A3, GPIO_DIR_OUT, "DAC Clk"},
      {PIN_A2, GPIO_DIR_OUT, "DAC SYNC"},
      {PIN_A4, GPIO_DIR_OUT, "DAC Data"}, 
      {PIN_A1, GPIO_DIR_OUT, "DAC Latch"}, 
      
      {PIN_C0, GPIO_DIR_OUT, "Led 1"}, 
      {8, GPIO_ANALOG, "PA1_ISENSE"},
      {9, GPIO_ANALOG, "PA2_ISENSE"},
      {5, GPIO_ANALOG, "FFWR"},
      {4, GPIO_ANALOG, "RREV"},
      {7, GPIO_ANALOG, "RF_INDET"},
      {6, GPIO_ANALOG, "TMP"},
      {17, GPIO_ANALOG, "PREAMP_DET"},

      {PIN_D7, GPIO_DIR_OUT, "ANT_SEL"},
      {PIN_D6, GPIO_DIR_OUT, "ANT_SELN"},
      {PIN_B4, GPIO_DIR_OUT, "TX_RX_SEL"},
      {PIN_B5, GPIO_DIR_OUT, "TX_RX_ENV"},
      {PIN_D0, GPIO_DIR_OUT, "TX_RX_ENVM"},
      {PIN_D5, GPIO_DIR_OUT, "5V_EN"},
};

void gpio_init() {
  set_tris_a(0b00100000);
  set_tris_b(0b00001111);
  set_tris_c(0b10100000);
  set_tris_d(0b00000000);
  set_tris_e(0b00000111);

  output_a(0);
  output_b(0);
  output_d(0);
  output_e(0);

  setup_adc_ports(sAN8 | sAN9 | sAN5 | sAN4 | sAN7 | sAN6 | sAN17);//enable analog pins
  setup_adc(ADC_CLOCK_DIV_16|ADC_TAD_MUL_8);
}

int gpio_toggle(int gpio) {
    /* not implemented */
    return 0;
}

#INLINE
void gpio_set(int gpio) {
    output_high(gpios[gpio].pin);
}

void gpio_clear(int gpio) {
    output_low(gpios[gpio].pin);
}

#INLINE
void gpio_set_value(int gpio, int value) {
    if(value) {
      gpio_set(gpio);
    } else {
      gpio_clear(gpio);
    }
}

int gpio_get(int gpio) {
    return input(gpios[gpio].pin);
}

#INLINE
uint32_t gpio_get_analog(int gpio) {
    set_adc_channel(gpios[gpio].pin); // select forward power input
    //elay_us(20);d
    read_adc(ADC_START_ONLY);
    int1 done = adc_done();
    while(!done) {   
      done = adc_done();
    }
    return read_adc(ADC_READ_ONLY );
    //return read_adc();
}