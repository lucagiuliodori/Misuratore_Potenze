
typedef union _SYS_STA
{
	unsigned char VAL;
	struct
	{
		unsigned int TICK:1;
	};
}tu_SYS_STA;

tu_SYS_STA u_SYS_STA;
unsigned char sys_pgm_sta;

void SYSInit(unsigned char);
void SYSTask(void);
void SYSInt(void);
