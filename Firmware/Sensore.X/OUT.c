#include <xc.h>
#include "OUT.h"

#define mInitOut() TRISA&=0b00000110; TRISB&=0b00100111; TRISC&=0b00010000; ANSELA&=0b00000110; ANSELB&=0b00100111; ANSELC&=0b00010000; PORTA&=0b11111110; PORTC|=0b10000000

void OUTInit(unsigned char sta)
{
    switch(sta)
    {
        case 0:
            //Inizializzazione hardware.
            mInitOut();
            break;
//        case 1:
//            //Inizializzazione variabili.
//            break;
//        case 2:
//            //Inizializzazione applicazione.
//            break;
    }
}
