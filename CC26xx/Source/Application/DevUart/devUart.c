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
#include "boundedQueue.h"



static  char uart_RxBuf[128];  
static  char uart_TxBuf[128];  
static  void dev_uart_callback(unsigned short rxLen, unsigned short txLen);
extern RingQueue_t  rx_begp_queue;


static void dev_uart_callback(unsigned short rxLen, unsigned short txLen)
{
  //����н��յ����ݣ�����
  if(rxLen > 0)
  { 
    //�����յ������ݷ���ȥ
    //memcpy(uart_TxBuf, uart_RxBuf, rxLen);
    //NPITLUART_writeTransport(rxLen);  
    push_to_queue(rx_begp_queue, uart_RxBuf, rxLen);
  }
  else if(txLen > 0)//���ͳɹ�
  {
  
  }
    
}


 void dev_uart_wrtieTransport(void* ptr, uint16_t len)
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
   
  if(begp_init(dev_uart_wrtieTransport))
  {
    //��ʼ��  
    NPITLUART_initializeTransport(uart_RxBuf, uart_TxBuf, dev_uart_callback);
  }
  

}