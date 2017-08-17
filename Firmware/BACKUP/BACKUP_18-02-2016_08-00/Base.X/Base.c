
#include <xc.h>
#include "SYS.h"

#include "IN.h"
#include "DIN.h"
#include "OUT.h"
#include "NVM.h"
#include "SPI.h"
#include "CC1.h"
#include "RTX.h"
#include "PGM.h"
#include "Base.h"
#include "DPY.h"

#pragma config FCMEN=ON
#pragma config IESO=OFF
#pragma config CLKOUTEN=OFF
#pragma config BOREN=ON
#pragma config CP=ON
#pragma config MCLRE=OFF
#pragma config PWRTE=ON
#pragma config WDTE=ON
#pragma config FOSC=INTOSC
#pragma config LVP=OFF
#pragma config LPBOR=ON
#pragma config BORV=1
#pragma config STVREN=ON
#pragma config VCAPEN=OFF
#pragma config WRT=OFF

/*NOTE:

- timer0: tick di sistema con base tempi 10ms.
*/

void interrupt INTIsr(void)
{
    /*Controlla se si è verificato l'interrupt del timer 0.*/
    if(INTCONbits.T0IF==1)
    {
        //Cancella il flag dell'interrupt.
        INTCONbits.T0IF=0;

        //Chiama le funzioni interessate.
        SYSInt();
    }
}

void main(void)
{
    unsigned char i;

    //Inizializzazione.
    for(i=0;i<3;i++)
    {
        CLRWDT();

        SYSInit(i);
        INInit(i);
        OUTInit(i);
        DINInit(i);
        SPIInit(i);
        CC1Init(i);
        NVMInit(i);
        DPYInit(i);
        RTXInit(i);
        PGMInit(i);
    }

    //Abilita gli interrupt.
    INTCONbits.GIE=1;

    //MainLoop.
    while(1)
    {
        CLRWDT();
        //Task.
        SYSTask();
        DINTask();
        RTXTask();
    }
	
}
