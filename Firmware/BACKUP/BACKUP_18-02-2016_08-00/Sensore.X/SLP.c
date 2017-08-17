#include <xc.h>
#include "SLP.h"
#include "CC1.h"
#include "OUT.h"

void SLPSleep(void)
{
    unsigned char IOCBNTmp,INTCONTmp,WPUBTmp,WDTCONTmp,OPTION_REGTmp,FVRCONTmp,ADCON0Tmp,TESTBATTtmp;
    //Setta il CC1101 in sleep.
    CC1Sleep();
    //Attiva solo il pull-up del tasto sel.
    WPUBTmp=WPUB;
    WPUB=0b00000100;
    //Disattiva l'ADC.
    ADCON0Tmp=ADCON0;
    ADCON0=0b00000000;
    //Disattiva il VR interno al micro.
    FVRCONTmp=FVRCON;
    FVRCON=0b00000000;
    //Attiva solo l'interrupt sulla PORTB.
    IOCBNTmp=IOCBN;
    INTCONTmp=INTCON;
    INTCON=0b10001000;
    IOCBN=0b00000100;
    //Controlla lo stato del pin di test della tensione della batteria.
    TESTBATTtmp=0;
    if(pin_TESTBATT)
        TESTBATTtmp=1;
    //Setta il postscaler del watchdog-timer.
    WDTCONTmp=WDTCON;
    WDTCON=0b00011011;
    //Setta il prescaler per il watchdog-timer.
    OPTION_REGTmp=OPTION_REG;
    OPTION_REG=0b00001010;
    //Va in sleep.
    CLRWDT();
    SLEEP();
    //Ripristina i regitri.
    IOCBN=IOCBNTmp;
    INTCON=INTCONTmp;
    WPUB=WPUBTmp;
    ADCON0=ADCON0Tmp;
    FVRCON=FVRCONTmp;
    OUTSetTESTBATTOff();
    if(TESTBATTtmp)
        OUTSetTESTBATTOn();
    WDTCON=WDTCONTmp;
    OPTION_REG=OPTION_REGTmp;
    //Setta il flag di uscita dallo sleep.
    flg_slp.exitSleep=1;
}

