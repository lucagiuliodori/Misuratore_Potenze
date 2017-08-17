
#include <xc.h>
#include "SPI.h"

//Definizioni macro.
#define mSpiInit() TRISC|=0b00010000; TRISC&=0b11010111; SSPCONbits.SSPEN=0; SSPBUF=0x00; SSPCON1=0b00000001; SSPCON2=0b00000000; SSPSTAT=0b01000000; SSPCONbits.SSPEN=1

void SPIInit(unsigned char state)
{
	switch(state)
	{
		case 0:
			//Inizializzazione hardware.
			mSpiInit();
			break;
		case 1:
			//Inizializzazione variabili.
			break;
		case 2:
			//Inizializzazione applicazione.
			break;
	}
}

void SPIRead(unsigned char *dstPtr,unsigned char len)
{
	unsigned char i;
	
	for(i=0;i<len;i++)
	{
		PIR1bits.SSPIF=0;
		SSPBUF=0xFF;
		while(!(PIR1bits.SSPIF))
			NOP();
		(*(dstPtr+i))=SSPBUF;
	}
}

void SPIWrite(unsigned char *srcPtr,unsigned char len)
{
	unsigned char i;
	
	for(i=0;i<len;i++)
	{
		PIR1bits.SSPIF=0;
		SSPBUF=(*(srcPtr+i));
		while(!(PIR1bits.SSPIF))
			NOP();
	}
}
