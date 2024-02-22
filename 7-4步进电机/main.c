#include<reg52.h>

sbit KEY_IN_1 = P2^4;
sbit KEY_IN_2 = P2^5;
sbit KEY_IN_3 = P2^6;
sbit KEY_IN_4 = P2^7;
sbit KEY_OUT_1 = P2^3;
sbit KEY_OUT_2 = P2^2;
sbit KEY_OUT_3 = P2^1;
sbit KEY_OUT_4 = P2^0;
signed long beats = 0;
unsigned char KeySta[4][4] = { 
	{1,1,1,1},{1,1,1,1},{1,1,1,1},{1,1,1,1}
	};

unsigned char code KeyCodeMap[4][4] = { //矩阵按键编号到标准键盘键码的映射表
    { 0x31, 0x32, 0x33, 0x26 }, //数字键1、数字键2、数字键3、向上键
    { 0x34, 0x35, 0x36, 0x25 }, //数字键4、数字键5、数字键6、向左键
    { 0x37, 0x38, 0x39, 0x28 }, //数字键7、数字键8、数字键9、向下键
    { 0x30, 0x1B, 0x0D, 0x27 }  //数字键0、ESC键、  回车键、 向右键
	};
void  KeyDriver();
void main()
{
    EA = 1;       //使能总中断
    TMOD = 0x01;  //设置T0为模式1
    TH0  = 0xFC;  //为T0赋初值0xFC67，定时1ms
    TL0  = 0x67;
    ET0  = 1;     //使能T0中断
    TR0  = 1;     //启动T0
	
    while (1)
    {
        KeyDriver();   //调用按键驱动函数
    }
}

void StartMotor(signed long angle)
{
	EA = 0;
	beats = (angle*4076)/360;
	EA = 1;
}
void StopMotor()
{
	EA = 0;
	beats = 0;
	EA = 1;
}
void KeyAction(unsigned char keycode)
{
	static bit dirMotor = 0;
	
	if((keycode >= 0x30) &&(keycode <= 0x39))
	{
		if(dirMotor == 0)
		{
			StartMotor(360 * (keycode - 0x30));
		}
		else
		{
			StartMotor(-360 * (keycode - 0x30));	
		}
	}
	
	else if(keycode == 0x26)
	{
		dirMotor = 0;
	}
	else if(keycode == 0x28)
	{
		dirMotor = 1;
	}
	else if(keycode == 0x25)
	{
		StartMotor(90);
	}
	else if(keycode == 0x27)
	{
		StartMotor(-90);
	}


	else if(keycode == 0x1B)
	{
		StopMotor(); 	
	}	
	 
}
void  KeyDriver()
{
	unsigned char i, j;
	static	unsigned char backup [4][4] = {
	{1,1,1,1},{1,1,1,1},{1,1,1,1},{1,1,1,1}
	};

	for(i=0; i<4; i++)
		{
			for(j=0; j<4; j++)
			{
				if(backup[i][j] != KeySta[i][j])
				{
					if(backup[i][j] == 0)
					{
						KeyAction(KeyCodeMap[i][j]);
					}
					backup[i][j] = KeySta[i][j];
				}
			}	
		}

	
}

/* 按键扫描函数，需在定时中断中调用，推荐调用间隔1ms */
void KeyScan()
{
    unsigned char i;
    static unsigned char keyout = 0;   //矩阵按键扫描输出索引
    static unsigned char keybuf[4][4] = {  //矩阵按键扫描缓冲区
        {0xFF, 0xFF, 0xFF, 0xFF},  {0xFF, 0xFF, 0xFF, 0xFF},
        {0xFF, 0xFF, 0xFF, 0xFF},  {0xFF, 0xFF, 0xFF, 0xFF}
    };

    //将一行的4个按键值移入缓冲区
    keybuf[keyout][0] = (keybuf[keyout][0] << 1) | KEY_IN_1;
    keybuf[keyout][1] = (keybuf[keyout][1] << 1) | KEY_IN_2;
    keybuf[keyout][2] = (keybuf[keyout][2] << 1) | KEY_IN_3;
    keybuf[keyout][3] = (keybuf[keyout][3] << 1) | KEY_IN_4;
    //消抖后更新按键状态
    for (i=0; i<4; i++)  //每行4个按键，所以循环4次
    {
        if ((keybuf[keyout][i] & 0x0F) == 0x00)
        {   //连续4次扫描值为0，即4*4ms内都是按下状态时，可认为按键已稳定的按下
            KeySta[keyout][i] = 0;
        }
        else if ((keybuf[keyout][i] & 0x0F) == 0x0F)
        {   //连续4次扫描值为1，即4*4ms内都是弹起状态时，可认为按键已稳定的弹起
            KeySta[keyout][i] = 1;
        }
    }
    //执行下一次的扫描输出
    keyout++;                //输出索引递增
    keyout = keyout & 0x03;  //索引值加到4即归零
    switch (keyout)          //根据索引，释放当前输出引脚，拉低下次的输出引脚
    {
        case 0: KEY_OUT_4 = 1; KEY_OUT_1 = 0; break;
        case 1: KEY_OUT_1 = 1; KEY_OUT_2 = 0; break;
        case 2: KEY_OUT_2 = 1; KEY_OUT_3 = 0; break;
        case 3: KEY_OUT_3 = 1; KEY_OUT_4 = 0; break;
        default: break;
    }
}

void TurnMotor()
{
	unsigned char tmp;
	static unsigned char index = 0;
	unsigned char code BeatCode[8] = {
	0x0E, 0x0C, 0x0D, 0x09, 0x0B, 0x03, 0x07, 0x06};

	if(beats != 0)
	{
		if(beats > 0)
		{
			index++;
			index = index & 0x07;
			beats--;
		}
		else
		{
			index--;
			index = index & 0x07;
			beats++;
		}
		tmp = P1;
		tmp = tmp & 0xF0;
		tmp = tmp | BeatCode[index];			 
		P1 = tmp;
	}
	else
	{
		P1 = P1 | 0x0F;
	}
}
/* T0中断服务函数，用于数码管显示扫描与按键扫描 */
void InterruptTimer0() interrupt 1
{
	static bit div = 0;

	TH0 = 0xFC;
	TL0 = 0x67;

	KeyScan();

	div = ~ div;
	if(div == 1)
	{
		TurnMotor();
	}


}