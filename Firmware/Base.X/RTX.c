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
#define TICK_VIS        500                         //Timeout per la visualizzazione.

#define MAX_DECNUM 2    //Numero massimo di decimali.

#define MAX_TMRCALIBRATION 1000          //Massimo valore per il timer di calibrazione.

#define VAL_NOMINAL 230 //Valore RMS nominale della tensione.

enum _POS_PKT
{
    ID_DEST=0,
    TYPE_DEST=1,
    ID_SOURCE=2,
    TYPE_SOURCE=3,
    OP=4,
    VAL=5,
    BAT=8
};

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
static unsigned char pktRx[PKT_LEN],pktTx[PKT_LEN];
static unsigned short tmrCalibration,tmrVis;
static te_RTX_UPD_DPY visTypeDpy;

static void RTXSndPkt(unsigned char *,unsigned char);
static void RTXRcvPkt(unsigned char *,unsigned char);
static void RTXSetSensSta(unsigned char,unsigned char);
static void RTXSendRPgm(unsigned char id,te_DEV_TYPE type);
static void RTXSendRData(unsigned char id,te_DEV_TYPE type);
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
            rtx_flg.val=0;
            tmrCalibration=0;
            tmrVis=0;
            gen_power=0;
            con_power=0;
            gen_curr_sens=0;
            con_curr_sens=0;
            volt_sens=0;
            batt_gen_curr_sens=0;
            batt_con_curr_sens=0;
            batt_volt_sens=0;
            visTypeDpy=UPD_DPY_POWER;
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
		
    //Timer visualizzazione.
    if(tmrVis)
    {
        tmrVis--;
        if(!(tmrVis))
        {
            visTypeDpy=UPD_DPY_POWER;
            RTXUpdateDisplay(UPD_DPY_POWER);
        }
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
    float diffPower;
    char charIdSens[4],strRow1[DPY_COL_NUM],strRow2[DPY_COL_NUM];
    unsigned char i,iR;

    //Inizializza i vettori.
    for(iR=0;iR<sizeof(strRow1)/sizeof(char);iR++)
        strRow1[iR]=' ';
    for(iR=0;iR<sizeof(strRow2)/sizeof(char);iR++)
        strRow2[iR]=' ';

    if(val==UPD_DPY_PGMDONECURRSENSGEN || val==UPD_DPY_PGMDONECURRSENSCON || val==UPD_DPY_PGMDONEVOLTSENS || val==UPD_DPY_PGMRST)
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
            if(val==UPD_DPY_PGMDONECURRSENSGEN)
                itoa(charIdSens,pgm_cfg.gen_curr_sens_id,10);
            else if(val==UPD_DPY_PGMDONECURRSENSCON)
                itoa(charIdSens,pgm_cfg.con_curr_sens_id,10);
            else if(val==UPD_DPY_PGMDONEVOLTSENS)
                itoa(charIdSens,pgm_cfg.volt_sens_id,10);
            for(i=0;i<4;i++)
            {
                if(charIdSens[i]==0x00)
                    break;
                strRow2[iR++]=charIdSens[i];
            }
        }
    }
    else if(val==UPD_DPY_CURRSENSGEN)
    {
        iR=0;
        strRow1[iR++]='P';
        strRow1[iR++]='o';
        strRow1[iR++]='t';
        strRow1[iR++]='.';
        strRow1[iR++]=' ';
        strRow1[iR++]='g';
        strRow1[iR++]='e';
        strRow1[iR++]='n';
        strRow1[iR++]='e';
        strRow1[iR++]='r';
        strRow1[iR++]='a';
        strRow1[iR++]='t';
        strRow1[iR++]='a';

        //Sensore corrente consumata.
        iR=0;
        if(pgm_cfg.menu.gen_curr_sensIsProg)
        {
            iR=RTXVisDec(gen_power,strRow2,4,iR++)+1;
            strRow2[iR++]='W';
            strRow2[iR++]=' ';
            strRow2[iR++]='-';
            strRow2[iR++]=' ';
            strRow2[iR++]=0x00;     //Simbolo batteria.
            strRow2[iR++]='=';
            iR=RTXVisDec(batt_gen_curr_sens,strRow2,3,iR++)+1;
            strRow2[iR++]='V';
        }
        else
        {
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='W';
            strRow2[iR++]=' ';
            strRow2[iR++]='-';
            strRow2[iR++]=' ';
            strRow2[iR++]=0x00;     //Simbolo batteria.
            strRow2[iR++]='=';
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='V';
        }
    }
    else if(val==UPD_DPY_CURRSENSCON)
    {
        iR=0;
        strRow1[iR++]='P';
        strRow1[iR++]='o';
        strRow1[iR++]='t';
        strRow1[iR++]='.';
        strRow1[iR++]=' ';
        strRow1[iR++]='c';
        strRow1[iR++]='o';
        strRow1[iR++]='n';
        strRow1[iR++]='s';
        strRow1[iR++]='u';
        strRow1[iR++]='m';
        strRow1[iR++]='a';
        strRow1[iR++]='t';
        strRow1[iR++]='a';

        //Sensore corrente consumata.
        iR=0;
        if(pgm_cfg.menu.gen_curr_sensIsProg)
        {
            iR=RTXVisDec(con_power,strRow2,4,iR++)+1;
            strRow2[iR++]='W';
            strRow2[iR++]=' ';
            strRow2[iR++]='-';
            strRow2[iR++]=' ';
            strRow2[iR++]=0x00;     //Simbolo batteria.
            strRow2[iR++]='=';
            iR=RTXVisDec(batt_con_curr_sens,strRow2,3,iR++)+1;
            strRow2[iR++]='V';
        }
        else
        {
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='W';
            strRow2[iR++]=' ';
            strRow2[iR++]='-';
            strRow2[iR++]=' ';
            strRow2[iR++]=0x00;     //Simbolo batteria.
            strRow2[iR++]='=';
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='V';
        }
    }
    else if(val==UPD_DPY_VOLTSENS)
    {
        iR=0;
        strRow1[iR++]='T';
        strRow1[iR++]='e';
        strRow1[iR++]='n';
        strRow1[iR++]='s';
        strRow1[iR++]='.';
        strRow1[iR++]=' ';
        strRow1[iR++]='m';
        strRow1[iR++]='i';
        strRow1[iR++]='s';
        strRow1[iR++]='u';
        strRow1[iR++]='r';
        strRow1[iR++]='a';
        strRow1[iR++]='t';
        strRow1[iR++]='a';

        //Sensore corrente consumata.
        iR=0;
        if(pgm_cfg.menu.gen_curr_sensIsProg)
        {
            iR=RTXVisDec(volt_sens,strRow2,4,iR++)+1;
            strRow2[iR++]='V';
            strRow2[iR++]=' ';
            strRow2[iR++]='-';
            strRow2[iR++]=' ';
            strRow2[iR++]=0x00;     //Simbolo batteria.
            strRow2[iR++]='=';
            iR=RTXVisDec(batt_volt_sens,strRow2,3,iR++)+1;
            strRow2[iR++]='V';
        }
        else
        {
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='V';
            strRow2[iR++]=' ';
            strRow2[iR++]='-';
            strRow2[iR++]=' ';
            strRow2[iR++]=0x00;     //Simbolo batteria.
            strRow2[iR++]='=';
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='?';
            strRow2[iR++]='V';
        }
    }
    else if(val==UPD_DPY_POWER)
    {
        //Calcola la differenza tra potenze.
        diffPower=gen_power-con_power;
        //Visualizza la potenza.
        iR=0;
        strRow1[iR++]='S';
        strRow1[iR++]='t';
        strRow1[iR++]='a';
        strRow1[iR++]='i';
        strRow1[iR++]=' ';
        if(diffPower>0)
        {
            strRow1[iR++]='g';
            strRow1[iR++]='e';
            strRow1[iR++]='n';
            strRow1[iR++]='e';
            strRow1[iR++]='r';
            strRow1[iR++]='a';
            strRow1[iR++]='n';
            strRow1[iR++]='d';
            strRow1[iR++]='o';
        }
        else
        {
            strRow1[iR++]='c';
            strRow1[iR++]='o';
            strRow1[iR++]='n';
            strRow1[iR++]='s';
            strRow1[iR++]='u';
            strRow1[iR++]='m';
            strRow1[iR++]='a';
            strRow1[iR++]='n';
            strRow1[iR++]='d';
            strRow1[iR++]='o';
            diffPower=-diffPower;
        }
        //Visualizza la potenza.
        iR=0;
        iR=RTXVisDec(diffPower,strRow2,4,iR)+1;
        //Setta gli altri caratteri.
        strRow2[iR++]='W';
        strRow2[iR++]=' ';
        //Visualizza il simbolo della batteria se ne risulta una scarica.
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
        CLRWDT();
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
    //Setta il tipo di visualizzazione.
    if(visTypeDpy==UPD_DPY_VOLTSENS)
        visTypeDpy=UPD_DPY_POWER;
    else
        visTypeDpy++;
    //Controlla se avviare il timer di visualizzazione.
    if(visTypeDpy!=UPD_DPY_POWER)
        tmrVis=TICK_VIS;
    else
        tmrVis=0;
    //Setta il display.
    RTXUpdateDisplay(visTypeDpy);
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
    unsigned char dev_id;
    te_DEV_TYPE dev_type;
    tu_RTX_DATA valVoltCurr,valBatt;
    float volt4Calc;
    
    //Legge il numero di byte presenti nel FIFO.
    CC1Read(ADDR_CC1_BYRX,(&(numRxByte)),1);
    while(numRxByte>=PKT_LEN)
    {
        CLRWDT();
        //Legge un pacchetto.
        RTXRcvPkt(pktRx,PKT_LEN);
        //Controlla se il pacchetto ricevuto è di programmazione.
        if(pktRx[OP]==PGM)
        {
            //Controlla se è una comunicazione in broadcast e se è indirizzata ad una base.
            if(pktRx[TYPE_DEST]==BASE && pktRx[ID_DEST]==0x00)
            {
                //Memorizza il nuovo ID del sensore.
                if(pktRx[TYPE_SOURCE]==SENS_VOLT && (!(pgm_cfg.menu.volt_sensIsProg)))
                {
                    //Setta l'id, il tipo e il flag di programmazione.
                    pgm_cfg.volt_sens_id=pktRx[ID_SOURCE];
                    pgm_cfg.menu.volt_sensIsProg=1;
                    //Setta i dati per la risposta.
                    dev_id=pktRx[ID_SOURCE];
                    dev_type=pktRx[TYPE_SOURCE];
                    //Setta il flag di programmazione avvenuta.
                    rtx_flg.pgmDone=1;
                }
                else if(pktRx[TYPE_SOURCE]==SENS_CURRGEN && (!(pgm_cfg.menu.gen_curr_sensIsProg)))
                {
                    //Setta l'id, il tipo e il flag di programmazione.
                    pgm_cfg.gen_curr_sens_id=pktRx[ID_SOURCE];
                    pgm_cfg.menu.gen_curr_sensIsProg=1;
                    //Setta i dati per la risposta.
                    dev_id=pktRx[ID_SOURCE];
                    dev_type=pktRx[TYPE_SOURCE];
                    //Setta il flag di programmazione avvenuta.
                    rtx_flg.pgmDone=1;
                }
                else if(pktRx[TYPE_SOURCE]==SENS_CURRCON && (!(pgm_cfg.menu.con_curr_sensIsProg)))
                {
                    //Setta l'id, il tipo e il flag di programmazione.
                    pgm_cfg.con_curr_sens_id=pktRx[ID_SOURCE];
                    pgm_cfg.menu.con_curr_sensIsProg=1;
                    //Setta i dati per la risposta.
                    dev_id=pktRx[ID_SOURCE];
                    dev_type=pktRx[TYPE_SOURCE];
                    //Setta il flag di programmazione avvenuta.
                    rtx_flg.pgmDone=1;
                }
                //Controlla se è stata eseguita una programmazione.
                if(rtx_flg.pgmDone)
                {
                    //Cancella il flag.
                    rtx_flg.pgmDone=0;
                    //Esegue la memorizzazione dei dati.
                    PGMWriteNvm();
                    //Invia la risposta ai dati ricevuti.
                    RTXSendRPgm(dev_id,dev_type);
                    //Cambia la visualizzazione del display.
                    if(pktRx[TYPE_SOURCE]==SENS_VOLT)
                        RTXUpdateDisplay(UPD_DPY_PGMDONEVOLTSENS);
                    else if(pktRx[TYPE_SOURCE]==SENS_CURRGEN)
                        RTXUpdateDisplay(UPD_DPY_PGMDONECURRSENSGEN);
                    else if(pktRx[TYPE_SOURCE]==SENS_CURRCON)
                        RTXUpdateDisplay(UPD_DPY_PGMDONECURRSENSCON);
                    //Ritardo di visualizzazione.
                    DLYDelay_ms(2000);
                    //Cambia la visualizzazione del display.
                    RTXUpdateDisplay(UPD_DPY_POWER);
                }
            }
        }
        //Controlla se il pacchetto ricevuto è di risposta al dato.
        else if(pktRx[OP]==DAT)
        {
            if(pktRx[TYPE_DEST]==BASE && pktRx[ID_DEST]==pgm_cfg.base_id)
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
                if(pktRx[TYPE_SOURCE]==SENS_CURRGEN && pgm_cfg.menu.gen_curr_sensIsProg && (pktRx[ID_SOURCE]==pgm_cfg.gen_curr_sens_id))
                {
                    gen_curr_sens=valVoltCurr.val;
                    batt_gen_curr_sens=valBatt.val;
                    //Setta i dati per la risposta.
                    dev_id=pktRx[ID_SOURCE];
                    dev_type=pktRx[TYPE_SOURCE];
                    //Setta il flag per indicare che i dati sono arrivati correttamente.
                    rtx_flg.dataReceived=1;
                }
                else if((pktRx[TYPE_SOURCE]==SENS_CURRCON) && pgm_cfg.menu.gen_curr_sensIsProg && (pktRx[ID_SOURCE]==pgm_cfg.con_curr_sens_id))
                {
                    con_curr_sens=valVoltCurr.val;
                    batt_con_curr_sens=valBatt.val;
                    //Setta i dati per la risposta.
                    dev_id=pktRx[ID_SOURCE];
                    dev_type=pktRx[TYPE_SOURCE];
                    //Setta il flag per indicare che i dati sono arrivati correttamente.
                    rtx_flg.dataReceived=1;
                }
                else if((pktRx[TYPE_SOURCE]==SENS_VOLT) && pgm_cfg.menu.volt_sensIsProg && (pktRx[ID_SOURCE]==pgm_cfg.volt_sens_id))
                {
                    volt_sens=valVoltCurr.val;
                    batt_volt_sens=valBatt.val;
                    //Setta i dati per la risposta.
                    dev_id=pktRx[ID_SOURCE];
                    dev_type=pktRx[TYPE_SOURCE];
                    //Setta il flag per indicare che i dati sono arrivati correttamente.
                    rtx_flg.dataReceived=1;
                }
                
                //Controlla se i dati sono stati ricevuti correttamente.
                if(rtx_flg.dataReceived)
                {
                    //Cancella il falg.
                    rtx_flg.dataReceived=0;
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
                    RTXSendRData(dev_id,dev_type);
                    //Aggiorna il valore sul display.
                    if(!(tmrVis))
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

static void RTXSendRData(unsigned char id,te_DEV_TYPE type)
{
    //Crea il pacchetto da inviare.
    pktTx[ID_DEST]=id;
    pktTx[TYPE_DEST]=type;
    pktTx[ID_SOURCE]=pgm_cfg.base_id;
    pktTx[TYPE_SOURCE]=BASE;
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

static void RTXSendRPgm(unsigned char id,te_DEV_TYPE type)
{
    //Crea il pacchetto da inviare.
    pktTx[ID_DEST]=id;
    pktTx[TYPE_DEST]=type;
    pktTx[ID_SOURCE]=pgm_cfg.base_id;
    pktTx[TYPE_SOURCE]=BASE;
    pktTx[OP]=rPGM;
    pktTx[VAL]=0x00;
    pktTx[VAL+1]=0x00;
    pktTx[VAL+2]=0x00;
    pktTx[BAT]=0x00;
    pktTx[BAT+1]=0x00;
    pktTx[BAT+2]=0x00;

    //Invia il pacchetto.
    RTXSndPkt(pktTx,PKT_LEN);
}
