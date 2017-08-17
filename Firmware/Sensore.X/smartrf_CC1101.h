/***************************************************************
 *  SmartRF Studio(tm) Export
 *
 *  Radio register settings specifed with C-code
 *  compatible #define statements.
 *
 *  RF device: CC1101
 *
 ***************************************************************/

//#define	f433MHz		//Definisce la trasmissione a 433MHz
#define	f868MHz		//Definisce la trasmissione a 868MHz


#ifndef SMARTRF_CC1101_H
#define SMARTRF_CC1101_H

#define SMARTRF_RADIO_CC1101
#define SMARTRF_SETTING_IOCFG2     	0x47
#define SMARTRF_SETTING_IOCFG1     	0x46
#define SMARTRF_SETTING_IOCFG0     	0x4E
#define SMARTRF_SETTING_FIFOTHR    	0x0B
#define SMARTRF_SETTING_SYNC1           0xD3
#define SMARTRF_SETTING_SYNC0           0x91
#define SMARTRF_SETTING_PKTLEN          0x0B
#define SMARTRF_SETTING_PKTCTRL1   	0x08
#define SMARTRF_SETTING_PKTCTRL0        0x04
#define SMARTRF_SETTING_ADDR            0x00
#define SMARTRF_SETTING_CHANNR          0x00
#define SMARTRF_SETTING_FSCTRL1         0x06
#define SMARTRF_SETTING_FSCTRL0         0x00
#ifdef	f868MHz
#define SMARTRF_SETTING_FREQ2           0x21
#define SMARTRF_SETTING_FREQ1           0x63
#define SMARTRF_SETTING_FREQ0           0x91
#endif
#ifdef	f433MHz
#define SMARTRF_SETTING_FREQ2           0x10
#define SMARTRF_SETTING_FREQ1           0xA8
#define SMARTRF_SETTING_FREQ0           0x77
#endif
#define SMARTRF_SETTING_MDMCFG4         0xCA
#define SMARTRF_SETTING_MDMCFG3         0x83
#define SMARTRF_SETTING_MDMCFG2         0x02
#define SMARTRF_SETTING_MDMCFG1         0x02
#define SMARTRF_SETTING_MDMCFG0         0x3B
#define SMARTRF_SETTING_DEVIATN         0x34
#define SMARTRF_SETTING_MCSM2           0x07
#define SMARTRF_SETTING_MCSM1           0x3F
#define SMARTRF_SETTING_MCSM0           0x08
#define SMARTRF_SETTING_FOCCFG          0x16
#define SMARTRF_SETTING_BSCFG           0x6C
#define SMARTRF_SETTING_AGCCTRL2        0x03
#define SMARTRF_SETTING_AGCCTRL1        0x40
#define SMARTRF_SETTING_AGCCTRL0        0x91
#define SMARTRF_SETTING_WOREVT1         0x87
#define SMARTRF_SETTING_WOREVT0         0x6B
#define SMARTRF_SETTING_WORCTRL         0xFB
#define SMARTRF_SETTING_FREND1          0x56
#define SMARTRF_SETTING_FREND0     	0x16
#define SMARTRF_SETTING_FSCAL3          0xE9
#define SMARTRF_SETTING_FSCAL2          0x2A
#define SMARTRF_SETTING_FSCAL1          0x00
#define SMARTRF_SETTING_FSCAL0          0x1F
#define SMARTRF_SETTING_RCCTRL1         0x41
#define SMARTRF_SETTING_RCCTRL0         0x00
#define SMARTRF_SETTING_FSTEST          0x59
#define SMARTRF_SETTING_PTEST           0x7F
#define SMARTRF_SETTING_AGCTEST         0x3F
#define SMARTRF_SETTING_TEST2           0x81
#define SMARTRF_SETTING_TEST1           0x35
#define SMARTRF_SETTING_TEST0           0x09
#define SMARTRF_SETTING_PARTNUM         0x00
#define SMARTRF_SETTING_VERSION         0x04
#define SMARTRF_SETTING_FREQEST         0x00
#define SMARTRF_SETTING_LQI             0x00
#define SMARTRF_SETTING_RSSI            0x00
#define SMARTRF_SETTING_MARCSTATE       0x00
#define SMARTRF_SETTING_WORTIME1        0x00
#define SMARTRF_SETTING_WORTIME0        0x00
#define SMARTRF_SETTING_PKTSTATUS       0x00
#define SMARTRF_SETTING_VCO_VC_DAC      0x00
#define SMARTRF_SETTING_TXBYTES         0x00
#define SMARTRF_SETTING_RXBYTES         0x00
#define SMARTRF_SETTING_RCCTRL1_STATUS  0x00
#define SMARTRF_SETTING_RCCTRL0_STATUS  0x00

#endif
