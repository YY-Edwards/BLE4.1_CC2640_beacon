/*
* Filename:	customerInfoStorage.h
* Created:	2019/04/28
* Author:	EDWARDS
* Describe:	This file contains the beacon info of SNV definitions and
            prototypes.
* Version:	v1.0
* History:
*/




#ifndef _CUSTOMER_INFO_STORAGE_H_
#define _CUSTOMER_INFO_STORAGE_H_

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
   
  /*********************************************************************
 * MACROS
 */

  
#define C_INFO_SNV_READ                 0X01
#define C_INFO_SNV_WRITE                0X02
#define C_INFO_PER_SNV_MAX_LEN          252
#define C_INFO_SNV_BEACON_STORAGE_ID    0X87
   
#define CUSTOMER_UUID_LEN 16   
#define CUSTOMER_MAX_BEAON_NAME_LEN 20      
   
/*********************************************************************
 * TYPEDEFS
 */
#pragma pack(1)
  typedef struct{
    
    unsigned char       dev_name[CUSTOMER_MAX_BEAON_NAME_LEN];//assic
    unsigned short      adv_interval_ms;
    unsigned char       dev_tx_power;
    unsigned char       uuid[CUSTOMER_UUID_LEN];
    unsigned short      major;
    unsigned short      minor;
    unsigned short      rssi_one_meter;   
    
  }customerStorageBeaconInfo_t;//45bytes.
  
 #pragma pack() 
 
      
   
/*********************************************************************
 * API FUNCTIONS 
 */


 extern unsigned char customerInfo_snv_control(
                                        unsigned char   ncustomerInfo_snv_id,
                                        unsigned char   action_flag,
                                        unsigned char*  data_ptr,
                                        unsigned char   data_len
                                        );
extern void get_customerInfo(customerStorageBeaconInfo_t* p);
 
extern void set_customerInfo(customerStorageBeaconInfo_t* p);
 
extern void fill_customerInfo_byDefault(customerStorageBeaconInfo_t* p);


#ifdef __cplusplus
}
#endif

#endif /* _CUSTOMER_INFO_STORAGE_H_ */