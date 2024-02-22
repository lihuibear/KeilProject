#include<reg52.h>

sbit ADDR0 = P1^0;
sbit ADDR1 = P1^1;
sbit ADDR2 = P1^2;
sbit ADDR3 = P1^3;
sbit ENLED = P1^4;

sbit LED9 = P0^7;
sbit LED8 = P0^6;
sbit LED7 = P0^5;
sbit LED6 = P0^4;

sbit KEY1 = P2^4;
sbit KEY2 = P2^5;
sbit KEY3 = P2^6;
sbit KEY4 = P2^7;

void main() {
    bit flag9 = 1;
    bit flag8 = 1;
    bit flag7 = 1;
    bit flag6 = 1;

    ENLED = 0;
    ADDR3 = 1;
    ADDR2 = 1;
    ADDR1 = 1;
    ADDR0 = 0;
    P2 = 0xF7;

    while (1) {
        if (KEY1 != flag9) {
            if (flag9 == 0) {
                LED9 = !LED9;
            }
            flag9 = KEY1; 
        }
        if (KEY2 != flag8) {
            if (flag8 == 0) {
                LED8 = !LED8;
            }
            flag8 = KEY2; 
        }
        if (KEY3 != flag7) {
            if (flag7 == 0) {
                LED7 = !LED7;
            }
            flag7 = KEY3; 
        }
        if (KEY4 != flag6) {
            if (flag6 == 0) {
                LED6 = !LED6;
            }
            flag6 = KEY4; 
        }				
    }
}
