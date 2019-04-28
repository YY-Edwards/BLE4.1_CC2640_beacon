/*
* Filename:	customerInfoStorage.c
* Created:	2019/04/28
* Author:	EDWARDS
* Describe:	This file contains the customerInfoStorage application.
* Version:	v1.0
* History:
*/

/*********************************************************************
 * INCLUDES
 */

#include "customerInfoStorage.h"
#include "bcomdef.h"
#include "osal_snv.h"
#include "customerDefaultConfig.h"
#include "att.h"
#include <string.h>



/*********************************************************************
 * LOCAL FUNCTIONS
 */

 unsigned char customerInfo_snv_control(
                                        unsigned char   ncustomerInfo_snv_id,
                                        unsigned char   action_flag,
                                        unsigned char*  data_ptr,
                                        unsigned char   data_len
                                        )
 {
  
   if((ncustomerInfo_snv_id >= BLE_NVID_CUST_START)
      && (ncustomerInfo_snv_id <= BLE_NVID_CUST_END)
      &&(data_len <= C_INFO_PER_SNV_MAX_LEN)
      &&(data_ptr !=NULL))
   {
    if(action_flag == C_INFO_SNV_READ)
    {
        return osal_snv_read(ncustomerInfo_snv_id, data_len, data_ptr);   
    }
    else
    {
      
        return osal_snv_write(ncustomerInfo_snv_id, data_len, data_ptr);    
    }
   
   }
   else
   {
        return INVALIDPARAMETER;
   }
 }


 
void fill_customerInfo(customerStorageBeaconInfo_t* p)
{
  
  
  if(p!=NULL)
  {
    VOID strncpy((char*)p->dev_name, SETTING_BEACON_NAME, strlen(SETTING_BEACON_NAME));
    p->adv_interval_ms          = SETTING_BEACON_ADV_INTERVAL;
    p->dev_tx_power             = SETTING_BEACON_TX_POWER;
    
    CONST uint8 Setting_UUID[ATT_UUID_SIZE] =
    { 
      SETTING_BEACON_UUID
    };
    VOID memcpy(p->uuid, (void*)Setting_UUID, sizeof(Setting_UUID));
    
    p->major                    = SETTING_BEACON_MAJOR;
    p->minor                    = SETTING_BEACON_MINOR;
    p->rssi_one_meter           = SETTING_BEACON_RSSI;
  }
  
}
      
void get_customerInfo(customerStorageBeaconInfo_t* p)
 {
  if(p!=NULL)
  {
   
    uint8_t ret = customerInfo_snv_control(C_INFO_SNV_BEACON_STORAGE_ID,
                             C_INFO_SNV_READ,
                             (unsigned char*)p,
                              sizeof(customerStorageBeaconInfo_t));
  }
 
 }
 
 void set_customerInfo(customerStorageBeaconInfo_t* p)
 {
    if(p!=NULL)
  {
   
    customerInfo_snv_control(C_INFO_SNV_BEACON_STORAGE_ID,
                             C_INFO_SNV_WRITE,
                             (unsigned char*)p,
                              sizeof(customerStorageBeaconInfo_t));
  }
 
 
 
 }

                                        