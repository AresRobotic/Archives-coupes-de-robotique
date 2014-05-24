/* 
 * File:   main.c
 * Author: Benjamin
 *
 * Created on 24 mai 2014, 15:39
 */

#include <p30F4011.h>

// Configuration settings
_FOSC(CSW_FSCM_OFF & FRC_PLL8);  // Fosc=8x8MHz, Fcy=16MHz
_FWDT(WDT_OFF);                   // Watchdog timer off
_FBORPOR(MCLR_EN & PWRT_OFF);     /* Enable MCLR reset pin and turn off the power-up timers. */
_FGS(CODE_PROT_OFF);              /* Disable Code Protection */

#define SVM_1 LATBbits.LATB6
#define SVM_1_TRIS TRISBbits.TRISB6

void timer1_init();

unsigned char svm1_pos = 0;
unsigned int svm_conter = 0;

int main()
{
    timer1_init();
    
    SVM_1_TRIS = 0;
    while(1);
    return 0;
}

void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void)
{
    _T1IF = 0;           // Remise à zero du flag

    svm_conter++;
    if(svm_conter < 255)
    {
	SVM_1 = 1;
    }
    else if(svm_conter < 511)
    {
	if(svm_conter - 255 > 128)
	    SVM_1 = 0;
	else
	    SVM_1 = 1;
    }
    else if(svm_conter < 5100)
    {
	SVM_1 = 0;
    }
    else
	svm_conter = 0;


    

}

/* Configuration du timer */
void timer1_init()
{
    T1CON = 0;

    _T1IF = 0;
    _T1IE = 1;

    TMR1 = 0;                   // On vide le timer 1
    PR1 = 570;			// Valeur a laquelle le timer 1 genere une interruption

    T1CONbits.TSIDL = 0;        // Fonctionne en mode veille
    T1CONbits.TCKPS = 0b000;    // Prescaler de 1/1
    T1CONbits.TCS = 0;          // Focntionne avec l'horloge interne

    T1CONbits.TON = 1;          // Activation du timer 1
}