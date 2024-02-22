#include <reg52.h>

#define LCD1602_DB  P0   // LCD1602 数据总线接口定义在 P0 口
sbit LCD1602_RS = P1^0;  // 指定 LCD1602 的 RS 信号接口为 P1^0
sbit LCD1602_RW = P1^1;  // 指定 LCD1602 的 RW 信号接口为 P1^1
sbit LCD1602_E = P1^5;   // 指定 LCD1602 的 E 信号接口为 P1^5

void InitLcd1602();  // LCD1602 初始化函数声明
void LcdWaitReady();// 等待 LCD1602 准备就绪
void LcdWriteCmd(unsigned char cmd);	// 写入命令到 LCD1602
void LcdWriteDat(unsigned char dat);// 写入数据到 LCD1602
void LcdSetCursor(unsigned char x, unsigned char y);// 设置 LCD1602 的显示位置
void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str);  // 在指定位置显示字符串函数声明

void main()
{
    unsigned char str[] = "Kingst Studio";  // 要显示的字符串

    InitLcd1602();  // 初始化 LCD1602
    LcdShowStr(2, 0, str);  // 在第一行第二列显示 "Kingst Studio"数字是坐标
    LcdShowStr(0, 1, "Welcome to KST51");  // 在第二行第一列显示 "Welcome to KST51"
    
    while(1);  // 主程序循环
}

// 等待 LCD1602 准备就绪
void LcdWaitReady()
{
    unsigned char sta;

    LCD1602_DB = 0xFF;  // 设置数据总线为输入
    LCD1602_RS = 0;     // 设置 RS 为命令模式
    LCD1602_RW = 1;     // 设置 RW 为读模式
    do {
        LCD1602_E = 1;  // 使能 E 信号
        sta = LCD1602_DB;  // 读取状态
        LCD1602_E = 0;  // 禁能 E 信号
    } while(sta & 0x80);  // 检查忙标志
}

// 写入命令到 LCD1602
void LcdWriteCmd(unsigned char cmd)
{
    LcdWaitReady();  // 等待 LCD1602 准备就绪 忙位检测
    LCD1602_RS = 0;  // 命令模式
    LCD1602_RW = 0;  // 写入模式
    LCD1602_DB = cmd;  // 写入命令
    LCD1602_E = 1;  // 使能 E 信号
    LCD1602_E = 0;  // 禁能 E 信号
}

// 写入数据到 LCD1602
void LcdWriteDat(unsigned char dat)
{
    LcdWaitReady();  // 等待 LCD1602 准备就绪
    LCD1602_RS = 1;  // 数据模式
    LCD1602_RW = 0;  // 写入模式
    LCD1602_DB = dat;  // 写入数据
    LCD1602_E = 1;  // 使能 E 信号
    LCD1602_E = 0;  // 禁能 E 信号
}

// 设置 LCD1602 的显示位置
void LcdSetCursor(unsigned char x, unsigned char y)
{
    unsigned char addr;
    if(y == 0)
        addr = 0x00 + x;
    else
        addr = 0x40 + x;
    LcdWriteCmd(addr | 0x80);  // 设置光标位置
}

// 在 LCD1602 上显示字符串
void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str)
{
    LcdSetCursor(x, y);  // 设置光标位置
    while(*str != '\0')
    {
        LcdWriteDat(*str++);  // 逐个写入字符
    }
}

// 初始化 LCD1602
void InitLcd1602()
{
    LcdWriteCmd(0x38);  // 8位数据接口，两行显示，5x7 点阵字符
    LcdWriteCmd(0x0C);  // 开显示，不显示光标
    LcdWriteCmd(0x06);  // 光标右移，字符不移动
    LcdWriteCmd(0x01);  // 清屏
}
