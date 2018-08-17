/*
* Filename:	beaconconfig_profile.c
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

#include "beaconconfig_profile.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define SERVAPP_NUM_ATTR_SUPPORTED        18


//属性在属性表中的偏移值  
//在通知时使用
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
//为满足APP需求,特征1值属性和特征2值属性里使用Ti的128bitsUUID，过后可以根据需求自定义。
// Beacon Condig Profile Service UUID: 0x1803
CONST uint8 beaconconfigProfileServUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(BEACONCONFIGPROFILE_SERV_UUID), HI_UINT16(BEACONCONFIGPROFILE_SERV_UUID)
};

// Characteristic 1 UUID: 0x1804
CONST uint8 beaconconfigProfilechar1UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(BEACONCONFIGPROFILE_CHAR1_UUID), HI_UINT16(BEACONCONFIGPROFILE_CHAR1_UUID)
};

// Characteristic 2 UUID: 0x1805
CONST uint8 beaconconfigProfilechar2UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(BEACONCONFIGPROFILE_CHAR2_UUID), HI_UINT16(BEACONCONFIGPROFILE_CHAR2_UUID)
};

//以下3个特征作为调试使用
// Characteristic 3 UUID: 0xFFF3
CONST uint8 beaconconfigProfilechar3UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(BEACONCONFIGPROFILE_CHAR3_UUID), HI_UINT16(BEACONCONFIGPROFILE_CHAR3_UUID)
};

// Characteristic 4 UUID: 0xFFF4
CONST uint8 beaconconfigProfilechar4UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(BEACONCONFIGPROFILE_CHAR4_UUID), HI_UINT16(BEACONCONFIGPROFILE_CHAR4_UUID)
};

// Characteristic 5 UUID: 0xFFF5
CONST uint8 beaconconfigProfilechar5UUID[ATT_BT_UUID_SIZE] =
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

static beaconconfigProfileCBs_t *beaconconfigProfile_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Beacon Config Profile Service attribute
static CONST gattAttrType_t beaconconfigProfileService = { ATT_BT_UUID_SIZE, beaconconfigProfileServUUID };


// Beacon Config Profile Characteristic 1 Properties
//添加char1UUID的时，用到read and notify 两个通道
//由于属性包含GATT_PROP_NOTIFY方式，所以必须要有个通知开关simpleProfileChar1Config
//读写属性只有3个变量，而含有notify属性的特征值会多一个开关config，所以是4个变量
static uint8 beaconconfigProfileChar1Props = GATT_PROP_READ | GATT_PROP_NOTIFY;

// Characteristic 1 Value
//特征1的值是一个容量为21字节的数组
static uint8 beaconconfigProfileChar1[21] = {0};

// Beacon Config Profile Characteristic 1 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t *beaconconfigProfileChar1Config;

// Beacon Config Profile Characteristic 1 User Description
static uint8 beaconconfigProfileChar1UserDesp[30] = "Beacon char1 desp";


// Beacon Config Profile Characteristic 2 Properties
//添加char1UUID的时，用到write通道
static uint8 beaconconfigProfileChar2Props = GATT_PROP_WRITE;

// Characteristic 2 Value
//特征2的值是一个容量为21字节的数组
static uint8 beaconconfigProfileChar2[21] = {0};

// Beacon Config Profile Characteristic 2 User Description
static uint8 beaconconfigProfileChar2UserDesp[17] = "Beacon char2 desp";


static uint8 char1len = 1,char2len = 1;


//以下是3个测试特征
// Beacon Config Profile Characteristic 3 Properties
//write通道
static uint8 beaconconfigProfileChar3Props = GATT_PROP_WRITE;

// Characteristic 3 Value
//一个字节
static uint8 beaconconfigProfileChar3 = 0;

// Beacon Config Profile Characteristic 3 User Description
static uint8 beaconconfigProfileChar3UserDesp[17] = "Characteristic 3";


// Beacon Config Profile Characteristic 4 Properties
//notify通道
static uint8 beaconconfigProfileChar4Props = GATT_PROP_NOTIFY;

// Characteristic 4 Value
//一个字节
static uint8 beaconconfigProfileChar4 = 0;

// Beacon Config Profile Characteristic 4 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t *beaconconfigProfileChar4Config;
                                        
// Beacon Config Profile Characteristic 4 User Description
static uint8 beaconconfigProfileChar4UserDesp[17] = "Characteristic 4";


// Beacon Config Profile Characteristic 5 Properties
//read通道
static uint8 beaconconfigProfileChar5Props = GATT_PROP_READ;

// Characteristic 5 Value
//5个字节的数组
static uint8 beaconconfigProfileChar5[BEACONCONFIGPROFILE_CHAR5_LEN] = { 0, 0, 0, 0, 0 };

// Beacon Config Profile Characteristic 5 User Description
static uint8 beaconconfigProfileChar5UserDesp[17] = "Characteristic 5";

/*********************************************************************
 * Profile Attributes - Table
 */
//属性表配置
//特征值属性的“GATT_PROP_READ”与属性表的“GATT_PERMIT_READ”的区别：
/*

属性表是一列火车，它有SERVAPP_NUM_ATTR_SUPPORTED这么多节车厢，GATT_PERMIT_READ是每节车厢的钥匙。
此时第18节~21节车厢装的是宝箱char6，GATT_PROP_READ是宝箱char6的钥匙。
虽然两把都是钥匙，但是作用的对象不一样。
实际上GATT_PERMIT_READ是针对属性表使用的，而GATT_PROP_READ是针对特征值使用的。

*/

static gattAttribute_t beaconconfigProfileAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] = 
{
  // Beacon Config Profile Service
  //服务属性
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&beaconconfigProfileService      /* pValue */
  },

    // Characteristic 1 Declaration
  //特征1的声明属性
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &beaconconfigProfileChar1Props //特征值的权限：可读，通知
    },

      // Characteristic Value 1
    //特征1的值属性
      { 
        { ATT_BT_UUID_SIZE, beaconconfigProfilechar1UUID },
        GATT_PERMIT_READ, //能读
        0, 
        beaconconfigProfileChar1 //值的地址
      },
      
        // Characteristic 1 configuration
      { 
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, //可读，可写
        0, 
        (uint8 *)&beaconconfigProfileChar1Config //添加服务的时候动态分配内存
      },

      // Characteristic 1 User Description
      //此属性可以不使用
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        beaconconfigProfileChar1UserDesp 
      },

      

    // Characteristic 2 Declaration
       //特征2的声明属性
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, //能读
      0,
      &beaconconfigProfileChar2Props //可写 
    },

      // Characteristic Value 2
    //特征2的值属性
      { 
        { ATT_BT_UUID_SIZE, beaconconfigProfilechar2UUID },
        GATT_PERMIT_WRITE, //能写
        0, 
        &beaconconfigProfileChar2 
      },

      // Characteristic 2 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, //能读
        0, 
        beaconconfigProfileChar2UserDesp 
      },           
      
    // Characteristic 3 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, //能读
      0,
      &beaconconfigProfileChar3Props//可写 
    },

      // Characteristic Value 3
      { 
        { ATT_BT_UUID_SIZE, beaconconfigProfilechar3UUID },
        GATT_PERMIT_WRITE, //能写
        0, 
        &beaconconfigProfileChar3 
      },

      // Characteristic 3 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        beaconconfigProfileChar3UserDesp 
      },

    // Characteristic 4 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, //能读
      0,
      &beaconconfigProfileChar4Props//通知 
    },

      // Characteristic Value 4
      { 
        { ATT_BT_UUID_SIZE, beaconconfigProfilechar4UUID },
        0, //需要置空？
        0, 
        &beaconconfigProfileChar4 
      },

      // Characteristic 4 configuration
      { 
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        (uint8 *)&beaconconfigProfileChar4Config //添加服务的时候动态分配内存
      },
      
      // Characteristic 4 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        beaconconfigProfileChar4UserDesp 
      },
      
    // Characteristic 5 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, //能读
      0,
      &beaconconfigProfileChar5Props//可读 
    },

      // Characteristic Value 5
      { 
        { ATT_BT_UUID_SIZE, beaconconfigProfilechar5UUID },
        GATT_PERMIT_AUTHEN_READ, 
        0, 
        beaconconfigProfileChar5 
      },

      // Characteristic 5 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        beaconconfigProfileChar5UserDesp 
      },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t beaconconfigProfile_ReadAttrCB(uint16_t connHandle,
                                          gattAttribute_t *pAttr, 
                                          uint8_t *pValue, uint16_t *pLen,
                                          uint16_t offset, uint16_t maxLen,
                                          uint8_t method);
static bStatus_t beaconconfigProfile_WriteAttrCB(uint16_t connHandle,
                                           gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len,
                                           uint16_t offset, uint8_t method);

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Beacon Config Profile Service Callbacks
CONST gattServiceCBs_t beaconconfigProfileCBs =
{
  beaconconfigProfile_ReadAttrCB,  // Read callback function pointer
  beaconconfigProfile_WriteAttrCB, // Write callback function pointer
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

  //给特征1和特征4的配置分配空间
  // Allocate Client Characteristic Configuration table
  beaconconfigProfileChar1Config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) *
                                                            linkDBNumConns );
  
   beaconconfigProfileChar4Config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) *
                                                            linkDBNumConns );
  
  if ( beaconconfigProfileChar1Config == NULL  || beaconconfigProfileChar4Config == NULL)
  {     
    return ( bleMemAllocError );
  }
  //通知属性需要初始化通知开关属性
  
  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, beaconconfigProfileChar1Config );
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, beaconconfigProfileChar4Config );
  
  
  //判断上层用户添加服务的ID号
  if ( services & BEACONCONFIGPROFILE_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( beaconconfigProfileAttrTbl, 
                                          GATT_NUM_ATTRS( beaconconfigProfileAttrTbl ),
                                          GATT_MAX_ENCRYPT_KEY_SIZE,
                                          &beaconconfigProfileCBs );
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
bStatus_t BeaconConfigProfile_RegisterAppCBs( beaconconfigProfileCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    beaconconfigProfile_AppCBs = appCallbacks;
    
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
      //为适应App而调整
      if(len <= sizeof(beaconconfigProfileChar1) )
      {
        char1len = len;
        
        VOID memcpy(beaconconfigProfileChar1, value, len);
        
        
        //通知
         // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( beaconconfigProfileChar1Config, &beaconconfigProfileChar1, FALSE,
                                    beaconconfigProfileAttrTbl, GATT_NUM_ATTRS( beaconconfigProfileAttrTbl ),
                                    INVALID_TASK_ID, beaconconfigProfile_ReadAttrCB );

   
      }
//      if ( len == sizeof ( uint8 ) ) 
//      {
//        beaconconfigProfileChar1 = *((uint8*)value);
//      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case BEACONCONFIGPROFILE_CHAR2:
        if(len <= sizeof(beaconconfigProfileChar2) )
        {
           char2len = len;
          VOID memcpy(beaconconfigProfileChar2, value, len);  
        }
//      if ( len == sizeof ( uint8 ) ) 
//      {
//        beaconconfigProfileChar2 = *((uint8*)value);
//      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case BEACONCONFIGPROFILE_CHAR3:
      if ( len == sizeof ( uint8 ) ) 
      {
        beaconconfigProfileChar3 = *((uint8*)value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case BEACONCONFIGPROFILE_CHAR4:
      if ( len == sizeof ( uint8 ) ) 
      {
        beaconconfigProfileChar4 = *((uint8*)value);
        
        // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( beaconconfigProfileChar4Config, &beaconconfigProfileChar4, FALSE,
                                    beaconconfigProfileAttrTbl, GATT_NUM_ATTRS( beaconconfigProfileAttrTbl ),
                                    INVALID_TASK_ID, beaconconfigProfile_ReadAttrCB );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case BEACONCONFIGPROFILE_CHAR5:
      if ( len == BEACONCONFIGPROFILE_CHAR5_LEN ) 
      {
        VOID memcpy( beaconconfigProfileChar5, value, BEACONCONFIGPROFILE_CHAR5_LEN );
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
      VOID memcpy( value, beaconconfigProfileChar1, sizeof(beaconconfigProfileChar1) );

      //*((uint8*)value) = beaconconfigProfileChar1;
      break;

    case BEACONCONFIGPROFILE_CHAR2:
      VOID memcpy( value, beaconconfigProfileChar2, sizeof(beaconconfigProfileChar2) );

      //*((uint8*)value) = beaconconfigProfileChar2;
      break;      

    case BEACONCONFIGPROFILE_CHAR3:
      *((uint8*)value) = beaconconfigProfileChar3;
      break;  

    case BEACONCONFIGPROFILE_CHAR4:
      *((uint8*)value) = beaconconfigProfileChar4;
      break;

    case BEACONCONFIGPROFILE_CHAR5:
      VOID memcpy( value, beaconconfigProfileChar5, BEACONCONFIGPROFILE_CHAR5_LEN );
      break;      
      
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn          beaconconfigProfile_ReadAttrCB
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
static bStatus_t beaconconfigProfile_ReadAttrCB(uint16_t connHandle,
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
        //根据setparams时得到设置的参数长度值
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
 * @fn      beaconconfigProfile_WriteAttrCB
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
static bStatus_t beaconconfigProfile_WriteAttrCB(uint16_t connHandle,
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
          //长度判断
          if ( len > 200)
          {
            status = att_err_invalid_value_size;
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

          if( pAttr->pValue == &beaconconfigProfileChar2 )
          {
            //clear buff
            VOID memset(pCurValue, 0x00, sizeof(beaconconfigProfileChar2)) ; 
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

      case GATT_CLIENT_CHAR_CFG_UUID://APP开启通知功能
        
        //如果没添加会导致通知开关打不开，以至于从机无法主动发送数据到主机
        //CHAR1的通知开关
        if(pAttr->handle == simpleProfileAttrTbl[ATTRTBL_BEACON_CHAR1_CCC_IDX].handle) 
        {  
           status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
        }
         else    
        {    

          status = ATT_ERR_INVALID_HANDLE;    

        } 
        
          if(pAttr->handle == simpleProfileAttrTbl[ATTRTBL_BEACON_CHAR4_CCC_IDX].handle) 
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
    status = ATT_ERR_INVALID_HANDLE;
  }

  // If a characteristic value changed then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && beaconconfigProfile_AppCBs && beaconconfigProfile_AppCBs->pfnBeaconConfigProfileChange )
  {
    beaconconfigProfile_AppCBs->pfnBeaconConfigProfileChange( notifyApp );  
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

    //特征值1
    case BEACONCONFIGPROFILE_CHAR1:    
    {
      //读出CCC
      value  = GATTServApp_ReadCharCfg(connHandle, beaconconfigProfileChar1Config);   
      //判断CCC是否被打开
      if ( value & GATT_CLIENT_CFG_NOTIFY )                                     
      {    
        //分配发送数据缓冲区
        noti.pValue = GATT_bm_alloc(connHandle, ATT_HANDLE_VALUE_NOTI, len, NULL); 
        //分配成功，则发送数据
        if(noti.pValue != NULL)
        {
          //填充数据
          noti.handle = GUAProfileAttrTbl[ATTRTBL_BEACON_CHAR1_IDX].handle;    
          noti.len = len;    
          memcpy( noti.pValue, pValue, len);  
          //发送数据
          if (GATT_Notification(connHandle, &noti, FALSE) != SUCCESS)
          {
            GATT_bm_free((gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI);
          }
        }
      }    

      break;      

    } 
    
     //特征值4
    case BEACONCONFIGPROFILE_CHAR4:    
    {
      //读出CCC
      value  = GATTServApp_ReadCharCfg(connHandle, beaconconfigProfileChar4Config);   
      //判断CCC是否被打开
      if ( value & GATT_CLIENT_CFG_NOTIFY )                                     
      {    
        //分配发送数据缓冲区
        noti.pValue = GATT_bm_alloc(connHandle, ATT_HANDLE_VALUE_NOTI, sizeof(beaconconfigProfileChar4), NULL); 
        //分配成功，则发送数据
        if(noti.pValue != NULL)
        {
          //填充数据
          noti.handle = GUAProfileAttrTbl[ATTRTBL_BEACON_CHAR4_IDX].handle;    
          noti.len = len;    
          memcpy( noti.pValue, pValue, len);  
          //发送数据
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

/*********************************************************************
*********************************************************************/
