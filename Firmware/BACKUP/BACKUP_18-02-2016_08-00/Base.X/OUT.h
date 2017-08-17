
#define bitMask_SS 0b00100000
#define bitMask_SCK 0b00001000
#define bitMask_SDO 0b00100000
#define bitMask_RS 0b01000000
#define bitMask_RW 0b10000000
#define bitMask_E 0b00010000
#define bitMask_D4 0b00001000
#define bitMask_D5 0b00000100
#define bitMask_D6 0b00000010
#define bitMask_D7 0b00000001

#define port_SS LATA
#define port_SCK LATC
#define port_SDO LATC
#define port_RS LATA
#define port_RW LATA
#define port_E LATA
#define port_DATA LATA

#define tris_DPY TRISA

#define opin_SS LATAbits.LATA5
#define opin_SCK LATCbits.LATC3
#define opin_SDO LATCbits.LATC5
#define opin_RS LATAbits.LATA6
#define opin_RW LATAbits.LATA7
#define opin_E LATAbits.LATA4
#define opin_D4 LATAbits.LATA3
#define opin_D5 LATAbits.LATA2
#define opin_D6 LATAbits.LATA1
#define opin_D7 LATAbits.LATA0


#define OUTSetSSOff() opin_SS=0
#define OUTSetSCKOff() opin_SCK=0
#define OUTSetSDOOff() opin_SDO=0
#define OUTSetRSOff() opin_RS=0
#define OUTSetRWOff() opin_RW=0
#define OUTSetEOff() opin_E=0
#define OUTSetD4Off() opin_D4=0
#define OUTSetD5Off() opin_D5=0
#define OUTSetD6Off() opin_D6=0
#define OUTSetD7Off() opin_D7=0

#define OUTSetSSOn() opin_SS=1
#define OUTSetSCKOn() opin_SCK=1
#define OUTSetSDOOn() opin_SDO=1
#define OUTSetRSOn() opin_RS=1
#define OUTSetRWOn() opin_RW=1
#define OUTSetEOn() opin_E=1
#define OUTSetD4On() opin_D4=1
#define OUTSetD5On() opin_D5=1
#define OUTSetD6On() opin_D6=1
#define OUTSetD7On() opin_D7=1

#define OUTSetSSToggle() port_SS^=bitMask_SS
#define OUTSetSCKToggle() port_SCK^=bitMask_SCK
#define OUTSetSDOToggle() port_SDO^=bitMask_SDO
#define OUTSetRSToggle() port_RS^=bitMask_RS
#define OUTSetRWToggle() port_RW^=bitMask_RW
#define OUTSetEToggle() port_E^=bitMask_E
#define OUTSetD4Toggle() port_DATA^=bitMask_D4
#define OUTSetD5Toggle() port_DATA^=bitMask_D5
#define OUTSetD6Toggle() port_DATA^=bitMask_D6
#define OUTSetD7Toggle() port_DATA^=bitMask_D7

void OUTInit(unsigned char);
void OUTSetHTCDataNibble(unsigned char);
