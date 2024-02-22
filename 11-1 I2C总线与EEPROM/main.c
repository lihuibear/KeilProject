#include <reg52.h>      // 引入 8051 单片机的头文件
#include <intrins.h>    // 内部函数声明

#define I2CDelay()  {_nop_();_nop_();_nop_();_nop_();}  // 定义延时函数

sbit I2C_SCL = P3^7;    // 定义 I2C 时钟线引脚
sbit I2C_SDA = P3^6;    // 定义 I2C 数据线引脚

extern void InitLcd1602();                              // 外部函数声明：初始化液晶
extern void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str);  // 外部函数声明：在液晶上显示字符串

bit I2CAddresing(unsigned char addr);  // 函数声明：I2C 设备寻址

void main()
{
    bit ack = 0;                // 定义应答位变量
    unsigned char str[10];      // 定义字符串数组

    InitLcd1602();              // 初始化液晶显示屏

    // 访问设备地址为 0x50 的设备，并将访问结果显示在液晶上
    ack = I2CAddresing(0x50);
    str[0] = '5';
    str[1] = '0';
    str[2] = ':';
    str[3] = (unsigned char)ack + '0';
    str[4] = '\0';
    LcdShowStr(0, 0, str);

    // 访问设备地址为 0x62 的设备，并将访问结果显示在液晶上
    ack = I2CAddresing(0x62);
    str[0] = '6';
    str[1] = '2';
    str[2] = ':';
    str[3] = (unsigned char)ack + '0';
    str[4] = '\0';
    LcdShowStr(8, 0, str);
    
    while (1);  // 无限循环
}

// 函数：发送 I2C 起始信号
void I2CStart()
{
    I2C_SDA = 1;
    I2C_SCL = 1;
    I2CDelay();
    I2C_SDA = 0;
    I2CDelay();
    I2C_SCL = 0;
}

// 函数：发送 I2C 停止信号
void I2CStop()
{
    I2C_SCL = 0;
    I2C_SDA = 0;
    I2CDelay();
    I2C_SCL = 1;
    I2CDelay();
    I2C_SDA = 1;
    I2CDelay();
}

// 函数：向 I2C 总线写入数据
bit I2CWrite(unsigned char dat)
{
    bit ack = 0;            // 定义应答位变量
    unsigned char mask;     // 定义掩码变量

    for(mask=0x80; mask!=0; mask>>=1)  // 循环处理每一位数据
    {
        if((mask&dat) == 0)             // 如果当前位为 0
            I2C_SDA = 0;                // 将数据线拉低
        else                            // 如果当前位为 1
            I2C_SDA = 1;                // 将数据线拉高
        I2CDelay();
        I2C_SCL = 1;                    // 时钟线拉高，写入数据
        I2CDelay();
        I2C_SCL = 0;                    // 时钟线拉低
    }
    I2C_SDA = 1;                        // 释放数据线
    I2CDelay();
    I2C_SCL = 1;                        // 时钟线拉高，准备接收应答信号
    ack = I2C_SDA;                      // 读取应答信号
    I2CDelay();
    I2C_SCL = 0;                        // 时钟线拉低

    return ack;                         // 返回应答信号
}

// 函数：向 I2C 总线发送设备地址并检查应答信号
bit I2CAddresing(unsigned char addr)
{
    bit ack = 0;    // 定义应答位变量

    I2CStart();                 // 发送起始信号
    ack = I2CWrite(addr << 1);  // 发送设备地址，并接收应答信号
    I2CStop();                  // 发送停止信号

    return ack;                 // 返回应答信号
}
