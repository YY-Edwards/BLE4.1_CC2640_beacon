/*
* Filename:	user_custom.h
* Created:	2018/08/15
* Author:	xxx
* Describe:	user custom definitions
* Version:	v1.0
* History:
*/

#ifndef USERCUSTOM_H
#define USERCUSTOM_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
 
 #include "hci.h"

/*********************************************************************
 * CONSTANTS
 */
//1.设置发射功率
#define SETTING_BEACON_TX_POWER            HCI_EXT_TX_POWER_5_DBM //默认5dbm.
/*

可设定值范围如下：
 HCI_EXT_TX_POWER_MINUS_21_DBM      //-21dbm            
 HCI_EXT_TX_POWER_MINUS_18_DBM                  
 HCI_EXT_TX_POWER_MINUS_15_DBM                  
 HCI_EXT_TX_POWER_MINUS_12_DBM                  
 HCI_EXT_TX_POWER_MINUS_9_DBM         
 HCI_EXT_TX_POWER_MINUS_6_DBM                   
 HCI_EXT_TX_POWER_MINUS_3_DBM                 
 HCI_EXT_TX_POWER_0_DBM                       
 HCI_EXT_TX_POWER_1_DBM                        
 HCI_EXT_TX_POWER_2_DBM                         
 HCI_EXT_TX_POWER_3_DBM                        
 HCI_EXT_TX_POWER_4_DBM                       
 HCI_EXT_TX_POWER_5_DBM              //5dbm          

*/
    
//2.设置UUID
//根据手台设定的实际UUID来设置。
#define SETTING_BEACON_UUID      0x50, 0xdc, 0xb6, 0xf6, 0x91, 0x5a, 0x41, 0x42, \
                                  0xa6, 0xfe, 0xfd, 0xa7, 0xb4, 0x41, 0x86, 0x09    


//3.设置主修
#define SETTING_BEACON_MAJOR            27 //默认值十进制的27.
/*
可设定值范围0~65535
*/                                    

                                    
//4.设置辅修
#define SETTING_BEACON_MINOR            42568 //默认值为十进制的42568.
/*
可设定值范围0~65535
*/                                       

//5.设置广播间隔时间
#define SETTING_BEACON_ADV_INTERVAL      851 //单位毫秒，默认值851ms
/*
可设定值范围20ms~30.72s

*/


//6.设置设备名称
#define SETTING_BEACON_NAME			"loveyou" //默认名称jhbeacon
/*
可设字符最大8个有效字符
*/


//6.设置设备在1m处的接收的信号强度值
#define SETTING_BEACON_RSSI		197 //默认值为十进制的197
/*
取值范围：0~255
2的补码256-197=-59dBm，因此在APP上换算后显示的值为-59dbm
*/



/*********************************************************************
 * TYPEDEFS
 */

  
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEGATTPROFILE_H */
