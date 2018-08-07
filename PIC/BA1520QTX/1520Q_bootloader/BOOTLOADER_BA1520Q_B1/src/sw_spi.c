#include "BA1474.h"

#include "sw_spi.h"
#include "gpio.h"
#include "comm.h" /* for array_size */

enum {
  SW_SPI_DATA_IN   = 0,
  SW_SPI_CLK       = 1,
  SW_SPI_CS        = 2,
  SW_SPI_DATA_OUT  = 3,
};

static void IOWR(uint32_t dat) {
  int i;
  /* convert bit positions to GPIOs */
  uint8_t gpio [] = {
    -1, 
    GPIO_DAC_CLK,
    GPIO_DAC_CS,
    GPIO_DAC_DATA_OUT,
  };

  for(i=1; i<ARRAY_SIZE(gpio); i++) {
    uint8_t val = ((dat & (1 << i)) >> i);
//    if(i == SW_SPI_CS) {
//      /* active low CS */
//      val = (val +1) & 1;
//    }
    gpio_set_value(gpio[i], val);
  }
}

static int IORD() {
    /* not implemented in this project  */
    return 0;
}

static void SPIIoWr (uint32_t dat, int niter)
{
  IOWR(dat);
  //niter = niter <<2 ;
 // while(niter-- > 0) {
    //delay_us(1);
 // }

  //unsigned t0 = getSYSClockCounterValue();
  //while ((unsigned)(getSYSClockCounterValue()-t0) < niter);
}

// swSpi_write - write nbits bits to SPI bus data, MSB first, data aligned to LSB
// niter    - number of iterations in every step (SPI clock = sys_clk/(2*niter))
// fallingEdge  - 0 = master shifts data out on rising sclk edge, samples data out on the rising sclk edge 00
//				  1 = master shifts data out on falling sclk edge, samples data out on the rising sclk edge 01
//                2 = master shifts data out on rising sclk edge, samples data out on the falling sclk edge 10
//                3 = master shifts data out on falling sclk edge, samples data out on the falling sclk edge 11
// return value - read data
uint32_t swSpi_write (uint32_t data, uint32_t nbits, uint32_t niter, int fallingEdge)
{
  int oFallingEdge = fallingEdge & 1;
  int sFallingEdge = fallingEdge & 2; // when to sample the read data, on inactive (1) or active (0) edge of clock
  uint32_t sclkEdge = oFallingEdge << SW_SPI_CLK;
  uint32_t sclkPh0 = sclkEdge ^ (0 << SW_SPI_CLK);
  uint32_t sclkPh1 = sclkEdge ^ (1 << SW_SPI_CLK);
  IOWR((1<<SW_SPI_CS)+ sclkPh0);            // set sclk to intial phase before frame
  SPIIoWr((0<<SW_SPI_CS)+ sclkPh0, niter);     // start frame with sclk ready for shifting master data out
  data <<= (32-nbits); // align data to the left (MS bit)
  uint32_t readData = 0;
  int i;
  uint32_t dout = 0;
  for (i = 0; i < nbits; ++i)
  {  // shift out data
     dout = (unsigned long)(data >> 31) & 1; // MS bit first
     SPIIoWr((dout<<SW_SPI_DATA_OUT)+sclkPh0, niter); // apply data with inactive sclk
     //if (sFallingEdge) readData += readData + ((IORD()>>SW_SPI_DATA_IN)&1);      // sample data far enough after inactive edge
     SPIIoWr((dout<<SW_SPI_DATA_OUT)+sclkPh1, niter); // apply data with active sclk - shift data out
     //if (!sFallingEdge) readData += readData + ((IORD()>>SW_SPI_DATA_IN)&1);      // sample data far enough after active edge
     data += data;
  }
  SPIIoWr((dout<<SW_SPI_DATA_OUT)+(0 << SW_SPI_CLK), niter); // keep last data bit a little more with sclk low - suitable for both clock modes
  IOWR((1<<SW_SPI_CS));                                                         // finish frame with inactive chip select and sclk kept low
  return readData;
}
