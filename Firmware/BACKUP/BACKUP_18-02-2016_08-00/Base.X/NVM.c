#include <xc.h>
#include "NVM.h"

#define mInitNvm() PMCON1=0b00000000

void NVMInit(unsigned char sta)
{
    switch(sta)
    {
        case 0:
            //Inizializzazione hardware.
            mInitNvm();
            break;
//        case 1:
//            //Inizializzazione variabili.
//            break;
//        case 2:
//            //Inizializzazione applicazione.
//            break;
    }
}

void NVMRead(unsigned short addr,char *dest,unsigned char len)
{
    unsigned char i;

    PMCON1bits.CFGS=0;
    for(i=0;i<len;i++)
    {
        CLRWDT();
        PMADR=(addr+i);
        PMCON1bits.RD=1;
        while(PMCON1bits.RD);
        (*(dest+i))=PMDAT;
    }
}

void NVMWrite(unsigned short addr,char *source,unsigned char len)
{
    unsigned char i,j,latchAddr;
    unsigned short rowAddr,bufTmpRow[0x20];

    INTCONbits.GIE=0;
    PMCON1bits.WREN=1;
    PMCON1bits.CFGS=0;

    //Azzera gli indici per il puntatore "source".
    i=0;
    //Ricava l'indirizzo di riga e di latch.
    rowAddr=(addr&(~(0x001F)));
    latchAddr=(addr&0x001F);
    while(i<len)
    {
        CLRWDT();

        //Legge il contenuto della riga.
        for(j=0;j<sizeof(bufTmpRow)/sizeof(unsigned short);j++)
        {
            CLRWDT();
            PMADR=(rowAddr+j);
            PMCON1bits.RD=1;
            while(PMCON1bits.RD);
            bufTmpRow[j]=PMDAT;
        }

        //Cancellazione della riga.
        PMCON1bits.FREE=1;
        PMADR=rowAddr;
        PMCON2=0x55;
        PMCON2=0xAA;
        PMCON1bits.WR=1;
        while(PMCON1bits.WR);
        NOP();
        NOP();

        //Aggiorna il contenuto della nuova riga.
        for(;latchAddr<sizeof(bufTmpRow)/sizeof(unsigned short);latchAddr++)
        {
            CLRWDT();
            //Controlla se interrompere il ciclo.
            if(i>=len)
                break;
            //Setta il dato.
            bufTmpRow[latchAddr]=(*(source+i));
            i++;
        }
        //Azzera l'indirizzo del latch.
        latchAddr=0;

        //Scrive la riga sulla PGM.
        for(j=0;j<sizeof(bufTmpRow)/sizeof(unsigned short);j++)
        {
            //Setta l'indirizzo.
            PMADR=(rowAddr+j);
            PMDAT=bufTmpRow[j];
            //Controlla se è arrivato a fine latch.
            if(j==(sizeof(bufTmpRow)/sizeof(unsigned short)-1))
                PMCON1bits.LWLO=0;
            else
                PMCON1bits.LWLO=1;
            PMCON2=0x55;
            PMCON2=0xAA;
            PMCON1bits.WR=1;
            while(PMCON1bits.WR);
            NOP();
            NOP();
        }
        //Incrementa l'indirizzo della riga.
        rowAddr+=0x0020;
    }

    INTCONbits.GIE=1;
    PMCON1bits.WREN=0;
}	
