#include<reg52.h>

void TrunMotor(unsigned long angle);

void main()
{
		TrunMotor(360*25);
		while(1);
}

void delay()
{	
	unsigned int i = 200;
	while(i--);
}

void TrunMotor(unsigned long angle)
{
	unsigned char tmp;
	unsigned char index = 0;
	unsigned long beats = 0;
	unsigned char code BeatCode[8] = {
	0x0E, 0x0C, 0x0D, 0x09, 0x0B, 0x03, 0x07, 0x06};

	beats = (angle*4076)/360; //度数转换为节拍//4096为一圈需要的节拍 64 * 64  实际 1：63.684
														//(32 /9)* (22/11)*(26/9)*(31/10)
	while(beats--)
	{
		tmp = P1;
		tmp = tmp & 0xF0;
		tmp = tmp | BeatCode[index];			 
		P1 = tmp;
		index++;
		index = index & 0x07;
		delay();
	}
}