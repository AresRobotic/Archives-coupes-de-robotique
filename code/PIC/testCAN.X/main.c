#include <p30F4011.h>

#include <can.h>

_FOSC(CSW_FSCM_OFF & XT_PLL8);	    // Fosc=8x8/4MHz, Fcy=16MHz
_FWDT(WDT_OFF);			    // Watchdog timer off
_FBORPOR(MCLR_EN & PWRT_64);	    // Enable MCLR reset pin and turn off the power-up timers
_FGS(CODE_PROT_OFF);		    // Disable Code Protection

#define FCY 		16000000		    // 16 MHz
#define BITRATE 	125000			    // 125kHz
#define NTQ 		15			    // Number of Tq cycles which will make the CAN Bit Timing
#define BRP_VAL		((FCY/(2*NTQ*BITRATE))-1)   // Formulae used for C1CFG1bits.BRP

void initCAN(void);

int main(void)
{
    unsigned char TxData[8];
    unsigned int a;
    
    TRISFbits.TRISF1 = 0;
    TRISFbits.TRISF0 = 1;

    initCAN();

    TxData[0] = 'A';
    TxData[1] = 'r';
    TxData[2] = 'e';
    TxData[3] = 's';
    TxData[4] = ' ';
    TxData[5] = 'R';
    TxData[6] = 'o';
    TxData[7] = 'x';
    
    while(1)
    {
	CAN1SendMessage(( CAN_TX_SID(0x100)) & CAN_TX_EID_DIS & CAN_SUB_NOR_TX_REQ ,
			( CAN_TX_EID(0x00)) & CAN_NOR_TX_REQ , TxData , 8 , 0 );
	for(a=0;a<60000;a++);
    }
    return 0;

}


void initCAN(void)
{
    CAN1SetOperationMode(   CAN_IDLE_CON & CAN_MASTERCLOCK_0 &
	CAN_REQ_OPERMODE_CONFIG ) ;

    while ( C1CTRLbits.OPMODE <= 3 );

    CAN1Initialize( CAN_SYNC_JUMP_WIDTH2 & CAN_BAUD_PRE_SCALE(24),
	CAN_WAKEUP_BY_FILTER_DIS &
	CAN_PHASE_SEG2_TQ(3) &
	CAN_PHASE_SEG1_TQ(3) &
	CAN_PROPAGATIONTIME_SEG_TQ(1) &
	CAN_SEG2_FREE_PROG & CAN_SAMPLE1TIME ) ;

    CAN1SetFilter ( (char) 0 , CAN_FILTER_SID( 200 ) & CAN_RX_EID_DIS ,
	CAN_FILTER_EID( 0x00 )) ;

    CAN1SetMask (   (char)0 , CAN_MASK_SID ( 0x00 ) & CAN_MATCH_FILTER_TYPE ,
	CAN_MASK_EID( 0x00) ) ;

    CAN1SetTXMode( (char) 0 , CAN_TX_STOP_REQ & CAN_TX_PRIORITY_HIGH ) ;
    CAN1SetRXMode( (char) 0 , CAN_RXFUL_CLEAR & CAN_BUF0_DBLBUFFER_EN ) ;

    CAN1SetOperationMode(   CAN_IDLE_CON & CAN_CAPTURE_DIS & CAN_MASTERCLOCK_0 &
	CAN_REQ_OPERMODE_NOR ) ;
}