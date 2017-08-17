
#include <xc.h>
#include "Sensore.h"
#include "DLY.h"

void DLYDelay_us(unsigned short val)
{
    unsigned short i;

    for(i=0;i<val;i++)
    {
        CLRWDT();
        __delay_us(1);
    }
}

void DLYDelay_ms(unsigned short val)
{
    unsigned short i;

    for(i=0;i<val;i++)
    {
        CLRWDT();
        __delay_ms(1);
    }
}

