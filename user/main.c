#include "STC15F2K60S2.H"
#include "ds18b20.h"
#include "smg.h"
#include "pcf8591.h"
#include "hc573.h"
#include "key.h"
#include "led.h"
#include "relay.h"
#include "timer.h"

unsigned char smgpos_index = 0;
unsigned int sys_ticks = 0;

unsigned char show_mode = 0;   /*0 env_state 1 sport_state 2 temprg_state  3 nc_state  */
bit distance_state_show_mode = 0;

float tempture = 0.0;
unsigned int distance = 0;


unsigned char distance_prg = 30;
unsigned char temp_prg = 30;
unsigned int realy_count = 0; /**继电器吸合次数 */


/**状态控制数组，下标表示对应的位置，如下标为0表示对应的0号数码管 ,默认均为关闭状态*/
/** 不要char Led_Buf[8]={1}直接初始化整个数组为1，测试发现不行 应该是C51不支持？存疑中*/
unsigned char Led_Buf[8] = {0, 0, 0,0,0,0,0,0};
unsigned char Smg_Buf[8] = {20, 20, 20, 20,20, 20, 20, 20};   /**OFF */
unsigned char Smg_Point[8] = {0, 0, 0,0,0,0,0,0};



void show_process(void);
void data_process(void);
void key_process(void);
void ledrelay_process(void);

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
    unsigned char i;

    while(1) {
        tempture = read_tempture();
        show_process();
        for (i=0; i<8; i++) {
            smg_display(i, Smg_Buf[i], 0);
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
            Smg_Buf[6] = 18;  /*n*/
            
        break;

        case 1:  /* 运动状态 */
            Smg_Buf[0] = 16;  /**L */
            Smg_Buf[1] = 1;
            Smg_Buf[5] = distance / 100;
            Smg_Buf[6] = distance % 10 / 10;
            Smg_Buf[7] = distance % 10;
        break;

        case 2:
        break;


        case 3: /**数据统计状态 */
            Smg_Buf[0] = 18; /**n */
            Smg_Buf[1] = 12; /**c */
            Smg_Buf[4] = (((realy_count / 1000) == 0) ? 20 : (realy_count / 1000));
            Smg_Buf[5] = (((realy_count % 1000 / 100) == 0) ? 20 : (realy_count % 1000 / 100));
            Smg_Buf[6] = 0;
            Smg_Buf[7] = 0;
        break;

    }
}


void data_process(void)
{
    

}

void key_process(void)
{
    static unsigned char Key_Val, Key_Down, Key_Up, Key_Old;
    if(sys_ticks % 10) {  //键盘需要频繁扫描
        return; 
    }
    
    //Key_Val = Key_Matrix_Scan(0);
    Key_Val = Key_Matrix_Scan();

    /**官方提供的消抖代码 */
    Key_Down = Key_Val & (Key_Old ^ Key_Val);
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
	Key_Old = Key_Val;

    if (Key_Down == KEY_S4) {
        show_mode = (++show_mode) % 3;
    }

    if (Key_Down == KEY_S5 ) {
        
    }


    if (Key_Down == KEY_S8) {  //-
        
    }

    if (Key_Down == KEY_S9) {  //+
        
    }
}


/**
 * @brief 根据显示模式处理 LED 和继电器的状态
 *
 * 该函数根据全局变量 show_mode 的值，对不同的显示模式进行不同的 LED 和继电器控制操作。
 */
void ledrelay_process(void)
{
   
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
    
}
