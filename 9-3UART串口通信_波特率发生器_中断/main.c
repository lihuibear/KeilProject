#include <reg52.h>

void ConfigUART(unsigned int baud);

void main()
{
    EA = 1;   //使能总中断
    ConfigUART(9600);  //配置波特率为9600
    while(1);
}
void ConfigUART(unsigned int baud)
{
    SCON  = 0x50;  //配置串口为模式1
    TMOD &= 0x0F;  //清零T1的控制位
    TMOD |= 0x20;  //配置T1为模式2
    TH1 = 256 - (11059200/12/32)/baud;  //计算T1重载值
    TL1 = TH1;     //初值等于重载值
    ET1 = 0;       //禁止T1中断
    ES  = 1;       //使能串口中断
    TR1 = 1;       //启动T1
}

void InterruptUART() interrupt 4
{
	if(RI)
	{
		RI = 0;
		SBUF = SBUF + 1;
	}
	if(TI)
	{
		TI = 0;
	}
}
