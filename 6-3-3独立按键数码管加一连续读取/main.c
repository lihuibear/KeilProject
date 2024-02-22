#include <reg52.h>

sbit ADDR0 = P1^0;
sbit ADDR1 = P1^1;
sbit ADDR2 = P1^2;
sbit ADDR3 = P1^3;
sbit ENLED = P1^4;

sbit KEY4 = P2^7;

unsigned char code LedChar[] = {
    0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
    0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E
};

bit KeySta = 1;  // 按键状态标志

void main() {
    bit backup = 1;   // 用于记录按钮按下状态的备份
    unsigned char cnt = 0;  // 计数器

    EA = 1;  // 允许全局中断
    ENLED = 0;
    ADDR3 = 1;
    ADDR2 = 0;
    ADDR1 = 0;
    ADDR0 = 0;
    TMOD = 0x01;  // 设置定时器0为模式1（16位定时器）
    TH0 = 0xF8;   // 初始化定时器高8位
    TL0 = 0xCD;   // 初始化定时器低8位
    ET0 = 1;      // 允许定时器0中断
    TR0 = 1;      // 启动定时器0
    P2 = 0xF7;    // 设置P2口初始值
    P0 = LedChar[cnt];  // 初始化数码管显示

    while (1) {
        if (KeySta != backup) {
            if (backup == 0) {
                cnt++;
                if (cnt >= 10) {
                    cnt = 0;
                }
                P0 = LedChar[cnt];
            }
            backup = KeySta;  // 更新按钮状态备份
        }
    }
}

void InterruptTimer0() interrupt 1 {
    static unsigned char keybuf = 0xFF;

    TH0 = 0xF8;  // 重新加载定时器高8位
    TL0 = 0xCD;  // 重新加载定时器低8位

    keybuf = (keybuf << 1) | KEY4;	 
    if (keybuf == 0x00) {
        KeySta = 0;  // 检测到按键按下
    } else if (keybuf == 0xFF) {
        KeySta = 1;  // 检测到按键释放
    } else {
        // 按键状态未变化
    }
}
