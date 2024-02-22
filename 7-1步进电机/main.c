#include<reg52.h>

unsigned char code BeatCode[8] = {
	0x0E, 0x0C, 0x0D, 0x09, 0x0B, 0x03, 0x07, 0x06};
void delay();

void main()
{
	unsigned char tmp;
	unsigned char index = 0;

	while(1)
	{
		tmp = P1;
		tmp = tmp & 0xF0; //高四位不变，低四位清零
		tmp = tmp | BeatCode[index]; //高四位不变，低四位存下一次的节拍
		P1 = tmp; //赋值
		index++;
		index = index & 0x07; //加到8归零
		delay();
	}
}

void delay()
{	
	unsigned int i = 200;
	while(i--);
}
