
#include <reg52.h>

sbit IR_INPUT = P3^3;  //红外接收引脚

bit irflag = 0;  //红外接收标志，收到一帧正确数据后置1
unsigned char ircode[4];  //红外代码接收缓冲区

void InitInfrared()
{	
	IR_INPUT = 1;
	TMOD &= 0X0F;
	TMOD |= 0x10;
	TR1 = 0;
	ET1 = 0;
	IT1 = 1;
	EX1 = 1;
}
unsigned int GetHighTime()
{
	TH1 = 0;
	TL1 = 0;
	TR1 = 1;
	while(IR_INPUT)
	{
		if(TH1 > 0x40)
		{
			break;
		}
	}
	TR1 = 0;

	return(TH1 * 256 + TL1);
}

unsigned int GetLowTime()
{
	TH1 = 0;
	TL1 = 0;
	TR1 = 1;
	while(!IR_INPUT)
	{
		if(TH1 > 0x40)
		{
			break;
		}
	}
	TR1 = 0;

	return(TH1 * 256 + TL1);
}
void EXINT1_ISR() interrupt 2
{
	unsigned char i, j;
	unsigned int time;
	unsigned char byt;

	time = GetLowTime();
	if((time <7833) || (time > 8755))	   
	{
		IE1 = 0;
		return;	
	}

	time = GetHighTime();
	if((time<3686) || (time > 4608))
	{
		IE1 = 0;
		return;
	}
	for(i=0; i<4; i++)
	{
		for(j=0; j<8; j++)
		{
			time = GetLowTime();
			if((time<313) ||(time >718))
			{
				IE1 = 0;
				return;
			}
			time = GetHighTime();
			if((time>313) && (time <718))
			{
				byt >>= 1;	
			}
			else if((time>1345) && (time<1751))
			{
				byt >>= 1;
				byt |= 0x80;
			}
			else
			{
				IE1 = 0;
				return;
			}
		}
		ircode[i] = byt;
	}
	irflag = 1;
	IE1 = 0;
}


