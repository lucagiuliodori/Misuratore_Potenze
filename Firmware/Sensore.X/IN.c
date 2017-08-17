#include <xc.h>
#include "IN.h"

#define mInitIn() TRISA|=0b00000110; TRISB|=0b00100111; TRISC|=0b00010000; ANSELB&=0b11011000; ANSELC&=0b11101111; WPUB=0b00100111

void INInit(unsigned char sta)
{
    switch(sta)
    {
        case 0:
            //Inizializzazione hardware.
            mInitIn();
            break;
//        case 1:
//            //Inizializzazione variabili.
//            break;
//        case 2:
//            //Inizializzazione applicazione.
//            break;
    }
}
