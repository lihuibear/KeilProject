
#include <reg52.h>

bit flag300ms = 1;       //300ms定时标志
unsigned char T0RH = 0;  //T0重载值的高字节
unsigned char T0RL = 0;  //T0重载值的低字节

void ConfigTimer0(unsigned int ms);
unsigned char GetADCValue(unsigned char chn);
void ValueToString(unsigned char *str, unsigned char val);
extern void I2CStart();
extern void I2CStop();
extern unsigned char I2CReadACK();
extern unsigned char I2CReadNAK();
extern bit I2CWrite(unsigned char dat);
extern void InitLcd1602();
extern void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str);

void main()
{
    unsigned char val;
    unsigned char str[10];
    
    EA = 1;            //开总中断
    ConfigTimer0(10);  //配置T0定时10ms
    InitLcd1602();     //初始化液晶    
    LcdShowStr(0, 0, "AIN0  AIN1  AIN3");  //显示通道指示
    
    while (1)
    {
        if (flag300ms)
        {
            flag300ms = 0;
            //显示通道0的电压
            val = GetADCValue(0);     //获取ADC通道0的转换值
            ValueToString(str, val);  //转为字符串格式的电压值
            LcdShowStr(0, 1, str);    //显示到液晶上
            //显示通道1的电压
            val = GetADCValue(1);
            ValueToString(str, val);
            LcdShowStr(6, 1, str);
            //显示通道3的电压
            val = GetADCValue(3);
            ValueToString(str, val);
            LcdShowStr(12, 1, str);
        }
    }
}
unsigned char GetADCValue(unsigned char chn)
{
	unsigned char val;

	I2CStart();
	if(!I2CWrite(0x48<<1))
	{
		I2CStop();
		return 0;
	}
	I2CWrite(0x40 | chn);
	I2CStart();
	I2CWrite(0x48<<1 | 0x01);
	I2CReadACK();
	val = I2CReadNAK();
	I2CStop();

	return val;
}

void  ValueToString(unsigned char *str, unsigned char val)
{
	val = (val*25)/255;
	str[0] = (val/10) + '0';
	str[1] = '.';
	str[2] = (val%10) + '0';
	str[3] = 'V';
	str[4] = '\0';
}
/* 配置并启动T0，ms-T0定时时间 */
void ConfigTimer0(unsigned int ms)
{
    unsigned long tmp;  //临时变量
    
    tmp = 11059200 / 12;      //定时器计数频率
    tmp = (tmp * ms) / 1000;  //计算所需的计数值
    tmp = 65536 - tmp;        //计算定时器重载值
    tmp = tmp + 32;           //补偿中断响应延时造成的误差
    T0RH = (unsigned char)(tmp>>8);  //定时器重载值拆分为高低字节
    T0RL = (unsigned char)tmp;
    TMOD &= 0xF0;   //清零T0的控制位
    TMOD |= 0x01;   //配置T0为模式1
    TH0 = T0RH;     //加载T0重载值
    TL0 = T0RL;
    ET0 = 1;        //使能T0中断
    TR0 = 1;        //启动T0
}
/* T0中断服务函数，执行300ms定时 */
void InterruptTimer0() interrupt 1
{
    static unsigned char tmr300ms = 0;
    
    TH0 = T0RH;  //重新加载重载值
    TL0 = T0RL;
    tmr300ms++;
    if (tmr300ms >= 30)  //定时300ms
    {
        tmr300ms = 0;
        flag300ms = 1;
    }
}
