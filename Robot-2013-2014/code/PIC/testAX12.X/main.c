/* 
 * File:   main.c
 * Author: Benjamin
 *
 * Created on 13 mai 2014, 17:49
 */

#include <p30F4011.h>
#include <uart.h>

/*
 * 
 */

_FOSC(CSW_FSCM_OFF & XT_PLL8);  // Fosc=8x8/4MHz, Fcy=16MHz
_FWDT(WDT_OFF);                   // Watchdog timer off
_FBORPOR(MCLR_EN & PWRT_64);     /* Enable MCLR reset pin and turn off the power-up timers. */
_FGS(CODE_PROT_OFF);              /* Disable Code Protection */


void sendToAX12(char id, char *param, char len);
void AX12moveTo(char id, int angle);
void AX12setSpeed(char id, int speed);
void initUART(unsigned int b);


int main(void)
{
    //char data[] = {0x05 ,0x03 ,0x04 ,0x07 ,0x00};
    unsigned int i,r;//,b=0;

    initUART(3);
    AX12setSpeed(0xFE,0x005F);

    while(1)
    {
	for(i=0;i<32000;i++)for(r=0;r<160;r++);
	AX12moveTo(0xFE,0x0000);
	for(i=0;i<32000;i++)for(r=0;r<160;r++);
	AX12setSpeed(0xFE,0x00FF);
	for(i=0;i<32000;i++)for(r=0;r<160;r++);

	AX12setSpeed(0xFE,0x005F);
	for(i=0;i<32000;i++)for(r=0;r<160;r++);
	AX12moveTo(0xFE,0x03FF);
	for(i=0;i<32000;i++)for(r=0;r<160;r++);
	AX12setSpeed(0xFE,0x00FF);
	for(i=0;i<32000;i++)for(r=0;r<160;r++);

	/*for(b=0;b<300;b++)
	{
	     // Ferme et réouvre l'URAT
	    sendToAX12(0xFE,data,5);
	    for(i=0;i<32000;i++)for(r=0;r<10;r++);
	    initUART(7); // Ferme et réouvre l'URAT en 250K
	    AX12moveTo(0xFE,0x03FF);
	    for(i=0;i<32000;i++)for(r=0;r<10;r++);
	}*/
    }
    
    return 0;

}


void AX12moveTo(char id, int angle)
{
    char data[] = {0x03 ,0x1E ,0x00 ,0x00};
    data[2] = angle;
    data[3] = angle>>8;
    sendToAX12(id,data,4);
}

void AX12setSpeed(char id, int speed)
{
    char data[] = {0x03 ,0x20 ,0x00 ,0x00};
    data[2] = speed;
    data[3] = speed>>8;
    sendToAX12(id,data,4);
}

void sendToAX12(char id, char *param, char len)
{
    char sum=0;
    char i=0;
    while(BusyUART2());
    putcUART2(0xFF);
    putcUART2(0xFF);
    putcUART2(id);
    putcUART2(len+1);
    while(BusyUART2()); // Le buffer est de 4, on attend que tout ai été envoyé
    sum = id+len+1;

    for(i=0;i<len;i++)
    {
	sum += param[i];
	putcUART2(param[i]);
	while(BusyUART2());
    }

    putcUART2(~sum);
    while(BusyUART2());
}

void initUART(unsigned int b)
{
    /* Holds the value of baud register   */
    unsigned int baudvalue;
    /* Holds the value of uart config reg */
    unsigned int U2MODEvalue;

    /* Holds the information regarding uart TX & RX interrupt modes */
    unsigned int U2STAvalue;

    /* Turn off UART1module */
    CloseUART2();

    /* Configure uart1 receive and transmit interrupt */
    ConfigIntUART2(UART_RX_INT_DIS & UART_RX_INT_PR6 &
                   UART_TX_INT_DIS & UART_TX_INT_PR2);

    /* Configure UART1 module to transmit 8 bit data with one stopbit. Also Enable loopback mode  */
    baudvalue = b; // Fcy/(16*(baudvalue+1))

    U2MODEvalue = UART_EN & UART_IDLE_CON &
                  UART_DIS_WAKE & UART_EN_LOOPBACK  &
                  UART_EN_ABAUD & UART_NO_PAR_8BIT  &
                  UART_1STOPBIT;

    U2STAvalue  = UART_INT_TX_BUF_EMPTY  &
                  UART_TX_PIN_NORMAL &
                  UART_TX_ENABLE & UART_INT_RX_3_4_FUL &
                  UART_ADR_DETECT_DIS &
                  UART_RX_OVERRUN_CLEAR;

    OpenUART2(U2MODEvalue, U2STAvalue, baudvalue);
}