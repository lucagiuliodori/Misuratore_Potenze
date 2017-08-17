
#define pin_SEL PORTBbits.RB2
#define pin_SDI PORTCbits.RC4
#define pin_GDO0 PORTBbits.RB1
#define pin_GDO2 PORTBbits.RB0
#define pin_JMP PORTBbits.RB5

#define INGetSEL() !pin_SEL
#define INGetSDI() pin_SDI
#define INGetGDO0() pin_GDO0
#define INGetGDO2() pin_GDO2
#define INGetJMP() pin_JMP

void INInit(unsigned char);
