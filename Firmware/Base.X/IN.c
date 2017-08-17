#include <xc.h>
#include "IN.h"

#define mInitIn() TRISB|=0b00000111; TRISC|=0b00010000; ANSELB&=0b11111000; WPUB=0b00000111

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
