
typedef enum _CC1_STB
{
	SRES=0x30,
	SFSTXON=0x31,
	SXOFF=0x32,
	SCAL=0x33,
	SRX=0x34,
	STX=0x35,
	SIDLE=0x36,
	SWOR=0x38,
	SPWD=0x39,
	SFRX=0x3A,
	SFTX=0x3B,
	SWORRST=0x3C,
	SNOP=0x3D
}te_CC1_STB;

typedef enum _CC1_STA
{
	SLEEP=0x00,
	IDLE=0x01,
	XOFF=0x02,
	VCOON_MC=0x03,
	REGON_MC=0x04,
	MANCAL=0x05,
	VCOON=0x06,
	REGON=0x07,
	STARTCAL=0x08,
	BWBOOST=0x09,
	FS_LOCK=0x0A,
	IFADCON=0x0B,
	ENDCAL=0x0C,
	RX=0x0D,
	RX_END=0x0E,
	RX_RST=0x0F,
	TXRX_SWITCH=0x10,
	RXFIFO_OVERFLOW=0x11,
	FSTXON=0x12,
	TX=0x13,
	TX_END=0x14,
	RXTX_SWITCH=0x15,
	TXFIFO_UNDERFLOW=0x16
}te_CC1_STA;

typedef struct _CC1_FLG
{
	unsigned int PCK_RCV:1;
}ts_CC1_FLG;

ts_CC1_FLG s_CC1_FLG;

void CC1Init(unsigned char);
void CC1Int(void);
void CC1Tick(void);
void CC1Strobe(te_CC1_STB);
void CC1Read(unsigned char,unsigned char *,unsigned char);
void CC1Write(unsigned char,unsigned char *,unsigned char);
void CC1Sleep(void);
void CC1LoadREG(void);
