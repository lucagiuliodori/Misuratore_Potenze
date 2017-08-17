#include <xc.h>
#include "Base.h"
#include "DIN.h"
#include "NVM.h"
#include "OUT.h"
#include "DPY.h"
#include "DLY.h"
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
            PGMReadNvm();
            PGMCheckNvm();
            break;
//        case 2:
//            //Inizializzazione applicazione.
//            break;
    }
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
    pgm_cfg.base_id=RST_ID;
    pgm_cfg.gen_curr_sens_id=RST_ID;
    pgm_cfg.con_curr_sens_id=RST_ID;
    pgm_cfg.volt_sens_id=RST_ID;
    //Esegue la scrittura della NVM.
    PGMWriteNvm();
    //Esegue un reset del microcontrollore.
    RESET();
}

void PGMSelLong(void)
{
    //Genera il numero casuale per l'id della base.
    pgm_cfg.base_id=TMR1L^TMR1H;
    //Resetta l'id e il type dei sensori.
    pgm_cfg.gen_curr_sens_id=RST_ID;
    pgm_cfg.con_curr_sens_id=RST_ID;
    pgm_cfg.volt_sens_id=RST_ID;
    //Modifica il valore del menù per segnala che i sensori non sono programmati.
    pgm_cfg.menu.val=RST_EEP_MENU;
    //Esegue la scrittura della NVM.
    PGMWriteNvm();
    //Aggiorna il display.
    RTXUpdateDisplay(UPD_DPY_PGMRST);
    //Ritardo di visualizzazione.
    DLYDelay_ms(2000);
    //Cambia la visualizzazione del display.
    RTXUpdateDisplay(UPD_DPY_POWER);
}
