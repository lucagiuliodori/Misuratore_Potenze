#include "NVM.h"

#define VAL_EEP_CHECK1      0x5C	//Valore per l'indirizzo di controllo eeprom.
#define VAL_EEP_CHECK2      0xC5	//Valore per l'indirizzo di controllo eeprom.
#define RST_EEP_MENU        0x00	//Valore di reset per il men�.
#define RST_ID 0x00
#define RST_TYPE 0x00

typedef union _PGM_MENU
{
    unsigned char val;
    struct
    {
        unsigned int baseIdProg:1;
    };
}tu_PGM_MENU;

typedef struct _PGM_CFG
{
    unsigned char valChk1;
    unsigned char valChk2;

    tu_PGM_MENU menu;
    unsigned char sensId;
    unsigned char baseId;
}ts_PGM_CFG;

typedef union _PGM_FLG
{
    unsigned char val;
    struct
    {
        unsigned int pgmonoff:1;
        unsigned int resetDone:1;
    };
}tu_PGM_FLG;

typedef struct _PGM_PGM_CFG
{
    ts_PGM_CFG pgm_cfg;
    unsigned char dummy[PGM_PAGESIZE-sizeof(ts_PGM_CFG)];   //Questo buffer "dummy" � stato inserito per riservare una pagina di PGM per le config (utile durante la modifica delle config).
}ts_PGM_PGM_CFG;

tu_PGM_FLG pgm_flg;
ts_PGM_CFG pgm_cfg;
const ts_PGM_PGM_CFG nvm_pgm_cfg @ 0x3820=
{
    VAL_EEP_CHECK1,
    VAL_EEP_CHECK2,
    RST_EEP_MENU,
    RST_ID,
    RST_ID
};

void PGMInit(unsigned char);
void PGMTick(void);
void PGMSelUp(void);
void PGMSelLong(void);
void PGMWriteNvm(void);
void PGMPgmOff(void);
#define PGMGetSensId() pgm_cfg.sensId;
#define PGMGetBaseId() pgm_cfg.baseId;
