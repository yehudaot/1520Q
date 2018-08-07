#include <BA1520QTX.h>
//#include <stdlib.h>
#include <ctype.h>
#include "PIC18F45K22_registers.h"


#define VERSION "BA1520QTX V1.0"
#define VER 1

//--------- frequency constants --------------------------------
#define FREQ_P_MULT           32L
#define FREQ_BASE        2185000L       // in KHz
#define FREQ_TOP         2400000L       // in KHz
#define FREQ_STEP            100L       // in KHz
#define FREQ_OSC           32000L       // in KHz   //was 10000 changed to 20000 1520Q
#define FREQ_MOD              ((FREQ_OSC / FREQ_STEP) / RFdiv)
//#define REFinMUL 		 2L

//========== power =============================================
UINT low_power_level, power_level, power_control;
byte bitlow_power_level,bitpower_level;// VERSION 3.3 VP VL Change 29.02.2016
UINT manual_negative = 0xFFFF;

#define A2D_POWER 5
#define A2D_PREV  4
#define A2D_TEMP  6
#define A2D_Vdd   0

#define DAC_POS_VOLT 0x2  //0010 - VOUT A
#define DAC_NEG_VOLT 0x6  //0110 - VOUT B
#define DAC_CONT_VOLT 0xA //1010 - VOUT C

#define FREQ_LOW_THRESH   22570
#define FREQ_HIGH_THRESH  23290

#define DEADBAND 6  //was 6 yehuda

#define BINARY_STATUS_LENTGH 26 // requested be the client to be 26 bytes (16.11.16)

UINT  manual_pos = 0xFFFF;

//========== timer =============================================
UCHAR  TMR_1mS_Flags;
//UCHAR  TMR_1mS_Count;
UCHAR  TMR_1mS_Cnt;
UCHAR  TMR_10mS_Count;
UCHAR  TMR_10mS_Flags;
UCHAR  TMR_10mS_Cnt;
UCHAR  TMR_100mS_Flags;
UCHAR  TMR_100mS_Cnt;
UCHAR  TMR_1sec_Flags;
UINT  TMR_SendStatus = 0;
UCHAR  SendStatusFlag = 0;
UCHAR  TMR_StartStatus = 0;
UCHAR  StartStatusFlag = 0;

UCHAR timerTick_1ms = 0;



//#bit TMR_1MS_KEYPAD     = TMR_1mS_Flags.0
#bit TMR_1MS_DELAY      = TMR_1mS_Flags.1

#bit TMR_100mS_BLINK    = TMR_100mS_Flags.0
#bit TMR_100MS_COMM_TO  = TMR_100mS_Flags.1
//#bit TMR_100MS_LEDS     = TMR_100mS_Flags.2
#bit TMR_100MS_POWER    = TMR_100mS_Flags.3

//#bit TMR_1sec_DISP_STAT =  TMR_1sec_Flags.0
//#bit TMR_1sec_RSSI      =  TMR_1sec_Flags.1
//#bit TMR_1Sec_TEMP      =  TMR_1sec_Flags.2

//============= I/O PIN assignments ============================

#define PLL_LAT  PIN_C1
#define PLL_CLK  PIN_C2
#define PLL_DAT  PIN_C3
#define PLL_LD   PIN_C0

#define CSN       PIN_B2  // FPGA

//#define RS485_EN  PIN_D5
#define RS485_EN  PIN_D5

//#define POWER_CTL PIN_D4

#define I_SENSE   PIN_B4     //was LED1
#define LED1      PIN_B5     //was LED2


#define D2A_RESET PIN_B3
#define D2A_CSB   PIN_B1
#define D2A_MOSI  PIN_D4
#define D2A_MISO  PIN_D1
#define D2A_SCLK  PIN_D0

#define POWER_EN PIN_D6
#define HILO     PIN_D2
#define STANDBY  PIN_D7
#define FPGA_RSTN PIN_C5

//========== COM1 variables ====================================
#define COM1_RX_LEN 32

UCHAR COM1_rcnt;
UCHAR COM1_rxi;
UCHAR COM1_rxo;
UCHAR COM1_rbuf[COM1_RX_LEN];

#define COMM_INIT       0
#define COMM_WAIT_DLR   1
#define COMM_WAIT_CR    2
#define COMM_DELAY      3


#define WAIT_ACK_TO 20 // 200mS wait for ack

UCHAR comm_state;
UCHAR comm_ridx;
UCHAR comm_buf[80];
UINT  comm_timeout;

//======= misc =================================================

//----------- setup ----------------------------------------
struct {
       UINT  bitrate;
	   UINT  pwr;
       UCHAR mode;
       UCHAR clock_polarity;
       UCHAR data_polarity;
       UCHAR clock_source;
       UCHAR data_source;
       UCHAR internal_pattern;
       UCHAR randomizer;
       UCHAR power_high;
       UCHAR SOQPSK;
       UCHAR power_amp;
       UINT  frequency;
       UINT  power_level;
       UCHAR phase_offset;
       UINT  negative_voltage[3];
	   UINT  cont_voltage[3];
       UINT  power_in[21];
       UINT  year;
       UCHAR week;
       UINT  unit_ID;
       UINT  power_low_level;
       UINT  cot;
       UINT  rc;
       UINT  rp;
       UCHAR UART_Time; // VERSION 3.3 UT
       UCHAR UART_Status; // VERSION 3.3 US 17.03.2016
       byte bitlow_power_level;// VERSION 3.3  VL Change 21.03.2016
       byte bitpower_level;// VERSION 3.3 VP Change 21.03.2016
       UINT	Block_per_second; // VERSION 3.3  Change 23.03.2016 Data block send speed 
       UINT setup_version; // the version of this specific setup
} setup;

UINT allow_write = 0;
UINT revp;// VERSION 3.3 Revers Power 0-good 1-bad
SINT current_temperature;
UINT current_power;// VERSION 3.3  FFWR 02.05.16
UINT gl_current_power_level = 0;
UINT gl_current_power_en_value = 0;
SINT stay_on =0;
UINT  tx_block_len , TX_Counter = 0;
UINT  status_tx_index;
UCHAR status_buffer[BINARY_STATUS_LENTGH]; 

UCHAR ttccp_login = 1;		//yehuda cancel LI17592


//========== function prototypes ===============================
void set_bitrate(UINT bitrate);
void send_FPGA_command(UCHAR length, UCHAR *data);
UCHAR get_FPGA_register(UCHAR addr, UCHAR *data);
UCHAR read_D2A(UCHAR addr);
UCHAR write_D2A(UCHAR addr, UCHAR din);
void read_setup(void);
void write_setup(void);
void update_all(void);
void update_FPGA(UCHAR addr, UCHAR value);
void FPGA_set_reg0(void);
void FPGA_set_reg6(void);
void FPGA_set_bitrate(void);
void init_system(void);
void power_output(void);
void COM1_send_block(UINT len);// VERSION 3.3 VP Change 21.03.2016
UINT convert_power(UINT analog);// VERSION 3.3 VP Change 30.03.2016

//========== include source files ==============================
#include "AD5314.c"
#include "AD9746.c"
#include "ADRF6703.c"
#include "BA1520QTX_intr.c"
#include "BA1520QTX_serial.c"


//========== functions =========================================
// // VERSION 3.3 US 30.03.2016
//========== functions =========================================
typedef struct {
  UINT analog;
  UINT Pout;
} POWER_TRANS;

const POWER_TRANS Ptrans[] = {

//  dec    vmeas    Pout
{  1  ,  998  },//  ,1
{  2  ,  500  },//  ,2
{  3  ,  353  },//  ,3
{  4  ,  260  },//  ,4
{  5  ,  210  },//  ,5
{  6  ,  170  },//  ,6
{  7  ,  144  },//  ,7
{  8  ,  124  },//  ,8
{  9  ,  110  },//  ,9
{  10  , 99  },// ,10
{  11  ,  88  },//  ,11
{  12  ,  79  },//  ,12
{  13  ,  72  },//  ,13
{  14  ,  67  },//  ,14
{  15  ,  62  },//  ,15
{  16  ,  57  },//  ,16
{  17  ,  52  },//  ,17
{  18  ,  50  },//  ,18
{  19  ,  46  },//  ,19
{  20  ,  44  },//  ,20
};
//=============================================================================
// VERSION 3.3  04.052016 FFWR
//=============================================================================
typedef struct {
  UINT analog1;
  UINT Pout1;
} POWER_TRANS1;

const POWER_TRANS1 Ptrans1[] = {

//  dec    vmeas    Pout1
{  80  ,  20  },//  0.29V  ,0
{  102  ,  21  },//  0.33V  ,1
{  118  ,  22  },//  0.38V  ,2
{  127  ,  23  },//  0.41V  ,3
{  143  ,  24  },//  0.46V  ,4
{  167  ,  25  },//  0.54V  ,5
{  183  ,  26  },//  0.59V  ,6
{  208  ,  27  },//  0.67V  ,7
{  226  ,  28  },//  0.73V  ,8
{  257  ,  29  },//  0.83V  ,9
{  288  ,  30  },//  0.93V  ,10
{  322  ,  31  },//  1.04V  ,11
{  360  ,  32  },//  1.16V  ,12
{  400  ,  33  },//  1.29V  ,13
{  446  ,  34  },//  1.44V  ,14
{  505  ,  35  },//  1.63V  ,15
{  564  ,  36  },//  1.82V  ,16
{  645  ,  37  },//  2.08V  ,17
{  722  ,  38  },//  2.33V  ,18
{  818  ,  39  },//  2.64V  ,19
{  992  ,  40  },//  3.2V  ,20
{  1005,   41  }, //  3.2V    ,21
{   1023,   55  }
};

//=============================================================================
//=============================================================================
void send_FPGA_command(UCHAR length, UCHAR *data)
  {
  UCHAR xbyte, cnt;
//  output_high(D2A_SCLK);
//  delay_us(10);
  output_low(CSN);
  delay_us(10);
  while (length--)
    {
    xbyte = *data++;
    for (cnt = 0; cnt < 8; cnt++, xbyte <<= 1)
      {
      if (xbyte & 0x80)
        output_high(D2A_MOSI);
      else
        output_low(D2A_MOSI);
      delay_us(1);
      output_high(D2A_SCLK);
      delay_us(1);
      output_low(D2A_SCLK);
      delay_us(1);
      }
    }
  delay_us(10);
  output_high(CSN);
  }

//=============================================================================
UCHAR get_FPGA_register(UCHAR addr, UCHAR *data)
  {
  UCHAR xbyte, cnt;
  output_low(CSN);
  for (cnt = 0; cnt < 8; cnt++, addr <<= 1)
    {
    if (addr & 0x80)
      output_high(D2A_MOSI);
    else
      output_low(D2A_MOSI);
    delay_us(5);
    output_high(D2A_SCLK);
    delay_us(5);
    output_low(D2A_SCLK);
    delay_us(5);
    }
  for (xbyte = 0, cnt = 0; cnt < 8; cnt++)
    {
    xbyte <<= 1;
    delay_us(5);
    output_high(D2A_SCLK);
    delay_us(3);
    if (input(D2A_MISO))
      {
      xbyte |= 1;
      delay_us(2);
      }
    delay_us(2);
    output_low(D2A_SCLK);
    delay_us(3);
    }
  *data = xbyte;
  output_high(CSN);
  output_low(D2A_SCLK);
  return xbyte;
  }


//=============================================================================
//BR(31:0)=Round(2^32 * (bit rate)/240.0MHz)
ULONG compute_bitrate_coefficient(ULONG bitrate)
  {
  float bitspersec, temp;
  bitspersec = (float)bitrate;
  temp = bitspersec / 240000000.0;
  temp *= 65536.0;
  temp *= 65536.0;
  return (ULONG)temp - 1;
  }

//=============================================================================
void update_FPGA(UCHAR addr, UCHAR value)
  {
  UCHAR buf[3];
  buf[0] = addr;
  buf[1] = value;
  send_FPGA_command(2, buf);
  }

//=============================================================================
void set_bitrate(UINT bitrate)
  {
  UCHAR buf[7];
  ULONG bitf;
  bitf = compute_bitrate_coefficient((ULONG)bitrate * 10000L);
  buf[0] = 2;
  buf[1] = make8(bitf, 0);
  buf[2] = make8(bitf, 1);
  buf[3] = make8(bitf, 2);
  buf[4] = make8(bitf, 3);
  send_FPGA_command(5, buf);
  }

//=============================================================================
void FPGA_set_reg0(void)
  {
  UCHAR buf[8];
  buf[0] = 0;
  buf[1] = setup.mode |
           (setup.clock_polarity << 4) |
           (setup.data_polarity  << 5) |
           (setup.randomizer     << 6) |
           (setup.SOQPSK         << 7);
  send_FPGA_command(2, buf);
  }

//=============================================================================
void FPGA_set_reg6(void)
  {
  UCHAR buf[8];
  buf[0] = 6;
  buf[1] = setup.clock_source |
           (setup.data_source << 1) |
           (setup.internal_pattern << 2);
  send_FPGA_command(2, buf);
  }

//=============================================================================
void FPGA_set_bitrate(void)
  {
  UCHAR buf[8];
  ULONG bitf;
  bitf = compute_bitrate_coefficient((ULONG)setup.bitrate * 10000L);
  buf[0] = 2; buf[1] = make8(bitf, 0);
  send_FPGA_command(2, buf); delay_ms(10);
  buf[0] = 3; buf[1] = make8(bitf, 1);
  send_FPGA_command(2, buf); delay_ms(10);
  buf[0] = 4; buf[1] = make8(bitf, 2);
  send_FPGA_command(2, buf); delay_ms(10);
  buf[0] = 5; buf[1] = make8(bitf, 3);
  send_FPGA_command(2, buf); delay_ms(10);
  }

//=============================================================================
void set_synchronizer_params(void)
  {
  UCHAR buf[8], idx;
  ULONG bitf;
  bitf = compute_bitrate_coefficient((ULONG)setup.bitrate * 10000L);
  buf[0] = setup.mode |
/*           (setup.clock_polarity << 4) |*/
           (setup.data_polarity  << 5) |
           (setup.randomizer     << 6) |
           (setup.SOQPSK         << 7);
  buf[1] = setup.phase_offset;
  buf[2] = make8(bitf, 0);
  buf[3] = make8(bitf, 1);
  buf[4] = make8(bitf, 2);
  buf[5] = make8(bitf, 3);
  buf[6] = setup.clock_source |
           (setup.data_source << 1) |
           (setup.internal_pattern << 2);
  for (idx = 0; idx < 7; idx++)
    {
    update_FPGA(idx, buf[idx]);
    delay_ms(5);
    }
  }

//=============================================================================
void write_int_eeprom(UINT addr, UCHAR *data, UINT size)
  {
  while (size--)
    write_eeprom(addr++, *data++);
  }

//=============================================================================
void read_int_eeprom(UINT addr, UCHAR *data, UINT size)
  {
  while (size--)
    *data++ = read_eeprom(addr++);
  }

//=============================================================================
void write_setup(void)
  {
  if (allow_write == 2975)
    write_int_eeprom(0, &setup, sizeof(setup));
  allow_write = 0;
  }

//=============================================================================
void read_setup(void)
  {
  read_int_eeprom(0, &setup, sizeof(setup));
  if (setup.clock_source == 0xFF || setup.data_source == 0xFF)
    memset(&setup, 0, sizeof(setup));
  }

//--------------------------------------------------------------
//=============================================================================
// VERSION 3.3 US 30.03.2016
//=============================================================================
UINT convert_power(UINT analog)
  {
  UINT idx;
  for (idx = 0; idx < 20; idx++)
    {
  if (analog >= Ptrans[idx].analog && analog < Ptrans[idx+1].analog)
    return Ptrans[idx].Pout;
  }
  return 0;
  }

//=============================================================================
// VERSION 3.3  04.052016
//=============================================================================
UINT convert_power1(UINT analog)// FFWR POWER
  {
  UINT idx;
  for (idx = 0; idx < 20; idx++)
    {
  if (analog >= Ptrans1[idx].analog1 && analog < Ptrans1[idx+1].analog1)
    return Ptrans1[idx].Pout1;
  }
  return 0;
  }

//=============================================================================
// this functions sets the POWER_EN discrete high / low
// depanding on the state of 1 discrete (STANDBY) and 2 variables
// (setup.cot & setup.pwr).
// the function also calls 'update_all' when a transition from 
// low to high occurs.
// the truth table for setting the discrete is:

// SB cot pwr Output
// 0   0   0    H
// 0   0   1    L
// 0   1   0    L
// 0   1   1    H
// 1   0   0    L
// 1   0   1    H
// 1   1   0    H
// 1   1   1    L

// the reduction of this function is: 
// not(sb^cot^pwr) or
// sb ^ (cot == pwr) (used below)
//=============================================================================
void set_power_en() 
{
    static UINT last_val = 0; // saving the last state of the discrete
    UINT dval = input(STANDBY) ^ (setup.cot == setup.pwr);

    // set the discrete
    if(dval) 
    {
        output_high(POWER_EN);
    }
    else
    {
        output_low(POWER_EN);       //yehuda shut down the fpga
        if(setup.cot == 1) 
        {
            // this was a special case in before the refactoring
            // it is not known why the call is only when cot == 1
            set_AD5314(DAC_POS_VOLT, 0);    
        }
    }

    // call update all on low to high transition
    if(last_val == 0 && dval == 1) 
    {
        delay_ms(50);
        update_all();    ////tamir 3/7/18 
    }
    last_val = dval;
    gl_current_power_en_value = dval;
}

//=============================================================================
// this function returns the requested power level (SV / VL) based on 
// 3 parameters, rc, rp and the discrete HILOW
// in high power mode (rc=1), if the discrete & rp are equal the function should
// return SV (power_level) if not, it should return VL (low_power_level).
// in low power mode (rc=0), if the discrete & rp are equal the function should
// return VL (low_power_level) if not, it should return SV (power_level).
//=============================================================================
UINT get_requested_power_level() {
    UINT levels[2];
    int l;
    levels[0] = power_level;
    levels[1] = low_power_level;

    l = (setup.rc & 0x1) ^ (input(HILO) == setup.rp);
    
    gl_current_power_level = levels[l];
    return levels[l];
}

//=============================================================================
void power_output(void)
  {
  UINT power, level;
  if (manual_pos != 0xFFFF)
    {
    if (TMR_100MS_POWER)
      {
      TMR_100MS_POWER = 0;
      set_AD5314(DAC_POS_VOLT, manual_pos);
      set_adc_channel(A2D_POWER); // select forward power input A2D_POWER
      delay_us(20);
      power = read_adc();
      current_power = convert_power1(power);
      }


    return;
    }
    
  if (setup.frequency < FREQ_LOW_THRESH){
    set_AD5314(DAC_NEG_VOLT, setup.negative_voltage[0]);
	delay_ms(1);
	set_AD5314(DAC_CONT_VOLT, setup.cont_voltage[0]);
	}
  else if (setup.frequency < FREQ_HIGH_THRESH){
    set_AD5314(DAC_NEG_VOLT, setup.negative_voltage[1]);
	delay_ms(1);
	set_AD5314(DAC_CONT_VOLT, setup.cont_voltage[1]);
	}
  else{
    set_AD5314(DAC_NEG_VOLT, setup.negative_voltage[2]);
	delay_ms(1);
	set_AD5314(DAC_CONT_VOLT, setup.cont_voltage[2]);
	}

  set_adc_channel(A2D_POWER); // select forward power input A2D_POWER
  delay_us(20);
  power = read_adc();
 

  current_power = convert_power1(power);
  set_power_en();     
  
  level = get_requested_power_level();


  if (power > level + DEADBAND || power < level - DEADBAND)
    {
    if (power < level)
      {
      if (power_control <= 1010)
        power_control += DEADBAND / 2;		
      }
    else if (power_control >= 150)			
      {
      power_control -= DEADBAND / 2;
      }
    set_AD5314(DAC_POS_VOLT, power_control);
    }	
}


//=============================================================================
#separate
void delay_mst(UINT delay)
  {
  TMR_1MS_DELAY = 0;
  while (delay)
    {
    if (TMR_1MS_DELAY)
      {
      TMR_1MS_DELAY = 0;
      delay--;
      }
    }
  }

//=============================================================================
void init_io_ports(void)
  {
  output_a(0);
  output_b(0);
  output_c(0);
  output_d(0);
  output_e(0);
  set_tris_a(0b11100001);
  set_tris_b(0b11010001);    
  set_tris_c(0b11010001);
  set_tris_d(0b10001110);
  set_tris_e(0b11111011);
  }

//=============================================================================
void init_system(void)
  {

  setup_timer_2(T2_DIV_BY_4,99,10);    // 1.0 ms interrupt

  setup_timer_3(T3_DISABLED | T3_DIV_BY_1);

  setup_timer_4(T4_DIV_BY_4,99,1);     // 100 us interrupt

  setup_timer_5(T5_DISABLED | T5_DIV_BY_1);

  setup_timer_6(T6_DISABLED,0,1);

  init_io_ports();

  setup_adc_ports(sAN0|sAN4|sAN5|sAN6|sAN11);
  setup_adc(ADC_CLOCK_DIV_16|ADC_TAD_MUL_8);

  setup_comparator(NC_NC_NC_NC);

  COM1_init();
  enable_interrupts(INT_TIMER2);
  enable_interrupts(GLOBAL);
  }


//=============================================================================
void update_all(void)
  {
  UINT freq, bitr;
  freq = setup.frequency;
  delay_ms(5);
  PLL_compute_freq_parameters(freq);
  delay_ms(5);
  PLL_update();
  delay_ms(5);
  FPGA_set_reg0();
  delay_ms(5);
  FPGA_set_reg6();
  delay_ms(5);
  FPGA_set_bitrate();
  bitr = setup.bitrate;
  power_control = 0;
  }

void timer_tick() 
{
    if(timerTick_1ms == 1) {
        timerTick_1ms = 0;
        if (++TMR_SendStatus >= (setup.Block_per_second / 2) && StartStatusFlag == 1 && setup.UART_Status == 1&& (stay_on == 1 || stay_on == 0)) // VERSION 3.3 17.1.2016
        {
            TMR_SendStatus = 0;
            SendStatusFlag = 1;
            TX_Counter++; //VERSION 3.3 21.3.2016 count the number of times the status block is sent
            if (!ttccp_login)
            {
              	COM1_send_block(BINARY_STATUS_LENTGH);
            }
        }
        if (++TMR_1mS_Cnt >= 10)
        {
            TMR_1mS_Cnt = 0;
            ++TMR_SendStatus; //1mSec count for rhe status VERSION 3.3 20.2.2016
            TMR_10mS_Count++;
            TMR_10mS_Flags = 0xFF;
            if (++TMR_10mS_Cnt >= 10)
            {
                TMR_10mS_Cnt = 0;
                TMR_100mS_Flags = 0xFF;
                if (++TMR_100mS_Cnt >= 10)
                {
                    TMR_100mS_Cnt = 0;
                    TMR_1sec_Flags = 0xFF;
                    if (++TMR_StartStatus >= setup.UART_Time) // VERSION 3.3 17.1.2016
           			{
           			    TMR_StartStatus = 0;
            			StartStatusFlag = 1;
            		}
                }
            }
        }
    }
}


//=============================================================================
void main(void)
  {
  int16 vouta = 1000;
output_high(POWER_EN);
delay_ms(1000);
  init_system(); 
  output_high(D2A_CSB);
  delay_ms(100);
  read_setup();
  
  if (!setup.UART_Time)// VERSION 3.3: 10.2.2016 
		setup.UART_Time = 10;
	if (setup.Block_per_second == 0)
    setup.Block_per_second = 100;
	//setup.UART_Status = 1;// VERSION 3.3: 10.2.2016 
  power_level = setup.power_in[setup.power_level];
  low_power_level = setup.power_in[setup.power_low_level];
  power_control = 0;
  power_output();
  

  PLL_initialize();
  delay_ms(50);
  update_all();

  output_high(D2A_RESET);
  delay_ms(50);
  output_low(D2A_RESET);
  

  COM1_send_str("\r\n");
  COM1_send_str(VERSION);
  COM1_send_str("\r\n");

  set_AD5314(DAC_POS_VOLT, vouta);


  delay_ms(500);
  output_high(POWER_EN);

update_all();
output_high(FPGA_RSTN);

#ignore_warnings 203
  while (1)
    {
    restart_wdt();
    timer_tick();
    if (TMR_100mS_BLINK)
      {
      TMR_100mS_BLINK = 0;
      output_toggle(LED1);
      delay_us(1);
      }
    
    if (StartStatusFlag == 1 && (COM1_rxo == 0) && (stay_on == 1 || stay_on == 0))
    	{        
				if (setup.UART_Status == 1)
		      {
			      stay_on = 1;
		      //StartStatusFlag = 0;
		      //if (!ttccp_login)
		        prepare_binary_status();		       
		      } 
		     
       }
 		if (stay_on == 2 || stay_on == 0) //US1 or US0 StartStatusFlag == 0  (StartStatusFlag == 1 && setup.UART_Status == 0)&& 
 			{  
		   
		   			comm_handler();
		   			if (COM1_rxo != 0 )
		   				stay_on = 2;
		  }    
		power_output();
		delay_ms(1);		//yehuda



   }
  }

