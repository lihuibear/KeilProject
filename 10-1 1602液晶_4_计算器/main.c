#include <reg52.h>
unsigned char step = 0;
unsigned char oprt = 0;
signed long num1 = 0;
signed long num2 = 0;
signed long result = 0;
unsigned char T0RH = 0;
unsigned char T0RL = 0;

void ConfigTimer0(unsigned int ms);
extern void KeyScan();
extern void KeyDriver();
extern void InitLcd1602();
extern void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str);
extern void LcdFullClear();
extern void LcdAreaClear(unsigned char x, unsigned char y, unsigned char len);

void main()
{
    EA = 1;           //开总中断
    ConfigTimer0(1);  //配置T0定时1ms
	InitLcd1602();    //初始化液晶
    LcdShowStr(15, 1, "0");  //初始显示一个数字0
    
    while (1)
    {
        KeyDriver();  //调用按键驱动
    }
}
unsigned char LongToString(unsigned char *str, signed long dat)
{
	signed char i = 0;
	unsigned char len = 0;
	unsigned char buf[12];

	if(dat < 0)
	{
		dat = -dat;
		*str++ = '-';
		len++;
	}
	do{
		buf[i++] = dat % 10;
		dat /= 10;
		}while(dat > 0);
	len += i;
	while(i-- > 0)
	{
		*str++ = buf[i] + '0';
	}
	*str = '\0';

	return len;
		
}
void ShowOprt(unsigned char y, unsigned char type)
{
	switch(type)
	{
		case 0: LcdShowStr(0, y, "+"); break;
		case 1:	LcdShowStr(0, y, "-"); break;
		case 2:	LcdShowStr(0, y, "*"); break;
		case 3:	LcdShowStr(0, y, "/"); break;
		default: break;
	}
}
void Reset()
{
	num1 = 0;
	num2 = 0;
	step = 0;
	LcdFullClear();
}
void NumKeyAction(unsigned char n)
{
	unsigned char len = 0;
	unsigned char str[12];

	if(step > 1)
	{
		Reset();
	}
	if(step == 0)
	{
		num1 = num1*10 + n;
		len = LongToString(str, num1);
		LcdShowStr(16 - len, 1, str);
	}
	else 
	{
		num2 = num2*10 + n;
		len = LongToString(str, num2);
		LcdShowStr(16 - len, 1, str);
	}	
}
void OprtKeyAction(unsigned char type)
{  
	unsigned char len;
	unsigned char str[12];
	if(step == 0)
	{
		len = LongToString(str, num1);
		LcdAreaClear(0, 0, 16-len);
		LcdShowStr(16-len, 0, str);
		ShowOprt(1, type);				 
		LcdAreaClear(1, 1, 14);			  
		LcdShowStr(15, 1, "0");
		oprt = type; 
		step = 1; 
	}
}
void GetResult()
{
	unsigned char len;
	unsigned char str[12];
	if(step == 1)
	{
		step = 2;
		switch(oprt)
		{
			case 0: result = num1 + num2; break;
			case 1: result = num1 - num2; break;
			case 2: result = num1 * num2; break;
			case 3: result = num1 / num2; break;
			default: break;
		}

		len = LongToString(str, num2); 
		ShowOprt(0, oprt);                    
		LcdAreaClear(1, 0, 16-1-len);
		LcdShowStr(16-len, 0, str);
		len = LongToString(str, result);
		LcdShowStr(0, 1, "=");
		LcdAreaClear(1, 1, 16-1-len);
		LcdShowStr(16-len, 1, str);   
	}										 
}

/* 配置并启动T0，ms-T0定时时间 */
void ConfigTimer0(unsigned int ms)
{
    unsigned long tmp;  //临时变量
    
    tmp = 11059200 / 12;      //定时器计数频率
    tmp = (tmp * ms) / 1000;  //计算所需的计数值
    tmp = 65536 - tmp;        //计算定时器重载值
    tmp = tmp + 12;           //补偿中断响应延时造成的误差
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