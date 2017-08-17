
#include <xc.h>
#include "IN.h"
#include "DIN.h"
#include "CC1.h"
#include "RTX.h"
#include "PGM.h"
#include "SYS.h"

//Definizione macro.
#define mTMR0Reset() TMR0=255-156
#define mSysInit() OPTION_REG=0b00000110; OSCCON=0b01110001; WDTCON=0b00010011; mTMR0Reset(); INTCON|=0b00100000; INTCON&=0b11111011; PIR1=0b00000000; PIR2=0b00000000; PIE1=0b00000000; PIE2=0b00000000	//prescaler timer0 1:128, frequenza 8MHz

void SYSInit(unsigned char state)
{
    switch(state)
    {
        case 0:
            //Inizializzazione hardware.
            mSysInit();
            break;
        case 1:
            //Inizializzazione variabili.
            u_SYS_STA.VAL=0;
            break;
//        case 2:
//            //Inizializzazione applicazione.
//            break;
    }
}

void SYSInt(void)
{
	//Setta il flag per il tick di sistema.
	u_SYS_STA.TICK=1;

	//Ricarica il timer 0.
	mTMR0Reset();
}	

void SYSTask(void)
{
    //Gestisce il tick di sistema.
    if(u_SYS_STA.TICK)
    {
        //Cancella il flag.
        u_SYS_STA.TICK=0;

        //Chiama le funzioni interessate.
        RTXTick();
        DINTick();
    }

    if(u_in_ev_down.sel)
    {
        //Cancella il flag.
        u_in_ev_down.sel=0;
        RTXSelDown();
    }
    if(u_in_ev_up.sel)
    {
        //Cancella il flag.
        u_in_ev_up.sel=0;
    }
    if(u_in_ev_long.sel)
    {
        //Cancella il flag.
        u_in_ev_long.sel=0;
        PGMSelLong();
    }
}