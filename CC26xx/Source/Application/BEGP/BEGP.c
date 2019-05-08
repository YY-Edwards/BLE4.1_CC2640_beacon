

#include "BEGP.h"


static     char                    begp_RxBuf[2*MAX_BEGP_LEN]={0}; 
static     begp_fragment_t         g_begp_fragment;
static     begp_parser_state_enum  begp_parser_state;
static     unsigned short          rx_begp_msg_len = 0;    
static     unsigned short          rx_begp_msg_idx = 0; 
volatile   RingQueue_t             rx_begp_queue =NULL;
BEGP_callback_t                    BEGP_sendRspCB =NULL;

static void process_BEGP_msg();
static void BEGP_onGetBeaconReq(unsigned char   identifier);
static void BEGP_onSetBeaconReq(unsigned char   identifier, void* data);
//static void BEGP_onUnknownReq();
//static void BEGP_onUnsupportTypeMsg();
static void BEGP_sendControlResponse(begp_fragment_t* begp_fragment_ptr, unsigned char field_len);


unsigned short assemble_begp_msg(unsigned short rx_bytes, unsigned short *bytes_assembled)
{
    int index;
    uint8_t ch;
    int whole_message_count = 0;//完整msg的数目
    int bytecount = 0;
    
    *bytes_assembled = 0;
    index = 0;
    while (0 < rx_bytes--){
            bytecount++;
            ch = begp_RxBuf[index++];
            switch (begp_parser_state)	//Simple state machine to get generic local BEGP msg
            {
            case FIND_START_BYTE:		//search for sync byte
                    if (BEGP_START_BYTE == ch){
                            g_begp_fragment.base.header.start_byte = ch;
                            begp_parser_state = LOW_LENGTH;	//get low byte of length
                    }
                    break;
            case LOW_LENGTH:	//here for low byte of length
                    g_begp_fragment.base.header.lengthL = ch;
                    rx_begp_msg_len = ch;
                    begp_parser_state = HIGH_LENGTH;	//get high byte of length
                    break;
            case HIGH_LENGTH:	//here for high byte of length
                    g_begp_fragment.base.header.lengthH= ch;
                    rx_begp_msg_len += (ch<<8);

                    if (sizeof(begp_fragment_t) < rx_begp_msg_len){
                            rx_begp_msg_len = sizeof(begp_fragment_t);//68bytes
                    }
                    rx_begp_msg_len += 1;//length remaining.
                    rx_begp_msg_idx = 3;
                    begp_parser_state = READ_DATA;	//get rest of data
                    break;


            case READ_DATA:	//try to read the rest of the message or to end of buffer
                    g_begp_fragment.all[rx_begp_msg_idx++] = ch;
                    if ((--rx_begp_msg_len) == 0){
                            rx_begp_msg_len = ((g_begp_fragment.base.header.lengthH) << 8) + (g_begp_fragment.base.header.lengthL) + 4; //Total length.
                            begp_parser_state = FIND_START_BYTE;	//go back to first stage
                            //call local begp message parsing routine
                            process_BEGP_msg();
                            *bytes_assembled += bytecount;
                            bytecount = 0;
                            whole_message_count++;
                    }
                    break;
            default:
                    begp_parser_state = FIND_START_BYTE;	//reset parser to first stage
                    break;
            } //end switch statement
    }
    return whole_message_count;
}



void BEGP_parse_task()
{

     while (!queue_is_empty(rx_begp_queue))
    {
      int recv_len =0;
      unsigned short bytes_consumed = 0;
      
      take_from_queue(rx_begp_queue, &begp_RxBuf[0], &recv_len, true);
      
      assemble_begp_msg(recv_len, &bytes_consumed);
    }
}



static void BEGP_sendControlResponse(begp_fragment_t* begp_fragment_ptr, unsigned char field_len)
{

  if(begp_fragment_ptr->base.header.start_byte == BEGP_DEFAULT_VALUE)
  {
    begp_fragment_ptr->base.header.start_byte = BEGP_START_BYTE;
  }
  if(begp_fragment_ptr->base.header.type == BEGP_DEFAULT_VALUE)
  {
    begp_fragment_ptr->base.header.type = BEGP_CONTROL_TYPE;
  }
  
  begp_fragment_ptr->base.header.lengthL = (unsigned char)(field_len & 0x00ff);
  begp_fragment_ptr->base.header.lengthH = (unsigned char)(field_len>>8);
  
  if(BEGP_sendRspCB)
  { 
     BEGP_sendRspCB(begp_fragment_ptr->all, field_len + sizeof(begp_header_t));
  } 
  
}



static void BEGP_onGetBeaconReq(unsigned char  recv_id)
{
  beaconInfo_content_t beaconAllInfo;
  memset(&beaconAllInfo, 0x00, sizeof(beaconInfo_content_t));
  get_customerInfo(&beaconAllInfo);//获取beacon的完整信息
  
  int field_len =0;
  uint16_t u16_temp= 0;
  
  begp_fragment_t         begp_reply_fragment;
  begp_reply_fragment.base.header.start_byte    = BEGP_DEFAULT_VALUE;
  begp_reply_fragment.base.header.lengthL       = BEGP_DEFAULT_VALUE;
  begp_reply_fragment.base.header.lengthH       = BEGP_DEFAULT_VALUE;
  begp_reply_fragment.base.header.type          = BEGP_DEFAULT_VALUE;
  
  begp_reply_fragment.base.field.begp_general_filed.identifier = recv_id;
  field_len += sizeof(begp_reply_fragment.base.field.begp_general_filed.identifier);
  switch(recv_id)
  {
    case BEGP_FIELD_IDENTIFIER_GET_BEACON_ALL://get beacon all info
      memcpy(begp_reply_fragment.base.field.begp_general_filed.data, &beaconAllInfo,  sizeof(beaconInfo_content_t));
      field_len += sizeof(beaconInfo_content_t);
      break;
    case BEGP_FIELD_IDENTIFIER_GET_BEACON_NAME:   
      memcpy(begp_reply_fragment.base.field.begp_general_filed.data, beaconAllInfo.dev_name,  sizeof(beaconAllInfo.dev_name));
      field_len += sizeof(beaconAllInfo.dev_name);
      break;
    case BEGP_FIELD_IDENTIFIER_GET_BEACON_ADV_INTERVAL:
      u16_temp = beaconAllInfo.adv_interval_ms;
      memcpy(begp_reply_fragment.base.field.begp_general_filed.data, &u16_temp,  sizeof(beaconAllInfo.adv_interval_ms));
      field_len += sizeof(beaconAllInfo.adv_interval_ms);
      break;
    case BEGP_FIELD_IDENTIFIER_GET_BEACON_TX_POWER:  
      memcpy(begp_reply_fragment.base.field.begp_general_filed.data, &(beaconAllInfo.dev_tx_power),  sizeof(beaconAllInfo.dev_tx_power));
      field_len += sizeof(beaconAllInfo.dev_tx_power);
      break;
    case BEGP_FIELD_IDENTIFIER_GET_BEACON_UUID:  
      memcpy(begp_reply_fragment.base.field.begp_general_filed.data, beaconAllInfo.uuid,  sizeof(beaconAllInfo.uuid));
      field_len +=  sizeof(beaconAllInfo.uuid);
      break;
    case BEGP_FIELD_IDENTIFIER_GET_BEACON_MAJOR:
      u16_temp = beaconAllInfo.major;
      memcpy(begp_reply_fragment.base.field.begp_general_filed.data, &u16_temp,  sizeof(beaconAllInfo.major));
      field_len += sizeof(beaconAllInfo.major);
      break;
    case BEGP_FIELD_IDENTIFIER_GET_BEACON_MINOR:
      u16_temp = beaconAllInfo.minor;
      memcpy(begp_reply_fragment.base.field.begp_general_filed.data, &u16_temp,  sizeof(beaconAllInfo.minor));
      field_len += sizeof(beaconAllInfo.minor);
      break;
    case BEGP_FIELD_IDENTIFIER_GET_BEACON_RSSI: 
      u16_temp = beaconAllInfo.rssi_one_meter;
      memcpy(begp_reply_fragment.base.field.begp_general_filed.data, &u16_temp,  sizeof(beaconAllInfo.rssi_one_meter));
      field_len += sizeof(beaconAllInfo.rssi_one_meter);
      break;
  default:
      field_len = 0;
      break;
  }
  
  if(field_len!=0){
    
    BEGP_sendControlResponse(&begp_reply_fragment, field_len);
  }

}

static void BEGP_onSetBeaconReq(unsigned char  recv_id, void* data)
{
  
  beaconInfo_content_t beaconAllInfo;
  memset(&beaconAllInfo, 0x00, sizeof(beaconInfo_content_t));
  get_customerInfo(&beaconAllInfo);//获取beacon的完整信息

  
  switch(recv_id)
  {
    case BEGP_FIELD_IDENTIFIER_SET_BEACON_ALL://get beacon all info
      memcpy(&beaconAllInfo, data,  sizeof(beaconInfo_content_t));
      break;
    case BEGP_FIELD_IDENTIFIER_SET_BEACON_NAME: 
      memcpy(beaconAllInfo.dev_name, data,  sizeof(beaconAllInfo.dev_name));
      break;
    case BEGP_FIELD_IDENTIFIER_SET_BEACON_ADV_INTERVAL:
      beaconAllInfo.adv_interval_ms = *((uint16_t*)(data));
      break;
    case BEGP_FIELD_IDENTIFIER_SET_BEACON_TX_POWER:  
      beaconAllInfo.dev_tx_power = *((uint8_t*)(data));
      break;
    case BEGP_FIELD_IDENTIFIER_SET_BEACON_UUID:  
      memcpy(beaconAllInfo.uuid, data,  sizeof(beaconAllInfo.uuid));
      break;
    case BEGP_FIELD_IDENTIFIER_SET_BEACON_MAJOR:  
      beaconAllInfo.major = *((uint16_t*)(data));
      break;
    case BEGP_FIELD_IDENTIFIER_SET_BEACON_MINOR:  
      beaconAllInfo.minor = *((uint16_t*)(data));
      break;
    case BEGP_FIELD_IDENTIFIER_SET_BEACON_RSSI:  
      beaconAllInfo.rssi_one_meter = *((uint16_t*)(data));
      break;
  default:
      break;
  }
  
  set_customerInfo(&beaconAllInfo);
  
  
  int field_len =0;
  
  begp_fragment_t         begp_reply_fragment;
  begp_reply_fragment.base.header.start_byte    = BEGP_DEFAULT_VALUE;
  begp_reply_fragment.base.header.lengthL       = BEGP_DEFAULT_VALUE;
  begp_reply_fragment.base.header.lengthH       = BEGP_DEFAULT_VALUE;
  begp_reply_fragment.base.header.type          = BEGP_DEFAULT_VALUE;
  
  begp_reply_fragment.base.field.begp_field_set_beacon_reply.identifier = recv_id;
  field_len += sizeof(begp_reply_fragment.base.field.begp_field_set_beacon_reply.identifier);
  begp_reply_fragment.base.field.begp_field_set_beacon_reply.rsp = BEGP_RESPONSE_ACK;
  field_len += sizeof(begp_reply_fragment.base.field.begp_field_set_beacon_reply.rsp);
  
  BEGP_sendControlResponse(&begp_reply_fragment, field_len);
}



void process_BEGP_msg()
{
  if(BEGP_CONTROL_TYPE == g_begp_fragment.base.header.type)
  {
    char id= g_begp_fragment.base.field.begp_general_filed.identifier;
    switch(id & BEGP_FIELD_FILTER_IDENTIFIER)
    {
    case BEGP_FIELD_IDENTIFIER_GET_BEACON://get beacon                  
      BEGP_onGetBeaconReq(id);
      
      break;
    case BEGP_FIELD_IDENTIFIER_SET_BEACON://set beacon     
      BEGP_onSetBeaconReq(id,
                          g_begp_fragment.base.field.begp_general_filed.data);
      
      break;
      
    default:
      break;  
    }
  }
}

bool begp_init(BEGP_callback_t cb)
{
  bool ret= false;
  begp_parser_state = FIND_START_BYTE;
  rx_begp_msg_len = 0;
  rx_begp_msg_idx =0;
  VOID memset(&g_begp_fragment, 0X00, sizeof(g_begp_fragment));
  VOID memset(&begp_RxBuf, 0X00, sizeof(begp_RxBuf));
  
  BEGP_sendRspCB = cb;
  rx_begp_queue = create_queue(BEGP_ALLOCATION_MAX_QUEUE_DEEP, BEGP_ALLOCATION_MAX_QUEUE_SIZE);
  if(rx_begp_queue)
  {
    ret = true;
  }
  
  return ret;
}