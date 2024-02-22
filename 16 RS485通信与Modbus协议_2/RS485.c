
#include <reg52.h>
#include <intrins.h>

sbit RS485_DIR = P1^7;  //RS485����ѡ������

bit flagOnceTxd = 0;  //���η�����ɱ�־����������һ���ֽ�
bit cmdArrived = 0;   //������־�������յ���λ���·�������
unsigned char cntRxd = 0;
unsigned char pdata bufRxd[40]; //���ڽ��ջ�����

unsigned char regGroup[5];  //Modbus�Ĵ����飬��ַΪ0x00��0x04

extern bit flagBuzzOn;
extern void LcdShowStr(unsigned char x, unsigned char y, const unsigned char *str);
extern unsigned int GetCRC16(unsigned char *ptr,  unsigned char len);

void ConfigUART(unsigned int baud)  //�������ú�����baudΪ������
{
    RS485_DIR = 0; //RS485����Ϊ���շ���
    SCON = 0x50;   //���ô���Ϊģʽ1
    TMOD &= 0x0F;  //����T1�Ŀ���λ
    TMOD |= 0x20;  //����T1Ϊģʽ2
    TH1 = 256 - (11059200/12/32) / baud;  //����T1����ֵ
    TL1 = TH1;     //��ֵ��������ֵ
    ET1 = 0;       //��ֹT1�ж�
    ES  = 1;       //ʹ�ܴ����ж�
    TR1 = 1;       //����T1
}
unsigned char UartRead(unsigned char *buf, unsigned char len) //�������ݶ�ȡ���������ݽ���ָ��buf����ȡ���ݳ���len������ֵΪʵ�ʶ�ȡ�������ݳ���
{
    unsigned char i;
    
    if (len > cntRxd) //��ȡ���ȴ��ڽ��յ������ݳ���ʱ��
    {
        len = cntRxd; //��ȡ��������Ϊʵ�ʽ��յ������ݳ���
    }
    for (i=0; i<len; i++) //�������յ�������
    {
        *buf = bufRxd[i];
        buf++;
    }
    cntRxd = 0;  //������ռ�����
    
    return len;  //����ʵ�ʶ�ȡ����
}
void DelayX10us(unsigned char t)  //�����ʱ��������ʱʱ��(t*10)us
{
    do {
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
    } while (--t);
}
void UartWrite(unsigned char *buf, unsigned char len) //��������д�뺯���������ڷ��ͺ���������������ָ��buf�����ݳ���len
{
    RS485_DIR = 1;  //RS485����Ϊ����
    while (len--)   //��������
    {
        flagOnceTxd = 0;
        SBUF = *buf;
        buf++;
        while (!flagOnceTxd);
    }
    DelayX10us(5);  //�ȴ�����ֹͣλ��ɣ���ʱʱ���ɲ����ʾ���
    RS485_DIR = 0;  //RS485����Ϊ����
}

void UartDriver() //�������������������յ������ִ����Ӧ����
{
    unsigned char i;
    unsigned char cnt;
    unsigned char len;
    unsigned char buf[30];
    unsigned char str[4];
    unsigned int  crc;
    unsigned char crch, crcl;

    if (cmdArrived) //�������ʱ����ȡ���������
    {
        cmdArrived = 0;
        len = UartRead(buf, sizeof(buf)); //�����յ��������ȡ����������
        if (buf[0] == 0x01)  //�˶Ե�ַ�Ծ����Ƿ���Ӧ��������еı�����ַΪ0x01
        {
            crc = GetCRC16(buf, len-2); //����CRCУ��ֵ
            crch = crc >> 8;
            crcl = crc & 0xFF;
            if ((buf[len-2] == crch) && (buf[len-1] == crcl)) //�ж�CRCУ���Ƿ���ȷ
            {
                switch (buf[1]) //��������ִ�в���
                {
                    case 0x03:  //��ȡһ���������ļĴ���
                        if ((buf[2] == 0x00) && (buf[3] <= 0x05)) //�Ĵ�����ַ֧��0x0000��0x0005
                        {
                            if (buf[3] <= 0x04)
                            {
                                i = buf[3];      //��ȡ�Ĵ�����ַ
                                cnt = buf[5];    //��ȡ����ȡ�ļĴ�������
                                buf[2] = cnt*2;  //��ȡ���ݵ��ֽ�����Ϊ�Ĵ�����*2����Modbus����ļĴ���Ϊ16λ
                                len = 3;
                                while (cnt--)
                                {
                                    buf[len++] = 0x00;          //�Ĵ������ֽڲ�0
                                    buf[len++] = regGroup[i++]; //�Ĵ������ֽ�
                                }
                            }
                            else  //��ַ0x05Ϊ������״̬
                            {
                                buf[2] = 2;  //��ȡ���ݵ��ֽ���
                                buf[3] = 0x00;
                                buf[4] = flagBuzzOn;
                                len = 5;
                            }
                            break;
                        }
                        else  //�Ĵ�����ַ����֧��ʱ�����ش�����
                        {
                            buf[1] = 0x83;  //���������λ��1
                            buf[2] = 0x02;  //�����쳣��Ϊ02-��Ч��ַ
                            len = 3;
                            break;
                        }
                        
                    case 0x06:  //д�뵥���Ĵ���
                        if ((buf[2] == 0x00) && (buf[3] <= 0x05)) //�Ĵ�����ַ֧��0x0000��0x0005
                        {
                            if (buf[3] <= 0x04)
                            {
                                i = buf[3];             //��ȡ�Ĵ�����ַ
                                regGroup[i] = buf[5];   //����Ĵ�������
                                cnt = regGroup[i] >> 4; //��ʾ��Һ����
                                if (cnt >= 0xA)
                                    str[0] = cnt - 0xA + 'A';
                                else
                                    str[0] = cnt + '0';
                                cnt = regGroup[i] & 0x0F;
                                if (cnt >= 0xA)
                                    str[1] = cnt - 0xA + 'A';
                                else
                                    str[1] = cnt + '0';
                                str[2] = '\0';
                                LcdShowStr(i*3, 0, str);
                            }
                            else  //��ַ0x05Ϊ������״̬
                            {
                                flagBuzzOn = (bit)buf[5]; //�Ĵ���ֵת��Ϊ�������Ŀ���
                            }
                            len -= 2; //����-2�����¼���CRC������ԭ֡
                            break;
                        }
                        else  //�Ĵ�����ַ����֧��ʱ�����ش�����
                        {
                            buf[1] = 0x86;  //���������λ��1
                            buf[2] = 0x02;  //�����쳣��Ϊ02-��Ч��ַ
                            len = 3;
                            break;
                        }
                        
                    default:  //������֧�ֵĹ�����
                        buf[1] |= 0x80;  //���������λ��1
                        buf[2] = 0x01;   //�����쳣��Ϊ01-��Ч����
                        len = 3;
                        break;
                }
                crc = GetCRC16(buf, len); //����CRCУ��ֵ
                buf[len++] = crc >> 8;    //CRC���ֽ�
                buf[len++] = crc & 0xFF;  //CRC���ֽ�
                UartWrite(buf, len);      //������Ӧ֡
            }
        }
    }
}

void UartRxMonitor(unsigned char ms)  //���ڽ��ռ�غ���
{
    static unsigned char cntbkp = 0;
    static unsigned char idletmr = 0;

    if (cntRxd > 0)  //���ռ�����������ʱ��������߿���ʱ��
    {
        if (cntbkp != cntRxd)  //���ռ������ı䣬���ս��յ�����ʱ��������м�ʱ
        {
            cntbkp = cntRxd;
            idletmr = 0;
        }
        else
        {
            if (idletmr < 5)  //���ռ�����δ�ı䣬�����߿���ʱ���ۻ�����ʱ��
            {
                idletmr += ms;
                if (idletmr >= 5)  //����ʱ�䳬��4���ֽڴ���ʱ�伴��Ϊһ֡����������
                {
                    cmdArrived = 1; //����������־
                }
            }
        }
    }
    else
    {
        cntbkp = 0;
    }
}
void InterruptUART() interrupt 4  //UART�жϷ�����
{
	if (RI)  //���յ��ֽ�
    {
		RI = 0;   //�ֶ���������жϱ�־λ
        if (cntRxd < sizeof(bufRxd)) //���ջ�������δ����ʱ��
        {
            bufRxd[cntRxd++] = SBUF; //��������ֽڣ�������������
        }
	}
	if (TI)  //�ֽڷ������
    {
		TI = 0;   //�ֶ����㷢���жϱ�־λ
        flagOnceTxd = 1;  //���õ��η�����ɱ�־
	}
}
