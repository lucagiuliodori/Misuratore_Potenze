
typedef union _FLG_SPL
{
    unsigned char val;
    struct
    {
        unsigned int goSleep:1;
        unsigned int exitSleep:1;
    };
}tu_FLG_SPL;

tu_FLG_SPL flg_slp;

void SLPSleep(void);
