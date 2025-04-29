#include "relay.h"
#include "hc573.h"


unsigned char temp = 0x00;
unsigned char temp_old = 0xff;

/**
 * @brief 控制继电器的开关状态
 *
 * 该函数根据传入的 enable 参数控制继电器的打开或关闭。
 * 通过位操作设置继电器状态，并仅在状态改变时更新硬件。
 *
 * @param enable 控制开关，1 表示打开继电器，0 表示关闭继电器
 */

void relay_ctrl(bit enable)
{
    if(enable) {
        temp |= 0x10;    //第4位，打开继电器
    }
	else{
        temp &= ~(0x10);
    } 
	
    if(temp != temp_old) {
		P0 = temp;
        //调用函数更符合软件开发规范，但是51单片机资源有限，调用函数会增加开销，这里直接对P2进行位操作
		//SelectHC573(CS_ULN2003);   
        P2 = P2 & 0x1f | 0xa0;
		P2 &= 0x1f;
		temp_old = temp;
	}
}