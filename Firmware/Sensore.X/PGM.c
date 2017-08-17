#include <xc.h>
#include "Sensore.h"
#include "DIN.h"
#include "NVM.h"
#include "LED.h"
#include "OUT.h"
#include "SLP.h"
#include "RTX.h"
#include "PGM.h"

#define mPgmInit()          T1CON=0b00000001     //Attiva il timer 1 per ricavare il valore casuale per l'id del sense.

static void PGMReadNvm(void);
static void PGMNvmReset(void);
static void PGMCheckNvm(void);

void PGMInit(unsigned char state)
{
    switch(state)
    {
        case 0:
            //Inizializzazione hardware.
            mPgmInit();
            break;
        case 1:
            //Inizializzazione variabili.
            pgm_flg.val=0;
            PGMReadNvm();
            PGMCheckNvm();
            break;
//        case 2:
//            //Inizializzazione applicazione.
//            break;
    }
}	

void PGMTick(void)
{
}

static void PGMReadNvm(void)
{
    NVMRead((unsigned short)(&(nvm_pgm_cfg)),(char *)(&(pgm_cfg)),sizeof(ts_PGM_CFG));
}	

void PGMWriteNvm(void)
{
    NVMWrite((unsigned short)(&(nvm_pgm_cfg)),(char *)(&(pgm_cfg)),sizeof(ts_PGM_CFG));
}

static void PGMCheckNvm(void)
{
    if(pgm_cfg.valChk1!=VAL_EEP_CHECK1 || pgm_cfg.valChk2!=VAL_EEP_CHECK2)
        PGMNvmReset();        //Esegue il reset dei valori.
}	

static void PGMNvmReset(void)
{
    //Setta i valori.
    pgm_cfg.valChk1=VAL_EEP_CHECK1;
    pgm_cfg.valChk2=VAL_EEP_CHECK2;
    pgm_cfg.menu.val=RST_EEP_MENU;
    pgm_cfg.sensId=RST_ID;
    pgm_cfg.baseId=RST_ID;
    //Esegue la scrittura della NVM.
    PGMWriteNvm();
    //Esegue un reset del microcontrollore.
    RESET();
}

void PGMPgmOff(void)
{
    //Spegne il led.
    OUTSetLEDOff();
    //Setta il flag.
    pgm_flg.pgmonoff=0;
}

void PGMSelUp(void)
{
    //Controlla se è stato eseguito il reset.
    if(!(pgm_flg.resetDone))
    {
        //Controlla se il proprio id è uguale a quello di default, in caso di esito positivo ne genera uno diverso.
        if(pgm_cfg.sensId==RST_ID)
        {
            //Genera un nuovo id.
            pgm_cfg.sensId=TMR1L^TMR1H;
            //Esegue la scrittura della NVM.
            PGMWriteNvm();
        }        
        //Segnala che è stata richiesta la programmazione dall'utente.
        pgm_flg.pgmonoff=1;
        //Attiva il led per segnalare che la programmazione è attiva.
        OUTSetLEDOn();
        //Inizia l'invio di pacchetti di programmazione.
        RTXStartSendPgm();
    }
    //Cancella il flag di esecuzione del reset.
    pgm_flg.resetDone=0;
}

void PGMSelLong(void)
{
    //Modifica il valore del menù per segnala che il senspre non è programmato.
    pgm_cfg.menu.val=RST_EEP_MENU;
    //Genera un nuovo id.
    pgm_cfg.sensId=TMR1L^TMR1H;
    if(pgm_cfg.sensId==RST_ID)
        pgm_cfg.sensId++;
    //Esegue la scrittura della NVM.
    PGMWriteNvm();
    //Esegue alcuni lampeggi (sospensivi) per segnalare che la procedura è andata a buon fine.
    LEDNormalFlash(3);
    //Setta il flag per indicare che è stato eseguito il reset.
    pgm_flg.resetDone=1;
}
