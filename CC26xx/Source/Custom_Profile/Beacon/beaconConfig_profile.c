/*
* Filename:	beaconConfig_profile.c
* Created:	2018/08/15
* Author:	EDWARDS
* Describe:	This file contains the Beacon config profile sample Beacon service 
			profile for use with the BLE sample application.
* Version:	v1.0
* History:
*/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "beaconConfig_profile.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define SERVAPP_NUM_ATTR_SUPPORTED        18


//���������Ա��е�ƫ��ֵ  
//��֪ͨʱʹ��
#define ATTRTBL_BEACON_CHAR1_IDX                   2  
#define ATTRTBL_BEACON_CHAR1_CCC_IDX               3  

#define ATTRTBL_BEACON_CHAR4_IDX                   12  
#define ATTRTBL_BEACON_CHAR4_CCC_IDX               13  


/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
//Ϊ����APP����,�����������ֵΪ����128bitsUUID��������Ը��������Զ��塣
// Beacon Condig Profile Service UUID: 0x1803

CONST uint8 beaconConfigProfileServUUID[ATT_UUID_SIZE] =
{ 
  TI_BASE_UUID_128(BEACONCONFIGPROFILE_SERV_UUID)
};

//CONST uint8 beaconConfigProfileServUUID[ATT_BT_UUID_SIZE] =
//{ 
//  LO_UINT16(BEACONCONFIGPROFILE_SERV_UUID), HI_UINT16(BEACONCONFIGPROFILE_SERV_UUID)
//};


//// Characteristic 1 UUID: 0x00001804-
//CONST uint8 beaconConfigProfileChar1UUID[ATT_UUID_SIZE] =
//{ 
//  TI_BASE_UUID_128(BEACONCONFIGPROFILE_CHAR1_UUID)
//};
//
//// Characteristic 2 UUID: 0x00001805-
//CONST uint8 beaconConfigProfileChar2UUID[ATT_UUID_SIZE] =
//{ 
//  TI_BASE_UUID_128(BEACONCONFIGPROFILE_CHAR2_UUID)
//};

// Characteristic 1 UUID: 0x1804
CONST uint8 beaconConfigProfileChar1UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(BEACONCONFIGPROFILE_CHAR1_UUID), HI_UINT16(BEACONCONFIGPROFILE_CHAR1_UUID)
};

// Characteristic 2 UUID: 0x1805
CONST uint8 beaconConfigProfileChar2UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(BEACONCONFIGPROFILE_CHAR2_UUID), HI_UINT16(BEACONCONFIGPROFILE_CHAR2_UUID)
};

//����3��������Ϊ����ʹ��
// Characteristic 3 UUID: 0xFFF3
CONST uint8 beaconConfigProfileChar3UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(BEACONCONFIGPROFILE_CHAR3_UUID), HI_UINT16(BEACONCONFIGPROFILE_CHAR3_UUID)
};

// Characteristic 4 UUID: 0xFFF4
CONST uint8 beaconConfigProfileChar4UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(BEACONCONFIGPROFILE_CHAR4_UUID), HI_UINT16(BEACONCONFIGPROFILE_CHAR4_UUID)
};

// Characteristic 5 UUID: 0xFFF5
CONST uint8 beaconConfigProfileChar5UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(BEACONCONFIGPROFILE_CHAR5_UUID), HI_UINT16(BEACONCONFIGPROFILE_CHAR5_UUID)
};

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static beaconConfigProfileCBs_t *beaconConfigProfile_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Beacon Config Profile Service attribute
//static CONST gattAttrType_t beaconConfigProfileService = { ATT_BT_UUID_SIZE, beaconConfigProfileServUUID };
static CONST gattAttrType_t beaconConfigProfileService = { ATT_UUID_SIZE, beaconConfigProfileServUUID };


// Beacon Config Profile Characteristic 1 Properties
//���char1UUID��ʱ���õ�read and notify ����ͨ��
//�������԰���GATT_PROP_NOTIFY��ʽ�����Ա���Ҫ�и�֪ͨ����simpleProfileChar1Config
//��д����ֻ��3��������������notify���Ե�����ֵ���һ������config��������4������
static uint8 beaconConfigProfileChar1Props = GATT_PROP_READ | GATT_PROP_NOTIFY;

// Characteristic 1 Value
//����1��ֵ��һ������Ϊ21�ֽڵ�����
static uint8 beaconConfigProfileChar1[21] = {0};

// Beacon Config Profile Characteristic 1 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t *beaconConfigProfileChar1Config;

// Beacon Config Profile Characteristic 1 User Description
static uint8 beaconConfigProfileChar1UserDesp[30] = "Beacon char1 desp";


// Beacon Config Profile Characteristic 2 Properties
//���char1UUID��ʱ���õ�writeͨ��
static uint8 beaconConfigProfileChar2Props = GATT_PROP_WRITE_NO_RSP;

// Characteristic 2 Value
//����2��ֵ��һ������Ϊ21�ֽڵ�����
static uint8 beaconConfigProfileChar2[21] = {0};

// Beacon Config Profile Characteristic 2 User Description
static uint8 beaconConfigProfileChar2UserDesp[17] = "Beacon char2 desp";


static uint8 char1len = 1,char2len = 1;


//������3����������
// Beacon Config Profile Characteristic 3 Properties
//writeͨ��
static uint8 beaconConfigProfileChar3Props = GATT_PROP_WRITE;

// Characteristic 3 Value
//һ���ֽ�
static uint8 beaconConfigProfileChar3 = 0;

// Beacon Config Profile Characteristic 3 User Description
static uint8 beaconConfigProfileChar3UserDesp[17] = "Characteristic 3";


// Beacon Config Profile Characteristic 4 Properties
//notifyͨ��
static uint8 beaconConfigProfileChar4Props = GATT_PROP_NOTIFY;

// Characteristic 4 Value
//һ���ֽ�
static uint8 beaconConfigProfileChar4 = 0;

// Beacon Config Profile Characteristic 4 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t *beaconConfigProfileChar4Config;
                                        
// Beacon Config Profile Characteristic 4 User Description
static uint8 beaconConfigProfileChar4UserDesp[17] = "Characteristic 4";


// Beacon Config Profile Characteristic 5 Properties
//readͨ��
static uint8 beaconConfigProfileChar5Props = GATT_PROP_READ;

// Characteristic 5 Value
//5���ֽڵ�����
static uint8 beaconConfigProfileChar5[BEACONCONFIGPROFILE_CHAR5_LEN] = { 0, 0, 0, 0, 0 };

// Beacon Config Profile Characteristic 5 User Description
static uint8 beaconConfigProfileChar5UserDesp[17] = "Characteristic 5";

/*********************************************************************
 * Profile Attributes - Table
 */
//���Ա�����
//����ֵ���Եġ�GATT_PROP_READ�������Ա�ġ�GATT_PERMIT_READ��������
/*

���Ա���һ�л𳵣�����SERVAPP_NUM_ATTR_SUPPORTED��ô��ڳ��ᣬGATT_PERMIT_READ��ÿ�ڳ����Կ�ס�
��ʱ��18��~21�ڳ���װ���Ǳ���char6��GATT_PROP_READ�Ǳ���char6��Կ�ס�
��Ȼ���Ѷ���Կ�ף��������õĶ���һ����
ʵ����GATT_PERMIT_READ��������Ա�ʹ�õģ���GATT_PROP_READ���������ֵʹ�õġ�

*/

static gattAttribute_t beaconConfigProfileAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] = 
{
  // Beacon Config Profile Service
  //������������
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&beaconConfigProfileService      /* pValue */
  },

    // Characteristic 1 Declaration
  //����1����������
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &beaconConfigProfileChar1Props //����������ֵ�ĺ��壺���������Ե����ʡ�����ֵ��Ȩ�ޣ��ɶ���֪ͨ
    },

      // Characteristic Value 1
    //����1��ֵ����
//      { 
//        { ATT_UUID_SIZE, beaconConfigProfileChar1UUID },
//        GATT_PERMIT_READ, //�ܶ�
//        0, 
//        beaconConfigProfileChar1 //ֵ�ĵ�ַ
//      },
      { 
        { ATT_BT_UUID_SIZE, beaconConfigProfileChar1UUID },
        GATT_PERMIT_READ, //�ܶ�
        0, 
        beaconConfigProfileChar1 //ֵ�ĵ�ַ
      },
      
        // Characteristic 1 configuration
      { 
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, //�ɶ�����д
        0, 
        (uint8 *)&beaconConfigProfileChar1Config //��ӷ����ʱ��̬�����ڴ�
      },

      // Characteristic 1 User Description
      //�����Կ��Բ�ʹ��
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        beaconConfigProfileChar1UserDesp 
      },

      

    // Characteristic 2 Declaration
       //����2����������
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, //�ܶ�
      0,
      &beaconConfigProfileChar2Props //��д�޻�Ӧ 
    },

      // Characteristic Value 2
    //����2��ֵ����
//      { 
//        { ATT_UUID_SIZE, beaconConfigProfileChar2UUID },
//        GATT_PERMIT_WRITE, //��д
//        0, 
//        beaconConfigProfileChar2 
//      },
      { 
        { ATT_BT_UUID_SIZE, beaconConfigProfileChar2UUID },
        GATT_PERMIT_WRITE, //��д
        0, 
        beaconConfigProfileChar2 
      },

      // Characteristic 2 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, //�ܶ�
        0, 
        beaconConfigProfileChar2UserDesp 
      },           
      
    // Characteristic 3 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, //�ܶ�
      0,
      &beaconConfigProfileChar3Props//��д 
    },

      // Characteristic Value 3
      { 
        { ATT_BT_UUID_SIZE, beaconConfigProfileChar3UUID },
        GATT_PERMIT_WRITE, //��д
        0, 
        &beaconConfigProfileChar3 
      },

      // Characteristic 3 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        beaconConfigProfileChar3UserDesp 
      },

    // Characteristic 4 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, //�ܶ�
      0,
      &beaconConfigProfileChar4Props//֪ͨ 
    },

      // Characteristic Value 4
      { 
        { ATT_BT_UUID_SIZE, beaconConfigProfileChar4UUID },
        0, //��Ҫ�ÿգ�
        0, 
        &beaconConfigProfileChar4 
      },

      // Characteristic 4 configuration
      { 
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        (uint8 *)&beaconConfigProfileChar4Config //��ӷ����ʱ��̬�����ڴ�
      },
      
      // Characteristic 4 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        beaconConfigProfileChar4UserDesp 
      },
      
    // Characteristic 5 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, //�ܶ�
      0,
      &beaconConfigProfileChar5Props//�ɶ� 
    },

      // Characteristic Value 5
      { 
        { ATT_BT_UUID_SIZE, beaconConfigProfileChar5UUID },
        GATT_PERMIT_AUTHEN_READ, 
        0, 
        beaconConfigProfileChar5 
      },

      // Characteristic 5 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        beaconConfigProfileChar5UserDesp 
      },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t beaconConfigProfile_ReadAttrCB(uint16_t connHandle,
                                          gattAttribute_t *pAttr, 
                                          uint8_t *pValue, uint16_t *pLen,
                                          uint16_t offset, uint16_t maxLen,
                                          uint8_t method);
static bStatus_t beaconConfigProfile_WriteAttrCB(uint16_t connHandle,
                                           gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len,
                                           uint16_t offset, uint8_t method);

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Beacon Config Profile Service Callbacks
CONST gattServiceCBs_t beaconConfigProfileCBs =
{
  beaconConfigProfile_ReadAttrCB,  // Read callback function pointer
  beaconConfigProfile_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      BeaconConfigProfile_AddService
 *
 * @brief   Initializes the Beacon Config Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t BeaconConfigProfile_AddService( uint32 services )
{
  uint8 status;

  //������1������4�����÷���ռ�
  // Allocate Client Characteristic Configuration table
  beaconConfigProfileChar1Config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) *
                                                            linkDBNumConns );
  
   beaconConfigProfileChar4Config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) *
                                                            linkDBNumConns );
  
  if ( beaconConfigProfileChar1Config == NULL  || beaconConfigProfileChar4Config == NULL)
  {     
    return ( bleMemAllocError );
  }
  //֪ͨ������Ҫ��ʼ��֪ͨ��������
  
  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, beaconConfigProfileChar1Config );
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, beaconConfigProfileChar4Config );
  
  
  //�ж��ϲ��û���ӷ����ID��
  if ( services & BEACONCONFIGPROFILE_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( beaconConfigProfileAttrTbl, 
                                          GATT_NUM_ATTRS( beaconConfigProfileAttrTbl ),
                                          GATT_MAX_ENCRYPT_KEY_SIZE,
                                          &beaconConfigProfileCBs );
  }
  else
  {
    status = SUCCESS;
  }

  return ( status );
}

/*********************************************************************
 * @fn      BeaconConfigProfile_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call 
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t BeaconConfigProfile_RegisterAppCBs( beaconConfigProfileCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    beaconConfigProfile_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

/*********************************************************************
 * @fn      BeaconConfigProfile_SetParameter
 *
 * @brief   Set a Beacon Config Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to write
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t BeaconConfigProfile_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case BEACONCONFIGPROFILE_CHAR1:
      //Ϊ��ӦApp������
      if(len <= sizeof(beaconConfigProfileChar1) )
      {
        char1len = len;
        
        VOID memcpy(beaconConfigProfileChar1, value, len);
        
        
        //֪ͨ
         // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( beaconConfigProfileChar1Config, beaconConfigProfileChar1, FALSE,
                                    beaconConfigProfileAttrTbl, GATT_NUM_ATTRS( beaconConfigProfileAttrTbl ),
                                    INVALID_TASK_ID, beaconConfigProfile_ReadAttrCB );

   
      }
//      if ( len == sizeof ( uint8 ) ) 
//      {
//        beaconConfigProfileChar1 = *((uint8*)value);
//      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case BEACONCONFIGPROFILE_CHAR2:
        if(len <= sizeof(beaconConfigProfileChar2) )
        {
           char2len = len;
          VOID memcpy(beaconConfigProfileChar2, value, len);  
        }
//      if ( len == sizeof ( uint8 ) ) 
//      {
//        beaconConfigProfileChar2 = *((uint8*)value);
//      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case BEACONCONFIGPROFILE_CHAR3:
      if ( len == sizeof ( uint8 ) ) 
      {
        beaconConfigProfileChar3 = *((uint8*)value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case BEACONCONFIGPROFILE_CHAR4:
      if ( len == sizeof ( uint8 ) ) 
      {
        beaconConfigProfileChar4 = *((uint8*)value);
        
        // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( beaconConfigProfileChar4Config, &beaconConfigProfileChar4, FALSE,
                                    beaconConfigProfileAttrTbl, GATT_NUM_ATTRS( beaconConfigProfileAttrTbl ),
                                    INVALID_TASK_ID, beaconConfigProfile_ReadAttrCB );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case BEACONCONFIGPROFILE_CHAR5:
      if ( len == BEACONCONFIGPROFILE_CHAR5_LEN ) 
      {
        VOID memcpy( beaconConfigProfileChar5, value, BEACONCONFIGPROFILE_CHAR5_LEN );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
      
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn      BeaconConfigProfile_GetParameter
 *
 * @brief   Get a Beacon Config Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t BeaconConfigProfile_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case BEACONCONFIGPROFILE_CHAR1:
      VOID memcpy( value, beaconConfigProfileChar1, sizeof(beaconConfigProfileChar1) );

      //*((uint8*)value) = beaconConfigProfileChar1;
      break;

    case BEACONCONFIGPROFILE_CHAR2:
      VOID memcpy( value, beaconConfigProfileChar2, sizeof(beaconConfigProfileChar2) );

      //*((uint8*)value) = beaconConfigProfileChar2;
      break;      

    case BEACONCONFIGPROFILE_CHAR3:
      *((uint8*)value) = beaconConfigProfileChar3;
      break;  

    case BEACONCONFIGPROFILE_CHAR4:
      *((uint8*)value) = beaconConfigProfileChar4;
      break;

    case BEACONCONFIGPROFILE_CHAR5:
      VOID memcpy( value, beaconConfigProfileChar5, BEACONCONFIGPROFILE_CHAR5_LEN );
      break;      
      
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn          beaconConfigProfile_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t beaconConfigProfile_ReadAttrCB(uint16_t connHandle,
                                          gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen,
                                          uint16_t offset, uint16_t maxLen,
                                          uint8_t method)
{
  bStatus_t status = SUCCESS;

  // If attribute permissions require authorization to read, return error
  if ( gattPermitAuthorRead( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }
 
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      // No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;
      // gattserverapp handles those reads

      // characteristics 1 and 2 have read permissions
      // characteritisc 3 does not have read permissions; therefore it is not
      //   included here
      // characteristic 4 does not have read permissions, but because it
      //   can be sent as a notification, it is included here
      case BEACONCONFIGPROFILE_CHAR1_UUID://R/N      
        //����setparamsʱ�õ����õĲ�������ֵ
        *pLen =  char1len;
         VOID memcpy( pValue, pAttr->pValue,  *pLen);
        
        break;
     // case BEACONCONFIGPROFILE_CHAR2_UUID://w
        
        
      case BEACONCONFIGPROFILE_CHAR4_UUID://N
        *pLen = 1;
        pValue[0] = *pAttr->pValue;
        break;

      case BEACONCONFIGPROFILE_CHAR5_UUID://R
        *pLen = BEACONCONFIGPROFILE_CHAR5_LEN;
        VOID memcpy( pValue, pAttr->pValue, BEACONCONFIGPROFILE_CHAR5_LEN );
        break;
        
      default:
        // Should never get here! (characteristics 3 and 4 do not have read permissions)
        *pLen = 0;
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }
  else
  {
    // 128-bit UUID
    *pLen = 0;
    status = ATT_ERR_INVALID_HANDLE;
  }

  return ( status );
}

/*********************************************************************
 * @fn      beaconConfigProfile_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t beaconConfigProfile_WriteAttrCB(uint16_t connHandle,
                                           gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len,
                                           uint16_t offset, uint8_t method)
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
  
  // If attribute permissions require authorization to write, return error
  if ( gattPermitAuthorWrite( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      //case BEACONCONFIGPROFILE_CHAR1_UUID:
      case BEACONCONFIGPROFILE_CHAR2_UUID://W
      case BEACONCONFIGPROFILE_CHAR3_UUID://W

        //Validate the value
        // Make sure it's not a blob oper
        if ( offset == 0 )
        {
          //�����ж�
          if ( len > 200)
          {
            status = ATT_ERR_INVALID_VALUE_SIZE;
          }
        }
        else
        {
          status = ATT_ERR_ATTR_NOT_LONG;
        }
        
        //Write the value
        if ( status == SUCCESS )
        {
          uint8 *pCurValue = (uint8 *)pAttr->pValue;        

          if( pAttr->pValue == beaconConfigProfileChar2 )
          {
            //clear buff
            VOID memset(pCurValue, 0x00, sizeof(beaconConfigProfileChar2)) ; 
            VOID memcpy(pCurValue, pValue, len) ; 
            char2len = len;//reset char2 len
            notifyApp = BEACONCONFIGPROFILE_CHAR2;        
          }
          else
          {       
            *pCurValue = pValue[0];
            notifyApp = BEACONCONFIGPROFILE_CHAR3;           
          }
        }
             
        break;

      case GATT_CLIENT_CHAR_CFG_UUID://APP����֪ͨ����
        
        //���û��ӻᵼ��֪ͨ���ش򲻿��������ڴӻ��޷������������ݵ�����
        //CHAR1��֪ͨ����
        if(pAttr->handle == beaconConfigProfileAttrTbl[ATTRTBL_BEACON_CHAR1_CCC_IDX].handle) 
        {  
           status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
        }
        else if(pAttr->handle == beaconConfigProfileAttrTbl[ATTRTBL_BEACON_CHAR4_CCC_IDX].handle) 
        {  
           status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
        }
        else    
        {    

          status = ATT_ERR_INVALID_HANDLE;    

        } 
        
        break;
        
      default:
        // Should never get here! (characteristics 2 and 4 do not have write permissions)
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }
  else
  {
    // 128-bit UUID
    //status = ATT_ERR_INVALID_HANDLE;
    //����128bits UUID ʱ�����
      
    const uint8 uuid[ATT_UUID_SIZE] = 
    { 
    	//Ti�ⲿ��д���д����Ѿ�������
      TI_BASE_UUID_128(BUILD_UINT16( pAttr->type.uuid[12], pAttr->type.uuid[13]))
    };
    
    if( pAttr->pValue == beaconConfigProfileChar2 )
    {
      //clear buff
      VOID memset(pAttr->pValue, 0x00, sizeof(beaconConfigProfileChar2)) ; 
      VOID memcpy(pAttr->pValue, pValue, len) ; 
      char2len = len;//reset char2 len
      notifyApp = BEACONCONFIGPROFILE_CHAR2;   
      status = SUCCESS;
    }
  }

  // If a characteristic value changed then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && beaconConfigProfile_AppCBs && beaconConfigProfile_AppCBs->pfnBeaconConfigProfileChange )
  {
    beaconConfigProfile_AppCBs->pfnBeaconConfigProfileChange( notifyApp );  
  }
  
  return ( status );
}

/*********************************************************************
 * @fn      BeaconConfigProfile_Notify
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 *
 */
void BeaconConfigProfile_Notify( uint8 param, uint16 connHandle, uint8 *pValue, uint8 len)
{
  
  attHandleValueNoti_t  noti;    
  uint16 value;    

  switch ( param )  
  {  

    //����ֵ1
    case BEACONCONFIGPROFILE_CHAR1:    
    {
      //����CCC
      value  = GATTServApp_ReadCharCfg(connHandle, beaconConfigProfileChar1Config);   
      //�ж�CCC�Ƿ񱻴�
      if ( value & GATT_CLIENT_CFG_NOTIFY )                                     
      {    
        //���䷢�����ݻ�����
        noti.pValue = GATT_bm_alloc(connHandle, ATT_HANDLE_VALUE_NOTI, len, NULL); 
        //����ɹ�����������
        if(noti.pValue != NULL)
        {
          //�������
          noti.handle = beaconConfigProfileAttrTbl[ATTRTBL_BEACON_CHAR1_IDX].handle;    
          noti.len = len;    
          memcpy( noti.pValue, pValue, len);  
          //��������
          if (GATT_Notification(connHandle, &noti, FALSE) != SUCCESS)
          {
            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
          }
        }
      }    

      break;      

    } 
    
     //����ֵ4
    case BEACONCONFIGPROFILE_CHAR4:    
    {
      //����CCC
      value  = GATTServApp_ReadCharCfg(connHandle, beaconConfigProfileChar4Config);   
      //�ж�CCC�Ƿ񱻴�
      if ( value & GATT_CLIENT_CFG_NOTIFY )                                     
      {    
        //���䷢�����ݻ�����
        noti.pValue = GATT_bm_alloc(connHandle, ATT_HANDLE_VALUE_NOTI, sizeof(beaconConfigProfileChar4), NULL); 
        //����ɹ�����������
        if(noti.pValue != NULL)
        {
          //�������
          noti.handle = beaconConfigProfileAttrTbl[ATTRTBL_BEACON_CHAR4_IDX].handle;    
          noti.len = len;    
          memcpy( noti.pValue, pValue, len);  
          //��������
          if (GATT_Notification(connHandle, &noti, FALSE) != SUCCESS)
          {
            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
          }
        }
      }    

      break;         
    }

    default:  
      break; 
  
}
}

/*********************************************************************
*********************************************************************/
