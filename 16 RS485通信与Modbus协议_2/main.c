
#include <reg52.h>

sbit BUZZ = P1^6;  //��������������

bit flagBuzzOn = 0;   //������������־
unsigned char T0RH = 0;  //T0����ֵ�ĸ��ֽ�
unsigned char T0RL = 0;  //T0����ֵ�ĵ��ֽ�

void ConfigTimer0(unsigned int ms);
extern void LcdInit();
extern void ConfigUART(unsigned int baud);
extern void UartRxMonitor(unsigned char ms);
extern void UartDriver();

void main ()
{
    EA = 1;           //�����ж�
    ConfigTimer0(1);  //����T0��ʱ1ms
    ConfigUART(9600); //���ò�����Ϊ9600
    LcdInit();        //��ʼ��Һ��
    
    while(1)
    {
        UartDriver();
    }
}

void ConfigTimer0(unsigned int ms)  //T0���ú���
{
    unsigned long tmp;
    
    tmp = 11059200 / 12;      //��ʱ������Ƶ��
    tmp = (tmp * ms) / 1000;  //��������ļ���ֵ
    tmp = 65536 - tmp;        //���㶨ʱ������ֵ
    tmp = tmp + 34;           //�����ж���Ӧ��ʱ��ɵ����
    
    T0RH = (unsigned char)(tmp >> 8);  //��ʱ������ֵ���Ϊ�ߵ��ֽ�
    T0RL = (unsigned char)tmp;
    TMOD &= 0xF0;   //����T0�Ŀ���λ
    TMOD |= 0x01;   //����T0Ϊģʽ1
    TH0 = T0RH;     //����T0����ֵ
    TL0 = T0RL;
    ET0 = 1;        //ʹ��T0�ж�
    TR0 = 1;        //����T0
}
void InterruptTimer0() interrupt 1  //T0�жϷ�����
{
    TH0 = T0RH;  //��ʱ�����¼�������ֵ
    TL0 = T0RL;
    if (flagBuzzOn)  //���������л�ر�
        BUZZ = ~BUZZ;
    else
        BUZZ = 1;
    UartRxMonitor(1);  //���ڽ��ռ��
}
