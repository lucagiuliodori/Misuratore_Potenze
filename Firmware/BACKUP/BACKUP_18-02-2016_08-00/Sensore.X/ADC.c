#include <xc.h>
#include <math.h>
#include "Sensore.h"
#include "OUT.h"
#include "DLY.h"
#include "ADC.h"

#define ADCON0_BATT 0b00000101
#define ADCON0_CURR 0b00001001

#define ADCON1_BATT 0b10000011      //Per la misurazione della batteria occorre settare come riferimento di tensione il VR interno.
#define ADCON1_CURR 0b10000011      //Per la misurazione della corrente occorre settare come riferimento di tensione la tensione di alimentazione.

#define MAX_CNTSUM_BATT 10   //Numero di somme per il calcolo della media.
#define MAX_CNTSUM_CURR 20   //Numero di somme per il calcolo del'RMS.

#define RES 33  //Valore in ohm della resistenza sul sensore di corrente.
#define WIND_RATIO 2000 //Rapporto spire del sensore in corrente.

typedef enum _ADC_CH
{
    ADC_CH_OFF,
    ADC_CH_BATT,
    ADC_CH_CURR
}te_ADC_CH;

#define mADCInit() ADCON0=ADCON0_BATT; ADCON1=ADCON1_BATT; FVRCON=0b10000001; ANSELA|=0b00000110;

te_ADC_CH adc_ch;

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
            adc_ch=ADC_CH_OFF;
            batt=0;
            battV=0;
            rmsCurr=0;
            break;
//        case 2:
//            //Inizializzazione applicazione.
//            break;
    }
}

void ADCConv(void)
{
    unsigned char cntSum;
    unsigned short sumBatt,curTmp;
    unsigned long sumCurr;

    while(1)
    {
        //Controlla quale canale è attivo.
        switch(adc_ch)
        {
            case ADC_CH_OFF:
                //Attiva l'uscita per il controllo della tensione della batteria.
                OUTSetTESTBATTOn();
                //Azzera la somma.
                sumBatt=0;
                //Azzera il contatore.
                cntSum=0;
                //Setta la prossima conversione AD.
                ADCON1=ADCON1_BATT;
                ADCON0=ADCON0_BATT;
                adc_ch=ADC_CH_BATT;
                break;
            case ADC_CH_BATT:
                //Controlla se sono state raggiunte il numero massimo di somme per il calcolo della media.
                if(cntSum==MAX_CNTSUM_BATT)
                {
                    //Calcola la media.
                    batt=sumBatt/MAX_CNTSUM_BATT;
                    //Calcola la tensione in volt.
                    /*"*1.024/1024" per ottenere i mV corrispondenti;
                     *"*11" perchè il partitore utilizzato ha un rapporto di 1/11.*/
                    battV=batt*1.024/1024*11;
                    //Azzera la somma.
                    sumCurr=0;
                    //Azzera il contatore.
                    cntSum=0;
                    //Disattiva l'uscita per il controllo della tensione della batteria e attiva quella della temperatura.
                    OUTSetTESTBATTOff();
                    //Setta la prossima conversione AD.
                    ADCON1=ADCON1_CURR;
                    ADCON0=ADCON0_CURR;
                    adc_ch=ADC_CH_CURR;
                }
                else
                {
                    //Legge e somma il valore (approssimato a 8 bit).
                    sumBatt+=(ADRESH<<8|ADRESL);
                    //Incrementa il contatore.
                    cntSum++;
                }
                break;
            case ADC_CH_CURR:
                //Controlla se sono state raggiunte il numero massimo di somme per il calcolo della media.
                if(cntSum==MAX_CNTSUM_CURR)
                {
                    //Calcola l'RMS della corrente.
                    
                    /*Moltiplica per "(1.024/1024)^2" per ottenere i valore letto in mV.
                     *Divido per "RES^2" per ottenere il valore in corrente sul secondario.
                     *Moltiplica per "WIND_RATIO^2" per ottenere il valore di corrente sul primario.
                     *Divide per "MAX_CNTSUM_CURR" e calcola la radice quadrata per il calcolo dell'RMS.*/
                    rmsCurr=sumCurr*(1.024*1.024)/(1024*1024);
                    rmsCurr/=(RES*RES);
                    rmsCurr*=(WIND_RATIO*WIND_RATIO);
                    rmsCurr/=MAX_CNTSUM_CURR;
                    rmsCurr=sqrt(rmsCurr);
                    //Conversioni completate.
                    adc_ch=ADC_CH_OFF;
                    return;
                }
                else
                {
                    //Legge e somma il valore (approssimato a 8 bit).
                    sumCurr+=((ADRESH<<8|ADRESL)*(ADRESH<<8|ADRESL));     //Non viene usata la funzione "pow" per ottimizzare lo spazion usato e per velocizzare le operazioni.
                    //Incrementa il contatore.
                    cntSum++;
                }
                break;
        }
        //Ritardo sospensivo per l'attivazione della conversione.
        __delay_us(10);
        //Attivazione della conversione.
        ADCON0bits.GO=1;
        //Attende il completamento della conversione.
        while(ADCON0bits.GO)
            CLRWDT();
        //Se è la conversione per la misurazione della corrente, attende 8ms per il calcolo dell'RMS.
        if(adc_ch==ADC_CH_CURR)
        DLYDelay_ms(8);
    }
}
