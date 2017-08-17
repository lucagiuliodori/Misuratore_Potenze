#include <xc.h>
#include "DLY.h"
#include "OUT.h"
#include "Base.h"
#include "HTC.h"

static void HTCSendData(unsigned char);

void HTCInit(unsigned char sta)
{
   switch(sta)
    {
        case 0:
            //Inizializzazione hardware.
            break;
        case 1:
            //Inizializzazione variabili.
            break;
        case 2:
            //Inizializzazione applicazione.
            break;
    }
}

static void HTCSendData(unsigned char val)
{
    //TODO: controllare l'implementazione della seconda parte del byte.
    
    //Setta le linee dato per il settaggio dati a 4 bit.
    OUTSetHTCDataNibble((val>>4)&0b00001111);
    DLYDelay_us(1);
    //Setta l'enable.
    OUTSetEOn();
    DLYDelay_us(1);
    OUTSetEOff();
    DLYDelay_us(1);

    OUTSetHTCDataNibble(val&0b00001111);
    DLYDelay_us(1);
    //Setta l'enable.
    OUTSetEOn();
    DLYDelay_us(1);
    OUTSetEOff();
    DLYDelay_us(1);
    
}

void HTCDisplayInit(te_HTC_DIRCUR cursorDir,unsigned char displayMov,unsigned char displayOnOff,
        unsigned char cursorOnOff,unsigned char cursorBlink,te_HTC_MOVCURDPY movCursorDisplay,
        te_HTC_MOVDIR dirCursorDisplay,te_HTC_LENDATA lenData,
        te_HTC_NUMLINE numLine,te_HTC_FONTSIZE fontSize)
{
    //Ritardo di 40ms per la stabilizzazione dell'alimentazione.
    DLYDelay_ms(40);
    //Setta l'enable.
    OUTSetEOff();
    //Setta i pin di RS e di RW.
    OUTSetRSOff();
    OUTSetRWOff();
    HTCSendData(0b00110000);

    //Ritardo secondo procedura di inizializzazione.
    DLYDelay_ms(5);
    HTCSendData(0b00110000);

    //Ritardo secondo procedura di inizializzazione.
    DLYDelay_us(100);
    HTCSendData(0b00110000);
}

void HTCDisplayClear(void)
{

}
void HTCCursorReset(void);

void HTCSetAddressCGRAM(unsigned char);
void HTCSetAddressDDRAM(unsigned char);

unsigned char HTCGetAddressCounter(void);
BOOL HTCIsBusy(void);

unsigned char HTCReadData(void);
void HTCWriteData(unsigned char);
