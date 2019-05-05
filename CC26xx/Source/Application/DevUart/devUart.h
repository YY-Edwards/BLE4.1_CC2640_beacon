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
  
#define MAX_FIELDS_LEN 64
  
/*********************************************************************
 * TYPEDEFS
 */
  
  //beacon_external_general_protocol
  //BEGP:header + fields + parity
#pragma pack(1)
  
  //header
  typedef struct{
  
    unsigned char       start_byte;//fixed:0xE9
    unsigned short      length; //To calculate the LENGTH take the
                                //sum of L0(FIELDS) through LN-1(FIELDS) plus the parity bytes (if parity is used).
    unsigned char       type;  //0x00:control/configuration
                               //When a control packet is received the chip returns a control packet
                               //with response fields that contain response data for some control packets
                               //or indication of errors in the control packet.
    
                               //0x01:
                               //0x02:
  }begp_header_t;
  
  
  
   //field
  
  //A field consists of a field identifier followed by field data. 
  //The length of field data is dependent upon the field identifier.
  
  
typedef struct{
    
    unsigned char       identifier;//fixed:0x68 
    
  }begp_field_get_beacon_req_t;//identifier:0x68; 1bytes.
  
typedef struct{
    
    unsigned char       identifier;//fixed:0x68 
    unsigned char       dev_name[20];//assic
    unsigned short      adv_interval_ms;
    unsigned char       dev_tx_power;
    unsigned char       uuid[16];
    unsigned short      major;
    unsigned short      minor;
    unsigned short      rssi_one_meter;  
    
  }begp_field_get_beacon_reply_t;//identifier:0x68; 46bytes.
  
  typedef struct{
    
    unsigned char       identifier;//fixed:0x58
    unsigned char       dev_name[20];//assic
    unsigned short      adv_interval_ms;
    unsigned char       dev_tx_power;
    unsigned char       uuid[16];
    unsigned short      major;
    unsigned short      minor;
    unsigned short      rssi_one_meter;   
    
  }begp_field_set_beacon_req_t;//identifier:0x58; 46bytes.
  
    typedef struct{
    
    unsigned char       identifier;//fixed:0x58
    unsigned char       rsp;//0x00(anything different indicates error)
    
  }begp_field_set_beacon_reply_t;//identifier:0x58; 2bytes.
  
  
  
  typedef struct{
    
     unsigned char       identifier;  
     unsigned char       data[MAX_FIELDS_LEN-1]; 
  
  }begp_general_filed_t;
  
  
   typedef union{
    
     begp_field_get_beacon_req_t        begp_field_get_beacon_req;
     begp_field_get_beacon_reply_t      begp_field_get_beacon_reply;
     
     begp_field_set_beacon_req_t        begp_field_set_beacon_req;
     begp_field_set_beacon_reply_t      begp_field_set_beacon_reply;
     
     begp_general_filed_t               begp_general_filed;
     unsigned char                      element[MAX_FIELDS_LEN]; 
  
  }begp_field_t;
  
  
     //partiy
  typedef struct{
  
    unsigned char       pkt_parity;//fixed:0x2f  
    unsigned char       pkt_byte;  //The second byte of the parity field is the
                                   //parity byte. It is obtained by ¡°Exclusive-oring¡± every byte in the packet, except for the START_BYTE and the
                                   //PARITY_BYTE, together. 
 
  }begp_parity_t;
  
  
 //A control packet must contain one or more control fields 
  typedef struct{
  
    begp_header_t       header;
    begp_field_t        field;
    //begp_parity_t       parity;
  
  }begp_fragment_t;
  
  
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