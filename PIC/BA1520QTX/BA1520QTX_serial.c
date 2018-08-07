

UCHAR comm_ptr, ttccp = 1, debug_mode = 1;    //yehuda 1520Q cancel LI17592
UCHAR FPGA_image[32],D2A_image[20];
UCHAR ttccp_error_message[40];

//UCHAR rp_command = setup.rp ,  pwr_command = setup.pwr;
UCHAR rp_command,  pwr_command;
//bit revp_status; // VERSION 3.3: 2.2.2016   //////////yehuda 1520Q cancel US,UT and BG 

//-----------------------------------------------------------------------------
UCHAR COM1_get_chr(void)
{
	UCHAR x;
	x = COM1_rbuf[COM1_rxo];
	if (++COM1_rxo >= COM1_RX_LEN)
    COM1_rxo = 0;
	if (COM1_rcnt)
    COM1_rcnt--;
	return x;
}

//-----------------------------------------------------------------------------
void COM1_send_str(UCHAR *str)
{
	UCHAR x, pos, tab_stop;
	disable_interrupts(int_RDA);
	output_high(RS485_EN);
	delay_us(100);
	pos = 0;
	while (*str)
    {
		x = *str++;
		if (x != '\t')
		{
			TXREG1 = x;
			pos++;
			delay_us(300); 
			x = RCREG1;
		}
		else
		{
			tab_stop = 32;
			if (pos >= tab_stop)
			tab_stop = pos + 2;
			while (pos < tab_stop)
			{
				TXREG1 = ' ';
				pos++;
				delay_us(300);
				x = RCREG1;
			}
		}
	}
	delay_us(100);
	output_low(RS485_EN);
	x = RCREG1;
	enable_interrupts(int_RDA);
}

//-----------------------------------------------------------------------------
void COM1_send_block(UINT len) // VERSION 3.3: 17.1.2016 
{
	UCHAR pos;//, tab_stop;
	pos = 0;
	tx_block_len = len ;
	//status_tx_index = 0;
	
	//TXREG1 = status_buffer[0];
	clear_interrupt(int_TBE);
	enable_interrupts(int_TBE);
	//TXREG1 = status_buffer[0];
	//output_high(RS485_EN); 
}
//-----------------------------------------------------------------------------
void COM1_init(void)
{
	ttccp_error_message[0] = 0;
	COM1_rxi = COM1_rxo = COM1_rcnt = 0;
	comm_state = COMM_INIT;
	enable_interrupts(int_RDA);
}

//----------------------------------------------------------------------------
#separate
UINT get_char(void)
{
	return comm_buf[comm_ptr++];
}

//----------------------------------------------------------------------------
#separate
UINT peek_char(void)
{
	//  skip_spc();
	return comm_buf[comm_ptr];
}

//----------------------------------------------------------------------------
#separate
void skip_spc(void)
{
	while (comm_buf[comm_ptr] && (comm_buf[comm_ptr] == ',' || comm_buf[comm_ptr] == ' '))
    comm_ptr++;
}

//----------------------------------------------------------------------------
#separate
SINT get_int(void)
{
	SINT num, sign = 1;
	skip_spc();
	if (comm_buf[comm_ptr])
    {
		num = 0;
		if (peek_char() == '-')
		{
			sign = -1;
			get_char();
		}
		while (isdigit(comm_buf[comm_ptr]))
		num = (num * 10) + (comm_buf[comm_ptr++] - '0');
	}
	//  skip_spc();
	return num * sign;
}

//----------------------------------------------------------------------------
#separate
ULONG str_to_long(void)
{
	ULONG num;
	skip_spc();
	if (comm_buf[comm_ptr])
    {
		num = 0;
		while (isdigit(comm_buf[comm_ptr]))
		num = (num * 10) + (comm_buf[comm_ptr++] - '0');
	}
	//  skip_spc();
	return num;
}

//----------------------------------------------------------------------------
#separate
ULONG get_hex(void)
{
	ULONG num;
	UCHAR chr;
	skip_spc();
	if (peek_char())
    {
		num = 0;
		while (isxdigit(peek_char()))
		{
			chr = get_char();
			chr = toupper(chr);
			if (chr <= '9')
			chr -= '0';
			else
			chr = chr - ('A' - 10);
			num = num * 16 + (ULONG)chr;
		}
	}
	return num;
}

//----------------------------------------------------------------------------
UINT get_frequency(void)
{
	UINT freq;
	freq = get_int() * 10;
	if (peek_char() == '.')
    {
		get_char(); // skip '.'
		freq += get_char() - '0';
	}
	return freq;
}

//----------------------------------------------------------------------------
void inc_dec(UCHAR chr)
{
	UINT incdec;
	switch (chr)
    {
		case 'i':
		if (++D2A_image[11] == 0)
        if (++D2A_image[12] > 3)
		{
			D2A_image[11] = 0;
			D2A_image[12] = 0;
		}
		write_D2A(11, D2A_image[11]);
		write_D2A(12, D2A_image[12]);
		break;
		case 'I':
		if (++D2A_image[15] == 0)
        if (++D2A_image[16] > 3)
		{
			D2A_image[15] = 0;
			D2A_image[16] = 0;
		}
		write_D2A(15, D2A_image[15]);
		write_D2A(16, D2A_image[16]);
		break;
		case 'd':
		if (--D2A_image[11] == 255)
        if (--D2A_image[12] > 3)
		{
			D2A_image[11] = 255;
			D2A_image[12] = 3;
		}
		write_D2A(11, D2A_image[11]);
		write_D2A(12, D2A_image[12]);
		break;
		case 'D':
		if (--D2A_image[15] == 255)
        if (--D2A_image[16] > 3)
		{
			D2A_image[15] = 255;
			D2A_image[16] = 3;
		}
		write_D2A(15, D2A_image[15]);
		write_D2A(16, D2A_image[16]);
		break;
		case '+':
		incdec = make16(D2A_image[14],D2A_image[13]) & 0x3FF;
		if (++incdec > 0x3FF)
        incdec = 0;
		incdec |= (UINT)(D2A_image[14]  & 0xC0) << 8;
		D2A_image[13] = make8(incdec, 0);
		D2A_image[14] = make8(incdec, 1);
		write_D2A(11, D2A_image[13]);
		write_D2A(12, D2A_image[14]);
		break;
		case '=':
		incdec = make16(D2A_image[18],D2A_image[17]) & 0x3FF;
		if (++incdec > 0x3FF)
        incdec = 0;
		incdec |= (UINT)(D2A_image[18]  & 0xC0) << 8;
		D2A_image[17] = make8(incdec, 0);
		D2A_image[18] = make8(incdec, 1);
		write_D2A(11, D2A_image[17]);
		write_D2A(12, D2A_image[18]);
		break;
		case '-':
		incdec = make16(D2A_image[14],D2A_image[13]) & 0x3FF;
		if (--incdec > 0x3FF)
        incdec = 0x3FF;
		incdec |= (UINT)(D2A_image[14]  & 0xC0) << 8;
		D2A_image[13] = make8(incdec, 0);
		D2A_image[14] = make8(incdec, 1);
		write_D2A(11, D2A_image[13]);
		write_D2A(12, D2A_image[14]);
		break;
		case '_':
		incdec = make16(D2A_image[18],D2A_image[17]) & 0x3FF;
		if (--incdec > 0x3FF)
        incdec = 0x3FF;
		incdec |= (UINT)(D2A_image[18]  & 0xC0) << 8;
		D2A_image[17] = make8(incdec, 0);
		D2A_image[18] = make8(incdec, 1);
		write_D2A(11, D2A_image[17]);
		write_D2A(12, D2A_image[18]);
		break;
		case '>':
		if (setup.phase_offset < 255)
        setup.phase_offset++;
		update_FPGA(1, setup.phase_offset);
		break;
		case '<':
		if (setup.phase_offset)
        setup.phase_offset--;
		update_FPGA(1, setup.phase_offset);
		break;
	}
}

//----------------------------------------------------------------------------
void list_help1(void)
{
	COM1_send_str("\r\n");
	COM1_send_str("$AI <channel><cr>  \tRequest analog channel value (10 bits)\r\n");
	COM1_send_str("$AT <level><cr>  \tSet attenuator to <level>\r\n");
	COM1_send_str("$CF <freq><cr>  \tChange frequency (2185.0 - 2300.0 in 0.1 incs)\r\n");
	COM1_send_str("$DI<cr>  \tRequest digital inputs\r\n");
	COM1_send_str("$GD <addr><cr>  \tGet D2A register contents\r\n");
	COM1_send_str("$GF <addr><cr>  \tGet FPGA register contents\r\n");
	COM1_send_str("$OT <output><cr>  \tSet digital outputs to HEX value\r\n");
	COM1_send_str("$RD<cr>  \tReset D2A\r\n");
	COM1_send_str("$SD <addr> <data><cr>  \tSet D2A register <addr> to <data>\r\n");
	COM1_send_str("$SF <addr> <data><cr>  \tSet FPGA register <addr> to <data>\r\n");
	COM1_send_str("\r\n");
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void send_fail_message(void) // VERSION 3.3 17.1.2016
{
	//COM1_send_str("\r\nFAIL\r\n");
}


//----------------------------------------------------------------------------
float read_temperature(void) 
{
	float temp;
	SINT val;
	set_adc_channel(A2D_TEMP); // read temperature
	delay_us(50);
	val = read_adc();
	temp = (float)val / 1024.0 * 3.3;
	temp -= 0.75;
	temp *= 100;
	temp += 25.0;
	return temp;
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/*  //////////yehuda 1520Q cancel US,UT and BG  
SINT Bits_status1(void)// VERSION 3.3 21.1.2016
{
	// float temp;
	byte t = 0;
	t =          (bit)setup.data_source ;
	t = t + ( (bit)setup.data_polarity <<  1);
	t = t + ( (bit)setup.randomizer <<     2);
	t = t + ( (bit)setup.clock_source <<   3);
	t = t + ( (bit)setup.clock_polarity << 4);
	t = t + ( (bit)setup.SOQPSK <<         5);
	t = t + ( (bit)0 <<       6);
	t = t + ( (bit)0 <<       7);
	
	//  setup.SOQPSK, setup.randomizer, setup.data_polarity, rp, setup.data_source,
	return t;
}
*/
//----------------------------------------------------------------------------
/*  //////////yehuda 1520Q cancel US,UT and BG  
SINT Bits_status2(void)// VERSION 3.3 2.2.2016
{
	// float temp;
	set_adc_channel(A2D_PREV); // select forward power input
	delay_us(20);
	revp = read_adc();
	if (revp <= 580)
    revp_status = 0;// Q strcpy(revstat, "GOOD");
	else
	revp_status = 1;//Q strcpy(revstat, "BAD");
	
	byte t = 0;
	t =       (bit)setup.cot ; //RB
	t = t + ( (bit)setup.rc <<  1);
	t = t + ( (bit)gl_current_power_en_value << 2);//RF
	t = t + ( (bit)(gl_current_power_level == power_level) <<  3);
	t = t + ( (bit)setup.UART_Status << 4);
	t = t + ( (bit)revp_status <<       5);
	t = t + ( (bit)0 <<       6);
	t = t + ( (bit)0 <<       7);
	
	
	return t;
}
*/
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------  
SINT  set_power_level(void)
{
	UINT idx;
	idx = get_int();
	setup.bitpower_level = idx;// VERSION 3.3 29.02.2016
	if (idx <= 40 && idx >= 20)
    {
		setup.power_level = idx - 20;
		power_level = (setup.power_in[setup.power_level]); // * 33) / 50;
		return 1;
	}
	return 0;
}


//----------------------------------------------------------------------------
SINT  set_low_power_level(void)
{
	UINT idx;
	idx = get_int();
	setup.bitlow_power_level  = idx;// VERSION 3.3 29.02.2016
	if (idx <= 40 && idx >= 20)
    {
		setup.power_low_level = idx - 20;
		low_power_level = (setup.power_in[setup.power_low_level]); // * 33) / 50;
		return 1;
	}
	return 0;
}

void update_temperature_string() 
{
	if(setup.pwr)
	{
		sprintf(ttccp_error_message, "\rTEMP=%6.2fdeg\r\nOK\n", read_temperature());
	}
	else
	{
		COM1_send_str("\r\n\nNO TEMP-RF OFF \r\n");
	}
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// VERSION 3.3 17.1.2016
//----------------------------------------------------------------------------
/*
	The message structure is defined below.
	
	Byte# Description Remarks
	1 Header  Constant - 47 Hex
	2 Temperature Send 2's complement
	3 Power level
	4 Current
	5 PLL lock
	6 Checksum  Checksum of all preceding bytes
	
	Table 1 - Status Packet structure
	
	The status message consists of 6 serial bytes of 10 bits, for a total of 60 bits.
	Transmission time is 0.52mS, max PRI 10mS.
*/

/*  //////////yehuda 1520Q cancel US,UT and BG  
void prepare_binary_status(void)
{
	UINT chksum = 0, idx;
	SINT current_temperature;  
	power_output();
	byte BitsStatus1,BitsStatus2;
	BitsStatus1 = Bits_status1();
	BitsStatus2 = Bits_status2();
	
	current_temperature = (SINT)(read_temperature() * 100);
	//measured_current = read_current();
	//power_level_stat = current_power;
	status_buffer[0] = 0xFB;                              // Synchronization Byte 1
	status_buffer[1] = 0x60;                              // Synchronization Byte 2
	status_buffer[2] = TX_Counter;                        // The counter increments by one each time an update is sent 0 to 255
	status_buffer[3] = (setup.frequency >> 8) & 0xff;      // MSB TX frequency 
	status_buffer[4] = setup.frequency & 0xff;            // LSB TX frequency 
	status_buffer[5] = setup.mode;                        // State of modulation state
	status_buffer[6] = BitsStatus1;                       // Bits status of: spare, spare, Differential encoding ,Clock inverted ,Clock source ,Data randomization ,Data inverted, Data source
	if(setup.pwr && gl_current_power_en_value)
	{
		status_buffer[7] = (current_temperature >> 8) & 0xff;  // MSB TX Temperature 
		status_buffer[8] = current_temperature & 0xff;        // LSB TX Temperature 
	} 
	else 
	{
		// signal that the transmiter is off
		status_buffer[7] = 0x7f;
		status_buffer[8] = 0xff;
	}
	status_buffer[9] = (setup.bitrate >> 8) & 0xff;       // MSB Bit rate state
	status_buffer[10] = setup.bitrate & 0xff;             // LSB Bit rate state 
	status_buffer[11] = current_power;                    // TX power in dBm
	status_buffer[12] = setup.internal_pattern;           // Data type while using internal data
	status_buffer[13] = BitsStatus2;                      // Bits status of: Reverse Power N/A
	status_buffer[14] = setup.bitlow_power_level;         // TX low power in dBm (VL)
	status_buffer[15] = setup.bitpower_level;             // TX low power in dBm (VP)
	status_buffer[16] = revp/2;                           // Revers power div by 2 in order to make it 1 BIT
	status_buffer[17] = VER;                              // TX VERSION
	status_buffer[18] = (setup.unit_ID >> 8) & 0xff;      // MSB TX Serial number
	status_buffer[19] = setup.unit_ID & 0xff;             // LSB TX Serial number
	status_buffer[20] = (setup.setup_version >> 8)&0xff;  // MSB of the config version
	status_buffer[21] = setup.setup_version & 0xff;       // LSB of the config version
	status_buffer[22] = 0xAA;                             // Spare
	status_buffer[23] = 0xAA;                             // Spare
	
	for (idx = 0; idx < BINARY_STATUS_LENTGH-2; idx++)
	{
		chksum += status_buffer[idx];
	}
	// add the checksum to the end of the binary message, LSB first (request by the client 16.11.16)
	status_buffer[BINARY_STATUS_LENTGH-2] = chksum & 0xff;
	status_buffer[BINARY_STATUS_LENTGH-1] =(chksum >> 8) & 0xff;
}
*/


//----------------------------------------------------------------------------
// command format:
//   $<cmd> [<number>[,<number>]]<cr>
// where:
//    <cmd>       a two letter operation specifier
//    <number>    value to be used in operation
//    <cr>        0x0D character ending command string
//
UCHAR process_dollar_commands(void)
{
	UCHAR chr, idx, device, addr, data, buf[40];
	ULONG freq, bitrate;
	UINT  value;
	chr = 2;
	comm_ptr = 0;
	switch (toupper(get_char()))
    {
		case 'H':
		list_help1();
		return 0;
		case 'A':
		switch (toupper(get_char()))
        {
			case 'T':
			idx = get_int();
			
			// write to attenuator
			break;
			case 'I':
			idx = get_int();
			if (idx && idx < 4)
            {
				set_adc_channel(idx-1);
				delay_us(30);
				value = read_adc();
				sprintf(buf, "$AR %lu\r", value);
				COM1_send_str(buf);
			}
			break;
		}
		break;
		case 'C':
		chr = toupper(get_char());
		if (chr == 'F')
        {
			freq = get_frequency();
			PLL_compute_freq_parameters(freq);
			PLL_update();
		}
		else if (chr == 'P')
        {
			idx = get_int();
			if (idx < 2)
			{
				setup.clock_polarity = idx & 1;
				sprintf(buf, "$CP %u\r", idx);
				COM1_send_str(buf);
			}
		}
		break;
		case 'D':
		if (toupper(get_char()) == 'I')
        {
			idx = input_c(); // change to REAL input ports
			sprintf(buf, "$DR %02X\r", idx);
			COM1_send_str(buf);
		}
		break;
		case 'O':
		if (toupper(get_char()) == 'T')
        {
			value = get_hex();
			// output bits
		}
		break;
		case 'R':
		if (toupper(get_char()) == 'D')
        {
			output_high(D2A_RESET);
			delay_ms(100);
			output_low(D2A_RESET);
		}
		break;
		case 'B':             // bit rate
		if (toupper(get_char()) == 'R')
        {
			bitrate = str_to_long();
			set_bitrate(bitrate);
		}
		break;
		case 'G':
		device = get_char();
		addr = get_hex();
		switch (toupper(device))
        {
			case 'F':
			chr = get_FPGA_register(addr, &data);
			sprintf(buf, " \r\n$SF %02X %02X\r", addr, chr);
			COM1_send_str(buf);
			break;
			case 'D':
			data = read_D2A(addr);
			sprintf(buf, " \r\n$SD %02X %02X\r", addr, data);
			COM1_send_str(buf);
			break;
		}
		break;
		case 'F': // fill tables
		skip_spc();
		idx = get_char(); // get table designator
		addr = get_int(); // get table index
		value = get_int(); // get value to put into table
		switch (toupper(idx))
        {
			case 'N': // negative voltage
			if (addr < 3)
            {
				setup.negative_voltage[addr] = value;
			}
			break;
			case 'P': // positive voltage
			if (addr < 21)
            {
				setup.power_in[addr] = value;
			}
			break;
			case 'C': // cont voltage
			if (addr < 3)
            {
				setup.cont_voltage[addr] = value;
			}
			break;

		}
		break;
		case 'S':
		device = get_char();
		addr = get_hex();
		data = get_hex();
		switch (toupper(device))
        {
			case 'F':
			buf[0] = addr;
			buf[1] = data;
			FPGA_image[addr] = data;
			send_FPGA_command(2, buf);
			break;
			case 'D':
			D2A_image[addr] = data;
			write_D2A(addr, data);
			break;
			case 'V':
			allow_write = 2975;
			write_setup();
			break;
		}
		break;
		case 'T':
		if (toupper(get_char()) == 'T')
        if (toupper(get_char()) == 'C')
		if (toupper(get_char()) == 'C')
		if (toupper(get_char()) == 'P')
		ttccp = 1;
		break;
		case '1': // year
		setup.year = get_int();
		break;
		case '2': // week
		setup.week = get_int();
		break;
		case '3': // unit ID
		setup.unit_ID = get_int();
		break;



case 'W': 
		power_output();
		delay_ms(50);
		//ret = 1;
		update_all();
		break;



		default:
		return 0;
	}
	return 0;
}

//----------------------------------------------------------------------------
#separate
void get_new_bitrate(void)
{
	ULONG bitrate, sub;
	bitrate = get_int();//* 100;
	get_char();
	sub = get_int();
	
	if (peek_char() == '.')
	{
		get_char();
		sub = get_int();
		bitrate += sub;
	}
	
	setup.bitrate = bitrate * 100 + sub;
	//	setup.bitrate1= setup.bitrate;
	FPGA_set_bitrate();
	
}

//----------------------------------------------------------------------------
void list_help(void)
{
	COM1_send_str("\r\n\n");
	COM1_send_str("DS <data soure><cr>  \tSet data source (0-1)\r\n");
	COM1_send_str("DP <data polarity><cr>  \tSet data polarity (0-1)\r\n");
	COM1_send_str("DE <setup SOQPSK><cr>  \tSet setup SOQPSK (0-1)\r\n");
	COM1_send_str("RP <power height><cr>  \tSet power height (0-1)\r\n");
	COM1_send_str("RF <power comand><cr>  \tSet power comand (0-1)\r\n");
	COM1_send_str("RA <randomizer><cr>  \tSet with/without randomizer (0-1)\r\n");
	COM1_send_str("FR <frequency><cr>  \tSet frequency (2200.0-2400.0)\r\n");
	COM1_send_str("MO <mode><cr>  \tSet mode (0-3)\r\n");
	COM1_send_str("IC <bitrate><cr>  \tSet birtate (1.00-30.00DBps)\r\n");
	COM1_send_str("ID <internal pattern><cr>  \tSet internal pattern (0-3)\r\n");
	COM1_send_str("VE <cr>  \tdisplay version info\r\n");
	COM1_send_str("VS <major>.<minor> <cr>  \tset the setup file version\r\n");
	COM1_send_str("VP <power level><cr>  \tSet power level (20-40)\r\n");
	COM1_send_str("VL <power high><cr>  \tSet high power level (20-40)\r\n");
	COM1_send_str("VM <negative power level><cr>  \tSet negative power level\r\n");
	COM1_send_str("VC <posetive power level><cr>  \tSet posetive power level\r\n");
	COM1_send_str("CS <clock phase><cr>  \tSet clock phase (0-1)\r\n");
	//COM1_send_str("UT <UART Time><cr>  \tSet the stop time, default 15 (0-240)\r\n");// VERSION 3.3  21.03.2016
	//COM1_send_str("US <UART Status><cr>  \tSet the Block (0-1)\r\n");// VERSION 3.3  21.03.2016
	//COM1_send_str("BG <UART Change><cr>  \tSet the UART refresh rate [Hz] (1-20)\r\n");// VERSION 3.3  23.03.2016  //yehuda 1520Q remove BG command
	COM1_send_str("SV <save all><cr>  \tSave parameters\r");
	COM1_send_str("\r\n");
}

//----------------------------------------------------------------------------
#separate
void process_ttccp_commands(void)
{
	UCHAR ret = 0, chr, buf[40], query = 0, c1, c2, revstat[12], addr;
	ULONG freq;		//, sub;   //////////yehuda 1520Q cancel US,UT and BG 
	UINT  val, rp; //VERSION 3.3 us UINT  val, revp, rp;
	chr = 2;
	comm_ptr = 0;
    if (peek_char() == ':') // addressed message?
    {
		get_char(); // skip ':'
		addr = get_int();
		if (get_char() != ':')
		break;
		if (setup.unit_id != addr)
		goto aaa;
	}
    break;
	c1 = toupper(get_char());
	c2 = toupper(get_char());
	skip_spc();
	
	if (peek_char() == 13) // is this a query?
    query = 1; // YES
	switch (c1)
    {
		case 'L': // login or logout
		switch (c2)
        {
			case 'I':               // login
			val = get_int();
			if (val == 17592)
            ttccp_login = 1;
			debug_mode = 1;
			break;
			case 'O':              // logout
			ttccp_login = 0;
			break;
		}
		break;
		case '$':
//		if (!ttccp_login) break;
		if (c2 == 'R') // switch to $ commands ONLY after ttccp login
        {
			val = get_int();
			if (val != 17591)
            return;
			ttccp = 0;
			COM1_send_str("\r\n\n*");
		}
		break;
		
		/*  case ':':
			if(c2 == setup.unit_id+48)
			if(toupper(get_char()) == ':')
			comm_ptr = 2;
			break;
		*/
		case 'H':
//		if (!ttccp_login) break;
		COM1_send_str("\r\t HELP LIST \r");
		list_help();
		break;
		
		case 'F':
//		if (!ttccp_login) break;
		switch (c2)
        {
			case 'R': // set frequency
			if(c2 == 'R')
            {
				if (query)
				{
					sprintf(buf, "FR %lu\r", setup.frequency);
					COM1_send_str(buf);
				}
				else
				{
					freq = get_frequency();
					setup.frequency = freq;
					PLL_compute_freq_parameters(freq);
					PLL_update();
					delay_ms(50);
					PLL_update();
					ret = 1;
				}
			}
            else
			COM1_send_str("\r\nFAULT\r\n");
			break;
		}
		break;
		
		case 'M':
//		if (!ttccp_login) break;
		if (c2 == 'O')  // mode - Addr0 bit 0-3
		{
            if(query)
            {
				sprintf(buf, "MO %u\r", setup.mode);
				COM1_send_str(buf);
			}
			val = get_int();
			if (val <= 3)
            {
				setup.mode = val;
				FPGA_set_reg0();
				ret = 1;
			}
			else
            {
				COM1_send_str("\r\nFAIL\r\n");
				ret = 0;
				break;
			}
		}
		else
		COM1_send_str("\r\nFAIL\r\n");
		break;
		
		case 'D':
//		if (!ttccp_login) break;
		switch (c2)
        {
			// case 'B':  // debug mode
			//   debug_mode = 1;
			//   break;
			case 'S':   // Addr 6 bit 1   setup.data_source
			if (query)
            {
				sprintf(buf, "DS %u\r", setup.data_source);
				COM1_send_str(buf);
			}
			else
            {
				val = get_int();
				if (val < 2)
				{
					setup.data_source = val;
					FPGA_set_reg6();
				}
				else
				{
					COM1_send_str("\r\nFAIL\r\n");
					sprintf(ttccp_error_message, "ERR DS %u\r", setup.data_source);
					ret = 0;
					break;
				}
			}
			ret = 1;
			break;
			
			case 'P':   // Addr 0 bit 5   setup.data_polarity
			if (query)
            {
				sprintf(buf, "DP %u\r", setup.data_polarity);
				COM1_send_str(buf);
			}
			else
            {
				val = get_int();
				if (val < 2)
				{
					setup.data_polarity = 1 - val;	// VERSION 3.6: DP0: INVERT, DP1: NORMAL
													// Instead: 	DP0: NORMAL, DP1: INVERT
					FPGA_set_reg0();
				}
				else
				{
					COM1_send_str("\r\nFAIL\r\n");
					sprintf(ttccp_error_message, "ERR DP %u\r", setup.data_polarity);
					ret = 0;
					break;
				}
			}
			ret = 1;
			break;
			
			case 'E':   // Addr 0 bit 7   setup.SOQPSK
			if (query)
            {
				sprintf(buf,"DE %u\r", setup.SOQPSK);
				COM1_send_str(buf);
			}
			else
            {
				val = get_int();
				if (val < 2)
				{
					setup.SOQPSK = val;
					FPGA_set_reg0();
				}
				else
				{
					COM1_send_str("\r\nFAIL\r\n");
					sprintf(ttccp_error_message, "ERR DE %u\r", setup.SOQPSK);
					ret = 0;
					break;
				}
			}
			ret = 1;
			break;
		}
		break;
		
		case 'R':
//		if (!ttccp_login) break;
		switch (c2)
        {
			case 'P':  // HI or LO power - discrete output + DAC   setup.power_high
			if (query)
            {
				sprintf(buf, "RP %u\r", setup.power_high);
				COM1_send_str(buf);
			}
			else
            {
				val = get_int();
				if (val < 2)
				{
					if(setup.rc == 1)
					{
						if(val == 0)
						{
							setup.rp =1;
						}
						else
						
						setup.rp = 0;
					}
					if(val == 1)
					{
						rp_command = setup.rp = 0;
						delay_ms(50);
						update_all();
						delay_ms(1);
						update_all();
					}
					else
					{
						rp_command = setup.rp = 1;
						delay_ms(50);
						update_all();
						delay_ms(1);
						update_all();
					}
				}
				else
				{
					COM1_send_str("\r\nFAIL\r\n");
					sprintf(ttccp_error_message, "ERR RP %u\r", rp_command);
					ret = 0;
					break;
				}
			}
			ret = 1;
			break;
			
			case 'F': // discrete output - power amp on/off       setup.power_amp
			if (query)
            {
				sprintf(buf, "RF %u\r", pwr_command);
				COM1_send_str(buf);
			}
			else
            {
				val = get_int();
				if (val == 0)
				{
					if(setup.cot || setup.rc)
					{
						pwr_command = 0;
						setup.pwr = pwr_command;
						delay_ms(50);
						update_all();
						delay_ms(1);
						update_all();
						ret = 1;
					}
					else if(setup.cot == 0 || setup.rc)
					{
						pwr_command = 0;
						setup.pwr = pwr_command;
						delay_ms(50);
						update_all();
						delay_ms(1);
						update_all();
					}
				}
				else
				{
					if (val == 1)
					{
						if(setup.cot)  
						{
							pwr_command = 1;
							setup.pwr = pwr_command;
							power_output();
							delay_ms(50);
							update_all();
							delay_ms(1);
							update_all(); 
							ret = 1;
						}
						else if(setup.cot == 0)	
						{
							pwr_command = 1;
							setup.pwr = pwr_command;
							power_output();
							delay_ms(50);
							update_all();
							delay_ms(1);
							update_all();
							ret = 1;
						}
					}
					else
					{
						COM1_send_str("\r\nFAIL\r\n");
						sprintf(ttccp_error_message, "ERR RF %u\r", pwr_command);
						ret = 0;
						break;
					}
				}
			}
			ret = 1;
			break;





			
			case 'C':
         	if (query)
            {
				sprintf(buf, "RC %lu\r", setup.rc);
				COM1_send_str(buf);
			}
			else
            {
				val = get_int();
				if(val < 2)
				{
					if(val == 0)
					{
						setup.rc = 0;
						delay_ms(50);
						update_all();
						delay_ms(1);
						update_all();
					}
					if(val == 1)
					{
						setup.rc = 1;
						delay_ms(50);
						update_all();
						delay_ms(1);
						update_all();
					}
				}
				else
				COM1_send_str("\r\nFAULT\r\n");
			}
			break;
			
			case 'B':
            if (query)
            {
				sprintf(buf, "RB %lu\r", setup.cot);
				COM1_send_str(buf);
			}
			else
            {
				val = get_int();
				if(val < 2)
				{
					setup.cot = val;
					pwr_command++;
					pwr_command &= 1;
					delay_ms(50);
					update_all();
					delay_ms(1);
					update_all();
				}
				else
				COM1_send_str("\r\nFAULT\r\n");
			}
			break;







			
			case 'A': // Addr 0 bit 6     setup.randomizer
			if (query)
            {
				sprintf(buf, "RA %u\r", setup.randomizer);
				COM1_send_str(buf);
			}
			else
            {
				val = get_int();
				if (val < 2)
				{
					setup.randomizer = val;
					FPGA_set_reg0();
					ret = 1;
				}
				else
				{
					COM1_send_str("\r\nFAIL\r\n");
					sprintf(ttccp_error_message, "ERR RA %u\r", setup.randomizer);
					ret = 0;
					break;
				}
			}
			ret = 1;
			break;
			
			case 'L':    // recall
			read_setup();
			update_all();
			ret = 1;
			break;
			case 'E':   // reset
			reset_cpu();
			break;
		}
		break;
		
		case 'W':            // query
//		if (!ttccp_login) break;
		if (c2 == 'A')
        {
			//xxxx
			ret = 1;
		}
		break;
		
		case 'S':
//		if (!ttccp_login) break;
		switch (c2)
        {
			case 'P': // power down - discrete outputs
			ret = 1;
			break;
			case 'V':
			allow_write = 2975;
			write_setup();
			ret = 1;
			break;
		}
		break;
		
/*  //////////yehuda 1520Q cancel US,UT and BG 
		case 'U': // VERSION 3.3 UT US
		if (!ttccp_login) break;
		switch (c2)
        {
			case 'T': 
			if (query)
            {
				sprintf(buf, "UT %u\r", setup.UART_Time);
				COM1_send_str(buf);
			}
			else
			{
	            val = get_int();
	            if (val < 240 && val > 10)
				{
					setup.UART_Time = val;	              
				}
	            else
				{
					COM1_send_str("\r\nFAIL\r\n");
					sprintf(ttccp_error_message, "ERR UT %u\r", setup.UART_Time);
					ret = 0;
					break;
				}
			}
			ret = 1;
			break; 
*/	
/*	  //////////yehuda 1520Q cancel US,UT and BG 	
			case 'S':
			{
				if (query)
				{
					sprintf(buf, "US %u\r",setup.UART_Status);
					COM1_send_str(buf);
				}
				else
				{
					val = get_int();
					if (val < 2)
					{
						setup.UART_Status = val;
					}
					else
					{
			  			COM1_send_str("\r\nFAIL\r\n");
						sprintf(ttccp_error_message, "ERR US %u\r", setup.UART_Status);
						ret = 0;
						break;
					}
				}
				break;
			}
			break;
		}
		break;
*/

/*		///////////////////////////yehuda 1520Q remove BG command
		case 'B': // VERSION 3.6 BG Block Ghange and Check Function 23.0.2016
		if (!ttccp_login) break;
		switch (c2)
        {
			case 'G':     
			if (query)
            {
				sprintf(buf, "BG %lu\r", setup.Block_per_second);
				COM1_send_str(buf);
			}
			else
            {
				sub = get_int();
				if (sub >= 1 && sub <= 20)
				{

					setup.Block_per_second = (1000 / sub);
				}
				else
				{
					COM1_send_str("\r\nFAIL\r\n");
					sprintf(ttccp_error_message, "ERR BG %lu\r", 1000 / setup.Block_per_second);
					ret = 0;
					break;
				}
			}
			ret = 1;
			break;
		}
		break;
*/		
      	
		case 'I':
//		if (!ttccp_login) break;
		switch (c2)
        {
			case 'D':     // Addr 6 bits 2-5     setup.internal_pattern
			if (query)
            {
				sprintf(buf, "ID %u\r", setup.internal_pattern);
				COM1_send_str(buf);
			}
			else
            {
				val = get_int();
				if (val < 4)
				{
					setup.internal_pattern = val;
					FPGA_set_reg6();
				}
				else
				{
					COM1_send_str("\r\nFAIL\r\n");
					sprintf(ttccp_error_message, "ERR ID %u\r", setup.internal_pattern);
					ret = 0;
					break;
				}
			}
			ret = 1;
			break;
			
			case 'C':    // Addr 2 - 5 bit rate
			if (query)
            {
				sprintf(buf, "IC %lu\r", setup.bitrate);
				COM1_send_str(buf);
			}
			else
            {
				get_new_bitrate();
			}
			ret = 1;
			break;
		}
		break;
		
		case 'T':
//		if (!ttccp_login) break;
		if (c2 == 'E')
        {
            update_temperature_string();
            ret = 0;
		}
		break;
		
		case 'V':
//		if (!ttccp_login) break;
		switch (c2)
        {
			case 'E':  // return version
			if(c2 =='E')
			{
				sprintf(buf, "VE %s VS %lu.%lu ID=%02lu DT=%02lu%02u\r", 
                VERSION, ((setup.setup_version >> 8) & 0xff), (setup.setup_version & 0xff),
                setup.unit_ID, setup.year, setup.week);
				COM1_send_str(buf);
				ret = 1;
			}
			else
			COM1_send_str("\r\nFAIL\r\n");
			break;
			case 'S': // set setup version
			{
				if (query) {
					sprintf(buf, "VS %lu%lu\r", (setup.setup_version >> 8) & 0xff, setup.setup_version & 0xff);
					COM1_send_str(buf);
					ret = 1;
					} else {
					val = get_int();
					if (peek_char() == '.')
					{
						get_char(); // skip '.'
						rp = get_int();
						rp += val << 8;
						setup.setup_version = rp;   
						ret = 1;                 
						break;
					}
					COM1_send_str("\r\nFAIL\r\n");                
				}
				break;
			} 
			case 'L':  // set low power level
			if(c2 == 'L')
			{
				set_low_power_level();
				ret = 1;
			}
			else
			COM1_send_str("\r\nFAIL\r\n");
			break;
			
			case 'P':  // VP command power level control like in Generic TX  $P
			if (query)
            {
				sprintf(buf, "VP %lu\r", setup.power_level + 20);
				COM1_send_str(buf);
			}
			else
			set_power_level();
			ret = 1;
			break;
			
			case 'M': // manual power level
			if (query)
            {
				sprintf(buf, "VM %lu\r", manual_negative);
				COM1_send_str(buf);
			}
			else
            {
				manual_negative = get_int();
				set_AD5314(DAC_NEG_VOLT, val);
			}
			break;
			case 'C': // manual power level
			if (query)
            {
				sprintf(buf, "VC %lu\r", manual_pos);
				COM1_send_str(buf);
			}
			else
            {
				val = get_int();
				manual_pos = val;
				set_AD5314(DAC_POS_VOLT, val);
			}
			break;
		}
		break;
		
		case 'C':
//		if (!ttccp_login) break;
		if (c2 == 'S')  // Addr 6 bit 0    setup.clock_source
        {
			if (query)
            {
				sprintf(buf, "CS %u\r", setup.clock_source);
				COM1_send_str(buf);
			}
			else
            {
				val = get_int();
				if (val < 2)
				{
					setup.clock_source = val;
					FPGA_set_reg6();
				}
				else
				{
					COM1_send_str("\r\nFAIL\r\n");
					sprintf(ttccp_error_message, "ERR CS %u\r", setup.clock_source);
					ret = 0;
					break;
				}
			}
			
			ret = 1;
		}
		else if (c2 == 'P')  // Addr 6 bit 0    setup.clock_source
        {
			if (query)
            {
				sprintf(buf, "CP %u\r", setup.clock_polarity);
				COM1_send_str(buf);
			}
			else
            {
				val = get_int();
				if (val < 2)
				{
					setup.clock_polarity = val;
					FPGA_set_reg0();
				}
				else
				{
					COM1_send_str("\r\nFAIL\r\n");
					sprintf(ttccp_error_message, "ERR CP %u\r", setup.clock_polarity);
					ret = 0;
					break;
				}
			}
			
			ret = 1;
		}
		break;
		
		case 'G':
//		if (!ttccp_login) break;
		switch (c2)
        {
			case 'P':
			set_adc_channel(A2D_POWER); // read temperature
			delay_us(50);
			val = read_adc();
			sprintf(ttccp_error_message, "\nGP %lu\r\n", val);
			ret = 0;
			break;
			case 'T':
            update_temperature_string();
            ret = 0;
            break;
			
			case 'R':
            set_adc_channel(A2D_PREV); // select forward power input
            delay_us(20);
            val = read_adc();
            sprintf(ttccp_error_message, "\nGR %lu\r\n", val);
			ret = 0;
		}
		break;
		break;
		
		case 'Q': //statusl      if (!ttccp_login) break;
		
        COM1_send_str("\r\n\nOK Request Status \r\n\n");
        COM1_send_str(VERSION);
        sprintf(buf, "\r\nID=%lu DT=%02lu%02u\r",
		setup.unit_ID, setup.year, setup.week);
        COM1_send_str(buf);
		
        set_adc_channel(A2D_PREV); // select forward power input
        delay_us(20);
        revp = read_adc();
		
        if (revp <= 580)
		strcpy(revstat, "GOOD");
        else
		strcpy(revstat, "BAD");
		
        if(setup.rp)
		rp=0;
        else
		rp=1;
		
		
		sprintf(buf, "\r\n\nFREQ=%lu, REV=%s, FFWR=%lu, IC=%lu.%luMbps, MO=%u, CS=%u, RF=%lu\r"
		setup.frequency, revstat, current_power, setup.bitrate/100, setup.bitrate%100, setup.mode, setup.clock_source, setup.pwr);
		COM1_send_str(buf);
		
		sprintf(buf, "\r\n\nDE=%u, RA=%u, DP=%u, RP=%lu, DS=%u, ID=%u, VL=%lu, VP=%lu, RB=%lu, RC=%lu, CP=%u  \r"
		setup.SOQPSK, setup.randomizer, 1 - setup.data_polarity, rp, setup.data_source,
		setup.internal_pattern, setup.power_low_level+20, setup.power_level+20, setup.cot, setup.rc,
		setup.clock_polarity);
		COM1_send_str(buf);
		
		///sprintf(buf, "\r\n\nUT=%u, US=%u BG= %lu [Hz] %lu [mSec]\r" setup.UART_Time, setup.UART_Status, 1000 / setup.Block_per_second , setup.Block_per_second); // VERSION 3.3 17.1.2016
		///COM1_send_str(buf);
		
        update_temperature_string();
        ret = 0;
		break;
		default:
      	{
//			if (!ttccp_login) break;// VERSION 3.3 27.01.2016
			COM1_send_str("\r\nFAIL\r\n");
			
		}
	}
	aaa:
	return;
}
//----------------------------------------------------------------------------
#separate
void ttccp_handler(void)
{
	UINT chr, ret;
	
	switch (comm_state)
    {
		case COMM_INIT:
		comm_ridx = 0;
		comm_state = COMM_WAIT_CR;
		comm_timeout = 0;
		if (ttccp_login)
		COM1_send_str("\r>");
		break;
		case COMM_WAIT_CR:
		if (COM1_rcnt)
        {
			comm_timeout = 0;
			chr = COM1_get_chr();
			comm_buf[comm_ridx++] = chr;
			if (comm_ridx > 70)
			{
				comm_state = 0;
				break;
			}
			if (chr == 13 || chr == ';')
			{
				if (chr == ';')
				{
					comm_buf[comm_ridx-1] = 13;
					comm_ridx = 0;
				}
				else
				COM1_init();
				process_ttccp_commands();
				if (debug_mode)
				if (!ttccp_login) break;// VERSION 3.3 27.01.2016
				COM1_send_str("\r\nOK\n");
				if (ret == 1)
				{
					if (!ttccp_login) break;// VERSION 3.3 27.01.2016
					COM1_send_str("\n\r");
				}
				else
				COM1_send_str(ttccp_error_message);
			}
		}
		
		if (comm_state > COMM_WAIT_DLR)
        if (TMR_100MS_COMM_TO)
		{
			TMR_100MS_COMM_TO = 0;
			if (++comm_timeout > 10000) // time out after 10 seconds from last char
            comm_state = 0;
		}
		break;
		case COMM_DELAY:
		break;
	}
}

//----------------------------------------------------------------------------
#separate
void dollar_handler(void)
{
	UINT chr, ret;
	
	switch (comm_state)
    {
		case COMM_INIT:
		comm_ridx = 0;
		comm_state++;
		break;
		case COMM_WAIT_DLR:
		#ignore_warnings 201
		if (COM1_rcnt)
        if ((chr = COM1_get_chr()) == '$')
		{
			comm_state++;
			comm_timeout = 0;
		}
        else
		inc_dec(chr);
		break;
		case COMM_WAIT_CR:
		if (COM1_rcnt)
        {
			comm_timeout = 0;
			chr = COM1_get_chr();
			comm_buf[comm_ridx++] = chr;
			if (comm_ridx > 70)
			{
				comm_state = 0;
				break;
			}
			if (chr == 13)
			{
				ret = process_dollar_commands();
				if (ret == 1)
				{
					//            store_setup();
					//            update_all();
				}
				if (ret != 255)
				COM1_send_str("\r\nOK\r\n*");
				COM1_init();
			}
			else if (chr == 27)
			{
				COM1_send_str("\r\n\nBREAK\r\n");
				comm_state = 0;
			}
		}
		
		if (comm_state > COMM_WAIT_DLR)
        if (TMR_100MS_COMM_TO)
		{
			TMR_100MS_COMM_TO = 0;
			if (++comm_timeout > 10000) // time out after 10 seconds from last char
            comm_state = 0;
		}
		break;
		case COMM_DELAY:
		break;
	}
}


//----------------------------------------------------------------------------
#separate
void comm_handler(void)
{
	UINT chr;
	
	if (OERR)
    {
		OERR = 0;
		CREN = 0;
		delay_us(5);
		CREN = 1;
	}
	if (FERR)
    {
		FERR = 0;
		chr = RCREG1;
	}
	if (ttccp)
    ttccp_handler();
	else
    dollar_handler();
}

