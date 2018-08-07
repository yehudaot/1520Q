/*
 * gpio.h
 *
 *  Created on: Mar 8, 2017
 *      Author: asafe
 */

#ifndef INCLUDE_GPIO_H_
#define INCLUDE_GPIO_H_

#include <stdint.h>
#include "gpio.h"

enum {
    GPIO_DIR_IN     = 0,
    GPIO_DIR_OUT    = 1,
    GPIO_ANALOG     = 2,

    GPIO_VALUE_LOW = 0,
    GPIO_VALUE_HIGH = 1,
};

typedef struct gpio_st {
    uint32_t pin;
    int dir;
    char *gpio_name;
} st_gpio;

/* this is the deceleration that a developer will use to access a specific GPIO */
/* this list should be identical in order to the <Bold> st_gpio gpios [] </Bold> list in gpio.c */

enum {
    GPIO_DAC_CLK        = 0,
    GPIO_DAC_CS         = 1,
    GPIO_DAC_DATA_OUT   = 2,
    GPIO_DAC_LDAC       = 3,

    GPIO_LED            = 4,

    GPIO_PA1_ISENSE     = 5,
    GPIO_PA2_ISENSE     = 6,
    GPIO_FFWR           = 7,
    GPIO_RREV           = 8,
    GPIO_RF_INDET       = 9,
    GPIO_TMP            = 10,
    GPIO_PREAMP_DET     = 11,

    GPIO_ANT_SEL        = 12,
    GPIO_ANT_SELN       = 13,
    GPIO_TX_RX_SEL      = 14,
    GPIO_TX_RX_ENV      = 15,
    GPIO_TX_RX_ENVM     = 16,

    GPIO_5V_EN          = 17,

};

/* Inputs */
enum {

};


void gpio_init();
int gpio_toggle(int gpio);
#INLINE
void gpio_set(int gpio);
void gpio_clear(int gpio);
#INLINE
void gpio_set_value(int gpio, int value);
int gpio_get(int gpio);
#INLINE
uint32_t gpio_get_analog(int gpio);

#endif /* INCLUDE_GPIO_H_ */
