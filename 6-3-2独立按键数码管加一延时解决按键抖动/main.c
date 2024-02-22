#include <reg52.h>

sbit ADDR0 = P1^0;
sbit ADDR1 = P1^1;
sbit ADDR2 = P1^2;
sbit ADDR3 = P1^3;
sbit ENLED = P1^4;

sbit KEY4 = P2^7;

// 数码管显示的字符编码
unsigned char code LedChar[] = {
    0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
    0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E
};

// 延时函数
void delay() {
    unsigned int i = 1000;
    while (i--);
}

void main() {
    bit backup = 1;   // 用于记录按钮按下状态的备份
    bit keybuf = 1;   // 用于记录按钮当前状态
    unsigned char cnt = 0;  // 计数器

    ENLED = 0;
    ADDR3 = 1;
    ADDR2 = 0;
    ADDR1 = 0;
    ADDR0 = 0;
    P2 = 0xF7;
    P0 = LedChar[cnt]; // 初始化数码管显示

    while (1) {
        keybuf = KEY4; // 读取按钮当前状态
        if (keybuf != backup) { // 如果按钮状态发生变化
            delay(); // 延时防抖
            if (keybuf == KEY4) { // 如果按钮仍然处于按下状态
                if (backup == 0) { // 如果之前是按下的状态
                    cnt++; // 计数器递增
                    if (cnt >= 10) {
                        cnt = 0;
                    }
                    P0 = LedChar[cnt]; // 更新数码管显示
                }
                backup = KEY4; // 更新按钮状态备份
            }
        }
    }
}
