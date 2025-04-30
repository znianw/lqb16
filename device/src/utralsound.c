#include "utralsound.h"

#include "delay.h"

sbit usTx = P1^0;
sbit usRx = P1^1;

void utralsound_init(void)
{
    unsigned char i = 0;
    for(i=0; i<8; i++) {
        usTx = 1;
        Delay12us();
        usTx = 0;
        Delay12us();
    }
}

/**
 * @brief 获取超声波测量的距离数据
 * 
 * 此函数通过控制超声波模块发射和接收信号，利用定时器计时，
 * 根据计时结果计算并返回测量得到的距离。若测量超时，则返回特定值。
 * 
 * @return unsigned int 测量得到的距离，单位为厘米（取决于计算系数），
 *                    测量超时返回 999
 */
unsigned int us_data(void)
{
    unsigned int time;
    CH = CL = 0;
    CCON = 0;
    
    EA = 0;
    utralsound_init();
    EA = 1;

    CR = 1;
    while(usRx && !CF);

    CR = 0;
    if (CF == 0) {
        time =  CH << 8 | CL;
        //return ((time / 100) * 17) / 100 +3;
        return time * 0.017;
    } else {
        CF = 0;
        return 999;
    }
}