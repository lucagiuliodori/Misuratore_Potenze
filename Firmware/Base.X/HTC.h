
typedef enum _HTC_DIRCUR
{
    HTC_DEC=0,
    HTC_INC=1
}te_HTC_DIRCUR;

typedef enum _HTC_MOVCURDPY
{
    HTC_CURSOR=0,
    HTC_DISPLAY=1
}te_HTC_MOVCURDPY;

typedef enum _HTC_MOVDIR
{
    HTC_LEFT=0,
    HTC_RIGTH=1
}te_HTC_MOVDIR;

typedef enum _HTC_LENDATA
{
    HTC_NIBBLE=0,
    HTC_BYTE=1
}te_HTC_LENDATA;

typedef enum _HTC_NUMLINE
{
    HTC_ONEROW=0,
    HTC_TWOROW=1
}te_HTC_NUMLINE;

typedef enum _HTC_FONTSIZE
{
    HTC_SMALLSIZE=0,    //5x7
    HTC_BIGSIZE=1       //5x10
}te_HTC_FONTSIZE;

void HTCInit(unsigned char);

void HTCDisplayClear(void);
void HTCCursorReset(void);

void HTCDisplayInit(te_HTC_DIRCUR,unsigned char,unsigned char, unsigned char,unsigned char,
        te_HTC_MOVCURDPY,te_HTC_MOVDIR,te_HTC_LENDATA,te_HTC_NUMLINE,te_HTC_FONTSIZE);

void HTCSetAddressCGRAM(unsigned char);
void HTCSetAddressDDRAM(unsigned char);

unsigned char HTCGetAddressCounter(void);
BOOL HTCIsBusy(void);

unsigned char HTCReadData(void);
void HTCWriteData(unsigned char);
