#include <reg52.h>

sbit DS1302_CE = P1^7;      // DS1302片选引脚
sbit DS1302_CK = P3^5;      // DS1302时钟引脚
sbit DS1302_IO = P3^4;      // DS1302数据引脚

bit flag200ms = 0;          // 200ms定时标志
unsigned char T0RH = 0;     // T0重载值的高字节
unsigned char T0RL = 0;     // T0重载值的低字节

void ConfigTimer0(unsigned int ms);                 // 配置定时器0
void InitDS1302();                                  // 初始化DS1302实时时钟
unsigned char DS1302SingleRead(unsigned char reg);  // 从DS1302单字节读取数据
extern void InitLcd1602();                          // 初始化LCD
extern void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str);  // 在LCD上显示字符串

void main()
{
    unsigned char i;
    unsigned char psec=0xAA;      // 秒备份，初值AA确保首次读取时间后会刷新显示
    unsigned char time[8];        // 当前时间数组
    unsigned char str[12];        // 字符串转换缓冲区

    EA = 1;                       // 开总中断
    ConfigTimer0(1);              // 配置定时器0，1ms间隔
    InitDS1302();                 // 初始化实时时钟
    InitLcd1602();                // 初始化液晶显示屏
    
    while (1)
    {
        if(flag200ms)             // 每200ms进入一次
        {
            flag200ms = 0;        // 清除标志位
            for(i=0; i<7; i++)   // 循环读取DS1302的时间数据
            {
                time[i] = DS1302SingleRead(i);	
            }
            if(psec != time[0])  // 每秒更新一次显示
            {
                // 构造日期时间字符串
                str[0] = '2';
                str[1] = '0';
                str[2] = (time[6] >> 4) + '0';  // 年十位
                str[3] = (time[6] & 0x0F) + '0'; // 年个位
                str[4] = '-';
                str[5] = (time[4] >> 4) + '0';  // 月十位
                str[6] = (time[4] & 0x0F) + '0'; // 月个位
                str[7] = '-';
                str[8] = (time[3] >> 4) + '0';  // 日十位
                str[9] = (time[3] & 0x0F) + '0'; // 日个位
                str[10] = '\0';
                LcdShowStr(0, 0, str);           // 在LCD上显示日期

                // 显示星期
                str[0] = (time[5] & 0x0F) + '0'; 
                str[1] = '\0';
                LcdShowStr(11, 0, "week");       // 在LCD上显示"week"
                LcdShowStr(15, 0, str);          // 在LCD上显示星期数

                // 构造时间字符串
                str[0] = (time[2] >> 4) + '0';   // 时十位
                str[1] = (time[2] & 0x0F) + '0'; // 时个位
                str[2] = ':';
                str[3] = (time[1] >> 4) + '0';   // 分十位
                str[4] = (time[1] & 0x0F) + '0'; // 分个位
                str[5] = ':';
                str[6] = (time[0] >> 4) + '0';   // 秒十位
                str[7] = (time[0] & 0x0F) + '0'; // 秒个位
                str[8] = '\0';
                LcdShowStr(4, 1, str);            // 在LCD上显示时间

                psec = time[0];                   // 更新秒备份
            }
        }
        
    }
}
// 向DS1302写入单字节数据
void DS1302ByteWrite(unsigned char dat)
{
    unsigned char mask;
    for(mask=0x01; mask!=0; mask<<=1)
    {
        if((mask&dat) != 0)
            DS1302_IO = 1;
        else
            DS1302_IO = 0;
        DS1302_CK = 1;
        DS1302_CK = 0;
    }
}
// 从DS1302读取单字节数据
unsigned char DS1302ByteRead()
{
    unsigned char dat = 0;
    unsigned char mask;

    for(mask=0x01; mask!=0; mask<<=1)
    {
        if(DS1302_IO != 0)
        {
            dat |= mask;
        }
        DS1302_CK = 1;
        DS1302_CK = 0;
    }

    return dat;
}
// 向DS1302写入单个寄存器数据
void DS1302SingleWrite(unsigned char reg, unsigned char dat)
{
    DS1302_CE = 1;
    DS1302ByteWrite((reg<<1) | 0x80);
    DS1302ByteWrite(dat);
    DS1302_CE = 0;
}
// 从DS1302读取单个寄存器数据
unsigned char  DS1302SingleRead(unsigned char reg)
{
    unsigned char dat;

    DS1302_CE = 1;
    DS1302ByteWrite((reg<<1) | 0x81);
    dat = DS1302ByteRead();
    DS1302_CE = 0;

    return dat;
}

// 初始化DS1302时钟
void InitDS1302()
{
    unsigned char i;
    unsigned char code InitTime[] = { // 初始时间
        0x00, 0x10, 0x21, 0x07, 0x02, 0x06, 0x21
    };
    // 秒、分、时、日、月、星期、年

    DS1302_CE = 0;
    DS1302_CK = 0;
    i = DS1302SingleRead(0); // 读取秒
	if((i & 0x80) != 0)
	{
		DS1302SingleWrite(7, 0x00);
		for(i=0; i<7; i++)
		{
			DS1302SingleWrite(i, InitTime[i]);
		}
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
/* T0中断服务函数，执行200ms定时 */
void InterruptTimer0() interrupt 1
{
    static unsigned char tmr200ms = 0;
    
    TH0 = T0RH;  //重新加载重载值
    TL0 = T0RL;
    tmr200ms++;
    if (tmr200ms >= 200)  //定时200ms
    {
        tmr200ms = 0;
        flag200ms = 1;
    }
}
