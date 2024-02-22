
#include <reg52.h>
#include <intrins.h>

sbit RS485_DIR = P1^7;  //RS485方向选择引脚

bit flagOnceTxd = 0;  //单次发送完成标志，即发送完一个字节
bit cmdArrived = 0;   //命令到达标志，即接收到上位机下发的命令
unsigned char cntRxd = 0;
unsigned char pdata bufRxd[40]; //串口接收缓冲区

unsigned char regGroup[5];  //Modbus寄存器组，地址为0x00～0x04

extern bit flagBuzzOn;
extern void LcdShowStr(unsigned char x, unsigned char y, const unsigned char *str);
extern unsigned int GetCRC16(unsigned char *ptr,  unsigned char len);

void ConfigUART(unsigned int baud)  //串口配置函数，baud为波特率
{
    RS485_DIR = 0; //RS485设置为接收方向
    SCON = 0x50;   //配置串口为模式1
    TMOD &= 0x0F;  //清零T1的控制位
    TMOD |= 0x20;  //配置T1为模式2
    TH1 = 256 - (11059200/12/32) / baud;  //计算T1重载值
    TL1 = TH1;     //初值等于重载值
    ET1 = 0;       //禁止T1中断
    ES  = 1;       //使能串口中断
    TR1 = 1;       //启动T1
}
unsigned char UartRead(unsigned char *buf, unsigned char len) //串口数据读取函数，数据接收指针buf，读取数据长度len，返回值为实际读取到的数据长度
{
    unsigned char i;
    
    if (len > cntRxd) //读取长度大于接收到的数据长度时，
    {
        len = cntRxd; //读取长度设置为实际接收到的数据长度
    }
    for (i=0; i<len; i++) //拷贝接收到的数据
    {
        *buf = bufRxd[i];
        buf++;
    }
    cntRxd = 0;  //清零接收计数器
    
    return len;  //返回实际读取长度
}
void DelayX10us(unsigned char t)  //软件延时函数，延时时间(t*10)us
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
void UartWrite(unsigned char *buf, unsigned char len) //串口数据写入函数，即串口发送函数，待发送数据指针buf，数据长度len
{
    RS485_DIR = 1;  //RS485设置为发送
    while (len--)   //发送数据
    {
        flagOnceTxd = 0;
        SBUF = *buf;
        buf++;
        while (!flagOnceTxd);
    }
    DelayX10us(5);  //等待最后的停止位完成，延时时间由波特率决定
    RS485_DIR = 0;  //RS485设置为接收
}

void UartDriver() //串口驱动函数，检测接收到的命令并执行相应动作
{
    unsigned char i;
    unsigned char cnt;
    unsigned char len;
    unsigned char buf[30];
    unsigned char str[4];
    unsigned int  crc;
    unsigned char crch, crcl;

    if (cmdArrived) //有命令到达时，读取处理该命令
    {
        cmdArrived = 0;
        len = UartRead(buf, sizeof(buf)); //将接收到的命令读取到缓冲区中
        if (buf[0] == 0x01)  //核对地址以决定是否响应命令，本例中的本机地址为0x01
        {
            crc = GetCRC16(buf, len-2); //计算CRC校验值
            crch = crc >> 8;
            crcl = crc & 0xFF;
            if ((buf[len-2] == crch) && (buf[len-1] == crcl)) //判断CRC校验是否正确
            {
                switch (buf[1]) //按功能码执行操作
                {
                    case 0x03:  //读取一个或连续的寄存器
                        if ((buf[2] == 0x00) && (buf[3] <= 0x05)) //寄存器地址支持0x0000～0x0005
                        {
                            if (buf[3] <= 0x04)
                            {
                                i = buf[3];      //提取寄存器地址
                                cnt = buf[5];    //提取待读取的寄存器数量
                                buf[2] = cnt*2;  //读取数据的字节数，为寄存器数*2，因Modbus定义的寄存器为16位
                                len = 3;
                                while (cnt--)
                                {
                                    buf[len++] = 0x00;          //寄存器高字节补0
                                    buf[len++] = regGroup[i++]; //寄存器低字节
                                }
                            }
                            else  //地址0x05为蜂鸣器状态
                            {
                                buf[2] = 2;  //读取数据的字节数
                                buf[3] = 0x00;
                                buf[4] = flagBuzzOn;
                                len = 5;
                            }
                            break;
                        }
                        else  //寄存器地址不被支持时，返回错误码
                        {
                            buf[1] = 0x83;  //功能码最高位置1
                            buf[2] = 0x02;  //设置异常码为02-无效地址
                            len = 3;
                            break;
                        }
                        
                    case 0x06:  //写入单个寄存器
                        if ((buf[2] == 0x00) && (buf[3] <= 0x05)) //寄存器地址支持0x0000～0x0005
                        {
                            if (buf[3] <= 0x04)
                            {
                                i = buf[3];             //提取寄存器地址
                                regGroup[i] = buf[5];   //保存寄存器数据
                                cnt = regGroup[i] >> 4; //显示到液晶上
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
                            else  //地址0x05为蜂鸣器状态
                            {
                                flagBuzzOn = (bit)buf[5]; //寄存器值转换为蜂鸣器的开关
                            }
                            len -= 2; //长度-2以重新计算CRC并返回原帧
                            break;
                        }
                        else  //寄存器地址不被支持时，返回错误码
                        {
                            buf[1] = 0x86;  //功能码最高位置1
                            buf[2] = 0x02;  //设置异常码为02-无效地址
                            len = 3;
                            break;
                        }
                        
                    default:  //其它不支持的功能码
                        buf[1] |= 0x80;  //功能码最高位置1
                        buf[2] = 0x01;   //设置异常码为01-无效功能
                        len = 3;
                        break;
                }
                crc = GetCRC16(buf, len); //计算CRC校验值
                buf[len++] = crc >> 8;    //CRC高字节
                buf[len++] = crc & 0xFF;  //CRC低字节
                UartWrite(buf, len);      //发送响应帧
            }
        }
    }
}

void UartRxMonitor(unsigned char ms)  //串口接收监控函数
{
    static unsigned char cntbkp = 0;
    static unsigned char idletmr = 0;

    if (cntRxd > 0)  //接收计数器大于零时，监控总线空闲时间
    {
        if (cntbkp != cntRxd)  //接收计数器改变，即刚接收到数据时，清零空闲计时
        {
            cntbkp = cntRxd;
            idletmr = 0;
        }
        else
        {
            if (idletmr < 5)  //接收计数器未改变，即总线空闲时，累积空闲时间
            {
                idletmr += ms;
                if (idletmr >= 5)  //空闲时间超过4个字节传输时间即认为一帧命令接收完毕
                {
                    cmdArrived = 1; //设置命令到达标志
                }
            }
        }
    }
    else
    {
        cntbkp = 0;
    }
}
void InterruptUART() interrupt 4  //UART中断服务函数
{
	if (RI)  //接收到字节
    {
		RI = 0;   //手动清零接收中断标志位
        if (cntRxd < sizeof(bufRxd)) //接收缓冲区尚未用完时，
        {
            bufRxd[cntRxd++] = SBUF; //保存接收字节，并递增计数器
        }
	}
	if (TI)  //字节发送完毕
    {
		TI = 0;   //手动清零发送中断标志位
        flagOnceTxd = 1;  //设置单次发送完成标志
	}
}
