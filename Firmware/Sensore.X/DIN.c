#include <xc.h>
#include "IN.h"
#include "DIN.h"

#define MAX_TMRDBNCSEL      5		//Debounce 50ms con base tempi 10ms.

#define MAX_TMRLONGSEL      200     //Pressione prolungata 5s con base tempi 10ms.

typedef union _FLG_DIN
{
    unsigned char val;
    struct
    {
        unsigned int tmrDbncSel:1;
        unsigned int tmrLongSel:1;
    };
}tu_FLG_DIN;

static tu_FLG_DIN u_flg_din;
static tu_STA_DIN u_in_sta_currTask,u_in_sta_tmpTask;
static unsigned char tmrDbncSel;
static unsigned short tmrLongSel;

void DINInit(unsigned char sta)
{
   switch(sta)
    {
//        case 0:
//            //Inizializzazione hardware.
//            break;
        case 1:
            //Inizializzazione variabili.
            u_flg_din.val=0;
            u_in_sta_currTask.val=0;
            u_in_sta_tmpTask.val=0;
            u_in_ev_up.val=0;
            u_in_ev_down.val=0;
            u_in_ev_long.val=0;
            break;
//        case 2:
//            //Inizializzazione applicazione.
//            break;
    }
}

void DINTick(void)
{

    //Timer per debounce tasti.
    if(u_flg_din.tmrDbncSel)
    {
        tmrDbncSel++;
        if(tmrDbncSel>MAX_TMRDBNCSEL)
        {
            u_flg_din.tmrDbncSel=0;
            tmrDbncSel=0;
            //Setta il flag.
            if(u_in_sta_currTask.sel)
                u_in_ev_down.sel=1;
            else
                u_in_ev_up.sel=1;
        }
    }
   
    //Timer per pressione prolungata tasti.
    if(u_flg_din.tmrLongSel)
    {
        tmrLongSel++;
        if(tmrLongSel>MAX_TMRLONGSEL)
        {
            //u_flg_din.tmrLongSel=0;    //Non azzera il flag perchè deve rilevare in modo continuo la pressione prolungata.
            tmrLongSel=0;
            //Setta il flag.
            u_in_ev_long.sel=1;
        }
    }
}

void DINTask(void)
{
    u_in_sta_tmpTask.val=0;
    if(INGetSEL())
        u_in_sta_tmpTask.sel=1;

    //Controlla se ci sono stati variazioni di stato.
    if(u_in_sta_currTask.sel!=u_in_sta_tmpTask.sel)
    {
        u_in_sta_currTask.sel=u_in_sta_tmpTask.sel;
        tmrDbncSel=0;
        u_flg_din.tmrDbncSel=1;
    }

    //Controllo pressioni prolungate.
    if(u_in_sta_currTask.sel)
        u_flg_din.tmrLongSel=1;
    else
    {
        tmrLongSel=0;           //L'azzeramento deve essere eseguito sul rilascio perchè deve rilevare in modo continuo la pressione prolungata.
        u_flg_din.tmrLongSel=0;
    }
}
