#include "STC15F2K60S2.H"
#include "ds18b20.h"
#include "smg.h"
#include "pcf8591.h"
#include "hc573.h"
#include "key.h"
#include "led.h"
#include "relay.h"
#include "timer.h"
#include "delay.h"
#include "utralsound.h"
#include "intrins.h"

unsigned char smgpos_index = 0;
unsigned int sys_ticks = 0;

unsigned char show_mode = 0;   /*0 env_state 1 sport_state 2 temprg_state  3 nc_state  10 距离参数*/
unsigned char light_drgree = 1; /**光照等级 */
bit distance_state_show_mode = 0;

float tempture = 0.0;
unsigned int distance = 0;    /**两次距离之差 */
unsigned int distance_pre = 0;
unsigned int distance_now = 0;


unsigned char distance_prg = 30;
unsigned char temp_prg = 20;
unsigned int realy_count = 0; /**继电器吸合次数 */

/**上电2s距离锁定变量 */
bit distancelock = 0;
unsigned int time2s = 0;

/**双按键2s变量 */
bit scan_flag = 0;
unsigned int times2000 = 0;

/**运动模式锁定变量 */
bit sport_statelock = 0;  /**0 未锁定  1 锁定 */
unsigned int time3s = 0;
bit near_flag = 0;  /**接近标志位 0不接近 1接近 */
unsigned char sport_state = 1;   /**当前运动模式 */
unsigned char sport_state_pre = 0;  /**上次运动模式 */

/**状态控制数组，下标表示对应的位置，如下标为0表示对应的0号数码管 ,默认均为关闭状态*/
/** 不要char Led_Buf[8]={1}直接初始化整个数组为1，测试发现不行 应该是C51不支持？存疑中*/
unsigned char Led_Buf[8] = {0, 0, 0,0,0,0,0,0};
unsigned char Smg_Buf[8] = {20, 20, 20, 20,20, 20, 20, 20};   /**OFF */
unsigned char Smg_Point[8] = {0, 0, 0,0,0,0,0,0};



void show_process(void);
void data_process(void);
void key_process(void);
void ledrelay_process(void);
void utralsound_process(void);

void Sys_Init(void)
{
    P0 = 0xff;
    SelectHC573(CS_LED);
    P2 &= 0x1F;  //关闭LED
    
    P0 = 0x00;   //关蜂鸣器，继电器 步进电机
    SelectHC573(CS_ULN2003);
     P2 &= 0x1F;
    
}


int main()
{
    bit first_read = 1;
    Timer1_Init();
    Sys_Init();

    tempture = read_tempture(first_read);
    first_read = 0;
    
    while(1) {
        utralsound_process();
        data_process();
        show_process();
        ledrelay_process();
        key_process();
    }
}

void utralsound_process(void)
{
    if (distancelock == 0) return;   //上电2s不采集数据

    if (sys_ticks == 0)  { /**systick的周期是1000ms，保证1s采集一次 */
        distance_now = us_data();
        near_flag = (distance_now < distance_prg);   //接近判断
        if (sport_statelock == 0) {  //未锁定
            distance = distance_now - distance_pre;
            distance_pre = distance_now;

            if (distance < 5) {
                sport_state = 1;  //静止
            }else if (distance < 10){
                sport_state = 2;  //L2 徘徊
            }else {
                sport_state = 3;  //L3 跑动
            }

            if (sport_state_pre != sport_state) {   //运动状态发生变化
                sport_statelock = 1;
            }
            sport_state_pre = sport_state;
        }
    }
}


void show_process(void)
{
    switch(show_mode) {
        case 0:  /**0 env_state */
            Smg_Buf[0] = 12;  /*c*/
            Smg_Buf[1] = ((int)tempture) /10;
            Smg_Buf[2] = ((int)tempture) %10;
            Smg_Buf[3] = 20;
            Smg_Buf[4] = 20;
            Smg_Buf[5] = 20;
            Smg_Buf[6] = 18;  /*n*/  
            Smg_Buf[7] = light_drgree;
        break;

        case 1:  /* 运动检测状态  显示超声波测量的距离 单位 cm*/
            Smg_Buf[0] = 16;  /**L */
            Smg_Buf[1] = sport_state;
            Smg_Buf[2] = 20;
            Smg_Buf[3] = 20;
            Smg_Buf[4] = 20;
            Smg_Buf[5] = distance_now / 100;
            Smg_Buf[6] = distance_now / 10 % 10;
            Smg_Buf[7] = distance_now % 10;
        break;

        case 2:  /**温度参数界面 */
            Smg_Buf[0] = 17;  /**P */
            Smg_Buf[1] = 12;  /**C */
            Smg_Buf[2] = 20;
            Smg_Buf[3] = 20;
            Smg_Buf[4] = 20;
            Smg_Buf[5] = 20;
            Smg_Buf[6] = temp_prg / 10;
            Smg_Buf[7] = temp_prg % 10;
        break;


        case 3: /**数据统计状态 */
            Smg_Buf[0] = 18; /**n */
            Smg_Buf[1] = 12; /**c */
            Smg_Buf[2] = 20;
            Smg_Buf[3] = 20;
            Smg_Buf[4] = (((realy_count / 1000) == 0) ? 20 : (realy_count / 1000));
            if (Smg_Buf[4] != 20) {  /**最高位不为0 但第二位为0 例如10xx这种 */
                Smg_Buf[5] = (((realy_count % 1000 / 100) == 0) ? 0 : (realy_count % 1000 / 100));
            }else {
                Smg_Buf[5] = (((realy_count % 1000 / 100) == 0) ? 20 : (realy_count % 1000 / 100));
            }

            if (Smg_Buf[5] != 20) {  /**20x这种情况 */
                Smg_Buf[6] = (((realy_count % 100 / 10) == 0) ? 0 : (realy_count % 100 / 10));
            }else {
                Smg_Buf[6] = (((realy_count % 100 / 10) == 0) ? 20 : (realy_count % 100 / 10));
            }
           
            if (Smg_Buf[6] != 20) {  
                Smg_Buf[7] = (((realy_count % 10 ) == 0) ? 0 : (realy_count % 10 ));
            }else {
                Smg_Buf[7] = (((realy_count % 10) == 0) ? 20 : (realy_count % 10 ));
            }
        break;

        case 10:
            Smg_Buf[0] = 17;  /**P */
            Smg_Buf[1] = 16;  /**L */
            Smg_Buf[2] = 20;
            Smg_Buf[3] = 20;
            Smg_Buf[4] = 20;
            Smg_Buf[5] = 20;
            Smg_Buf[6] = distance_prg / 10;
            Smg_Buf[7] = distance_prg % 10;
        break;
    }
}


void data_process(void)
{
    unsigned char vol_temp;

    if (sys_ticks % 200 != 0) {
        return;
    }

    #if 0
    if (sys_ticks % 100 == 0) {   /**1s内采10次超声波 */
            distance = us_data();
            vol_temp = PCF8591_Read(PCF8591_CH1);  /**读光敏电阻 51份为1v*/
        }
        if (sys_ticks % 500 == 0) {
            tempture = read_tempture(0);
    }
    #endif

    //distance = us_data();
    vol_temp = 255 - PCF8591_Read(PCF8591_CH1);  /**读光敏电阻 51份为1v*/
    tempture = read_tempture(0);
    if (vol_temp  >= 3*51) {
        light_drgree = 1;
    } else if (vol_temp >= 2*51 && vol_temp < 3*51) {
        light_drgree = 2;
    }else if (vol_temp >= 25 && vol_temp < 2*51){
        light_drgree = 3;
    }else {
        light_drgree = 4;
    }

}

void key_process(void)
{
    static unsigned char Key_Val, Key_Down, Key_Up, Key_Old;
    if(sys_ticks % 50) {  
        return; 
    }
    
    //Key_Val = Key_Matrix_Scan(0);
    Key_Val = Key_Matrix_Scan();

    /**官方提供的消抖代码 */
    Key_Down = Key_Val & (Key_Old ^ Key_Val);
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
	Key_Old = Key_Val;

    if (Key_Down == KEY_S4) { 
        if( 10 == show_mode) show_mode = 3; 
        else {
            show_mode++;
            if (4 == show_mode) show_mode = 0;
        }
    }

    if (Key_Down == KEY_S5 ) {  /**进入参数子界面 */
        if (show_mode == 2) {
            show_mode = 10; /**进入距离参数界面 */
            return;
        }
        if (show_mode == 10) {
            show_mode = 2; /**进入温度参数界面 */
        }
    }


    if (Key_Down == KEY_S8) {  //+
        if(show_mode == 2) {  //温度参数模式
            temp_prg = (temp_prg == 80) ? 80 : (temp_prg + 1); 
        }else if (show_mode == 10) { //距离参数模式
            distance_prg = (distance_prg == 80) ? 80 : (distance_prg + 5);
        }
    }

    if (Key_Down == KEY_S9) {  //-
        if(show_mode == 2) {  //温度参数模式
            temp_prg = (temp_prg == 20) ? 20 : (temp_prg - 1); 
        }else if (show_mode == 10) { //距离参数模式
            distance_prg = (distance_prg == 20) ? 20 : (distance_prg - 5);
        }
    }

    if (Key_Down == 89 && show_mode == 3) {
		scan_flag = 1;
    }
    if (times2000 == 2001) {
        realy_count = 0;
        scan_flag = 0;
        times2000 = 0;
    }
    if(Key_Up == 89 ) {
        scan_flag = 0;
        times2000 = 0;
    }
    
}

/**
 * @brief 根据显示模式处理 LED 和继电器的状态
 *
 * 该函数根据全局变量 show_mode 的值，对不同的显示模式进行不同的 LED 和继电器控制操作。
 */
void ledrelay_process(void)
{
    static bit relay_flag = 1; /**继电器吸合标志位 */
    if (near_flag) {  
        switch(light_drgree) {  //接近才判断
            case 1:
                Led_Buf[0] = LED_ON;
                Led_Buf[1] = LED_OFF;
                Led_Buf[2] = LED_OFF;
                Led_Buf[3] = LED_OFF;
            break;

            case 2:
                Led_Buf[0] = LED_ON;
                Led_Buf[1] = LED_ON;
                Led_Buf[2] = LED_OFF;
                Led_Buf[3] = LED_OFF;
            break;

            case 3:
                Led_Buf[0] = LED_ON;
                Led_Buf[1] = LED_ON;
                Led_Buf[2] = LED_ON;
                Led_Buf[3] = LED_OFF;
            break;

            case 4:
                Led_Buf[0] = LED_ON;
                Led_Buf[1] = LED_ON;
                Led_Buf[2] = LED_ON;
                Led_Buf[3] = LED_ON; 
            break;
        }
    }else {
        Led_Buf[0] = Led_Buf[1] = Led_Buf[2]= Led_Buf[3] = 0;
    }

    if (sport_state == 1) {
        Led_Buf[7] = LED_OFF;
    }else if (sport_state == 2){
        Led_Buf[7] = LED_ON;
    }else {
        if (sys_ticks % 100 == 0) {
            Led_Buf[7] = !Led_Buf[7];
        }
    }
    
    /**继电器状态判定 */ 
    if ((near_flag) && tempture > temp_prg) {
        relay_ctrl(1);
        if (1 == relay_flag) {/**如果继电器本来已经吸合了，不用再加次数 */
            realy_count++;
            relay_flag = 0;
        }
    }else {
        relay_ctrl(0);
        relay_flag = 1;
    }
}


/**
 * @brief 定时器1中断服务函数
 *
 * 该函数在定时器1产生中断时被调用，用于处理系统的定时任务，
 * 包括更新系统时钟滴答、数码管显示、LED显示和继电器控制等。
 *
 * @param 无
 * @return 无
 */
void Timer1_Handler(void) interrupt 3
{
    sys_ticks=(++sys_ticks) % 1000;

    if (scan_flag) {    //双键被按下
        if(++times2000 >= 2000)
            times2000 = 2001;
    }

    if (distancelock == 0) {  //上电锁定2s
        if(++time2s == 2000) {
            time2s = 0;
            distancelock = 1;
        }
    }

    if (sport_statelock) { //运动模式3s锁定
        if ((++time3s)  == 3000) {
            time3s = 0;
            sport_statelock = 0;
        }
    }
    smgpos_index = (++smgpos_index) % 8;
    smg_display(smgpos_index, Smg_Buf[smgpos_index], Smg_Point[smgpos_index]);
    led_display(smgpos_index, Led_Buf[smgpos_index]);


}
