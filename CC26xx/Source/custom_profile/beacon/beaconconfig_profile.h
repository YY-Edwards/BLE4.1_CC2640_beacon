/*
* Filename:	beaconconfig_profile.h
* Created:	2018/08/15
* Author:	EDWARDS
* Describe:	This file contains the beaconc onfig profile definitions and
            prototypes.
* Version:	v1.0
* History:
*/

#ifndef BEACONCONFIGPROFILE_H
#define BEACONCONFIGPROFILE_H

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

// Profile Parameters
#define BEACONCONFIGPROFILE_CHAR1                   0  // RW uint8 - Profile Characteristic 1 value 
#define BEACONCONFIGPROFILE_CHAR2                   1  // RW uint8 - Profile Characteristic 2 value
#define BEACONCONFIGPROFILE_CHAR3                   2  // RW uint8 - Profile Characteristic 3 value
#define BEACONCONFIGPROFILE_CHAR4                   3  // RW uint8 - Profile Characteristic 4 value
#define BEACONCONFIGPROFILE_CHAR5                   4  // RW uint8 - Profile Characteristic 4 value
  
//以下声明根据适应App需要而调整的参数；识别的UUID？   
#define CS_UUID(uuid)       0x00, 0x00, 0x18, 0x03, 0x49, 0x4c, 0x4f, 0x47,0x49,0x43, 0x54, 0x45, 0x43, 0x48, 0x00, 0x00   
   
// Beacon Config Profile Service UUID
#define BEACONCONFIGPROFILE_SERV_UUID             0x1803 // 用于修改ibeacon通道 
    
// Key Pressed UUID
#define BEACONCONFIGPROFILE_CHAR1_UUID            0x1804
#define BEACONCONFIGPROFILE_CHAR2_UUID            0x1805
#define BEACONCONFIGPROFILE_CHAR3_UUID            0xFFF3
#define BEACONCONFIGPROFILE_CHAR4_UUID            0xFFF4
#define BEACONCONFIGPROFILE_CHAR5_UUID            0xFFF5
  
// Simple Keys Profile Services bit fields
   //服务识别号
#define BEACONCONFIGPROFILE_SERVICE               0x00000010

// Length of Characteristic 5 in bytes
#define BEACONCONFIGPROFILE_CHAR5_LEN           5  

/*********************************************************************
 * TYPEDEFS
 */

  
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*beaconconfigProfileChange_t)( uint8 paramID );

typedef struct
{
  beaconconfigProfileChange_t        pfnBeaconConfigProfileChange;  // Called when characteristic value changes
} beaconconfigProfileCBs_t;

    

/*********************************************************************
 * API FUNCTIONS 
 */


/*
 * BeaconConfigProfile_AddService- Initializes the Simple GATT Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

extern bStatus_t BeaconConfigProfile_AddService( uint32 services );

/*
 * BeaconConfigProfile_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t BeaconConfigProfile_RegisterAppCBs( beaconconfigProfileCBs_t *appCallbacks );

/*
 * BeaconConfigProfile_SetParameter - Set a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t BeaconConfigProfile_SetParameter( uint8 param, uint8 len, void *value );
  
/*
 * BeaconConfigProfile_GetParameter - Get a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t BeaconConfigProfile_GetParameter( uint8 param, void *value );






/*
 * BeaconConfigProfile_Notify - Notify a Simple GATT Profile parameter.
 *
 *    param      - Profile parameter ID
 *    connHandle - connected handle
 *    pValue     — pointer to data to write
 *    len        - data length.        
 */                                

extern void BeaconConfigProfile_Notify( uint8 param, uint16 connHandle, uint8 *pValue, uint8 len); 

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEGATTPROFILE_H */
