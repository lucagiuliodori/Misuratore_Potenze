/*
 Struttura pacchetto:
 * 1° byte: ID destinatario
 * 2° byte: ID sorgente
 * 3° byte: tipo dispositivo (0 - base, 1 - sensore potenza prodotta, 2 - sensore potenza consumata, 3 - sensore tensione)
 * 4° byte: operazione
 * 5° - 6° byte: valore letto
 * 7° - 9° byte: tensione batterie (solo per i sensori)
 
 */

/*
Struttura pacchetto (lunghezza 9 byte; il tipo flot in xc8 è rappresentato con 3 byte):

 Lunghezza 9 byte pacchetto invio dati.
 - 1° byte ID destinazione (0x00 comunicazione in broadcast).
 - 2° byte: tipo dispositivo destinazione (0 - base, 1 - sensore potenza prodotto, 2 - sensore potenza consumata, 3 - sensore potenza bidirezionale, 4 - sensore tensione)
 - 3° byte ID sorgente.
 - 4° byte: tipo dispositivo sorgente (0 - base, 1 - sensore potenza prodotto, 2 - sensore potenza consumata, 3 - sensore potenza bidirezionale, 4 - sensore tensione)
 - 5° byte OPCODE.
 - 6°->8° byte valore letto del sensore di temperatura (LSB->MSB).
 - 9°->11° byte valore letto dalla tensione della batteria (LSB->MSB).

 Lunghezza 3 byte pacchetto programmazione.
 - 1° byte ID destinazione (0x00 comunicazione in broadcast).
 - 2° byte: tipo dispositivo destinazione (0 - base, 1 - sensore potenza prodotto, 2 - sensore potenza consumata, 3 - sensore potenza bidirezionale, 4 - sensore tensione)
 - 3° byte ID sorgente.
 - 4° byte: tipo dispositivo sorgente (0 - base, 1 - sensore potenza prodotto, 2 - sensore potenza consumata, 3 - sensore potenza bidirezionale, 4 - sensore tensione)
 - 5° byte OPCODE.
 - 6°->8° 0x000000 (LSB->MSB).
 - 9°->11° 0x000000 (LSB->MSB).

 OPCODE:

 - DAT: dati.
 - PGM: programmazione.
 - rDAT: risposta per conferma ricezione dati.
 - rPGM: risposta per conferma programmazione.
*/

typedef enum _RTX_UPD_DPY
{
    UPD_DPY_POWER,
    UPD_DPY_CURRSENSGEN,
    UPD_DPY_CURRSENSCON,
    UPD_DPY_VOLTSENS,
    UPD_DPY_PGMDONECURRSENSGEN,
    UPD_DPY_PGMDONECURRSENSCON,
    UPD_DPY_PGMDONEVOLTSENS,
    UPD_DPY_PGMRST
}te_RTX_UPD_DPY;

typedef union _RTX_FLG
{
    unsigned char val;
    struct
    {
        unsigned int pgmDone:1;         //Segnala che la programmazione è stata eseguita.
        unsigned int dataReceived:1;    //Segnala che i dati sono stati ricevuti.
    };
}tu_RTX_FLG;

tu_RTX_FLG rtx_flg;
float gen_power,con_power;
float gen_curr_sens,con_curr_sens,volt_sens;
float batt_gen_curr_sens,batt_con_curr_sens,batt_volt_sens;

void RTXInit(unsigned char);
void RTXTick(void);
void RTXReadPkt(void);
void RTXTask(void);
void RTXStartSendData(void);
#define RTXGetTemp() temp
#define RTXGetBatt() batt
void RTXUpdateDisplay(te_RTX_UPD_DPY);
void RTXSelDown(void);
