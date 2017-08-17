#include <xc.h>
#include <stdlib.h>
#include <stdio.h>
#include "Base.h"
#include "CC1.h"
#include "PGM.h"
#include "NVM.h"
#include "OUT.h"
#include "IN.h"
#include "DPY.h"
#include "DLY.h"
#include "smartrf_CC1101.h"
#include "RTX.h"

//Definizioni.
#define ADDR_CC1_FIFO		0x3F	//Indirizzo del CC1101 per la lettura del FIFO.
#define ADDR_CC1_STA		0x35	//Indirizzo del CC1101 per la lettura del byte di stato.
#define ADDR_CC1_BYTX		0x3A	//Indirizzo del CC1101 per la lettura del byte contenente il nuemro di byte in trasmissione e presenti nel FIFO.
#define ADDR_CC1_BYRX		0x3B	//Indirizzo del CC1101 per la lettura del byte contenente il nuemro di byte ricevuti e presenti nel FIFO.

#define TICK_TMRRCVPKT	10                         //Base tempi 10ms, 1s di timeout.
#define MAX_SNDDATA     5                           //Numero massimo di invii in caso di non ricezione della risposta.
#define PKT_LEN         SMARTRF_SETTING_PKTLEN      //Numero di byte per pacchetto.
#define TICK_VISBATT    500                         //Timeout per la visualizzazione della batteria.

#define MAX_DECNUM 2    //Numero massimo di decimali.

#define MAX_TMRCALIBRATION 1000          //Massimo valore per il timer di calibrazione.

#define VAL_NOMINAL 230 //Valore RMS nominale della tensione.

enum _POS_PKT
{
    ID_DEST=0,
    ID_SOURCE=1,
    TYPE=2,
    OP=3,
    VAL=4,
    BAT=6
}e_POS_PKT;

typedef enum _RTX_OP
{
    DAT=0x00,
    PGM=0x01,

    rDAT=0x80,
    rPGM=0x81
}te_RTX_OP;

typedef enum _DEV_TYPE
{
    BASE=0x00,
    SENS_CURRGEN=0x01,
    SENS_CURRCON=0x02,
    SENS_VOLT=0x03
}te_DEV_TYPE;

typedef union _RTX_DATA
{
    float val;
    unsigned char val_byte[3];  //un dato di tipo "float" per xc8 è di 3 byte.
}tu_RTX_DATA;

static char sta,numRxByte,numTxByte;
static unsigned char pktRx[PKT_LEN],pktTx[PKT_LEN],tmrRcvPkt;
static unsigned short tmrCalibration,tmrVisBatt;

static void RTXSndPkt(unsigned char *,unsigned char);
static void RTXRcvPkt(unsigned char *,unsigned char);
static void RTXSetSensSta(unsigned char,unsigned char);
static void RTXSendPgm(void);
static void RTXSendRData(unsigned char);
static void RTXSup(void);
static unsigned char RTXVisDec(float,char *,unsigned char,unsigned char);

void RTXInit(unsigned char state)
{
    switch(state)
    {
        case 0:
            //Inizializzazione hardware.
            break;
        case 1:
            //Inizializzazione variabili.
            tmrRcvPkt=0;
            rtx_flg.val=0;
            tmrCalibration=0;
            tmrVisBatt=0;
            gen_power=0;
            con_power=0;
            gen_curr_sens=0;
            con_curr_sens=0;
            volt_sens=0;
            batt_gen_curr_sens=0;
            batt_con_curr_sens=0;
            batt_volt_sens=0;
            break;
        case 2:
            //Avvia la calibrazione e la ricezione.
            CC1Strobe(SIDLE);
            CC1Strobe(SCAL);
            CC1Strobe(SRX);
            //Scrittua display.
            RTXUpdateDisplay(UPD_DPY_POWER);
            break;
	}
}

void RTXTick(void)
{
    //Chiama la funzione del supervisore.
    RTXSup();
    //Timer per la calibrazione.
    tmrCalibration++;
    if(tmrCalibration>MAX_TMRCALIBRATION)
    {
        tmrCalibration=0;
        CC1Strobe(SIDLE);
        CC1Strobe(SCAL);
        CC1Strobe(SFRX);
    }
		
    //Timer di attesa per la ricezione della risposta.
    if(tmrRcvPkt)
    {
        tmrRcvPkt--;
        if(!(tmrRcvPkt))
            RTXSendPgm();  //Se il timeout è scaduto, trasmette di nuovo il pacchetto di programmazione.
    }

    //Timer visualizzazione batteria.
    if(tmrVisBatt)
    {
        tmrVisBatt--;
        if(!(tmrVisBatt))
            RTXUpdateDisplay(UPD_DPY_POWER);
    }
}

void RTXTask(void)
{
    //Controlla se è stato ricevuto un pacchetto e quindi esegue la lettura dello stesso.
    if(!(INGetGDO2()))
        RTXReadPkt();
}

void RTXUpdateDisplay(te_RTX_UPD_DPY val)
{
    char charIdSens[4],strRow1[DPY_COL_NUM],strRow2[DPY_COL_NUM];
    unsigned char i,iR;

    //Inizializza i vettori.
    for(iR=0;iR<sizeof(strRow1)/sizeof(char);iR++)
        strRow1[iR]=' ';
    for(iR=0;iR<sizeof(strRow2)/sizeof(char);iR++)
        strRow2[iR]=' ';

    if(val==UPD_DPY_PGM || val==UPD_DPY_PGMDONECURRSENSGEN || val==UPD_DPY_PGMDONECURRSENSCON || val==UPD_DPY_PGMDONEVOLTSENS || val==UPD_DPY_PGMRST)
    {
        iR=0;
        strRow1[iR++]='P';
        strRow1[iR++]='r';
        strRow1[iR++]='o';
        strRow1[iR++]='g';
        strRow1[iR++]='r';
        strRow1[iR++]='a';
        strRow1[iR++]='m';
        strRow1[iR++]='m';
        strRow1[iR++]='a';
        strRow1[iR++]='z';
        strRow1[iR++]='i';
        strRow1[iR++]='o';
        strRow1[iR++]='n';
        strRow1[iR++]='e';
        strRow1[iR++]=':';
        if(val==UPD_DPY_PGMRST)
        {
            iR=5;
            strRow2[iR++]='R';
            strRow2[iR++]='E';
            strRow2[iR++]='S';
            strRow2[iR++]='E';
            strRow2[iR++]='T';
        }
        else
        {
            iR=5;
            strRow2[iR++]='I';
            strRow2[iR++]='D';
            strRow2[iR++]=':';
            if(val==UPD_DPY_PGM)
            {
                strRow2[iR++]='?';
                strRow2[iR++]='?';
                strRow2[iR++]='?';
            }
            else
            {
                if(val==UPD_DPY_PGMDONECURRSENSGEN)
                    itoa(charIdSens,pgm_cfg.gen_curr_sens.id,10);
                else if(val==UPD_DPY_PGMDONECURRSENSCON)
                    itoa(charIdSens,pgm_cfg.con_curr_sens.id,10);
                else if(val==UPD_DPY_PGMDONEVOLTSENS)
                    itoa(charIdSens,pgm_cfg.volt_sens.id,10);
                for(i=0;i<4;i++)
                {
                    if(charIdSens[i]==0x00)
                        break;
                    strRow2[iR++]=charIdSens[i];
                }
            }
        }
    }
    else if(val==UPD_DPY_BATT)
    {
        iR=0;
        strRow1[iR++]='B';
        strRow1[iR++]='a';
        strRow1[iR++]='t';
        strRow1[iR++]='t';
        strRow1[iR++]='.';
        strRow1[iR++]=':';
        strRow1[iR++]=' ';

        //Sensore corrente consumata.
        strRow1[iR++]='C';
        strRow1[iR++]='=';
        if(pgm_cfg.menu.con_curr_sensIsProg)
            iR=RTXVisDec(batt_con_curr_sens,strRow1,3,iR++)+1;
        else
        {
            strRow1[iR++]='?';
            strRow1[iR++]='.';
            strRow1[iR++]='?';
        }
        //Setta gli altri caratteri.
        strRow1[iR++]='V';
        
        //Sensore corrente prodotta.
        iR=0;
        strRow2[iR++]='P';
        strRow2[iR++]='=';
        if(pgm_cfg.menu.gen_curr_sensIsProg)
            iR=RTXVisDec(batt_gen_curr_sens,strRow2,3,iR++)+1;
        else
        {
            strRow2[iR++]='?';
            strRow2[iR++]='.';
            strRow2[iR++]='?';
        }
        //Setta gli altri caratteri.
        strRow2[iR++]='V';
        
        //Sensore tensione.
        strRow2[iR++]=' ';
        strRow2[iR++]='V';
        strRow2[iR++]='=';
        if(pgm_cfg.menu.volt_sensIsProg)
            iR=RTXVisDec(volt_sens,strRow2,3,iR++)+1;
        else
        {
            strRow2[iR++]='?';
            strRow2[iR++]='.';
            strRow2[iR++]='?';
        }
        //Setta gli altri caratteri.
        strRow2[iR++]='V';
    }
    else if(val==UPD_DPY_POWER)
    {
        //Potenza consumata.
        iR=0;
        strRow1[iR++]='C';
        strRow1[iR++]=':';
        strRow1[iR++]=' ';
        if(pgm_cfg.menu.con_curr_sensIsProg)
            iR=RTXVisDec(con_power,strRow1,3,iR)+1;
        else
        {
            strRow1[iR++]='?';
            strRow1[iR++]='?';
            strRow1[iR++]='?';
        }
        //Setta gli altri caratteri.
        strRow1[iR++]='W';
        strRow1[iR++]=' ';
        //Potenza prodotta.
        strRow1[iR++]='P';
        strRow1[iR++]=':';
        strRow1[iR++]=' ';
        if(pgm_cfg.menu.gen_curr_sensIsProg)
            iR=RTXVisDec(gen_power,strRow1,3,iR)+1;
        else
        {
            strRow1[iR++]='?';
            strRow1[iR++]='?';
            strRow1[iR++]='?';
        }
        //Setta gli altri caratteri.
        strRow1[iR++]='W';
        
        
        //Differenza potenza.
        iR=0;
        strRow2[iR++]='D';
        strRow2[iR++]=':';
        strRow2[iR++]=' ';
        if(pgm_cfg.menu.con_curr_sensIsProg && pgm_cfg.menu.gen_curr_sensIsProg)
            iR=RTXVisDec((con_power-gen_power),strRow2,3,iR)+1;
        else
        {
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='?';
        }
        //Setta gli altri caratteri.
        strRow2[iR++]='W';
        if((pgm_cfg.menu.con_curr_sensIsProg && batt_con_curr_sens<2.5) || (pgm_cfg.menu.gen_curr_sensIsProg && batt_gen_curr_sens<2.5) || (pgm_cfg.menu.volt_sensIsProg && batt_volt_sens<2.5))
            strRow2[sizeof(strRow2)/sizeof(char)-1]=0x00;
    }
    DPYSendStr(0,0,strRow1,sizeof(strRow1)/sizeof(char));
    DPYSendStr(1,0,strRow2,sizeof(strRow2)/sizeof(char));
}

static unsigned char RTXVisDec(float val,char *buff,unsigned char maxDigit,unsigned char offset)
{
    char *p;
    unsigned char strTmp[DPY_COL_NUM],isDec,posDec,i,j;
    int status;

    p=ftoa(val,(&(status)));
    //Ricava il vettore e la sua lunghezza.
    isDec=0;
    for(i=0;i<sizeof(strTmp)/sizeof(char);i++)
    {
        //Controlla la posizione della virgola.
        if((*(p+i))=='.')
        {
            isDec=1;
            posDec=i;
        }
        //Calcola il numero di cifre decimali.
        if(isDec)
        {
            if((i-posDec)>MAX_DECNUM)
                break;
        }
        //Controlla se è arrivato al carattere di fine stringa.
        if((*(p+i))==0x00)
            break;
        //Copia i caratteri.
        strTmp[i]=(*(p+i));
    }
    
    //Controlla il massimo numero di cifre scrivibili.
    if(i>=maxDigit)
        i=maxDigit-1;

    //Copia i valori.
    for(j=0;j<i;j++)
        (*(buff+j+offset))=strTmp[j];

    return (j+offset);  //Ritorna la posizione del cursore.
}

static void RTXSup(void)
{
    //Legge lo stato e attiva l'operazione appropriata.
    CC1Read(ADDR_CC1_STA,(&(sta)),1);

    //Operazione da eseguire in seguito alla lettura dello stato.
    switch(sta)
    {
        case IDLE:
            CC1Strobe(SRX);
            break;
        case RXFIFO_OVERFLOW:
            CC1Strobe(SFRX);
            CC1Strobe(SRX);
            break;
        case TXFIFO_UNDERFLOW:
            CC1Strobe(SFTX);
            CC1Strobe(SRX);
            break;
    }
}

void RTXSelDown(void)
{
    if(pgm_cfg.menu.val)
    {
        if(tmrVisBatt)
        {
            //Visualizza la temperatura.
            RTXUpdateDisplay(UPD_DPY_POWER);
            //Arresta il timer.
            tmrVisBatt=0;
        }
        else
        {
            //Visualizza la tensione della batteria.
            RTXUpdateDisplay(UPD_DPY_BATT);
            //Avvia il timer.
            tmrVisBatt=TICK_VISBATT;
        }
    }
}

static void RTXSndPkt(unsigned char *src,unsigned char len)
{
    //Legge lo stato e attende che il CC1101 è in idle o in ricezione.
    do
    {
        CLRWDT();
        CC1Read(ADDR_CC1_STA,(&(sta)),1);
    }
    while(!(((sta&0b00011111)==0x0D) || ((sta&0b00011111)==0x01)));
    //Setta lo stato di idle.
    CC1Strobe(SIDLE);
    CC1Read(ADDR_CC1_BYRX,(&(numRxByte)),1);
    CC1Read(ADDR_CC1_BYTX,(&(numTxByte)),1);
    //Esegue il flush del TX.
    CC1Strobe(SFTX);
    CC1Read(ADDR_CC1_BYTX,(&(numTxByte)),1);
    //Carica il buffer di trasmissione.
    CC1Write(ADDR_CC1_FIFO,src,len);
    //Avvia la trasmissione (terminata la trasmissione il CC1101 passa automaticamente allo stato di ricezione).
    CC1Strobe(STX);
}

static void RTXRcvPkt(unsigned char *dst,unsigned char len)
{
    //Legge il buffer di ricezione.
    CC1Read(ADDR_CC1_FIFO,dst,len);
}

void RTXReadPkt(void)
{
    tu_RTX_DATA valVoltCurr,valBatt;
    float volt4Calc;
    
    //Legge il numero di byte presenti nel FIFO.
    CC1Read(ADDR_CC1_BYRX,(&(numRxByte)),1);
    while(numRxByte>=PKT_LEN)
    {
        CLRWDT();
        //Legge un pacchetto.
        RTXRcvPkt(pktRx,PKT_LEN);
        //Controlla se è attiva la programmazione.
        if(rtx_flg.pgmonoff)
        {
            if((pktRx[ID_DEST]==pgm_cfg.base.id))  //Controlla se l'id della base corrisponde.
            {
                //Controlla se il pacchetto ricevuto è di programmazione.
                if(pktRx[OP]==rPGM)
                {
                    //Memorizza il nuovo ID del sensore.
                    if(pktRx[TYPE]==SENS_VOLT)
                    {
                        //Setta l'id, il tipo e il flag di programmazione.
                        pgm_cfg.volt_sens.id=pktRx[ID_SOURCE];
                        pgm_cfg.volt_sens.type=pktRx[TYPE];
                        pgm_cfg.menu.volt_sensIsProg=1;
                    }
                    else if(pktRx[TYPE]==SENS_CURRGEN)
                    {
                        //Setta l'id, il tipo e il flag di programmazione.
                        pgm_cfg.gen_curr_sens.id=pktRx[ID_SOURCE];
                        pgm_cfg.gen_curr_sens.type=pktRx[TYPE];
                        pgm_cfg.menu.gen_curr_sensIsProg=1;
                    }
                    else if(pktRx[TYPE]==SENS_CURRCON)
                    {
                        //Setta l'id, il tipo e il flag di programmazione.
                        pgm_cfg.con_curr_sens.id=pktRx[ID_SOURCE];
                        pgm_cfg.con_curr_sens.type=pktRx[TYPE];
                        pgm_cfg.menu.con_curr_sensIsProg=1;
                    }
                    PGMWriteNvm();
                    //Segnala che la programmazione è stata eseguita.
                    rtx_flg.pgmDone=1;
                    //Arresta l'invio dei pacchetti di programmazione (è sufficiente arresta il time-out).
                    tmrRcvPkt=0;
                    //Cambia la visualizzazione del display.
                    if(pktRx[TYPE]==SENS_VOLT)
                        RTXUpdateDisplay(UPD_DPY_PGMDONEVOLTSENS);
                    else if(pktRx[TYPE]==SENS_CURRGEN)
                        RTXUpdateDisplay(UPD_DPY_PGMDONECURRSENSGEN);
                    else if(pktRx[TYPE]==SENS_CURRCON)
                        RTXUpdateDisplay(UPD_DPY_PGMDONECURRSENSCON);
                    //Ritardo di visualizzazione.
                    DLYDelay_ms(2000);
                    //Cambia la visualizzazione del display.
                    RTXUpdateDisplay(UPD_DPY_POWER);
                    //Chiama la funzione per l'arresto della programmazione.
                    RTXPgmOff();
                }
            }
        }
        else
        {
            if((pktRx[ID_DEST]==pgm_cfg.base.id) && ((pktRx[ID_SOURCE]==pgm_cfg.gen_curr_sens.id) || (pktRx[ID_SOURCE]==pgm_cfg.con_curr_sens.id)) || (pktRx[ID_SOURCE]==pgm_cfg.volt_sens.id))
            {
                //Controlla se il pacchetto ricevuto è di risposta al dato.
                if(pktRx[OP]==DAT)
                {
                    //Ricava il valore della corrente o della tensione.
                    valVoltCurr.val_byte[0]=pktRx[VAL];
                    valVoltCurr.val_byte[1]=pktRx[VAL+1];
                    valVoltCurr.val_byte[2]=pktRx[VAL+2];
                    //Ricava il valore della batteria.
                    valBatt.val_byte[0]=pktRx[BAT];
                    valBatt.val_byte[1]=pktRx[BAT+1];
                    valBatt.val_byte[2]=pktRx[BAT+2];
                    //Memorizza i valori.
                    if((pktRx[TYPE]==SENS_CURRGEN) && pgm_cfg.menu.gen_curr_sensIsProg && (pktRx[ID_SOURCE]==pgm_cfg.gen_curr_sens.id))
                    {
                        gen_curr_sens=valVoltCurr.val;
                        batt_gen_curr_sens=valBatt.val;
                    }
                    else if((pktRx[TYPE]==SENS_CURRCON) && pgm_cfg.menu.gen_curr_sensIsProg && (pktRx[ID_SOURCE]==pgm_cfg.con_curr_sens.id))
                    {
                        con_curr_sens=valVoltCurr.val;
                        batt_con_curr_sens=valBatt.val;
                    }
                    else if((pktRx[TYPE]==SENS_VOLT) && pgm_cfg.menu.volt_sensIsProg && (pktRx[ID_SOURCE]==pgm_cfg.volt_sens.id))
                    {
                        volt_sens=valVoltCurr.val;
                        batt_volt_sens=valBatt.val;
                    }
                    
                    //Controlla il valore di tensione da usare.
                    volt4Calc=VAL_NOMINAL;
                    if(pgm_cfg.menu.volt_sensIsProg)
                        volt4Calc=volt_sens;
                    //Calcola le potenze.
                    if(pgm_cfg.menu.gen_curr_sensIsProg)
                        gen_power=volt4Calc*gen_curr_sens;
                    if(pgm_cfg.menu.con_curr_sensIsProg)
                        con_power=volt4Calc*con_curr_sens;
                        
                    //Invia la risposta ai dati ricevuti.
                    RTXSendRData(pktRx[ID_SOURCE]);
                    //Aggiorna il valore sul display.
                    if(tmrVisBatt)
                        RTXUpdateDisplay(UPD_DPY_BATT);
                    else
                        RTXUpdateDisplay(UPD_DPY_POWER);
                }
            }
        }
        //Legge il numero di byte rimanenti.
        CC1Read(ADDR_CC1_BYRX,(&(numRxByte)),1);
        //Esegue un controllo sullo stato.
        RTXSup();
    }
}

static void RTXSendRData(unsigned char sensId)
{
    //Crea il pacchetto da inviare.
    pktTx[ID_DEST]=sensId;
    pktTx[ID_SOURCE]=pgm_cfg.base.id;
    pktTx[TYPE]=BASE;
    pktTx[OP]=rDAT;
    pktTx[VAL]=0x00;
    pktTx[VAL+1]=0x00;
    pktTx[VAL+2]=0x00;
    pktTx[BAT]=0x00;
    pktTx[BAT+1]=0x00;
    pktTx[BAT+2]=0x00;

    //Invia il pacchetto.
    RTXSndPkt(pktTx,PKT_LEN);
}

static void RTXSendPgm(void)
{
    //Crea il pacchetto da inviare.
    pktTx[ID_DEST]=0;       //invio broadcast;
    pktTx[ID_SOURCE]=pgm_cfg.base.id;
    pktTx[TYPE]=BASE;
    pktTx[OP]=PGM;
    pktTx[VAL]=0x00;
    pktTx[VAL+1]=0x00;
    pktTx[VAL+2]=0x00;
    pktTx[BAT]=0x00;
    pktTx[BAT+1]=0x00;
    pktTx[BAT+2]=0x00;

    //Invia il pacchetto.
    RTXSndPkt(pktTx,PKT_LEN);

    //Avvia il timer per il rinvio del pacchetto.
    tmrRcvPkt=TICK_TMRRCVPKT;
}

void RTXPgmOn(void)
{
    //Setta il flag.
    rtx_flg.pgmonoff=1;
    //Avvia la trasmissione di un pacchetto di programmazione.
    RTXSendPgm();
}

void RTXPgmOff(void)
{
    //Setta il flag.
    rtx_flg.pgmonoff=0;
    //Arresta l'invio dei pacchetti di programmazione (è sufficiente arresta il time-out).
    tmrRcvPkt=0;
    //Arresta il timer per la visualizzazione della batteria (in caso di attivazione dopo reset).
    tmrVisBatt=0;
}
