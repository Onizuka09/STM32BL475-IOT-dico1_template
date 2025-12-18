/**
 ******************************************************************************
 * @file    lb_demo.c
 * @author  MCD Application Team
 * @brief   BLE Application
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2017 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "debug.h"

#include "ble_lib.h"
#include "blesvc.h"

#include "lb_demo.h"
#include "lb_server_app.h"

#include "scheduler.h"
#include "debug.h"
#include "tl_types.h"
#include "tl_ble_hci.h"
#include "lpm.h"
#include "tl_ble_reassembly.h"
#include "stm32l475e_iot01.h"

/* Private typedef -----------------------------------------------------------*/

/**
 * security parameters structure
 */ 
typedef struct _tSecurityParams
{
  /**
  * IO capability of the device
  */ 
  uint8_t ioCapability;
  
  /**
  * Authentication requirement of the device
  * Man In the Middle protection required?
  */ 
  uint8_t mitm_mode;
  
  /**
  * bonding mode of the device
  */ 
  uint8_t bonding_mode;
  
  /**
  * Flag to tell whether OOB data has 
  * to be used during the pairing process
  */ 
  uint8_t OOB_Data_Present; 
  
  /**
  * OOB data to be used in the pairing process if 
  * OOB_Data_Present is set to TRUE
  */ 
  uint8_t OOB_Data[16]; 
  
  /**
  * this variable indicates whether to use a fixed pin 
  * during the pairing process or a passkey has to be 
  * requested to the application during the pairing process
  * 0 implies use fixed pin and 1 implies request for passkey
  */ 
  uint8_t Use_Fixed_Pin; 
  
  /**
  * minimum encryption key size requirement
  */ 
  uint8_t encryptionKeySizeMin;
  
  /**
  * maximum encryption key size requirement
  */ 
  uint8_t encryptionKeySizeMax;
  
  /**
  * fixed pin to be used in the pairing process if
  * Use_Fixed_Pin is set to 1
  */ 
  uint32_t Fixed_Pin;
  
  /**
  * this flag indicates whether the host has to initiate
  * the security, wait for pairing or does not have any security
  * requirements.\n
  * 0x00 : no security required
  * 0x01 : host should initiate security by sending the slave security
  *        request command
  * 0x02 : host need not send the clave security request but it
  * has to wait for paiirng to complete before doing any other
  * processing
  */ 
  uint8_t initiateSecurity;
} tSecurityParams;

/**
 * global context
 * contains the variables common to all 
 * services
 */ 
typedef struct _tBLEProfileGlobalContext
{
  /**
   * security requirements of the host
   */ 
  tSecurityParams bleSecurityParam;
  
    /**
     * gap service handle
     */
  uint16_t gapServiceHandle;
  
  /**
   * device name characteristic handle
   */ 
  uint16_t devNameCharHandle;
  
  /**
   * appearance characteristic handle
   */ 
  uint16_t appearanceCharHandle;
  
  /**
   * connection handle of the current active connection
   * When not in connection, the handle is set to 0xFFFF
   */ 
    uint16_t connectionHandle[BLE_CFG_MAX_CONNECTION];

    /**
   * length of the UUID list to be used while advertising
   */ 
    uint8_t advtServUUIDlen;
  
  /**
   * the UUID list to be used while advertising
   */ 
    uint8_t advtServUUID[100];

}BleGlobalContext_t;

typedef struct
{
  BleGlobalContext_t LBRContext_legacy;
  LBR_Gap_Gatt_State Remote_Connection_Status[1];
  uint16_t connectionHandleRemote;
} LBRContext_t;

LBR_ConnHandle_Not_evt_t handleNotification;





static const char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'L','E','D','_','S','E','R','V','E','R'};

 



/* Private defines -----------------------------------------------------------*/

#define UNPACK_2_BYTE_PARAMETER(ptr)  \
        (uint16_t)((uint16_t)(*((uint8_t *)ptr))) |   \
        (uint16_t)((((uint16_t)(*((uint8_t *)ptr + 1))) << 8))

#define POOL_SIZE (CFG_TLBLE_EVT_QUEUE_LENGTH * ( sizeof(TL_PacketHeader_t) + TL_BLE_EVENT_FRAME_SIZE ))


/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
 * START of Section BLE_APP_CONTEXT
 */
static LBRContext_t LBRContext;

#if(LB_CLIENT!=0)
static LBR_ClientContext_t aLBRClientContext[BLE_CFG_CLT_MAX_NBR_CB];
#endif

/**
 * END of Section BLE_APP_CONTEXT
 */
uint16_t connection_handle;

/* Global variables ----------------------------------------------------------*/
/*******************************************************************************
 * START of Section BLE_STDBY_MEM
 */
static TL_CmdPacket_t CmdBuffer;

/*******************************************************************************
 * END of Section BLE_STDBY_MEM
 */


/*******************************************************************************
 * START of Section BLE_SHARED_NORET_MEM
 */
static uint8_t EvtPool[POOL_SIZE];

/*******************************************************************************
 * END of Section BLE_SHARED_NORET_MEM
 */

/* Private function prototypes -----------------------------------------------*/
//static void Add_Advertisment_Service_UUID(uint16_t servUUID);

/* Functions Definition ------------------------------------------------------*/
__weak void TL_Enable( void );

/* Private functions ----------------------------------------------------------*/


/* Public functions ----------------------------------------------------------*/

void LBR_Init(LBR_InitMode_t InitMode)
{
  uint8_t index;

  if(InitMode != LBR_Limited )
  {
    /**
     * Initialization of all transport layer
     */
    TL_BLE_HCI_Init(TL_BLE_HCI_InitFull, &CmdBuffer, EvtPool, POOL_SIZE);
    TL_Enable();

    /**
     * Initialization of the BLE Controller
     */
     SVCCTL_Init();

    /**
     * Initialization of the BLE App Context
     */
   
    LBRContext.Remote_Connection_Status[0] = LBC_IDLE;
    
   

   
    
    /**
     * Set TX Power to -2dBm.
     * This avoids undesired disconnection due to instability on 32KHz
     * internal oscillator for high transmission power.
     */
    aci_hal_set_tx_power_level(1,0x18);

    /**
     * Initialize IO capability
     */
    LBRContext.LBRContext_legacy.bleSecurityParam.ioCapability = CFG_LBR_IO_CAPABILITY;
    aci_gap_set_io_capability(LBRContext.LBRContext_legacy.bleSecurityParam.ioCapability);

    /**
     * Initialize authentication
     */
    LBRContext.LBRContext_legacy.bleSecurityParam.mitm_mode = CFG_MITM_PROTECTION_REQUIRED;
    LBRContext.LBRContext_legacy.bleSecurityParam.OOB_Data_Present = 0;
    LBRContext.LBRContext_legacy.bleSecurityParam.encryptionKeySizeMin = 8;
    LBRContext.LBRContext_legacy.bleSecurityParam.encryptionKeySizeMax = 16;
    LBRContext.LBRContext_legacy.bleSecurityParam.Use_Fixed_Pin = 1;
    LBRContext.LBRContext_legacy.bleSecurityParam.Fixed_Pin = 1234;
    LBRContext.LBRContext_legacy.bleSecurityParam.initiateSecurity = 0x01;
    LBRContext.LBRContext_legacy.bleSecurityParam.bonding_mode = 2;
    for (index=0; index<16 ;index++)
    {
      LBRContext.LBRContext_legacy.bleSecurityParam.OOB_Data[index] = (uint8_t)index;
    }

    aci_gap_set_auth_requirement(LBRContext.LBRContext_legacy.bleSecurityParam.mitm_mode,
                                 LBRContext.LBRContext_legacy.bleSecurityParam.OOB_Data_Present,
                                 LBRContext.LBRContext_legacy.bleSecurityParam.OOB_Data,
                                 LBRContext.LBRContext_legacy.bleSecurityParam.encryptionKeySizeMin,
                                 LBRContext.LBRContext_legacy.bleSecurityParam.encryptionKeySizeMax,
                                 LBRContext.LBRContext_legacy.bleSecurityParam.Use_Fixed_Pin,
                                 LBRContext.LBRContext_legacy.bleSecurityParam.Fixed_Pin,
                                 LBRContext.LBRContext_legacy.bleSecurityParam.bonding_mode);

    /**
     * Initialize whitelist
     */
    if(LBRContext.LBRContext_legacy.bleSecurityParam.bonding_mode)
    {
      aci_gap_configure_whitelist();
    }

    
    /**
     * Initialize LBS Applciation
     */
    LBSAPP_Init();
   // Start Advertise to be connected by Client
    SCH_SetTask(CFG_IdleTask_StartAdv); 

  
  }

  return;
}






/****************************************************************/
/*                                                              */
/*                     DEVICE IS SERVER                         */
/****************************************************************/

void LBR_Adv_Request(void)
{
  if(LBRContext.Remote_Connection_Status [0]!= LBC_CONNECTED )
  {
    tBleStatus result=0x00;
    
    result = aci_gap_set_discoverable(ADV_IND,
                             LEDBUTTON_CONN_ADV_INTERVAL_MIN,
                             LEDBUTTON_CONN_ADV_INTERVAL_MAX,
                             PUBLIC_ADDR,
                             NO_WHITE_LIST_USE, /* use white list */
                             sizeof(local_name),
                             local_name,
                             0,
                             NULL,
                             0,
                             0);
    /* Send Advertising data */
   
    if( result == BLE_STATUS_SUCCESS )  {
      APPL_MESG_DBG("  \r\n\r");
      APPL_MESG_DBG("** START ADVERTISING **  \r\n\r");

    }  
    else   
    {
      APPL_MESG_DBG("** START ADVERTISING **  Failed \r\n\r");
    }
  }
  return;
}

void SVCCTL_App_Notification(void *pckt)
{
  hci_event_pckt *event_pckt;
  evt_le_meta_event *meta_evt;
  evt_le_connection_complete * connection_complete_event;

  event_pckt = (hci_event_pckt*)((hci_uart_pckt *)pckt)->data;
  evt_disconn_complete *cc = (void *)event_pckt->data;
  switch(event_pckt->evt)
  {
    case EVT_DISCONN_COMPLETE:
      if(cc->handle == LBRContext.connectionHandleRemote)
      {
        LBRContext.connectionHandleRemote = LBC_IDLE;
        APPL_MESG_DBG("\r\n\r** DISCONNECTION EVENT WITH CLIENT \n");
        handleNotification.LBR_Evt_Opcode=LB_CLIENT_DISCON_EVT_EVT;
        handleNotification.ConnectionHandle=connection_handle;
        LBR_Notification(&handleNotification);
      
      }
      break; /* EVT_DISCONN_COMPLETE */

    case EVT_LE_META_EVENT:
      meta_evt = (evt_le_meta_event*)event_pckt->data;
      switch(meta_evt->subevent)
      {
        case EVT_LE_CONN_COMPLETE:
          /**
           * The connection is done, there is no need anymore to schedule the LP ADV
           */
          connection_complete_event = (evt_le_connection_complete *)meta_evt->data;
         
          connection_handle = connection_complete_event->handle;
      
          //CONNECTION WITH LB ROUTEUR 
           APPL_MESG_DBG("\r\n\r** CONNECTION EVENT WITH CLIENT \n");
          LBRContext.connectionHandleRemote = connection_handle;
          handleNotification.LBR_Evt_Opcode=LB_CLIENT_CONN_HANDLE_EVT;
          handleNotification.ConnectionHandle=connection_handle;
          LBR_Notification(&handleNotification);
         
          
         break; /* HCI_EVT_LE_CONN_COMPLETE */

        default:
          break;
      }
      break; /* HCI_EVT_LE_META_EVENT */

    default:
      break;
  }
  return;
}

void LBR_App_Key_Button_Action(void)
{
  SCH_SetTask(CFG_IdleTask_Button);
}


void BLESVC_InitCustomSvc(void)
{
  LBS_STM_Init();
  return;
}

