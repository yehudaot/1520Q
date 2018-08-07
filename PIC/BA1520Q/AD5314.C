
#define SW_SPI

#define DA2_CLK   PIN_A3 // AD5314
#define DA2_DAT   PIN_A4
#define DA2_SYN   PIN_A2
#define DA2_LAT   PIN_A1

void set_AD5314(int8 channel, int16 level)
{
	output_low(DA2_SYN);
	
//	if (channel)
//    output_high(DA2_DAT);    // send to VOUTB
//	else
//    output_low(DA2_DAT);    // send to VOUTA
//channel is 4 bit wide 
//bit 3 - address A1
//bit 2 - address A0
//bit 1 - if 1 normal operation, if 0 power down (PD)
//bit 0 - if 1 only address input regs updated no change in the content, if 0 all updated on complition of write
	
	#ifdef SW_SPI
		if(channel & 0x08)			//send bit 15 - A1
		output_high(DA2_DAT);
		else 
		output_low(DA2_DAT);
		output_high(DA2_CLK);  
		delay_cycles(15);
		output_low(DA2_CLK);
		delay_cycles(15);
		
		if(channel & 0x04)			//send bit 14 - A0
		output_high(DA2_DAT);
		else 
		output_low(DA2_DAT);
		output_high(DA2_CLK);
		delay_cycles(15);
		output_low(DA2_CLK);
		delay_cycles(15);
		
		if(channel & 0x02)			//send bit 13 - PD - if 1 normal operation, if 0 power down (PD)
		output_high(DA2_DAT);
		else 
		output_low(DA2_DAT);
		output_high(DA2_CLK);
		delay_cycles(15);
		output_low(DA2_CLK);
		delay_cycles(15);
		
		if(channel & 0x01)			//send bit 12 - LDAC - if 1 only address input regs updated no change in the content, if 0 all updated on complition of write
		output_high(DA2_DAT);
		else 
		output_low(DA2_DAT);
		output_high(DA2_CLK);
		delay_cycles(15);
		output_low(DA2_CLK);
		delay_cycles(15);
		
		
		output_high(DA2_CLK);    	//send bit 11 DATA
		if (level & 0x0200)
		output_high(DA2_DAT);
		else
		output_low(DA2_DAT);
		delay_cycles(15);
		output_low(DA2_CLK);
		delay_cycles(15);
		
		output_high(DA2_CLK);  		//send bit 10 DATA
		if (level & 0x0100)
		output_high(DA2_DAT);
		else
		output_low(DA2_DAT);
		output_low(DA2_CLK);
		
		output_high(DA2_CLK);		//send bit 9 DATA
		if (level & 0x0080)
		output_high(DA2_DAT);
		else
		output_low(DA2_DAT);
		delay_cycles(15);
		output_low(DA2_CLK);
		delay_cycles(15);
		
		output_high(DA2_CLK);		//send bit 8 DATA
		if (level & 0x0040)
		output_high(DA2_DAT);
		else
		output_low(DA2_DAT);
		delay_cycles(15);
		output_low(DA2_CLK);
		delay_cycles(15);
		
		output_high(DA2_CLK);		//send bit 7 DATA
		if (level & 0x0020)
		output_high(DA2_DAT);
		else
		output_low(DA2_DAT);
		output_low(DA2_CLK);
		
		output_high(DA2_CLK);		//send bit 6 DATA
		if (level & 0x0010)
		output_high(DA2_DAT);
		else
		output_low(DA2_DAT);
		delay_cycles(15);
		output_low(DA2_CLK);
		delay_cycles(15);
		
		output_high(DA2_CLK);		//send bit 5 DATA
		if (level & 0x0008)
		output_high(DA2_DAT);
		else
		output_low(DA2_DAT);
		output_low(DA2_CLK);
		
		output_high(DA2_CLK);		//send bit 4 DATA
		if (level & 0x0004)
		output_high(DA2_DAT);
		else
		output_low(DA2_DAT);
		delay_cycles(15);
		output_low(DA2_CLK);
		delay_cycles(15);
		
		output_high(DA2_CLK);		//send bit 3 DATA
		if (level & 0x0002)
		output_high(DA2_DAT);
		else
		output_low(DA2_DAT);
		output_low(DA2_CLK);
		
		output_high(DA2_CLK);		//send bit 2 DATA
		if (level & 0x0001)
		output_high(DA2_DAT);
		else
		output_low(DA2_DAT);
		delay_cycles(15);
		output_low(DA2_CLK);
		delay_cycles(15);
		
		
		output_high(DA2_CLK);		//send bit 1 X dont care
		delay_cycles(15);
		output_low(DA2_CLK);
		
		output_high(DA2_CLK);		//send bit 0 X dont care
		delay_cycles(15);
		delay_cycles(15);
		output_low(DA2_CLK);
		delay_cycles(15);
		#else						//this else is for #ifdef SW_SPI above  
		level = (level << 2) | 0x8000;
		SPI2BUF = (AD5314,make8(level,1));
		delay_cycles(15);
		SPI2BUF = (AD5314,make8(level,0));
		delay_cycles(15);
	#endif
	
	output_high(DA2_SYN);
	delay_cycles(15);
	output_low(DA2_LAT);
	delay_cycles(15);
	output_high(DA2_LAT);
	delay_cycles(15);
}
