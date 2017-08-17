typedef union _STA_DIN
{
    unsigned char val;

    struct
    {
        unsigned int sel:1;
    };
}tu_STA_DIN;

tu_STA_DIN u_in_ev_up,u_in_ev_down,u_in_ev_long;

void DINInit(unsigned char);
void DINTick(void);
void DINTask(void);
