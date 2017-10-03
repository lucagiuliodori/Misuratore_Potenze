#include <xc.h>
#include <math.h>
#include "Sensore.h"
#include "OUT.h"
#include "DLY.h"
#include "ADC.h"

#define ADCON0_BATT 0b00000101
#define ADCON0_CURR 0b00001001

/*Per garantire un TAD minumo di 1us,
 * con una frequenza del micro a 8MHz,
 * occorre settare la frequenza di
 * conversione a Fosc/8. in questo
 * modo il tempo di conversione
 * è pari a circa 11,5us (come indicato
 * da manuale).*/
#define ADCON1_BATT 0b10010011      //Per la misurazione della tensione sulla resistenza (collegata alla sonda) occorre settare come riferimento di tensione il VR interno.
#define ADCON1_CURR 0b10010011      //Per la misurazione della corrente occorre settare come riferimento di tensione la tensione di alimentazione.

#define MAX_ADCSAMPLES (100)        //Numero di valori da acquisire dall'ADC.
#define NUM_ADCSAMPLES_BATT (10)    //Numero di campioni da usare per il calcolo della media della tensione della batteria.
/*I campioni che si possono misurare sono
 MAX_ADCSAMPLES mentre quelli desiderati
 sono NUM_ADCSAMPLES_CURR.
 I campioni presi in più possono essere
 sfruttati per calcolare il valore medio
 ed assumerlo come campione, anzichè misurare
 i soli NUM_ADCSAMPLES_CURR campioni.*/
#define NUM_ADCSAMPLES_USEFULL_CURR (20)    //Numero di campioni da usare per il calcolo del'RMS della corrente misurata.

#define RES 33.2                    //Valore in ohm della resistenza sul sensore di corrente.
#define WIND_RATIO 2000             //Rapporto spire del sensore in corrente.

/*La misurazione della tensione della
 * batteria per il calcolo della media,
 * può essere eseguita con tempi
 * relativamente lunghi infatti, il valore
 * non varierà significativa per intervalli
 * di tempo brevi.*/
#define T_DELAYMS_ADCCONV_BATT (20)       //Ritardo (in ms) tra una conversione ed un'altra della tensione della batteria.
/* La misurazione della tensione ai capi
 * della resistenza (connessa all spira)
 * deve essere eseguita con tempi tali per cui
 * in un periodo dell'onda vengano
 * eseguite MAX_ADCSAMPLES misurazioni.
 * Nel caso dei 50Hz -> T=20ms da cui il
 * ritardo tra una conversione ed un'altra
 * deve essere di 20ms/MAX_ADCSAMPLES
 * Nel caso dei 60Hz -> T=17ms (circa) da cui il
 * ritardo tra una conversione ed un'altra
 * deve essere di 17ms/MAX_ADCSAMPLES.
 * A questi tempi deve essere tolto il tempo
 * di conversione che è pari a circa 11.5us
 * più eventuale ritardo introdotto dalle istruzioni
 * stesse.*/
#define T_DELAYUS_ADCCONV_CURR (200-17)      //Ritardo (in us) tra una conversione ed un'altra della tensione ai capi della resistenza (collegata alla spira).

#define mADCInit() ADCON0=ADCON0_BATT; ADCON1=ADCON1_BATT; FVRCON=0b10000001; ANSELA|=0b00000110;

typedef union _ADC_VAL
{
    unsigned short val;
    struct
    {
        unsigned char lVal;
        unsigned char hVal;
    };
}tu_ADC_VAL;

tu_ADC_VAL adc_vals[MAX_ADCSAMPLES];

void ADCInit(unsigned char state)
{
    switch(state)
    {
        case 0:
            //Inizializzazione hardware.
            mADCInit();
            break;
        case 1:
            //Inizializzazione variabili.
            batt=0;
            battV=0;
            rmsCurr=0;
            realRmsCurr=0;
            break;
//        case 2:
//            //Inizializzazione applicazione.
//            break;
    }
}

void ADCConv(void)
{
    unsigned char i,j;
    unsigned short sumBatt;
    unsigned long sumCurr;
    unsigned long sumEavCurr;
    unsigned long maxCurr;

    //Esegue le conversioni per la misurazione della tesione della batteria.
    //Attiva l'uscita per il controllo della tensione della batteria.
    OUTSetTESTBATTOn();
    //Setta i registri per la conversione AD.
    ADCON1=ADCON1_BATT;
    ADCON0=ADCON0_BATT;
    //Ritardo sospensivo per l'attivazione della conversione.
    __delay_us(10);
    for(i=0;i<NUM_ADCSAMPLES_BATT;i++)
    {
        //Attivazione della conversione.
        ADCON0bits.GO=1;
        //Attende il completamento della conversione.
        while(ADCON0bits.GO)
            CLRWDT();
        //Salvataggio dei valori.
        adc_vals[i].lVal=ADRESL;
        adc_vals[i].hVal=ADRESH;
        //Ritardo per l'avvio della prossima conversione.
        DLYDelay_ms(T_DELAYMS_ADCCONV_BATT);
    }
    //Calcola la media dei valori.
    sumBatt=0;
    for(i=0;i<NUM_ADCSAMPLES_BATT;i++)
        sumBatt+=adc_vals[i].val;
    batt=sumBatt/NUM_ADCSAMPLES_BATT;
    //Calcola la tensione in volt.
    /*"*1.024/1024" per ottenere i mV corrispondenti;
     *"*11" perchè il partitore utilizzato ha un rapporto di 1/11.*/
    battV=batt*1.024/1024*11;
    //Disattiva l'uscita per il controllo della tensione della batteria e attiva quella della temperatura.
    OUTSetTESTBATTOff();
    
    //Esegue le conversioni per la misurazione dell'RMS della corrente.
    //Setta i registri per la conversione AD.
    ADCON1=ADCON1_CURR;
    ADCON0=ADCON0_CURR;
    //Ritardo sospensivo per l'attivazione della conversione.
    __delay_us(10);
    for(i=0;i<MAX_ADCSAMPLES;i++)
    {
        //Attivazione della conversione.
        ADCON0bits.GO=1;
        //Attende il completamento della conversione.
        while(ADCON0bits.GO)
            CLRWDT();
        //Salvataggio dei valori.
        adc_vals[i].lVal=ADRESL;
        adc_vals[i].hVal=ADRESH;
        //Ritardo per l'avvio della prossima conversione.
        DLYDelay_ms(T_DELAYUS_ADCCONV_CURR);
    }
    //Calcola il valore massimo per il calcolo dell'RMS.
    maxCurr=0;
    for(i=0;i<MAX_ADCSAMPLES;i++)
    {
        if(adc_vals[i].val>maxCurr)
            maxCurr=adc_vals[i].val;
    }
    //Calcola l'RMS assumendo che l'onda sia perfettamente sinusoidale.
    rmsCurr=(float)maxCurr/sqrt(2);
    
    //Calcola la somma quadratica per il calcolo dell'RMS reale.
    sumCurr=0;
    for(i=0;i<MAX_ADCSAMPLES;)
    {
        /*I campioni misurati sono MAX_ADCSAMPLES
         ma quelli desiderati NUM_ADCSAMPLES_CURR.
         I campioni presi in più possono essere
         sfruttati per calcolare il valore medio
         ed assumerlo come campione, anzichè misurare
         i soli NUM_ADCSAMPLES_CURR campioni.*/
        sumEavCurr=0;
        for(j=0;j<(MAX_ADCSAMPLES/NUM_ADCSAMPLES_USEFULL_CURR);j++,i++)
            sumEavCurr+=adc_vals[i].val;
        sumEavCurr/=(MAX_ADCSAMPLES/NUM_ADCSAMPLES_USEFULL_CURR);
        sumCurr+=sumEavCurr*sumEavCurr;                     //Non viene usata la funzione "pow" per ottimizzare lo spazio usato e per velocizzare le operazioni.
    }
    //Calcola l'RMS reale della corrente.
    /*Moltiplica la somma ottenuta per 2 in quanto la semionda negativa sarà azzerata dal diodo; si assume quindi che questa sia approssimativamente uguale a quella positiva.
     *Moltiplica per "(1.024/1024)^2" per ottenere il valore letto in mV.
     *Moltiplica per "WIND_RATIO^2" per ottenere il valore di corrente sul primario.
     *Divide per "RES^2" per ottenere il valore in corrente sul secondario.
     *Divide per "MAX_CNTSUM_CURR" e calcola la radice quadrata per il calcolo dell'RMS.*/
    realRmsCurr=(float)sumCurr*2;
    realRmsCurr*=(float)1.024;
    realRmsCurr*=(float)1.024;
    realRmsCurr*=(float)WIND_RATIO;
    realRmsCurr*=(float)WIND_RATIO;
    realRmsCurr/=(float)1024;
    realRmsCurr/=(float)1024;
    realRmsCurr/=(float)RES;
    realRmsCurr/=(float)RES;
    realRmsCurr/=(float)NUM_ADCSAMPLES_USEFULL_CURR;
    realRmsCurr=sqrt(realRmsCurr);
}
