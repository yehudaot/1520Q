#include <18F45K22.h>
#device adc=10
#device pass_strings=in_ram

#define LOADER_END 0x1FFF
#include <boot.h>

#FUSES NOWDT                   //No Watch Dog Timer
#FUSES WDT128                  //Watch Dog Timer uses 1:128 Postscale
#FUSES INTRC_IO                //Internal RC Osc, no CLKOUT
#FUSES NOBROWNOUT              //No brownout reset
#FUSES WDT_SW                  //No Watch Dog Timer, enabled in Software
#FUSES NOLVP                   //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NOPUT
#FUSES NOXINST                 //Extended set extension and Indexed Addressing mode disabled (Legacy mode)

#use delay(clock=64000000)
#use rs232(baud=921600,xmit=PIN_C6,rcv=PIN_C7,ERRORS)
#USE TIMER(TIMER=1,TICK=1us,BITS=32,NOISR)
#use spi(DO=PIN_A4, CLK=PIN_A3, ENABLE=PIN_A2, ENABLE_ACTIVE=0, DATA_HOLD=20, BITS=16)

#opt 9

#use fast_io(a)
#use fast_io(b)
#use fast_io(c)
#use fast_io(d)
#use fast_io(e)

#define safe_write_program_memory(address, pointer, size) \
  do { disable_interrupts(GLOBAL); write_program_memory(address, pointer, size); enable_interrupts(GLOBAL); delay_us(1000); } while (0)

