
#define ipin_SEL PORTBbits.RB2
#define ipin_SDI PORTCbits.RC4
#define ipin_GDO0 PORTBbits.RB1
#define ipin_GDO2 PORTBbits.RB0
#define ipin_D4 PORTAbits.RA3
#define ipin_D5 PORTAbits.RA2
#define ipin_D6 PORTAbits.RA1
#define ipin_D7 PORTAbits.RA0

#define INGetSEL() !ipin_SEL
#define INGetSDI() ipin_SDI
#define INGetGDO0() ipin_GDO0
#define INGetGDO2() ipin_GDO2
#define INGetD4() ipin_D4
#define INGetD5() ipin_D5
#define INGetD6() ipin_D6
#define INGetD7() ipin_D7

void INInit(unsigned char);
