
#include <reg52.h>

extern void InitLcd1602();
extern void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str);
extern void I2CStart();
extern void I2CStop();
extern unsigned char I2CReadACK();
extern unsigned char I2CReadNAK();
extern bit I2CWrite(unsigned char dat);
void E2Read(unsigned char *buf, unsigned char addr, unsigned char len);
void E2Write(unsigned char *buf, unsigned char addr, unsigned char len);
void MemToStr(unsigned char *str, unsigned char *src, unsigned char len);

void main()
{
    unsigned char i;
    unsigned char buf[5];
    unsigned char str[20];

    InitLcd1602();   //初始化液晶
    E2Read(buf, 0x90, sizeof(buf));   //从E2中读取一段数据
    MemToStr(str, buf, sizeof(buf));  //转换为十六进制字符串
    LcdShowStr(0, 0, str);            //显示到液晶上
    for (i=0; i<sizeof(buf); i++)     //数据依次+1,+2,+3...
    {
        buf[i] = buf[i] + 1 + i;
    }
    E2Write(buf, 0x90, sizeof(buf));  //再写回到E2中
    
    while(1);
}
void MemToStr(unsigned char *str, unsigned char *src, unsigned char len)
{
	unsigned char tmp;

	while(len--)
	{
		tmp = *src >> 4;
		if(tmp <= 9)
			*str++ = tmp + '0';
		else
			*str++ = tmp - 10 + 'A';
		tmp = *src & 0x0F;
		if(tmp <= 9)
			*str++ = tmp + '0';
		else
			*str++ = tmp - 10 + 'A';
		*str++ = ' ';
		src++;		
	}
}
void E2Read(unsigned char *buf, unsigned char addr, unsigned char len)
{
	do{
		I2CStart();
		if(I2CWrite(0x50 << 1))
		{
			break;
		}
		I2CStop();
	  }while(1);
	I2CWrite(addr);
	I2CStart();
	I2CWrite((0x50 << 1) | 0x01);
	while(len > 1)
	{
		*buf++ = I2CReadACK();
		len--;
	}
	*buf = I2CReadNAK();
	I2CStop();
}
void E2Write(unsigned char *buf, unsigned char addr, unsigned char len)
{
	while(len--)
	{
		do{
		I2CStart();
		if(I2CWrite(0x50 << 1))
		{
			break;
		}
		I2CStop();
	  }while(1);
	  I2CWrite(addr++);
	  I2CWrite(*buf++);
	  I2CStop();	
	}
}

