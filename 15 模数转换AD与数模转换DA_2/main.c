#include <reg52.h>

unsigned char T0RH = 0;  //T0重载值的高字节
unsigned char T0RL = 0;  //T0重载值的低字节

void ConfigTimer0(unsigned int ms);
extern void KeyScan();
extern void KeyDriver();
extern void I2CStart();
extern void I2CStop();
extern bit I2CWrite(unsigned char dat);

void main()
{    
    EA = 1;             //开总中断
    ConfigTimer0(1);  //配置T0定时1ms
    
    while (1)
    {
        KeyDriver();  //调用按键驱动
    }
}

void SetDACOut(unsigned char val)
{
	I2CStart();
	if(!I2CWrite(0x48<<1))
	{
		I2CStop();
		return;
	}
	I2CWrite(0x40);
	I2CWrite(val);
	I2CStop();
}
void KeyAction(unsigned char keycode)
{
	static unsigned char volt = 0;

	if(keycode == 0x26)
	{
		if(volt < 25)
		{
			volt++;
			SetDACOut(volt*255/25);
		}
	}
	else if(keycode == 0x28)
	{
		if(volt > 0)
		{
			volt--;
			SetDACOut(volt*255/25);
		}
	}
}
/* 配置并启动T0，ms-T0定时时间 */
void ConfigTimer0(unsigned int ms)
{
    unsigned long tmp;  //临时变量
    
    tmp = 11059200 / 12;       //定时器计数频率
    tmp = (tmp * ms) / 1000;  //计算所需的计数值
    tmp = 65536 - tmp;         //计算定时器重载值
    tmp = tmp + 28;            //补偿中断响应延时造成的误差
    T0RH = (unsigned char)(tmp>>8);  //定时器重载值拆分为高低字节
    T0RL = (unsigned char)tmp;
    TMOD &= 0xF0;   //清零T0的控制位
    TMOD |= 0x01;   //配置T0为模式1
    TH0 = T0RH;     //加载T0重载值
    TL0 = T0RL;
    ET0 = 1;        //使能T0中断
    TR0 = 1;        //启动T0
}
/* T0中断服务函数，执行按键扫描 */
void InterruptTimer0() interrupt 1
{
    TH0 = T0RH;  //重新加载重载值
    TL0 = T0RL;
    KeyScan();   //按键扫描
}