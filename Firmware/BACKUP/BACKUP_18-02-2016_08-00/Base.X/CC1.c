#include <xc.h>
#include "SPI.h"
#include "OUT.h"
#include "smartrf_CC1101.h"
#include "CC1.h"

#define SIZE_INIT_VAL 0x2E
#define SIZE_PATABLE 8

//Definizioni macro.
#define mCc1Init() IOCBP=0b00000000; IOCBN=0b00000000

unsigned char cc1InitVal[]=
{
	SMARTRF_SETTING_IOCFG2,
	SMARTRF_SETTING_IOCFG1,
	SMARTRF_SETTING_IOCFG0,
	SMARTRF_SETTING_FIFOTHR,
	SMARTRF_SETTING_SYNC1,
	SMARTRF_SETTING_SYNC0,
	SMARTRF_SETTING_PKTLEN,
	SMARTRF_SETTING_PKTCTRL1,
	SMARTRF_SETTING_PKTCTRL0,
	SMARTRF_SETTING_ADDR,
	SMARTRF_SETTING_CHANNR,
	SMARTRF_SETTING_FSCTRL1,
	SMARTRF_SETTING_FSCTRL0,
	SMARTRF_SETTING_FREQ2,
	SMARTRF_SETTING_FREQ1,
	SMARTRF_SETTING_FREQ0,
	SMARTRF_SETTING_MDMCFG4,
	SMARTRF_SETTING_MDMCFG3,
	SMARTRF_SETTING_MDMCFG2,
	SMARTRF_SETTING_MDMCFG1,
	SMARTRF_SETTING_MDMCFG0,
	SMARTRF_SETTING_DEVIATN,
	SMARTRF_SETTING_MCSM2,
	SMARTRF_SETTING_MCSM1,
	SMARTRF_SETTING_MCSM0,
	SMARTRF_SETTING_FOCCFG,
	SMARTRF_SETTING_BSCFG,
	SMARTRF_SETTING_AGCCTRL2,
	SMARTRF_SETTING_AGCCTRL1,
	SMARTRF_SETTING_AGCCTRL0,
	SMARTRF_SETTING_WOREVT1,
	SMARTRF_SETTING_WOREVT0,
	SMARTRF_SETTING_WORCTRL,
	SMARTRF_SETTING_FREND1,
	SMARTRF_SETTING_FREND0,
	SMARTRF_SETTING_FSCAL3,
	SMARTRF_SETTING_FSCAL2,
	SMARTRF_SETTING_FSCAL1,
	SMARTRF_SETTING_FSCAL0,
	SMARTRF_SETTING_RCCTRL1,
	SMARTRF_SETTING_RCCTRL0,
	SMARTRF_SETTING_FSTEST,
	SMARTRF_SETTING_PTEST,
	SMARTRF_SETTING_AGCTEST,
	SMARTRF_SETTING_TEST2,
	SMARTRF_SETTING_TEST1,
	SMARTRF_SETTING_TEST0
};

unsigned char cc1PaTable[]=
{
	0x03,
	0x0F,
	0x1E,
	0x27,
	0x50,
	0x81,
	0xCB,
	0xC2
};	

void CC1Init(unsigned char state)
{
	
	switch(state)
	{
		case 0:
			//Inizializzazione hardware.
			mCc1Init();
			break;
		case 1:
			//Inizializzazione variabili.
			break;
		case 2:
			//Inizializzazione applicazione.
			CC1Strobe(SRES);
			CC1Write(0x00,cc1InitVal,SIZE_INIT_VAL);
			CC1Write(0x3E,cc1PaTable,SIZE_PATABLE);
			break;
	}
}

void CC1Strobe(te_CC1_STB e_stb)
{
	unsigned char cc1Stb,i;

	//Imposta il CS.
        OUTSetSSOff();

	//Scrive il command strobe.
	cc1Stb=((e_stb&0b00111111)|0b10000000);
	SPIWrite((&(cc1Stb)),1);

	//Imposta il CS.
        OUTSetSSOn();
}

void CC1Read(unsigned char addr,unsigned char *dst,unsigned char len)
{
	unsigned char cc1Addr;
	
	//Imposta il CS.
        OUTSetSSOff();

	//Scrive l'indirizzo da cui iniziare a leggere i dati.
	cc1Addr=((addr&0b00111111)|0b11000000);
	SPIWrite((&(cc1Addr)),1);
	
	//Legge i dati.
	SPIRead(dst,len);
	
	//Imposta il CS.
        OUTSetSSOn();
}

void CC1Write(unsigned char addr,unsigned char *src,unsigned char len)
{
	unsigned char cc1Addr;

	//Imposta il CS.
        OUTSetSSOff();

	//Scrive l'indirizzo da cui iniziare a scrivere i dati.
	cc1Addr=((addr&0b00111111)|0b01000000);
	SPIWrite((&(cc1Addr)),1);
	
	//Scrive i dati.
	SPIWrite(src,len);
	
	//Imposta il CS.
        OUTSetSSOn();
}
