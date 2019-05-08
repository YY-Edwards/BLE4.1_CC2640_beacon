#ifndef _BEGP_H_
#define _BEGP_H_

#ifdef __cplusplus
extern "C"
{
#endif
  
/*********************************************************************
 * INCLUDES
 */
#include "comdef.h"    
#include "customerInfoStorage.h"
#include "boundedQueue.h"
   
#include <stdint.h>   
  
 
  
/*********************************************************************
 * MACROS
 */

#define BEGP_START_BYTE 0xE9
#define BEGP_CONTROL_TYPE 0x00  
   
#define BEGP_RESPONSE_ACK 0x00    

#define BEGP_ALLOCATION_MAX_QUEUE_DEEP 7    
#define MAX_FIELDS_LEN 64
#define MAX_BEGP_LEN (MAX_FIELDS_LEN + 4)  
#define BEGP_ALLOCATION_MAX_QUEUE_SIZE MAX_BEGP_LEN  
  
/*********************************************************************
 * TYPEDEFS
 */
  
  //beacon_external_general_protocol
  //BEGP:header + fields + parity
#pragma pack(1)
    
 typedef void (*BEGP_callback_t)(void* p, uint16_t len); 
 
 typedef customerStorageBeaconInfo_t  beaconInfo_content_t; 
  
  //header
  typedef struct{
  
    unsigned char       start_byte;//fixed:0xE9
    unsigned char       lengthL;
    unsigned char       lengthH;
    //unsigned short      length; //To calculate the LENGTH take the
                                //sum of L0(FIELDS) through LN-1(FIELDS) plus the parity bytes (if parity is used).
    unsigned char       type;  //0x00:control/configuration
                               //When a control packet is received the chip returns a control packet
                               //with response fields that contain response data for some control packets
                               //or indication of errors in the control packet.
    
 //The response packet for most fields just echoes back the control field identifier followed by a 0x00
//byte to indicate that the control field was received successfully. For control fields that query for information, the response
//packet contains the Requested information (1 or more bytes depending upon the control field identifier).
    
                               //0x01:
                               //0x02:
  }begp_header_t;
  
  
  
   //field
  
  //A field consists of a field identifier followed by field data. 
  //The length of field data is dependent upon the field identifier.
  
 

#define BEGP_FIELD_IDENTIFIER_SET_BEACON 0X68    
typedef struct{
    
    unsigned char       identifier;//fixed:0x68 
    
  }begp_field_get_beacon_req_t;//identifier:0x68; 1bytes.
  
typedef struct{
    
    unsigned char       identifier;//fixed:0x68 
    beaconInfo_content_t beaconInfo_content;
//    unsigned char       dev_name[CUSTOMER_MAX_BEAON_NAME_LEN];//assic
//    unsigned short      adv_interval_ms;
//    unsigned char       dev_tx_power;
//    unsigned char       uuid[CUSTOMER_UUID_LEN];
//    unsigned short      major;
//    unsigned short      minor;
//    unsigned short      rssi_one_meter;  
    
  }begp_field_get_beacon_reply_t;//identifier:0x68; 46bytes.
  

#define BEGP_FIELD_IDENTIFIER_GET_BEACON 0X58  
  typedef struct{
    
    unsigned char       identifier;//fixed:0x58
    beaconInfo_content_t beaconInfo_content;
//    unsigned char       dev_name[CUSTOMER_MAX_BEAON_NAME_LEN];//assic
//    unsigned short      adv_interval_ms;
//    unsigned char       dev_tx_power;
//    unsigned char       uuid[CUSTOMER_UUID_LEN];
//    unsigned short      major;
//    unsigned short      minor;
//    unsigned short      rssi_one_meter;   
//    
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
  typedef union{
  
    struct {
      begp_header_t       header;
      begp_field_t        field;
      //begp_parity_t       parity;
    }base;
    
   unsigned char                      all[MAX_BEGP_LEN]; 
   
  }begp_fragment_t;
  
  
  
  
  typedef enum
{
  FIND_START_BYTE,
  LOW_LENGTH,
  HIGH_LENGTH,
  READ_DATA
}begp_parser_state_enum;
   

  
 #pragma pack() 
    
     
/*********************************************************************
 * API FUNCTIONS 
 */
  extern unsigned short assemble_begp_msg(unsigned short rx_bytes, unsigned short *bytes_assembled);
  extern bool begp_init(BEGP_callback_t cb);
  extern void BEGP_parse_task();
  
#ifdef __cplusplus
}
#endif

#endif /* _BEGP_H_ */