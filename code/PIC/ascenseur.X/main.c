/* 
 * File:   main.c
 * Author: Benjamin
 *
 * Created on 5 février 2014, 18:41
 */

#include <p30F4012.h>
#define FCH PORTDbits.RD1
#define FCB PORTDbits.RD0

#define VITESSEMAX 60
#define VITESSEMIN 30

#define GETPOS POSCNT-2000

// Configuration settings
_FOSC(CSW_FSCM_OFF & FRC_PLL16);  // Fosc=16x7.5MHz, Fcy=30MHz
_FWDT(WDT_OFF);                   // Watchdog timer off
_FBORPOR(MCLR_EN & PWRT_OFF);     /* Enable MCLR reset pin and turn off the power-up timers. */
_FGS(CODE_PROT_OFF);              /* Disable Code Protection */


void PWM_init();
void setSpeed(int pSpeed);
void QEI_init();
void CN_init();
void go(int p);
char actualise();
void timer1_init();

int speed;
int posDem = 0;
int r0,r1,po;

float delay=0;

int main()
{
    int tab[] = {1790,150,50,510,1000,1700};
    
    TRISD = 0b11;
    TRISB = 0b111100;
    _TRISC15 = 0;

    _LATB0 = 1;

    /* Configuration des modules */
    PWM_init();
    QEI_init();
    CN_init();
    timer1_init();

    /*setSpeed(50);
    for(;;);*/

    _LATB1 = 1;

    /* Initialisation verticale */
    setSpeed(100);
    while(FCH == 0);
    setSpeed(-10);
    while(FCB == 0);
    POSCNT = 2000; // Le 0 se trouve en 2000, le haut de l'ascenseur vers les 6000
    setSpeed(100);
    while(FCB != 0);
    setSpeed(0);

    _LATB1 = 0;

    po = 0;
    go(tab[po%6]);

    for(;;)
    {
        if(FCB != 0)
            POSCNT = 2000;

        if(speed > 0 && FCH != 0)
        {
            setSpeed(0);
            posDem = GETPOS;
        }
        if(speed < 0 && FCB != 0)
        {
            setSpeed(0);
            posDem = GETPOS;
        }

        if(actualise())
        {
            for(r0=0;r0<32000;r0++)
                for(r1=0;r1<200;r1++);

            po++;
            go(tab[po%6]);
        }
    }
}

void setSpeed(int pSpeed) // de -100% à 100%
{
    if(pSpeed > 0)
    {
	speed = VITESSEMIN + pSpeed*(VITESSEMAX-VITESSEMIN)/100;
	LATCbits.LATC15 = 0;
    }
    else if(pSpeed < 0)
    {
	speed = VITESSEMIN - pSpeed*(VITESSEMAX-VITESSEMIN)/100;
	LATCbits.LATC15 = 1;
    }
    else
	speed = 0;
    

    if(pSpeed > 0 && FCH != 0)
        speed = 0;
    if(pSpeed < 0 && FCB != 0)
        speed = 0;

    
    PDC1 = (int) (speed*7500./100.);
}

/* En dixieme de millimetre */
void go(int p)
{
    // 3000 = 132,6 mm
    posDem = (p/1326.)*3000.;
    delay = 0;
}

char actualise()
{
    
    if(posDem-15 < GETPOS && GETPOS < posDem+15)
    {
        setSpeed(0);
        delay = 0;
        return 1;
    }

    if(GETPOS > posDem)
    {
        if(GETPOS - posDem > 1000)
            setSpeed(-40);
        else
            setSpeed(-20);
	return 0;
    }
    if(GETPOS < posDem)
    {
	if(posDem - GETPOS > 1000)
            setSpeed(100);
        else
            setSpeed(60);
	return 0;
    }

    setSpeed(0);
    return 0;
}

/*  ----------------------------------------------------------------------------
 * Fonction d'interruption CN
 *  ----------------------------------------------------------------------------
 */

void __attribute__((__interrupt__, __auto_psv__)) _CNInterrupt (void)
{
    IFS0bits.CNIF = 0; // Remise à zero : Clear CN interrupt flag bit
}

/*  ----------------------------------------------------------------------------
 * Fonction d'interruption du timer
 *  ----------------------------------------------------------------------------
 */

void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void)
{

    _T1IF = 0;           // Remise à zero
}


/* Configuration des PWM */
void PWM_init()
{
    // PWM period = Tcy * prescale * PTPER = 0.33ns * 64 * 9470 = 20ms
    PWMCON1 = 0x0FFF; // Enable all PWM pairs in complementary mode
    PTCON = 0;
    _PTCKPS = 0;      // prescale=1:64 (0=1:1, 1=1:4, 2=1:16, 3=1:64)
    PTPER = 4000;      //
    PDC1 = 0;         // 0% duty cycle on channel 1 (max is 65536)
    PDC2 = 0;         // 0% duty cycle on channel 2 (max is 65536)
    PDC3 = 0;         // 0% duty cycle on channel 3 (max is 65536)
    PTMR = 0;         // Clear 15-bit PWM timer counter
    _PTEN = 1;        // Enable PWM time base
}

/* Configuration du QUI */
void QEI_init()
{
    _TRISB4 = 1;
    _TRISB5 = 1;

    ADPCFGbits.PCFG4 = 1;      // On passe les entrées en Digital
    ADPCFGbits.PCFG5 = 1;

    QEICONbits.QEISIDL = 0 ;
    QEICONbits.QEIM = 0b111 ;  // On n'utilise pas l'index
    QEICONbits.SWPAB = 1;      // Pas d'échange/ d'inversion entre A & B
    QEICONbits.PCDOUT = 0;     // DISABLE le sens de rotation du moteur
    QEICONbits.TQGATE = 0;
    QEICONbits.TQCKPS = 0b00;  // fréquence timer , ici à 1/1 TCY
    QEICONbits.POSRES = 0;     // Reset du compteur par nous même
    QEICONbits.TQCS = 0;       // Horloge interne (1 pour externe)
    DFLTCONbits.CEID = 1;      // Pas d'utilisation d'interruption
}

/* Configuration des CN */
void CN_init()
{
    CNEN1bits.CN0IE = 1;
    IFS0bits.CNIF = 0;

    IEC0bits.CNIE = 1;     // Activation des interuption sur changement d'etat sur le CN
}

/* Configuration du timer */
void timer1_init()
{
    T1CON = 0;
    
    _T1IF = 0;
    _T1IE = 1;

    TMR1 = 0;                   // On vide le timer 1
    PR1 = 76;                 // Valeur a laquelle le timer 1 genere une interruption
    
    T1CONbits.TSIDL = 0;        // Fonctionne en mode veille
    T1CONbits.TCKPS = 0b000;    // Prescaler de 1/1
    T1CONbits.TCS = 0;          // Focntionne avec l'horloge interne

    T1CONbits.TON = 1;          // Activation du timer 1
}