/*
* Filename:	devUart.h
* Created:	2019/04/28
* Author:	EDWARDS
* Describe:	This file contains the cc2640 uart operation and prototypes.
* Version:	v1.0
* History:
*/




#ifndef _DEV_UART_H_
#define _DEV_UART_H_

#ifdef __cplusplus
extern "C"
{
#endif


/*********************************************************************
 * INCLUDES
 */

  
#include "BEGP.h"
  
/*********************************************************************
 * CONSTANTS
 */

  
/*********************************************************************
 * MACROS
 */
  
/*********************************************************************
 * TYPEDEFS
 */
//Typedef for call back function mechanism to notify SimpleBLEPeripheral_taskFxn()  that
//an UART transaction has occurred
 typedef void (*devUart_postSemCB_t)(void); 
   
/*********************************************************************
 * API FUNCTIONS 
 */

extern void dev_uart_init(devUart_postSemCB_t cb);
//extern void dev_uart_init();
extern void dev_uart_wrtieTransport(void* ptr, uint16_t len);
extern void dev_uart_readTransport(unsigned char* ptr, unsigned short len);


#ifdef __cplusplus
}
#endif

#endif /* _DEV_UART_H_ */