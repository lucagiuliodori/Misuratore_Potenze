/*
 Struttura pacchetto:
 * 1° byte: ID destinatario
 * 2° byte: ID sorgente
 * 3° byte: tipo dispositivo (0 - base, 1 - sensore potenza prodotta, 2 - sensore potenza consumata, 3 - sensore tensione)
 * 4° byte: operazione
 * 5° - 6° byte: valore letto
 * 7° - 9° byte: tensione batterie (solo per i sensori)
 
 */

// OPCODE:
//
// - DAT: dati.
// - PGM: programmazione.
// - rDAT: risposta per conferma ricezione dati.
// - rPGM: risposta per conferma programmazione.

typedef union _RTX_FLG
{
    unsigned char val;
    struct
    {
        unsigned int pgmDone:1;     //Segnala che la programmazione è stata eseguita.
    };
}tu_RTX_FLG;

tu_RTX_FLG rtx_flg;

void RTXInit(unsigned char);
void RTXTick(void);
void RTXReadPkt(void);
void RTXTask(void);
void RTXStartSendData(void);
void RTXAwake(void);
