#define VAL_CALCURR 0     //Valore per la calibrazione del sensore di temperatura.

unsigned short batt;
float battV,rmsCurr;

void ADCInit(unsigned char state);
void ADCConv(void);
#define ADCGetBatt() batt
#define ADCGetBattV() battV
#define ADCGetRmsCurr() rmsCurr
#define ADCGetRmsCurrCal() rmsCurr+VAL_CALCURR
