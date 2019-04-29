/*
* Filename:	devUart.c
* Created:	2019/04/28
* Author:	EDWARDS
* Describe:	This file contains the cc2640 uart operation define.
* Version:	v1.0
* History:
*/
#include <ti/sysbios/knl/Task.h>  
#include <ti/sysbios/knl/Clock.h>  
#include <ti/sysbios/knl/Semaphore.h> 
#include <ti/sysbios/knl/Queue.h>    

#include <string.h>  
#include <stdio.h>  
#include "hci_tl.h"  

#include "../../npi/inc/npi_tl_uart.h"  
#include "devUart.h"



static unsigned char uart_RxBuf[256];  
static unsigned char uart_TxBuf[256];  
static void dev_uart_callback(unsigned short rxLen, unsigned short txLen);

static void dev_uart_callback(unsigned short rxLen, unsigned short txLen)
{
  //����н��յ����ݣ�����
  if(rxLen > 0)
  { 
    //�����յ������ݷ���ȥ
    memcpy(uart_TxBuf, uart_RxBuf, rxLen);
    NPITLUART_writeTransport(rxLen);  
  }
}



 void dev_uart_wrtieTransport(unsigned char* ptr, unsigned short len)
 {
    memset(uart_TxBuf, 0, sizeof(uart_TxBuf));
    memcpy(uart_TxBuf, ptr, len);
    NPITLUART_writeTransport(len);  
 }
 void dev_uart_readTransport(unsigned char* ptr, unsigned short len)
 {
 
 
 
 }

void dev_uart_init(void)
{
   //��ʼ��  
  NPITLUART_initializeTransport(uart_RxBuf, uart_TxBuf, dev_uart_callback);

}