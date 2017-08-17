#include <xc.h>
#include "Sensore.h"
#include "OUT.h"
#include "DLY.h"
#include "LED.h"

void LEDInit(unsigned char sta)
{
    switch(sta)
    {
//        case 0:
//            //Inizializzazione hardware.
//            break;
//        case 1:
//            //Inizializzazione variabili.
//            break;
        case 2:
            //Inizializzazione applicazione.
            OUTSetLEDOff();
            break;
    }
}

void LEDNormalFlash(unsigned char num)
{
    unsigned char i;
    //Esegue i  lampeggi (sospensivi).
    for(i=0;i<num;i++)
    {
        CLRWDT();
        OUTSetLEDOn();
        DLYDelay_ms(200);
        OUTSetLEDOff();
        DLYDelay_ms(200);
    }
}

void LEDFastFlash(unsigned char num)
{
    unsigned char i;
    //Esegue i  lampeggi (sospensivi).
    for(i=0;i<num;i++)
    {
        CLRWDT();
        OUTSetLEDOn();
        DLYDelay_ms(5);
        OUTSetLEDOff();
        DLYDelay_ms(5);
    }
}

void LEDVFastFlash(unsigned char num)
{
    unsigned char i;
    //Esegue i  lampeggi (sospensivi).
    for(i=0;i<num;i++)
    {
        CLRWDT();
        OUTSetLEDOn();
        DLYDelay_ms(2);
        OUTSetLEDOff();
        DLYDelay_ms(2);
    }
}
