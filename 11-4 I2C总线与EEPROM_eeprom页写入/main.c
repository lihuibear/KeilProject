
#include <reg52.h>

extern void InitLcd1602();
extern void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str);
extern void E2Read(unsigned char *buf, unsigned char addr, unsigned char len);
extern void E2Write(unsigned char *buf, unsigned char addr, unsigned char len);
void MemToStr(unsigned char *str, unsigned char *src, unsigned char len);

void main()
{
    unsigned char i;
    unsigned char buf[5];
    unsigned char str[20];

    InitLcd1602();   //初始化液晶
    E2Read(buf, 0x8E, sizeof(buf));   //从E2中读取一段数据
    MemToStr(str, buf, sizeof(buf));  //转换为十六进制字符串
    LcdShowStr(0, 0, str);            //显示到液晶上
    for (i=0; i<sizeof(buf); i++)     //数据依次+1,+2,+3...
    {
        buf[i] = buf[i] + 1 + i;
    }
    E2Write(buf, 0x8E, sizeof(buf));  //再写回到E2中
    
    while(1);
}
/* 将一段内存数据转换为十六进制格式的字符串，
   str-字符串指针，src-源数据地址，len-数据长度 */
void MemToStr(unsigned char *str, unsigned char *src, unsigned char len)
{
    unsigned char tmp;

    while (len--)
    {
        tmp = *src >> 4;           //先取高4位
        if (tmp <= 9)              //转换为0-9或A-F
            *str++ = tmp + '0';
        else
            *str++ = tmp - 10 + 'A';
        tmp = *src & 0x0F;         //再取低4位
        if (tmp <= 9)              //转换为0-9或A-F
            *str++ = tmp + '0';
        else
            *str++ = tmp - 10 + 'A';
        *str++ = ' ';              //转换完一个字节添加一个空格
        src++;
    }
}
