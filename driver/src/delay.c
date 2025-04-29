#include "delay.h"

void delay_ms(unsigned char xms) //可以自己设置延时时间,12MHZ
{
	unsigned char i,j;
	for(i=xms;i>0;i--)
	  for(j=124;j>0;j--);
}