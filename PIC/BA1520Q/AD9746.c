//=============================================================================
UCHAR read_D2A(UCHAR addr)
{
	UCHAR dout, ix;
	output_low(D2A_CSB);
	addr |= 0x80;
	for (ix = 0; ix < 8; ix++, addr <<= 1)
    {
		output_bit(D2A_MOSI, (addr & 0x80) != 0);
		delay_us(1);
		output_high(D2A_SCLK);
		delay_us(1);
		output_low(D2A_SCLK);
		delay_us(1);
	}
	for (ix = 0; ix < 8; ix++)
    {
		output_high(D2A_SCLK);
		delay_us(1);
		output_low(D2A_SCLK);
		delay_us(1);
		dout <<= 1;
		if (input(D2A_MISO))
		dout |= 1;
	}
	output_high(D2A_CSB);
	return dout;
}

//=============================================================================
UCHAR write_D2A(UCHAR addr, UCHAR din)
{
	UCHAR dout, ix;
	output_low(D2A_CSB);
	//  addr |= 0x80;
	for (ix = 0; ix < 8; ix++, addr <<= 1)
    {
		output_bit(D2A_MOSI, (addr & 0x80) != 0);
		delay_us(1);
		output_high(D2A_SCLK);
		delay_us(1);
		output_low(D2A_SCLK);
		delay_us(1);
	}
	for (ix = 0; ix < 8; ix++, din <<= 1)
    {
		output_bit(D2A_MOSI, (din & 0x80) != 0);
		delay_us(1);
		output_high(D2A_SCLK);
		delay_us(1);
		output_low(D2A_SCLK);
		delay_us(1);
	}
	output_high(D2A_CSB);
	return dout;
}

