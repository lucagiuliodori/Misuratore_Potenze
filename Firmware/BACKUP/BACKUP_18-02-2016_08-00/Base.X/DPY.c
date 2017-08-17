#include <xc.h>
#include "DLY.h"
#include "OUT.h"
#include "DPY.h"
#include "IN.h"

#define ADDR_PICBATT 0x00

//#define _USE_READBYTE

typedef enum _DPY_RS
{
    RS_ISTRUCTION,
    RS_DATA
}te_DPY_RS;

typedef enum _DPY_BF
{
    BF_NOTBUSY=0,
    BF_BUSY=1
}te_DPY_BF;

unsigned char picBatt[9]=
{
    0b00001110,
    0b00011111,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00011111,
};

static void DPYInitDisplay(void);
static void DPYWriteNibble(te_DPY_RS,unsigned char);
static void DPYWriteByte(te_DPY_RS,unsigned char);
#ifdef _USE_READBYTE
static te_DPY_BF DPYReadByte(te_DPY_RS,unsigned char *);
#endif
static void DPYSendChar(unsigned char);
static void DPYSetCur(unsigned char,unsigned char);
static void DPYDrawChar(unsigned char,char *);
static te_DPY_BF DPYIsBusy(void);

void DPYInit(unsigned char sta)
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
            DPYInitDisplay();
            break;
    }
}

static void DPYInitDisplay(void)
{
    unsigned char tmp;

    DLYDelay_ms(15);
    OUTSetRSOff();
    OUTSetRWOff();

    //Imosta la modalità a 4 bit.
    DPYWriteNibble(RS_ISTRUCTION,0b00000010);

    //Imposta la modalità a 4 bit, il numero di linee e la grandezza del font.
    tmp=0b00100000;
    #ifdef DPY_N_ONE
    //Nessuna istruzione.
    #endif
    #ifdef DPY_N_TWO
    tmp|=0b00001000;
    #endif
    #ifdef DPY_F_5X8
    //Nessuna istruzione.
    #endif
    #ifdef DPY_F_5X10
    tmp|=0b00000100;
    #endif
    DPYWriteByte(RS_ISTRUCTION,tmp);

    //Accende il display e il cursore.
    DPYWriteByte(RS_ISTRUCTION,0b00001110);

    //Setta la modalità di incremento.
    DPYWriteByte(RS_ISTRUCTION,0b00000110);

    //Cancella il display.
    DPYWriteByte(RS_ISTRUCTION,0b00000001);

    //Disegna il carattere per la batteria.
    DPYDrawChar(ADDR_PICBATT,picBatt);
}

static void DPYDrawChar(unsigned char addr,char *p)
{
    unsigned char i,tmpAddr;
    //Setta l'indirizzo e il dato.
    for(i=0;i<9;i++)
    {
        tmpAddr=(((i+addr)&0b00111111)|0b01000000);
        DPYWriteByte(RS_ISTRUCTION,tmpAddr);
        DPYWriteByte(RS_DATA,(*(p+i)));
    }
}

static void DPYSetCur(unsigned char row,unsigned char col)
{
    unsigned char addr;

    //Ricava l'indirizzo per la scrittura del carattere.
    addr=col;
    //Controlla se è la seconda riga.
    if(row==1)
        addr+=0x40;

    //Setta l'indirizzo.
    DPYWriteByte(RS_ISTRUCTION,(addr|0b10000000));
}

static void DPYSendChar(unsigned char c)
{
    //Scrive il carattere.
    DPYWriteByte(RS_DATA,c);
}

void DPYSendStr(unsigned char row,unsigned char col,char* data,unsigned char len)
{
    unsigned char i;

    //Setta la posizione iniziale del cursore.
    DPYSetCur(row,col);

    for(i=0;i<len;i++)
    {
        //Invia i caratteri.
        DPYSendChar((*(data+i)));
        while(DPYIsBusy())
            NOP();

        //Setta il cursore per il prossimo carattere.
        col++;
        if(col==DPY_COL_NUM)
        {
            col=0;
            row++;
            DPYSetCur(row,col);
        }
        else if(col==32)
        {
            col=0;
            row=0;
            DPYSetCur(row,col);
        }
    }
}

static void DPYWriteNibble(te_DPY_RS rs,unsigned char val)
{
    //N.B.: vengono considerati solo i 4 bit meno significativi.

    //Settaggio la linea E.
    OUTSetEOff();

    //Settaggio la linea RS.
    if(rs==RS_ISTRUCTION)
        OUTSetRSOff();
    else if(rs==RS_DATA)
        OUTSetRSOn();

    //Settaggio la linea RW.
    OUTSetRWOff();

    //Attesa.
    DLYDelay_us(100);

    //Settaggio la linea E.
    OUTSetEOn();
    //Attesa.
    DLYDelay_us(100);
    //Settaggio linee dati (MSB).
    OUTSetHTCDataNibble(val&0x0F);
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(100);
}

static void DPYWriteByte(te_DPY_RS rs,unsigned char val)
{
    //Settaggio la linea E.
    OUTSetEOff();

    //Settaggio la linea RS.
    if(rs==RS_ISTRUCTION)
        OUTSetRSOff();
    else if(rs==RS_DATA)
        OUTSetRSOn();

    //Settaggio la linea RW.
    OUTSetRWOff();

    //Attesa.
    DLYDelay_us(100);

    //Settaggio la linea E.
    OUTSetEOn();
    //Attesa.
    DLYDelay_us(100);
    //Settaggio linee dati (MSB).
    OUTSetHTCDataNibble((val>>4)&0b00001111);
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOn();
    //Attesa.
    DLYDelay_us(100);
    //Settaggio linee dati (LSB).
    OUTSetHTCDataNibble(val&0b00001111);
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(100);
}

#ifdef _USE_READBYTE
static te_DPY_BF DPYReadByte(te_DPY_RS rs,unsigned char *val)
{
    //Azzeramento valore.
    (*(val))=0;

    //Settaggio direzione pin.
    tris_DPY|=0b00001111;

    //Settaggio la linea E.
    OUTSetEOff();

    //Settaggio la linea RS.
    if(rs==RS_ISTRUCTION)
        OUTSetRSOff();
    else if(rs==RS_DATA)
        OUTSetRSOn();

    //Settaggio la linea RW.
    OUTSetRWOn();

    //Attesa.
    DLYDelay_us(100);

    //Settaggio la linea E.
    OUTSetEOn();
    //Settaggio linee dati (MSB).
    if(INGetD7())
        (*(val))|=0b10000000;
    if(INGetD6())
        (*(val))|=0b01000000;
    if(INGetD5())
        (*(val))|=0b00100000;
    if(INGetD4())
        (*(val))|=0b00010000;
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOn();
    //Settaggio linee dati (LSB).
    if(INGetD7())
        (*(val))|=0b00001000;
    if(INGetD6())
        (*(val))|=0b00000100;
    if(INGetD5())
        (*(val))|=0b00000010;
    if(INGetD4())
        (*(val))|=0b00000001;
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(100);

    //Settaggio direzione pin.
    tris_DPY&=0b11110000;

    if((*(val))&0b10000000)
        return BF_BUSY;
    else
        return BF_NOTBUSY;
}
#endif

static te_DPY_BF DPYIsBusy(void)
{
    te_DPY_BF bf;
    
    //Settaggio direzione pin.
    tris_DPY|=0b00001111;

    //Settaggio la linea E.
    OUTSetEOff();

    //Settaggio la linea RS.
    OUTSetRSOff();

    //Settaggio linea RW.
    OUTSetRWOn();

    //Attesa.
    DLYDelay_us(100);

    //Settaggio la linea E.
    OUTSetEOn();
    //Settaggio busy flag.
    bf=BF_NOTBUSY;
    if(INGetD7())
        bf=BF_BUSY;
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOn();
    //Attesa.
    DLYDelay_us(100);
    //Settaggio la linea E.
    OUTSetEOff();
    //Attesa.
    DLYDelay_us(100);

    //Settaggio direzione pin.
    tris_DPY&=0b11110000;

    return bf;
}
