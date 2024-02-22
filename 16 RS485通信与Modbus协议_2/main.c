
#include <reg52.h>

sbit BUZZ = P1^6;  //蜂鸣器控制引脚

bit flagBuzzOn = 0;   //蜂鸣器启动标志
unsigned char T0RH = 0;  //T0重载值的高字节
unsigned char T0RL = 0;  //T0重载值的低字节

void ConfigTimer0(unsigned int ms);
extern void LcdInit();
extern void ConfigUART(unsigned int baud);
extern void UartRxMonitor(unsigned char ms);
extern void UartDriver();

void main ()
{
    EA = 1;           //开总中断
    ConfigTimer0(1);  //配置T0定时1ms
    ConfigUART(9600); //配置波特率为9600
    LcdInit();        //初始化液晶
    
    while(1)
    {
        UartDriver();
    }
}

void ConfigTimer0(unsigned int ms)  //T0配置函数
{
    unsigned long tmp;
    
    tmp = 11059200 / 12;      //定时器计数频率
    tmp = (tmp * ms) / 1000;  //计算所需的计数值
    tmp = 65536 - tmp;        //计算定时器重载值
    tmp = tmp + 34;           //修正中断响应延时造成的误差
    
    T0RH = (unsigned char)(tmp >> 8);  //定时器重载值拆分为高低字节
    T0RL = (unsigned char)tmp;
    TMOD &= 0xF0;   //清零T0的控制位
    TMOD |= 0x01;   //配置T0为模式1
    TH0 = T0RH;     //加载T0重载值
    TL0 = T0RL;
    ET0 = 1;        //使能T0中断
    TR0 = 1;        //启动T0
}
void InterruptTimer0() interrupt 1  //T0中断服务函数
{
    TH0 = T0RH;  //定时器重新加载重载值
    TL0 = T0RL;
    if (flagBuzzOn)  //蜂鸣器鸣叫或关闭
        BUZZ = ~BUZZ;
    else
        BUZZ = 1;
    UartRxMonitor(1);  //串口接收监控
}
