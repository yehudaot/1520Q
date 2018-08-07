#ifndef SW_SPI_H_INCLUDED
#define SW_SPI_H_INCLUDED

#include <stdint.h>

enum { //SPI
	  SW_SPI_SCLK_EDGE_RISE_RISE = 0,  	// shift data out on rising edge and samples on rising
	  SW_SPI_SCLK_EDGE_FALLING = 1,  	// shift data out on falling edge and samples on rising
	  SW_SPI_SCLK_EDGE_RISING = 2,  	// shift data out on rising edge and samples on falling
	  SW_SPI_SCLK_EDGE_FALL_FALL = 3,   // shift data out on falling edge and samples on falling
};


uint32_t swSpi_write(uint32_t data, uint32_t nbits, uint32_t niter, int fallingEdge);

#endif  /* SW_SPI_H_INCLUDED */
