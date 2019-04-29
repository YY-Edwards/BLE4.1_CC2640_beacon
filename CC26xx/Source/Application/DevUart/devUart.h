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

/*********************************************************************
 * CONSTANTS
 */
/*********************************************************************
 * TYPEDEFS
 */
#pragma pack(1)

 #pragma pack() 
  
  
/*********************************************************************
 * MACROS
 */


   
/*********************************************************************
 * API FUNCTIONS 
 */

extern void dev_uart_init();
extern void dev_uart_wrtieTransport(unsigned char* ptr, unsigned short len);
extern void dev_uart_readTransport(unsigned char* ptr, unsigned short len);

#ifdef __cplusplus
}
#endif

#endif /* _DEV_UART_H_ */