
#include <reg52.h>

#define LCD1602_DB   P0

sbit LCD1602_RS = P1^0;
sbit LCD1602_RW = P1^1;
sbit LCD1602_E  = P1^5;

void LcdWaitReady()  //等待液晶准备好
{
    unsigned char sta;
    
    LCD1602_DB = 0xFF;
    LCD1602_RS = 0;
    LCD1602_RW = 1;
    do
    {
        LCD1602_E = 1;
        sta = LCD1602_DB; //读取状态字
        LCD1602_E = 0;
    } while (sta & 0x80); //bit7等于1表示液晶正忙，重复检测直到其等于0为止
}
void LcdWriteCmd(unsigned char cmd)  //写入命令函数
{
    LcdWaitReady();
    LCD1602_RS = 0;
    LCD1602_RW = 0;
    LCD1602_DB = cmd;
    LCD1602_E  = 1;
    LCD1602_E  = 0;
}
void LcdWriteDat(unsigned char dat)  //写入数据函数
{
    LcdWaitReady();
    LCD1602_RS = 1;
    LCD1602_RW = 0;
    LCD1602_DB = dat;
    LCD1602_E  = 1;
    LCD1602_E  = 0;
}
void LcdShowStr(unsigned char x, unsigned char y, const unsigned char *str)  //显示字符串，屏幕起始坐标(x,y)，字符串指针str
{
    unsigned char addr;
    
    //由输入的显示坐标计算显示RAM的地址
    if (y == 0)
        addr = 0x00 + x; //第一行字符地址从0x00起始
    else
        addr = 0x40 + x; //第二行字符地址从0x40起始
    
    //由起始显示RAM地址连续写入字符串
    LcdWriteCmd(addr | 0x80); //写入起始地址
    while (*str != '\0')      //连续写入字符串数据，直到检测到结束符
    {
        LcdWriteDat(*str);
        str++;
    }
}
void LcdInit()  //液晶初始化函数
{
    LcdWriteCmd(0x38);  //16*2显示，5*7点阵，8位数据接口
    LcdWriteCmd(0x0C);  //显示器开，光标关闭
    LcdWriteCmd(0x06);  //文字不动，地址自动+1
    LcdWriteCmd(0x01);  //清屏
}
