/*******************************************************************************
  Filename:       simpleBLEPeripheral.c
  Revised:        $Date: 2016-01-07 16:59:59 -0800 (Thu, 07 Jan 2016) $
  Revision:       $Revision: 44594 $

  Description:    This file contains the Simple BLE Peripheral sample 
                  application for use with the CC2650 Bluetooth Low Energy 
                  Protocol Stack.

  Copyright 2013 - 2015 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED �AS IS� WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
*******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Queue.h>

#include "hci_tl.h"
#include "gatt.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "simpleGATTprofile.h"
  
//����beacon_profile
#include "beaconConfig_profile.h"   
#include "customerDefaultConfig.h"
   
//����ͷ�ļ�
#include "customerInfoStorage.h"   
#include "devUart.h"

#include "BEGP.h"

#if defined(SENSORTAG_HW)
#include "bsp_spi.h"
#endif // SENSORTAG_HW

#if defined(FEATURE_OAD) || defined(IMAGE_INVALIDATE)
#include "oad_target.h"
#include "oad.h"
#endif //FEATURE_OAD || IMAGE_INVALIDATE

#include "peripheral.h"
#include "gapbondmgr.h"

#include "osal_snv.h"
#include "ICallBleAPIMSG.h"

#include "util.h"
#include "board_lcd.h"
#include "board_key.h"
#include "Board.h"

#include "simpleBLEPeripheral.h"

#include <ti/drivers/lcd/LCDDogm1286.h>

/*********************************************************************
 * CONSTANTS
 */
// Advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          480//300ms;

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL

#ifndef FEATURE_OAD
// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic
// parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     80

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic
// parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     800
#else
// Minimum connection interval (units of 1.25ms, 8=10ms) if automatic
// parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     8

// Maximum connection interval (units of 1.25ms, 8=10ms) if automatic
// parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     8
#endif // FEATURE_OAD

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter
// update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is
// formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

// How often to perform periodic event (in msec)
#define SBP_PERIODIC_EVT_PERIOD               5000

//�趨������ѯ��ʱʱ��
#define SBP_UART_RX_EVT_PERIOD                       500


//�趨��̬�㲥�Ͷ�̬ɨ���Ӧ�ĵ�һ�δ����ӳ�
#define SBP_ADV_UPDATE_EVT_PERIOD                    350
#define SBP_SCAN_RSP_UPDATE_EVT_PERIOD               350

#ifdef FEATURE_OAD
// The size of an OAD packet.
#define OAD_PACKET_SIZE                       ((OAD_BLOCK_SIZE) + 2)
#endif // FEATURE_OAD

// Task configuration
#define SBP_TASK_PRIORITY                     1

#ifndef SBP_TASK_STACK_SIZE
#define SBP_TASK_STACK_SIZE                   644
#endif

// Internal Events for RTOS application       
#define SBP_STATE_CHANGE_EVT                  0x0001
#define SBP_CHAR_CHANGE_EVT                   0x0002
#define SBP_PERIODIC_EVT                      0x0004
#define SBP_CONN_EVT_END_EVT                  0x0008
//���ӷ������¼���
#define SBP_BEACON_CHAR_CHANGE_EVT              0x0010  

//�����Զ���������¼�
#define SBP_UART_MONITORING_EVT                 0x0020  
//���Ӷ�̬�㲥���ݺͶ�̬ɨ���Ӧ�����¼�
#define SBP_ADV_UPDATE_EVT                      0x0040 
#define SBP_SCAN_RSP_UPDATE_EVT                 0x0080 

/*********************************************************************
 * TYPEDEFS
 */

// App event passed from profiles.
typedef struct
{
  appEvtHdr_t hdr;  // event header.
} sbpEvt_t;

/*********************************************************************
 * LOCAL VARIABLES
 */

// Entity ID globally used to check for source and/or destination of messages
static ICall_EntityID selfEntity;

// Semaphore globally used to post events to the application thread
static ICall_Semaphore sem;

// Clock instances for internal periodic events.
static Clock_Struct periodicClock;

//���嶨ʱ���ṹ������
static Clock_Struct UartMonitoringClock;

//����㲥���ݸ��º�ɨ���Ӧ���ݸ��¶�ʱ���ṹ������
static Clock_Struct AdvUpdateClock;
static Clock_Struct ScanRspUpdateClock;

// Queue object used for app messages
static Queue_Struct appMsg;
static Queue_Handle appMsgQueue;

#if defined(FEATURE_OAD)
// Event data from OAD profile.
static Queue_Struct oadQ;
static Queue_Handle hOadQ;
#endif //FEATURE_OAD

// events flag for internal application events.
static uint16_t events;

// Task configuration
Task_Struct sbpTask;
Char sbpTaskStack[SBP_TASK_STACK_SIZE];

// Profile state and parameters
//static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP - SCAN RSP data (max size = 31 bytes)

static uint8_t scanRspData[] =
{
  // complete name
  //0x14,   // length of this data
  0x09,
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  0x6A,//'j'
  0x68,//'h'
  0x62,//'b'
  0x65,//'e'
  0x61,//'a'
  0x63,//'c'
  0x6f,//'o'
  0x6e,//'n'
  
  //Advertising Interval
  0x03,   // length of this data
  GAP_ADTYPE_ADV_INTERVAL,
  LO_UINT16(DEFAULT_ADVERTISING_INTERVAL),//300ms
  HI_UINT16(DEFAULT_ADVERTISING_INTERVAL),
  //0       // 0dBm 

  // connection interval range
  0x05,   // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),   // 100ms
  HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
  LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),   // 1s
  HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),

  // Tx power level
  0x02,   // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  (HCI_EXT_TX_POWER_5_DBM - HCI_EXT_TX_POWER_0_DBM)//5dBm
  //0       // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8_t advertData[] =
{
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

    // 25 byte beacon advertisement data
  // Preamble: Company ID - 0x000D for TI, refer to https://www.bluetooth.org/en-us/specification/assigned-numbers/company-identifiers
  // Data type: Beacon (0x02)
  // Data length: 0x15
  // UUID: 50dcb6f6-915a-4142-a6fe-fda7b4418609 (jihua beacon)
  // Major: 10 (0x000A)
  // Minor: 13 (0x000D)
  // Measured Power: -59 (0xc5)
  0x1A, // length of this data including the data type byte
  GAP_ADTYPE_MANUFACTURER_SPECIFIC, // manufacturer specific adv data type
  0x0D, // Company ID - Fixed
  0x00, // Company ID - Fixed
  0x02, // Data Type - Fixed
  0x15, // Data Length - Fixed
  0x50, // UUID - Variable based on different use cases/applications
  0xdc, // UUID
  0xb6, // UUID
  0xf6, // UUID
  0x91, // UUID
  0x5a, // UUID
  0x41, // UUID
  0x42, // UUID
  0xa6, // UUID
  0xfe, // UUID
  0xfd, // UUID
  0xa7, // UUID
  0xb4, // UUID
  0x41, // UUID
  0x86, // UUID
  0x09, // UUID
  0x00, // Major
  0x0A, // Major
  0x00, // Minor
  0x0D, // Minor
  0xc5  // Power - The 2's complement of the calibrated Tx Power
  
//  // service UUID, to notify central devices what services are included
//  // in this peripheral
//  0x03,   // length of this data
//  GAP_ADTYPE_16BIT_MORE,      // some of the UUID's, but not all
//#ifdef FEATURE_OAD
//  LO_UINT16(OAD_SERVICE_UUID),
//  HI_UINT16(OAD_SERVICE_UUID)
//#else
//  LO_UINT16(SIMPLEPROFILE_SERV_UUID),
//  HI_UINT16(SIMPLEPROFILE_SERV_UUID),
//#endif //!FEATURE_OAD
};

//customerStorageBeaconInfo_t test_customerStorageBeaconInfo;
customerStorageBeaconInfo_t last_customerStorageBeaconInfo;

// GAP GATT Attributes
//static uint8_t attDeviceName[GAP_DEVICE_NAME_LEN] = "jhbeacon";

// Globals used for ATT Response retransmission
static gattMsgEvent_t *pAttRsp = NULL;
static uint8_t rspTxRetry = 0;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void SimpleBLEPeripheral_init( void );
static void SimpleBLEPeripheral_taskFxn(UArg a0, UArg a1);

static uint8_t SimpleBLEPeripheral_processStackMsg(ICall_Hdr *pMsg);
static uint8_t SimpleBLEPeripheral_processGATTMsg(gattMsgEvent_t *pMsg);
static void SimpleBLEPeripheral_processAppMsg(sbpEvt_t *pMsg);
static void SimpleBLEPeripheral_processStateChangeEvt(
                                                     gaprole_States_t newState);
static void SimpleBLEPeripheral_processCharValueChangeEvt(uint8_t paramID);
static void SimpleBLEPeripheral_performPeriodicTask(void);

static void SimpleBLEPeripheral_sendAttRsp(void);
static void SimpleBLEPeripheral_freeAttRsp(uint8_t status);

static void SimpleBLEPeripheral_stateChangeCB(gaprole_States_t newState);
#ifndef FEATURE_OAD
static void SimpleBLEPeripheral_charValueChangeCB(uint8_t paramID);
#endif //!FEATURE_OAD
static void SimpleBLEPeripheral_enqueueMsg(uint8_t event, uint8_t state);

#ifdef FEATURE_OAD
void SimpleBLEPeripheral_processOadWriteCB(uint8_t event, uint16_t connHandle,
                                           uint8_t *pData);
#endif //FEATURE_OAD

//��������Ļص������ʹ�������
static void BeaconProfileCharChangeCB(uint8_t paramID);
static void BeaconProfile_processCharValueChangeEvt(uint8_t paramID);

//���崮�ڼ���¼��Ĵ�������
static void SimpleBLEPeripheral_UART_performBEGPTask(void);

static void SimpleBLEPeripheral_enableUARTMonitoringEvtCB(void);




static void SimpleBLEPeripheral_clockHandler(UArg arg);

//�������½ӿ�
static void update_adv_beacon_data();
static void update_scanrsp_data();




/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t SimpleBLEPeripheral_gapRoleCBs =
{
  SimpleBLEPeripheral_stateChangeCB     // Profile State Change Callbacks
};

// GAP Bond Manager Callbacks
static gapBondCBs_t simpleBLEPeripheral_BondMgrCBs =
{
  NULL, // Passcode callback (not used by application)
  NULL  // Pairing / Bonding state Callback (not used by application)
};

// Simple GATT Profile Callbacks
#ifndef FEATURE_OAD
static simpleProfileCBs_t SimpleBLEPeripheral_simpleProfileCBs =
{
  SimpleBLEPeripheral_charValueChangeCB // Characteristic value change callback
};
//ע��ص�����
static beaconConfigProfileCBs_t SimpleBLEPeripheral_BeaconProfileCBs =
{
  BeaconProfileCharChangeCB // Characteristic value change callback
};

#endif //!FEATURE_OAD

#ifdef FEATURE_OAD
static oadTargetCBs_t simpleBLEPeripheral_oadCBs =
{
  SimpleBLEPeripheral_processOadWriteCB // Write Callback.
};
#endif //FEATURE_OAD



static void SimpleBLEPeripheral_enableUARTMonitoringEvtCB(void)
{
  // //������ʱ��
  Util_startClock(&UartMonitoringClock);  
}


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleBLEPeripheral_createTask
 *
 * @brief   Task creation function for the Simple BLE Peripheral.
 *
 * @param   None.
 *
 * @return  None.
 */
void SimpleBLEPeripheral_createTask(void)
{
  Task_Params taskParams;

  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = sbpTaskStack;
  taskParams.stackSize = SBP_TASK_STACK_SIZE;
  taskParams.priority = SBP_TASK_PRIORITY;

  //Task_create(): statically .
  //Task_construct():and those created dynamically .
  Task_construct(&sbpTask, SimpleBLEPeripheral_taskFxn, &taskParams, NULL);
}

static void update_adv_beacon_data()
{
      //�趨�Զ���㲥����
    //�趨UUID
    VOID memcpy(&advertData[9], last_customerStorageBeaconInfo.uuid, ATT_UUID_SIZE);
    
    //�趨MAJOR
    advertData[25]= HI_UINT16(last_customerStorageBeaconInfo.major);
    advertData[26]= LO_UINT16(last_customerStorageBeaconInfo.major);
    //�趨MINOR
    advertData[27]= HI_UINT16(last_customerStorageBeaconInfo.minor);
    advertData[28]= LO_UINT16(last_customerStorageBeaconInfo.minor);
    //�趨RSSI
    advertData[29]= last_customerStorageBeaconInfo.rssi_one_meter;
  
}
static void update_scanrsp_data()
{
      //����Զ���ɨ���Ӧ����
    VOID memset(&scanRspData[2], 0x00, 8);//�ݶ��豸�������֧��8��assic�ַ�
    
    //�趨����
    VOID memcpy(&scanRspData[2], (const void*)last_customerStorageBeaconInfo.dev_name, strlen((const char *)last_customerStorageBeaconInfo.dev_name));
    //VOID memcpy(&scanRspData[2], SETTING_BEACON_NAME, sizeof(SETTING_BEACON_NAME) - 1);
    //�趨�㲥���
    scanRspData[12] = LO_UINT16((uint16)(last_customerStorageBeaconInfo.adv_interval_ms/0.625));
    scanRspData[13] = HI_UINT16((uint16)(last_customerStorageBeaconInfo.adv_interval_ms/0.625));
//    scanRspData[12] = LO_UINT16((uint16)(SETTING_BEACON_ADV_INTERVAL/0.625));
//    scanRspData[13] = HI_UINT16((uint16)(SETTING_BEACON_ADV_INTERVAL/0.625));;
    //�趨���书��
    //if(SETTING_BEACON_TX_POWER < LL_EXT_TX_POWER_0_DBM)//����
    if(last_customerStorageBeaconInfo.dev_tx_power < LL_EXT_TX_POWER_0_DBM)
    {
      scanRspData[22] = (last_customerStorageBeaconInfo.dev_tx_power*3 - 21);
      //scanRspData[22] = (SETTING_BEACON_TX_POWER*3 - 21);
    }
    else
    {
      scanRspData[22] = last_customerStorageBeaconInfo.dev_tx_power - HCI_EXT_TX_POWER_0_DBM;
      //scanRspData[22] = SETTING_BEACON_TX_POWER - HCI_EXT_TX_POWER_0_DBM;
    }
}


/*********************************************************************
 * @fn      SimpleBLEPeripheral_init
 *
 * @brief   Called during initialization and contains application
 *          specific initialization (ie. hardware initialization/setup,
 *          table initialization, power up notification, etc), and
 *          profile initialization/setup.
 *
 * @param   None.
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_init(void)
{
  // ******************************************************************
  // N0 STACK API CALLS CAN OCCUR BEFORE THIS CALL TO ICall_registerApp
  // ******************************************************************
  // Register the current thread as an ICall dispatcher application
  // so that the application can send and receive messages.
  ICall_registerApp(&selfEntity, &sem);

  // Hard code the BD Address till CC2650 board gets its own IEEE address
  //uint8 bdAddress[B_ADDR_LEN] = { 0xAD, 0xD0, 0x0A, 0xAD, 0xD0, 0x0A };
  //HCI_EXT_SetBDADDRCmd(bdAddress);

  // Set device's Sleep Clock Accuracy
  //HCI_EXT_SetSCACmd(500);
  
  // Create an RTOS queue for message from profile to be sent to app.
  appMsgQueue = Util_constructQueue(&appMsg);

  // Create one-shot clocks for internal periodic events.
  Util_constructClock(&periodicClock, SimpleBLEPeripheral_clockHandler,
                      SBP_PERIODIC_EVT_PERIOD, 0, false, SBP_PERIODIC_EVT);
  
  
//    //������ص����ɼ��¼�
//  Util_constructClock(&powerleavel_periodicClock, Mypoewer_clockHandler,
//                     POWERCHECK_PERIOD, 0, false, POWERCHECK_EVENT);
    
#ifndef SENSORTAG_HW
  Board_openLCD();
#endif //SENSORTAG_HW
  
#if SENSORTAG_HW
  // Setup SPI bus for serial flash and Devpack interface
  bspSpiOpen();
#endif //SENSORTAG_HW
  
//  //clear info
//  VOID memset(&test_customerStorageBeaconInfo, 0x00, sizeof(test_customerStorageBeaconInfo));
//  //get
//  get_customerInfo(&test_customerStorageBeaconInfo);
//  
//  //set
//  fill_customerInfo(&test_customerStorageBeaconInfo);
//  set_customerInfo(&test_customerStorageBeaconInfo);
  //get
//  VOID memset(&last_customerStorageBeaconInfo, 0x00, sizeof(last_customerStorageBeaconInfo));
//  fill_customerInfo_byDefault(&last_customerStorageBeaconInfo);
//  set_customerInfo(&last_customerStorageBeaconInfo);
  
  //��SNV�л�ȡ���µ�������Ϣ
  VOID memset(&last_customerStorageBeaconInfo, 0x00, sizeof(last_customerStorageBeaconInfo));
  get_customerInfo(&last_customerStorageBeaconInfo);
  if((last_customerStorageBeaconInfo.major ==0)
     && (last_customerStorageBeaconInfo.minor ==0)
     && (strlen((char const*)(last_customerStorageBeaconInfo.dev_name)) == 0))
  {//��һ���ϵ磬����ΪĬ��ֵ
    fill_customerInfo_byDefault(&last_customerStorageBeaconInfo);
    set_customerInfo(&last_customerStorageBeaconInfo);
  }
  
  
    //ע�ᴮ�ڽ������ݼ���¼�
    //�����ڶ������������˴�����Ȼ��ȴ�������-Util_startClock()
    //�˺��������趨��һ���������ӳ�ʱ�䣬�Լ�����������ʱ��
  Util_constructClock(&UartMonitoringClock, SimpleBLEPeripheral_clockHandler,
                    SBP_UART_RX_EVT_PERIOD, 0, false, SBP_UART_MONITORING_EVT);
  
    //��������,ע��ص�
  dev_uart_init(SimpleBLEPeripheral_enableUARTMonitoringEvtCB);
  
  
  

  //ע�ᶯ̬�㲥�¼�
  Util_constructClock(&AdvUpdateClock, SimpleBLEPeripheral_clockHandler,
                    SBP_ADV_UPDATE_EVT_PERIOD, 0, false, SBP_ADV_UPDATE_EVT);
  //ע�ᶯ̬ɨ���Ӧ�¼�
  Util_constructClock(&ScanRspUpdateClock, SimpleBLEPeripheral_clockHandler,
                    SBP_SCAN_RSP_UPDATE_EVT_PERIOD, 0, false, SBP_SCAN_RSP_UPDATE_EVT);
  
  // Setup the GAP
  GAP_SetParamValue(TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL);

  // Setup the GAP Peripheral Role Profile
  {
    // For all hardware platforms, device starts advertising upon initialization
    uint8_t initialAdvertEnable = TRUE;

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16_t advertOffTime = 0;

    uint8_t enableUpdateRequest = DEFAULT_ENABLE_UPDATE_REQUEST;//�������Ӹ��²���
    uint16_t desiredMinInterval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16_t desiredMaxInterval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16_t desiredSlaveLatency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16_t desiredConnTimeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    //�򿪹㲥
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t),
                         &initialAdvertEnable);
    GAPRole_SetParameter(GAPROLE_ADVERT_OFF_TIME, sizeof(uint16_t),
                         &advertOffTime);
    
    update_scanrsp_data();
    GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(scanRspData),//����ɨ���Ӧ������
                         scanRspData);

    update_adv_beacon_data();
    GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData),
                         advertData);//�趨�㲥������

    //uint8_t advType = GAP_ADTYPE_ADV_SCAN_IND; //use scannable undirected adv
    uint8_t advType = GAP_ADTYPE_ADV_IND;
    GAPRole_SetParameter(GAPROLE_ADV_EVENT_TYPE, sizeof(uint8_t), &advType);
    
    
    GAPRole_SetParameter(GAPROLE_PARAM_UPDATE_ENABLE, sizeof(uint8_t),
                         &enableUpdateRequest);
    GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL, sizeof(uint16_t),
                         &desiredMinInterval);
    GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL, sizeof(uint16_t),
                         &desiredMaxInterval);
    GAPRole_SetParameter(GAPROLE_SLAVE_LATENCY, sizeof(uint16_t),
                         &desiredSlaveLatency);
    GAPRole_SetParameter(GAPROLE_TIMEOUT_MULTIPLIER, sizeof(uint16_t),
                         &desiredConnTimeout);
  }

  // Set the GAP Characteristics
  
   //1.���豸�趨����
   GGS_SetParameter(GGS_DEVICE_NAME_ATT, strlen((const char *)last_customerStorageBeaconInfo.dev_name), last_customerStorageBeaconInfo.dev_name);

  //2.���豸�趨���书��
  //���÷��书�ʣ�5dbm
  {
    HCI_EXT_SetTxPowerCmd(last_customerStorageBeaconInfo.dev_tx_power);

  }
  
  // Set advertising interval
  {
    //3.���豸�趨�㲥���
    uint16_t advInt = (uint16_t)((float)(last_customerStorageBeaconInfo.adv_interval_ms)/0.625);

    GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MIN, advInt);
    GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MAX, advInt);
    GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MIN, advInt);
    GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MAX, advInt);
  }

  // Setup the GAP Bond Manager
  {
    uint32_t passkey = 0; // passkey "000000"
    uint8_t pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8_t mitm = TRUE;
    uint8_t ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
    uint8_t bonding = TRUE;

    GAPBondMgr_SetParameter(GAPBOND_DEFAULT_PASSCODE, sizeof(uint32_t),
                            &passkey);
    GAPBondMgr_SetParameter(GAPBOND_PAIRING_MODE, sizeof(uint8_t), &pairMode);
    GAPBondMgr_SetParameter(GAPBOND_MITM_PROTECTION, sizeof(uint8_t), &mitm);
    GAPBondMgr_SetParameter(GAPBOND_IO_CAPABILITIES, sizeof(uint8_t), &ioCap);
    GAPBondMgr_SetParameter(GAPBOND_BONDING_ENABLED, sizeof(uint8_t), &bonding);
  }

   // Initialize GATT attributes
  GGS_AddService(GATT_ALL_SERVICES);           // GAP
  GATTServApp_AddService(GATT_ALL_SERVICES);   // GATT attributes
  //DevInfo_AddService();                        // Device Information Service

#ifndef FEATURE_OAD
  BeaconConfigProfile_AddService(GATT_ALL_SERVICES);//add beacon service
  //SimpleProfile_AddService(GATT_ALL_SERVICES); // Simple GATT Profile
#endif //!FEATURE_OAD

#ifdef FEATURE_OAD
  VOID OAD_addService();                 // OAD Profile
  OAD_register((oadTargetCBs_t *)&simpleBLEPeripheral_oadCBs);
  hOadQ = Util_constructQueue(&oadQ);
#endif

#ifdef IMAGE_INVALIDATE
  Reset_addService();
#endif //IMAGE_INVALIDATE
  
  
#ifndef FEATURE_OAD
  
  //init characteristic value
  uint8_t BeaconProfile_char1value[21] = { 1, 2, 3, 4, 5 };
  
  BeaconConfigProfile_SetParameter(BEACONCONFIGPROFILE_CHAR1, sizeof(uint8_t),
                                   &BeaconProfile_char1value[0]);
  BeaconConfigProfile_SetParameter(BEACONCONFIGPROFILE_CHAR2, sizeof(uint8_t),
                                   &BeaconProfile_char1value[1]);
  BeaconConfigProfile_SetParameter(BEACONCONFIGPROFILE_CHAR3, sizeof(uint8_t),
                             &BeaconProfile_char1value[2]);
  BeaconConfigProfile_SetParameter(BEACONCONFIGPROFILE_CHAR4, sizeof(uint8_t),
                             &BeaconProfile_char1value[3]);
  BeaconConfigProfile_SetParameter(BEACONCONFIGPROFILE_CHAR5, sizeof(uint8_t),
                             &BeaconProfile_char1value[4]);
    
  //Register callback with BeaconConfigProfile
  BeaconConfigProfile_RegisterAppCBs(&SimpleBLEPeripheral_BeaconProfileCBs);   
  
//  // Setup the SimpleProfile Characteristic Values
//  {
//    uint8_t charValue1 = 9;
//    uint8_t charValue2 = 2;
//    uint8_t charValue3 = 3;
//    uint8_t charValue4 = 4;
//    uint8_t charValue5[SIMPLEPROFILE_CHAR5_LEN] = { 1, 2, 3, 4, 5 };
//
//    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR1, sizeof(uint8_t),
//                               &charValue1);
//    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR2, sizeof(uint8_t),
//                               &charValue2);
//    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR3, sizeof(uint8_t),
//                               &charValue3);
//    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR4, sizeof(uint8_t),
//                               &charValue4);
//    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR5, SIMPLEPROFILE_CHAR5_LEN,
//                               charValue5);
//  }
//
//  // Register callback with SimpleGATTprofile
//  SimpleProfile_RegisterAppCBs(&SimpleBLEPeripheral_simpleProfileCBs);
#endif //!FEATURE_OAD
  
  // Start the Device
  VOID GAPRole_StartDevice(&SimpleBLEPeripheral_gapRoleCBs);
  
  // Start Bond Manager
  VOID GAPBondMgr_Register(&simpleBLEPeripheral_BondMgrCBs);

  // Register with GAP for HCI/Host messages
  GAP_RegisterForMsgs(selfEntity);
  
  // Register for GATT local events and ATT Responses pending for transmission
  GATT_RegisterForMsgs(selfEntity);
  
#if defined FEATURE_OAD
#if defined (HAL_IMAGE_A)
  LCD_WRITE_STRING("BLE Peripheral A", LCD_PAGE0);
#else
  LCD_WRITE_STRING("BLE Peripheral B", LCD_PAGE0);
#endif // HAL_IMAGE_A
#else
  LCD_WRITE_STRING("BLE Peripheral", LCD_PAGE0);
#endif // FEATURE_OAD
}       

/*********************************************************************
 * @fn      SimpleBLEPeripheral_taskFxn
 *
 * @brief   Application task entry point for the Simple BLE Peripheral.
 *
 * @param   a0, a1 - not used.
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_taskFxn(UArg a0, UArg a1)
{
  //������ȡ�Ƿ�ջ���
  Task_Stat statbuf;
  
  // Initialize application
  SimpleBLEPeripheral_init();

  // Application main loop
  for (;;)
  {
    // Waits for a signal to the semaphore associated with the calling thread.
    // Note that the semaphore associated with a thread is signaled when a
    // message is queued to the message receive queue of the thread or when
    // ICall_signal() function is called onto the semaphore.
    ICall_Errno errno = ICall_wait(ICALL_TIMEOUT_FOREVER);

    if (errno == ICALL_ERRNO_SUCCESS)
    {
      ICall_EntityID dest;
      ICall_ServiceEnum src;
      ICall_HciExtEvt *pMsg = NULL;

      if (ICall_fetchServiceMsg(&src, &dest,
                                (void **)&pMsg) == ICALL_ERRNO_SUCCESS)
      {
        uint8 safeToDealloc = TRUE;
        
        if ((src == ICALL_SERVICE_CLASS_BLE) && (dest == selfEntity))
        {
          ICall_Event *pEvt = (ICall_Event *)pMsg;
          
          // Check for BLE stack events first
          if (pEvt->signature == 0xffff)
          {
            if (pEvt->event_flag & SBP_CONN_EVT_END_EVT)
            {
              // Try to retransmit pending ATT Response (if any)
              SimpleBLEPeripheral_sendAttRsp();
            }
          }
          else
          {
            // Process inter-task message
            safeToDealloc = SimpleBLEPeripheral_processStackMsg(
                                                             (ICall_Hdr *)pMsg);
          }
        }

        if (pMsg && safeToDealloc)
        {
          ICall_freeMsg(pMsg);
        }
      }

      // If RTOS queue is not empty, process app message.
      while (!Queue_empty(appMsgQueue))
      {
        sbpEvt_t *pMsg = (sbpEvt_t *)Util_dequeueMsg(appMsgQueue);
        if (pMsg)
        {
          // Process message.
          SimpleBLEPeripheral_processAppMsg(pMsg);

          // Free the space from the message.
          ICall_free(pMsg);
        }
      }
    }

    if (events & SBP_PERIODIC_EVT)
    {
      events &= ~SBP_PERIODIC_EVT;

      Util_startClock(&periodicClock);

      // Perform periodic application task
      SimpleBLEPeripheral_performPeriodicTask();
    }
    
    
    if (events & SBP_UART_MONITORING_EVT)//ȷ���Զ���Ĵ��ڼ���¼�����
    {
      events &= ~SBP_UART_MONITORING_EVT;//clear event    

      // Perform periodic application task
      SimpleBLEPeripheral_UART_performBEGPTask();
      
      Task_stat(Task_self(), &statbuf); /* call func to get status */
      if (statbuf.used > (statbuf.stackSize * 9 / 10)) {
      //System_printf("Over 90% of task's stack is in use.\n");
      }
    }
    
     if (events & SBP_ADV_UPDATE_EVT)//��̬�㲥�¼�
    {
      events &= ~SBP_ADV_UPDATE_EVT;
      
       // For all hardware platforms, device starts advertising upon initialization
       uint8_t initialAdvertEnable = FALSE;
       
       //�رչ㲥
       GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t),
                         &initialAdvertEnable);
      
      // Perform periodic application task       
      //��SNV�л�ȡ���µ�������Ϣ
      get_customerInfo(&last_customerStorageBeaconInfo);
      update_adv_beacon_data(); 
       
      GAP_UpdateAdvertisingData(selfEntity,
                                TRUE,
                                sizeof(advertData),
                                advertData);      //���¹㲥���� 
        
      initialAdvertEnable = TRUE;
       //�򿪹㲥
       GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t),
                         &initialAdvertEnable);
    }
    
     if (events & SBP_SCAN_RSP_UPDATE_EVT)//��̬ɨ���Ӧ�¼�
    {
      events &= ~SBP_SCAN_RSP_UPDATE_EVT;
      
       // For all hardware platforms, device starts advertising upon initialization
       uint8_t initialAdvertEnable = FALSE;
       
       //�رչ㲥
       GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t),
                         &initialAdvertEnable);
      // Perform periodic application task
      //��SNV�л�ȡ���µ�������Ϣ
      get_customerInfo(&last_customerStorageBeaconInfo);
      update_scanrsp_data(); 
       
      GAP_UpdateAdvertisingData(selfEntity,
                          FALSE,
                          sizeof(scanRspData),
                          scanRspData);      //����ɨ���Ӧ���� 
   
       initialAdvertEnable = TRUE;
       //�򿪹㲥
       GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t),
                         &initialAdvertEnable);
    }
    

    
#ifdef FEATURE_OAD
    while (!Queue_empty(hOadQ))
    {
      oadTargetWrite_t *oadWriteEvt = Queue_dequeue(hOadQ);

      // Identify new image.
      if (oadWriteEvt->event == OAD_WRITE_IDENTIFY_REQ)
      {
        OAD_imgIdentifyWrite(oadWriteEvt->connHandle, oadWriteEvt->pData);
      }
      // Write a next block request.
      else if (oadWriteEvt->event == OAD_WRITE_BLOCK_REQ)
      {
        OAD_imgBlockWrite(oadWriteEvt->connHandle, oadWriteEvt->pData);
      }

      // Free buffer.
      ICall_free(oadWriteEvt);
    }
#endif //FEATURE_OAD
  }
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_processStackMsg
 *
 * @brief   Process an incoming stack message.
 *
 * @param   pMsg - message to process
 *
 * @return  TRUE if safe to deallocate incoming message, FALSE otherwise.
 */
static uint8_t SimpleBLEPeripheral_processStackMsg(ICall_Hdr *pMsg)
{
  uint8_t safeToDealloc = TRUE;
    
  switch (pMsg->event)
  {
    case GATT_MSG_EVENT:
      // Process GATT message
      safeToDealloc = SimpleBLEPeripheral_processGATTMsg(
                                                        (gattMsgEvent_t *)pMsg);
      break;

    case HCI_GAP_EVENT_EVENT:
      {
        // Process HCI message
        switch(pMsg->status)
        {
          case HCI_COMMAND_COMPLETE_EVENT_CODE:
            // Process HCI Command Complete Event
            break;
            
          default:
            break;
        }
      }
      break;
      
    default:
      // do nothing
      break;
  }
  
  return (safeToDealloc);
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_processGATTMsg
 *
 * @brief   Process GATT messages and events.
 *
 * @return  TRUE if safe to deallocate incoming message, FALSE otherwise.
 */
static uint8_t SimpleBLEPeripheral_processGATTMsg(gattMsgEvent_t *pMsg)
{
  // See if GATT server was unable to transmit an ATT response
  if (pMsg->hdr.status == blePending)
  {
    // No HCI buffer was available. Let's try to retransmit the response
    // on the next connection event.
    if (HCI_EXT_ConnEventNoticeCmd(pMsg->connHandle, selfEntity,
                                   SBP_CONN_EVT_END_EVT) == SUCCESS)
    {
      // First free any pending response
      SimpleBLEPeripheral_freeAttRsp(FAILURE);
      
      // Hold on to the response message for retransmission
      pAttRsp = pMsg;
      
      // Don't free the response message yet
      return (FALSE);
    }
  }
  else if (pMsg->method == ATT_FLOW_CTRL_VIOLATED_EVENT)
  {
    // ATT request-response or indication-confirmation flow control is
    // violated. All subsequent ATT requests or indications will be dropped.
    // The app is informed in case it wants to drop the connection.
    
    // Display the opcode of the message that caused the violation.
    LCD_WRITE_STRING_VALUE("FC Violated:", pMsg->msg.flowCtrlEvt.opcode,
                           10, LCD_PAGE5);
  }    
  else if (pMsg->method == ATT_MTU_UPDATED_EVENT)
  {
    // MTU size updated
    LCD_WRITE_STRING_VALUE("MTU Size:", pMsg->msg.mtuEvt.MTU, 10, LCD_PAGE5);
  }
  
  // Free message payload. Needed only for ATT Protocol messages
  GATT_bm_free(&pMsg->msg, pMsg->method);
  
  // It's safe to free the incoming message
  return (TRUE);
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_sendAttRsp
 *
 * @brief   Send a pending ATT response message.
 *
 * @param   none
 *
 * @return  none
 */
static void SimpleBLEPeripheral_sendAttRsp(void)
{
  // See if there's a pending ATT Response to be transmitted
  if (pAttRsp != NULL)
  {
    uint8_t status;
    
    // Increment retransmission count
    rspTxRetry++;
    
    // Try to retransmit ATT response till either we're successful or
    // the ATT Client times out (after 30s) and drops the connection.
    status = GATT_SendRsp(pAttRsp->connHandle, pAttRsp->method, 
                          &(pAttRsp->msg));
    if ((status != blePending) && (status != MSG_BUFFER_NOT_AVAIL))
    {
      // Disable connection event end notice
      HCI_EXT_ConnEventNoticeCmd(pAttRsp->connHandle, selfEntity, 0);
      
      // We're done with the response message
      SimpleBLEPeripheral_freeAttRsp(status);
    }
    else
    {
      // Continue retrying
      LCD_WRITE_STRING_VALUE("Rsp send retry:", rspTxRetry, 10, LCD_PAGE5);
    }
  }
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_freeAttRsp
 *
 * @brief   Free ATT response message.
 *
 * @param   status - response transmit status
 *
 * @return  none
 */
static void SimpleBLEPeripheral_freeAttRsp(uint8_t status)
{
  // See if there's a pending ATT response message
  if (pAttRsp != NULL)
  {
    // See if the response was sent out successfully
    if (status == SUCCESS)
    {
      LCD_WRITE_STRING_VALUE("Rsp sent, retry:", rspTxRetry, 10, LCD_PAGE5);
    }
    else
    {
      // Free response payload
      GATT_bm_free(&pAttRsp->msg, pAttRsp->method);
      
      LCD_WRITE_STRING_VALUE("Rsp retry failed:", rspTxRetry, 10, LCD_PAGE5);
    }
    
    // Free response message
    ICall_freeMsg(pAttRsp);
    
    // Reset our globals
    pAttRsp = NULL;
    rspTxRetry = 0;
  }
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_processAppMsg
 *
 * @brief   Process an incoming callback from a profile.
 *
 * @param   pMsg - message to process
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_processAppMsg(sbpEvt_t *pMsg)
{
  switch (pMsg->hdr.event)
  {
    case SBP_STATE_CHANGE_EVT:
      SimpleBLEPeripheral_processStateChangeEvt((gaprole_States_t)pMsg->
                                                hdr.state);
      break;

    case SBP_CHAR_CHANGE_EVT:
      SimpleBLEPeripheral_processCharValueChangeEvt(pMsg->hdr.state);
      break;
    
    //���ӷ������¼��Ĵ�������  
    case SBP_BEACON_CHAR_CHANGE_EVT:
      
      BeaconProfile_processCharValueChangeEvt(pMsg->hdr.state);
      
      break;

    default:
      // Do nothing.
      break;
  }
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_stateChangeCB
 *
 * @brief   Callback from GAP Role indicating a role state change.
 *
 * @param   newState - new state
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_stateChangeCB(gaprole_States_t newState)
{
  SimpleBLEPeripheral_enqueueMsg(SBP_STATE_CHANGE_EVT, newState);
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_processStateChangeEvt
 *
 * @brief   Process a pending GAP Role state change event.
 *
 * @param   newState - new state
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_processStateChangeEvt(gaprole_States_t newState)
{
#ifdef PLUS_BROADCASTER
  static bool firstConnFlag = false;
#endif // PLUS_BROADCASTER

  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8_t ownAddress[B_ADDR_LEN];
        uint8_t systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, 
                             systemId);

        // Display device address
        LCD_WRITE_STRING(Util_convertBdAddr2Str(ownAddress), LCD_PAGE1);
        LCD_WRITE_STRING("Initialized", LCD_PAGE2);
      }
      break;

    case GAPROLE_ADVERTISING:
      LCD_WRITE_STRING("Advertising", LCD_PAGE2);
      break;

#ifdef PLUS_BROADCASTER   
    /* After a connection is dropped a device in PLUS_BROADCASTER will continue
     * sending non-connectable advertisements and shall sending this change of 
     * state to the application.  These are then disabled here so that sending 
     * connectable advertisements can resume.
     */
    case GAPROLE_ADVERTISING_NONCONN:
      {
        uint8_t advertEnabled = FALSE;
      
        // Disable non-connectable advertising.
        GAPRole_SetParameter(GAPROLE_ADV_NONCONN_ENABLED, sizeof(uint8_t),
                           &advertEnabled);
      
        advertEnabled = TRUE;
      
        // Enabled connectable advertising.
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t),
                             &advertEnabled);
        
        // Reset flag for next connection.
        firstConnFlag = false;
        
        SimpleBLEPeripheral_freeAttRsp(bleNotConnected);
      }
      break;
#endif //PLUS_BROADCASTER   

    case GAPROLE_CONNECTED:
      {
        uint8_t peerAddress[B_ADDR_LEN];

        GAPRole_GetParameter(GAPROLE_CONN_BD_ADDR, peerAddress);

        Util_startClock(&periodicClock);
        
        LCD_WRITE_STRING("Connected", LCD_PAGE2);
        LCD_WRITE_STRING(Util_convertBdAddr2Str(peerAddress), LCD_PAGE3);

        #ifdef PLUS_BROADCASTER
          // Only turn advertising on for this state when we first connect
          // otherwise, when we go from connected_advertising back to this state
          // we will be turning advertising back on.
          if (firstConnFlag == false)
          {
            uint8_t advertEnabled = FALSE; // Turn on Advertising

            // Disable connectable advertising.
            GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t),
                                 &advertEnabled);
            
            // Set to true for non-connectabel advertising.
            advertEnabled = TRUE;

            // Enable non-connectable advertising.
            GAPRole_SetParameter(GAPROLE_ADV_NONCONN_ENABLED, sizeof(uint8_t),
                                 &advertEnabled);
            firstConnFlag = true;
          }
        #endif // PLUS_BROADCASTER
      }
      break;

    case GAPROLE_CONNECTED_ADV:
      LCD_WRITE_STRING("Connected Advertising", LCD_PAGE2);
      break;

    case GAPROLE_WAITING:
      Util_stopClock(&periodicClock);
      
      SimpleBLEPeripheral_freeAttRsp(bleNotConnected);

      LCD_WRITE_STRING("Disconnected", LCD_PAGE2);

      // Clear remaining lines
      LCD_WRITE_STRING("", LCD_PAGE3);
      LCD_WRITE_STRING("", LCD_PAGE4);
      LCD_WRITE_STRING("", LCD_PAGE5);
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      SimpleBLEPeripheral_freeAttRsp(bleNotConnected);
      
      LCD_WRITE_STRING("Timed Out", LCD_PAGE2);
      
      // Clear remaining lines
      LCD_WRITE_STRING("", LCD_PAGE3);
      LCD_WRITE_STRING("", LCD_PAGE4);
      LCD_WRITE_STRING("", LCD_PAGE5);

      #ifdef PLUS_BROADCASTER
        // Reset flag for next connection.
        firstConnFlag = false;
      #endif //#ifdef (PLUS_BROADCASTER)
      break;

    case GAPROLE_ERROR:
      LCD_WRITE_STRING("Error", LCD_PAGE2);
      break;

    default:
      LCD_WRITE_STRING("", LCD_PAGE2);
      break;
  }

  // Update the state
  //gapProfileState = newState;
}

extern volatile   bool                    BEGP_beacon_basicUpdate_flag;
extern volatile   bool                    BEGP_beacon_additionalUpdate_flag;
static void SimpleBLEPeripheral_UART_performBEGPTask(void)
{
  BEGP_parse_task();
  
  if(BEGP_beacon_basicUpdate_flag == true)
  {
    BEGP_beacon_basicUpdate_flag = false;//clear flag
      
     //������ʱ��
    Util_startClock(&AdvUpdateClock);//post event
  }
  if(BEGP_beacon_additionalUpdate_flag == true)
  {
    BEGP_beacon_additionalUpdate_flag =false;//clear flag

     //������ʱ��
    Util_startClock(&ScanRspUpdateClock); //post event
  }
}


#ifndef FEATURE_OAD
//�������Ļص�����
static void BeaconProfileCharChangeCB(uint8_t paramID)
{
  //����ע����¼�
  SimpleBLEPeripheral_enqueueMsg(SBP_BEACON_CHAR_CHANGE_EVT, paramID);
}
//�������Ĵ�������

static void BeaconProfile_processCharValueChangeEvt(uint8_t paramID)
{
  uint8_t newValue[30]={0};

  switch(paramID)
  {
    case BEACONCONFIGPROFILE_CHAR2:
      
      BeaconConfigProfile_GetParameter(BEACONCONFIGPROFILE_CHAR2, &newValue);
     

      break;


    default:
      // should not reach here!
      break;
  }
  

}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_charValueChangeCB
 *
 * @brief   Callback from Simple Profile indicating a characteristic
 *          value change.
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_charValueChangeCB(uint8_t paramID)
{
  SimpleBLEPeripheral_enqueueMsg(SBP_CHAR_CHANGE_EVT, paramID);
}
#endif //!FEATURE_OAD

/*********************************************************************
 * @fn      SimpleBLEPeripheral_processCharValueChangeEvt
 *
 * @brief   Process a pending Simple Profile characteristic value change
 *          event.
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_processCharValueChangeEvt(uint8_t paramID)
{
#ifndef FEATURE_OAD
  uint8_t newValue;

  switch(paramID)
  {
    case SIMPLEPROFILE_CHAR1:
      SimpleProfile_GetParameter(SIMPLEPROFILE_CHAR1, &newValue);

      LCD_WRITE_STRING_VALUE("Char 1:", (uint16_t)newValue, 10, LCD_PAGE4);
      break;

    case SIMPLEPROFILE_CHAR3:
      SimpleProfile_GetParameter(SIMPLEPROFILE_CHAR3, &newValue);

      LCD_WRITE_STRING_VALUE("Char 3:", (uint16_t)newValue, 10, LCD_PAGE4);
      break;

    default:
      // should not reach here!
      break;
  }
#endif //!FEATURE_OAD
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_performPeriodicTask
 *
 * @brief   Perform a periodic application task. This function gets called
 *          every five seconds (SBP_PERIODIC_EVT_PERIOD). In this example,
 *          the value of the third characteristic in the SimpleGATTProfile
 *          service is retrieved from the profile, and then copied into the
 *          value of the the fourth characteristic.
 *
 * @param   None.
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_performPeriodicTask(void)
{
#ifndef FEATURE_OAD
  uint8_t valueToCopy;

  // Call to retrieve the value of the third characteristic in the profile
  if (SimpleProfile_GetParameter(SIMPLEPROFILE_CHAR3, &valueToCopy) == SUCCESS)
  {
    // Call to set that value of the fourth characteristic in the profile.
    // Note that if notifications of the fourth characteristic have been
    // enabled by a GATT client device, then a notification will be sent
    // every time this function is called.
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR4, sizeof(uint8_t),
                               &valueToCopy);
  }
#endif //!FEATURE_OAD
}


#if defined(FEATURE_OAD)
/*********************************************************************
 * @fn      SimpleBLEPeripheral_processOadWriteCB
 *
 * @brief   Process a write request to the OAD profile.
 *
 * @param   event      - event type:
 *                       OAD_WRITE_IDENTIFY_REQ
 *                       OAD_WRITE_BLOCK_REQ
 * @param   connHandle - the connection Handle this request is from.
 * @param   pData      - pointer to data for processing and/or storing.
 *
 * @return  None.
 */
void SimpleBLEPeripheral_processOadWriteCB(uint8_t event, uint16_t connHandle,
                                           uint8_t *pData)
{
  oadTargetWrite_t *oadWriteEvt = ICall_malloc( sizeof(oadTargetWrite_t) + \
                                             sizeof(uint8_t) * OAD_PACKET_SIZE);
  
  if ( oadWriteEvt != NULL )
  {
    oadWriteEvt->event = event;
    oadWriteEvt->connHandle = connHandle;
    
    oadWriteEvt->pData = (uint8_t *)(&oadWriteEvt->pData + 1);
    memcpy(oadWriteEvt->pData, pData, OAD_PACKET_SIZE);

    Queue_enqueue(hOadQ, (Queue_Elem *)oadWriteEvt);
    
    // Post the application's semaphore.
    Semaphore_post(sem);
  }
  else
  {
    // Fail silently.
  }
}
#endif //FEATURE_OAD

/*********************************************************************
 * @fn      SimpleBLEPeripheral_clockHandler
 *
 * @brief   Handler function for clock timeouts.
 *
 * @param   arg - event type
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_clockHandler(UArg arg)
{
  // Store the event.
  events |= arg;

  // Wake up the application.
  Semaphore_post(sem);
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_enqueueMsg
 *
 * @brief   Creates a message and puts the message in RTOS queue.
 *
 * @param   event - message event.
 * @param   state - message state.
 *
 * @return  None.
 */
static void SimpleBLEPeripheral_enqueueMsg(uint8_t event, uint8_t state)
{
  sbpEvt_t *pMsg;

  // Create dynamic pointer to message.
  if ((pMsg = ICall_malloc(sizeof(sbpEvt_t))))
  {
    pMsg->hdr.event = event;
    pMsg->hdr.state = state;

    // Enqueue the message.
    Util_enqueueMsg(appMsgQueue, sem, (uint8*)pMsg);
  }
}


/*********************************************************************
*********************************************************************/
