
//--------------------------------------------------------------------------
#int_TIMER4
void  TIMER4_isr(void)
{
	
}

//--------------------------------------------------------------------------
#int_TIMER2
void TIMER2_isr(void) // 1mS tick
{
    timerTick_1ms = 1;
	//    TMR_1mS_Flags = 0xFF;
	//    TMR_1mS_Count++;
	//    if (++TMR_1mS_Cnt >= 10) {
	//        TMR_1mS_Cnt = 0;
	//        TMR_10mS_Flags = 0xFF;
	//        if (++TMR_10mS_Cnt >= 10) {
	//            TMR_10mS_Cnt = 0;
	//            TMR_100mS_Flags = 0xFF;
	//            if (++TMR_100mS_Cnt >= 10)
	//            {
	//                TMR_100mS_Cnt = 0;
	//                TMR_1sec_Flags = 0xFF;
	//            }
	//        }
	//    }
}


//--------------------------------------------------------------------------
#int_EXT
void  EXT_isr(void)
{
	
}

//--------------------------------------------------------------------------
#int_EXT1
void  EXT1_isr(void)
{
	
}

//--------------------------------------------------------------
#int_RDA
void  RDA_isr(void)
{
	int8 x;
	x = RCREG1;
	COM1_rbuf[COM1_rxi] = x;
	if (++COM1_rxi >= COM1_RX_LEN)
    COM1_rxi = 0;
	if (COM1_rcnt < COM1_RX_LEN)
    COM1_rcnt++;
}

#int_TBE // VERSION 3.3 17.1.2016
void TBE_isr(void)
{
    disable_interrupts(int_TIMER2);
    output_high(RS485_EN);
    status_tx_index = 0;
    while (tx_block_len--)
    {
        output_high(RS485_EN);
        TXREG1 = status_buffer[status_tx_index++];
        delay_us(500); 
        output_low(RS485_EN);
	}
    disable_interrupts(int_TBE);    
    enable_interrupts(int_TIMER2);
}


