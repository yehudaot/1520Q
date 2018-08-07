/*
	REGISTERS FRO PLL ADF4350:
	Flow= 2185MHz
	Fhigh=2400MHz
	Steps: 100KHz
	Ref : 10MHz
	Phase Detector: 10MHz
	
	REG0 = Calculated
	REG1= Calculated
	REG2=0x000002
	REG3=0x70000b
	REG4=0x02a7a4
	REG5=0x0000e5
	REG6=0x0000e5
	REG7=0x1edd06
	REg8=0x000007
	
	Examples for REG0, REG1:
	2185MHz
	REG0:DA8000
	REG1: 8008191
	
	2300MHz
	REG0:E60000
	REG1: 8008191
	
*/


//--------- ADF4350 handling functions -------------------------

//ULONG  PLL_int, PLL_frac, PLL_mod, PLL_frequency, PLL_RFdiv;
ULONG  PLL_int, PLL_frac, PLL_mod;
/*
	#define PLL_CLK       PIN_C2
	#define PLL_DAT       PIN_C3
	#define PLL_LAT       PIN_C1
*/

void PLL_send_bits(UCHAR data)
{
	UCHAR count;
	for (count = 8; count ; count--, data <<= 1)
    {
		if (data & 0X80)                
		output_high(PLL_DAT);
		else
		output_low(PLL_DAT);
		delay_us(10);
		output_high(PLL_CLK);
		delay_us(10);
		output_low(PLL_CLK);
		
		delay_us(25);
	}
	output_low(PLL_DAT);
	output_low(PLL_CLK);
}

void PLL_send_word(ULONG prog)
{
	restart_wdt();
	disable_interrupts(GLOBAL);
	//PLL_send_bits(make8(prog, 3));               // send LSB
	PLL_send_bits(make8(prog, 2));               // send LSB
	PLL_send_bits(make8(prog, 1));               // send middle byte
	PLL_send_bits(make8(prog, 0));               // send MSB
	delay_us(40);
	output_high(PLL_LAT);                 // latch 24 bits into register
	delay_us(5);
	output_low(PLL_LAT);                 // latch 24 bits into register
	output_low(PLL_DAT);                 // latch 24 bits into register
	enable_interrupts(GLOBAL);
	restart_wdt();
}

//                                    R0          R1            R2          R3           R4           R5
//const ULONG PLL_REGISTERS[6] = { 0x00DC0000L, 0x08008191L, 0x18005EC2L, 0x000004B3L, 0x0095003CL, 0x00400000L };
//const ULONG PLL_REGISTERS[8] = { 0x0001e0L, 0x003201L, 0x000002L, 0x70000bL, 0x02a7a4L, 0x0000e5L, 0x1edd06L, 0x000007L };
const ULONG PLL_REGISTERS[8] = { 0x0001c8L, 0x000c81L, 0x000642L, 0x70000bL, 0x02a7a4L, 0x0000e5L, 0x1edd06L, 0x000007L };

void PLL_initialize(void)
{
	ULONG prog;
	UCHAR idx, count;
	for (count = 0, idx = 7; count < 8; count++, idx--)
    {
		prog = PLL_REGISTERS[idx];
		prog |= (ULONG)idx;
		PLL_send_word(prog);
	}
	delay_ms(250);
	PLL_send_word(prog);
	delay_ms(250);
}

//--------- frequency computation functions --------------------
//the ADRF software use clock 38.4 clock, here i use 20.0 clock
void PLL_compute_freq_parameters(UINT Frequest)
{
	ULONG Fvco, D, RFdiv = 2;
	restart_wdt();
	D = Frequest;
	if (Frequest >= 22000)
    {
		RFdiv = 1;
	}
	restart_wdt();
	D = D * RFdiv * FREQ_STEP;
	restart_wdt();
	PLL_int = D / FREQ_OSC;
	restart_wdt();
	PLL_frac = (D - (PLL_int * FREQ_OSC)) / (FREQ_STEP * RFdiv);
	PLL_mod = (FREQ_MOD << 3) + 1L;
	restart_wdt();
	PLL_int *= 8L;
	PLL_frac *= 8L;
	PLL_frac += 2L;
	Fvco++;
}


void PLL_update(void)
{
	//ULONG prog;
	restart_wdt();
	PLL_send_word(PLL_int);
	delay_us(20);
	restart_wdt();
	PLL_send_word(PLL_mod);
	delay_us(20);
	restart_wdt();
	PLL_send_word(PLL_frac);
	
}

#separate
void PLL_reset(void)
{
	output_low(PLL_DAT);
	output_low(PLL_CLK);
	delay_us(1000);
	PLL_initialize();
}


