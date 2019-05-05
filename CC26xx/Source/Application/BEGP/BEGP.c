

#include "BEGP.h"


static     char                    begp_RxBuf[2*MAX_BEGP_LEN]={0}; 
static     begp_fragment_t         begp_fragment;
static     begp_parser_state_enum  begp_parser_state;
static     unsigned short          rx_begp_msg_len = 0;    
static     unsigned short          rx_begp_msg_idx = 0; 
volatile   RingQueue_t             rx_begp_queue =NULL;
BEGP_callback_t                    BEGP_sendRspCB =NULL;

static void process_BEGP_msg();
static void BEGP_onGetBeaconReq();
static void BEGP_onSetBeaconReq(beaconInfo_content_t* p);
//static void BEGP_onUnknownReq();
//static void BEGP_onUnsupportTypeMsg();
static void BEGP_sendControlResponse(unsigned char rx_identifier,  void* data, unsigned char len);


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
                            begp_fragment.base.header.start_byte = ch;
                            begp_parser_state = LOW_LENGTH;	//get low byte of length
                    }
                    break;
            case LOW_LENGTH:	//here for low byte of length
                    begp_fragment.base.header.lengthL = ch;
                    rx_begp_msg_len = ch;
                    begp_parser_state = HIGH_LENGTH;	//get high byte of length
                    break;
            case HIGH_LENGTH:	//here for high byte of length
                    begp_fragment.base.header.lengthH= ch;
                    rx_begp_msg_len += (ch<<8);

                    if (sizeof(begp_fragment_t) < rx_begp_msg_len){
                            rx_begp_msg_len = sizeof(begp_fragment_t);//68bytes
                    }
                    rx_begp_msg_len += 1;//length remaining.
                    rx_begp_msg_idx = 3;
                    begp_parser_state = READ_DATA;	//get rest of data
                    break;


            case READ_DATA:	//try to read the rest of the message or to end of buffer
                    begp_fragment.all[rx_begp_msg_idx++] = ch;
                    if ((--rx_begp_msg_len) == 0){
                            rx_begp_msg_len = ((begp_fragment.base.header.lengthH) << 8) + (begp_fragment.base.header.lengthL) + 4; //Total length.
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



static void BEGP_sendControlResponse(unsigned char rx_identifier,  void* data, unsigned char len)
{
  unsigned short field_size= 0;
  begp_fragment_t         begp_reply_fragment;
  begp_reply_fragment.base.header.start_byte    = BEGP_START_BYTE;
  begp_reply_fragment.base.header.type          = BEGP_CONTROL_TYPE;
  
  begp_reply_fragment.base.field.begp_general_filed.identifier = rx_identifier;
  field_size++;
  VOID memcpy(&(begp_reply_fragment.base.field.begp_general_filed.data), data, len);
  field_size += len;
  
  begp_reply_fragment.base.header.lengthL = (unsigned char)(field_size & 0x00ff);
  begp_reply_fragment.base.header.lengthH = (unsigned char)(field_size>>8);
  
  if(BEGP_sendRspCB)
  { 
     BEGP_sendRspCB(begp_reply_fragment.all, field_size + sizeof(begp_header_t));
  } 
  
}



static void BEGP_onGetBeaconReq()
{
  begp_field_get_beacon_reply_t get_beacon_reply;
  memset(&get_beacon_reply, 0x00, sizeof(begp_field_get_beacon_reply_t));
  
  get_beacon_reply.identifier = BEGP_FIELD_IDENTIFIER_GET_BEACON;
  get_customerInfo(&(get_beacon_reply.beaconInfo_content));
  
  BEGP_sendControlResponse(get_beacon_reply.identifier,
                  &(get_beacon_reply.beaconInfo_content),
                  sizeof(beaconInfo_content_t)
                  );

}

static void BEGP_onSetBeaconReq(beaconInfo_content_t* p)
{
  set_customerInfo(p);
  begp_field_set_beacon_reply_t set_beacon_reply;
  set_beacon_reply.identifier   = BEGP_FIELD_IDENTIFIER_SET_BEACON;
  set_beacon_reply.rsp          = BEGP_RESPONSE_ACK;
  BEGP_sendControlResponse(set_beacon_reply.identifier,
                    &(set_beacon_reply.rsp),
                    sizeof(set_beacon_reply.rsp)
                    );
}



void process_BEGP_msg()
{
  if(BEGP_CONTROL_TYPE == begp_fragment.base.header.type)
  {
    char id= begp_fragment.base.field.begp_general_filed.identifier;
    switch(id)
    {
    case BEGP_FIELD_IDENTIFIER_GET_BEACON://get beacon info
      
      BEGP_onGetBeaconReq();
      
      break;
    case BEGP_FIELD_IDENTIFIER_SET_BEACON://set beacon info
      
      BEGP_onSetBeaconReq((beaconInfo_content_t*)(begp_fragment.base.field.begp_general_filed.data));
      
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
  VOID memset(&begp_fragment, 0X00, sizeof(begp_fragment));
  VOID memset(&begp_RxBuf, 0X00, sizeof(begp_RxBuf));
  
  BEGP_sendRspCB = cb;
  rx_begp_queue = create_queue(3, MAX_BEGP_LEN);
  if(rx_begp_queue)
  {
    ret = true;
  }
  
  return ret;
}