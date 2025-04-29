#ifndef _HCSR04_H_
#define _HCSR04_H_

sbit HCSR04_TX = P1^0;
sbit HCSR04_RX = P1^1;
void send_wave();

#endif