/******************************************************************************
 * @file     app.c
 *
 * @brief    for TLSR chips
 *
 * @author   public@telink-semi.com;
 * @date     Sep. 30, 2010
 *
 * @attention
 *
 *  Copyright (C) 2019-2020 Telink Semiconductor (Shanghai) Co., Ltd.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *****************************************************************************/

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "application/keyboard/keyboard.h"

#include "vendor/common/blt_led.h"
#include "vendor/common/blt_soft_timer.h"
#include "vendor/common/blt_common.h"

#include "app_ui.h"
#include "battery_check.h"
#include "rc_ir.h"
#include "app_audio.h"
#include "app_flash_write.h"
#include "application/audio/gl_audio.h"
#include "app_ota.h"
#include "app_ir.h"
#include "app_custom.h"
#include "app_config.h"
#include "app_flash_write.h"

#include "../../application/audio/gl_audio.h"


#if (MP_TEST_MODE)
#include "app_test.h"
#endif


#if (__PROJECT_8258_BLE_REMOTE__ || __PROJECT_8278_BLE_REMOTE__)


#define     ADV_INTERNAL_SWITCH_TIME            30   //30s
#define     ADV_IDLE_ENTER_DEEP_TIME            120  //120
#define     ADV_RECONN_ENTER_DEEP_COUNT         1
#define     ADV_IDLE_ENTER_DEEP_COUNT           5
#define     CONN_IDLE_ENTER_DEEP_TIME           30   //60 s

#define     MY_DIRECT_ADV_TMIE                  2000000


#define     MY_APP_ADV_CHANNEL                  BLT_ENABLE_ADV_ALL
#define     MY_ADV_INTERVAL_MIN                 ADV_INTERVAL_20MS
#define     MY_ADV_INTERVAL_MAX                 ADV_INTERVAL_25MS

#if (MCU_CORE_TYPE == MCU_CORE_8278)
    #define        MY_RF_POWER_INDEX            RF_POWER_P3p50dBm
#else
    #define        MY_RF_POWER_INDEX            RF_POWER_P3p01dBm
#endif

#define        BLE_DEVICE_ADDRESS_TYPE          BLE_DEVICE_ADDRESS_PUBLIC

_attribute_data_retention_    own_addr_type_t   app_own_address_type = OWN_ADDRESS_PUBLIC;



//#define RX_FIFO_SIZE            96                    //-24//64
//#define RX_FIFO_SIZE            160                    //-24//64
#define RX_FIFO_SIZE            160                    //-24//64
#define RX_FIFO_NUM                8

//#define TX_FIFO_SIZE            88                    //-12//40
#define TX_FIFO_SIZE            148                    //-12//40
#define TX_FIFO_NUM                16

//#define MTU_SIZE_SETTING        72
#define MTU_SIZE_SETTING         180
//#define MTU_SIZE_SETTING        210

#if 0
    MYFIFO_INIT(blt_rxfifo, RX_FIFO_SIZE, RX_FIFO_NUM);
#else
_attribute_data_retention_  u8              blt_rxfifo_b[RX_FIFO_SIZE * RX_FIFO_NUM] = {0};
_attribute_data_retention_    my_fifo_t    blt_rxfifo = {
                                                RX_FIFO_SIZE,
                                                RX_FIFO_NUM,
                                                0,
                                                0,
                                                blt_rxfifo_b,};
#endif


#if 0
    MYFIFO_INIT(blt_txfifo, TX_FIFO_SIZE, TX_FIFO_NUM);
#else
    _attribute_data_retention_  u8              blt_txfifo_b[TX_FIFO_SIZE * TX_FIFO_NUM] = {0};
    _attribute_data_retention_    my_fifo_t    blt_txfifo = {
                                                    TX_FIFO_SIZE,
                                                    TX_FIFO_NUM,
                                                    0,
                                                    0,
                                                    blt_txfifo_b,};
#endif


//u8 flag_mtu=0;
_attribute_data_retention_    u8 flag_dle=0;
_attribute_data_retention_    int device_in_connection_state=0;

_attribute_data_retention_    u32 advertise_begin_tick;
_attribute_data_retention_    u8 advertise_downgrade_flag=0;

_attribute_data_retention_    u8    sendTerminate_before_enterDeep = 0;

_attribute_data_retention_    u32    latest_user_event_tick;

_attribute_data_retention_    u32    lowBattDet_tick   = 0;

_attribute_data_retention_    u32    mtuExchange_check_tick=0;
_attribute_data_retention_    u32    mtuExchange_check_timer=12000000;


_attribute_data_retention_    u32    ir_flash_erase_tick=0;

_attribute_data_retention_    u32    delay3s_to_sleep_tick=0;

_attribute_data_retention_    u16    enterdeep_time_count = 0;
_attribute_data_retention_    u16    enterdeep_time = 0;
_attribute_data_retention_    u16    service_change = 0;

_attribute_data_retention_    u8  device_timeout_state=0;
_attribute_data_retention_    u8 flag_schedule_ota=0;
_attribute_data_retention_    u16 count_schedule_ota=0;

_attribute_data_retention_    u8 app_slave_terminate=0;
_attribute_data_retention_    u8 app_is_set_mtusize=0;


u8  mac_public[6];

extern u8 is_pairing_mode;
extern u8 is_reconn_mode;
extern void app_pairing_led(void);
extern ble_sts_t    blc_att_requestMtuSizeExchange (u16 connHandle, u16 mtu_size);
extern void is_flash_info_full(void);
extern void battery_refresh(void);
extern void device_led_off(u8 led);
extern void app_connect_led(void);
extern u8 my_devNamelen;
extern u8 my_devName[20];
extern u8 ir_flash_erase_flag;
extern void ir_flash_set_flag(u8 flag);
extern u8 init_ir_key_event_notify(void);

void app_enter_deep_timeset(void)
{
    enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
}

void advertise_tick_retime(void)
{
    advertise_begin_tick = clock_time();
    advertise_downgrade_flag = 0;
    enterdeep_time_count = 0;
}

//////////////////////////////////////////////////////////////////////////////
//     Adv Packet, Response Packet
//////////////////////////////////////////////////////////////////////////////
const u8    tbl_advData_g10_part2[] = {
     0x02, 0x01, 0x05,                          // BLE limited discoverable mode and BR/EDR not supported
     0x03, 0x19, 0x80, 0x01,                    // 384, Generic Remote Control, Generic category
     0x05, 0x02, 0x12, 0x18, 0x0F, 0x18,        // incomplete list of service class UUIDs (0x1812, 0x180F)
};

void app_set_advdata_rspdata(void)
{
    u8 tbl_scanRsp[30];
    u8 tbl_advData[40];
    u8 len=0;

    tbl_scanRsp[0] = my_devNamelen+1;
    tbl_scanRsp[1] = 0x09;
    memcpy(&tbl_scanRsp[2],my_devName,my_devNamelen);

    tbl_advData[0] = tbl_scanRsp[0];
    tbl_advData[1] = 0x09;
    memcpy(&tbl_advData[2],my_devName,my_devNamelen);
    len = 2 + my_devNamelen;

    memcpy(&tbl_advData[len],tbl_advData_g10_part2,sizeof(tbl_advData_g10_part2));
    len += sizeof(tbl_advData_g10_part2);

    bls_ll_setAdvData( (u8 *)tbl_advData, len);
    bls_ll_setScanRspData( (u8 *)tbl_scanRsp, my_devNamelen+2);

/*
    printf("adv data=\r\n");
    for(u8 i=0;i<len;i++)
    {
       printf(" %x ",tbl_advData[i]);
    }

    printf("\r\nscan data=\r\n");
    for(u8 i=0;i<my_devNamelen+2;i++)
    {
       printf(" %x ",tbl_scanRsp[i]);
    }
*/
}

void direct_adv(u8 e, u8 *p, int n)
{
    printf("direct_adv\r\n");
    is_reconn_mode = 0x55;
    bls_ll_setAdvEnable(1);  //must: set adv enable
}


void app_adv_interval_downgrade_direct(void)
{
    smp_param_save_t  bondInfo;
    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    //direct adv
    if(bond_number)   //at least 1 bonding device exist
    {
       printf("app_adv_internal_downgrade_direct\r\n");
       bls_smp_param_loadByIndex( bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )

       u8 status = bls_ll_setAdvParam( ADV_INTERVAL_30MS, ADV_INTERVAL_35MS,
                                        ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, app_own_address_type,
                                        bondInfo.peer_addr_type,  bondInfo.peer_addr,
                                        MY_APP_ADV_CHANNEL,
                                        ADV_FP_NONE);
       if(status != BLE_SUCCESS) {      while(1); }  //debug: adv setting err
    }
}

void app_adv_interval_downgrade_indirect(void)
{
    printf("app_adv_internal_downgrade_indirect\r\n");
    bls_ll_setAdvParam( ADV_INTERVAL_30MS, ADV_INTERVAL_35MS,
                        ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
                        0,  NULL,
                        MY_APP_ADV_CHANNEL,
                        ADV_FP_NONE);
}

typedef void (*app_adv_interval_downgradeCb_t)(void);

_attribute_data_retention_ app_adv_interval_downgradeCb_t     app_adv_interval_downgradeCb = NULL;
void app_adv_interval_downgrade_register(app_adv_interval_downgradeCb_t cb)
{
    app_adv_interval_downgradeCb = cb;
}


void app_set_adv_interval_downgrade_indirect(void)
{
    app_adv_interval_downgrade_register(app_adv_interval_downgrade_indirect);
}

void app_set_adv_interval_downgrade_direct(void)
{
    app_adv_interval_downgrade_register(app_adv_interval_downgrade_direct);
}

void app_adv_indirect(void)
{
    printf("app_adv_indirect\r\n");

    app_set_advdata_rspdata();

    bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
        ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
        0,    NULL,
        MY_APP_ADV_CHANNEL,
        ADV_FP_NONE);
    bls_ll_setAdvEnable(1);
    app_set_adv_interval_downgrade_indirect();
}

void app_adv_direct(void)
{
    printf("app_adv_direct\r\n");
    is_reconn_mode = 0x55;
    smp_param_save_t  bondInfo;
    bls_ll_setAdvEnable(0);  //adv enable
    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    if(bond_number)   //at least 1 bonding device exist
    {
       app_set_adv_interval_downgrade_direct();
       bls_smp_param_loadByIndex( bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )

       u8 status = bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
                                        ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, app_own_address_type,
                                        bondInfo.peer_addr_type,  bondInfo.peer_addr,
                                        MY_APP_ADV_CHANNEL,
                                        ADV_FP_NONE);
       if(status != BLE_SUCCESS) {      while(1); }  //debug: adv setting err
       bls_ll_setAdvEnable(1);  //adv enable
    }
    else
    {
        app_set_adv_interval_downgrade_indirect();
        bls_ll_setAdvEnable(1);  //adv enable
    }
}

u8 effective_rx_dle = 0;
void    task_datalengthexchange (u8 e, u8 *p, int n)
{
    flag_dle |= 1;
    printf("task_datalengthexchange\n");
    ll_data_extension_t *data_p = (ll_data_extension_t *)p;
    effective_rx_dle = data_p->connEffectiveMaxRxOctets;
    //printf("connEffective:rx:%d __ tx: %d\n",data_p->connEffectiveMaxRxOctets,data_p->connEffectiveMaxTxOctets);

    set_audio_frame_size(data_p->connMaxTxOctets);
    //set_audio_frame_size(23);
}

void app_service_change(void)
{
    u8 service_data[4]={BATT_PS_H,0,0xff,0};
    if(service_change == 0x55)
    {
        printf("app_service_change\r\n");
        bls_att_pushIndicateData(GenericAttribute_ServiceChanged_DP_H,service_data,sizeof(service_data));
        analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) & (~OTA_FLG));
        service_change = 0;
    }
}

void mtu_exchange_loop(void)
{
    if(mtuExchange_check_tick && clock_time_exceed(mtuExchange_check_tick, mtuExchange_check_timer ))
    { 
        mtuExchange_check_tick = 0;
        app_schedule_ota_send_notify_to_continue_ota();
        if((flag_dle&0x10) == 0)
        {
            printf("mtu_exchange_loop_mtu\r\n");
            blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, MTU_SIZE_SETTING);
        }
        if((flag_dle&0x10) == 0x10)
        {
            if(app_mtu_size < MTU_SIZE_SETTING)
            {
                printf("mtu_exchange_loop_mtu\r\n");
                blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, MTU_SIZE_SETTING);
            }
        }
        if((flag_dle&0x01) == 0)
        {
            printf("mtu_exchange_loop_dle\r\n");
            blc_ll_exchangeDataLength(LL_LENGTH_REQ,(TX_FIFO_SIZE-18));
        }
        if((flag_dle&0x80) == 0)
        {
            printf("mtu_exchange_loop_update conn parm\r\n");
            bls_l2cap_requestConnParamUpdate (8, 8, 99, 400);
        }
        //app_service_change();
    }
}

int app_enable_adv_timeout(void)
{
     printf("app_enable_adv_timeout\r\n");
     is_pairing_mode = 0x55;
     app_pairing_led();
     app_adv_indirect();
     enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
     if(google_voice_ctl){
         google_voice_ctl = 0;
     }
     if(ui_mic_enable){
        ui_enable_mic (0);
     }
     return -1;
}

ble_sts_t app_terminate(u8 reason)
{
    app_slave_terminate = 0x55;
    ble_sts_t status = bls_ll_terminateConnection(reason);
    return status;
}

int app_terminate_timeout(void)
{
     ble_sts_t status = app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN);
     printf("app_terminate_timeout status=%x\r\n",status);
     return -1;
}

void     ble_ll_reject(u8 e,u8 *p, int n) //*p is terminate reason
{
     printf("ble_ll_reject\r\n");
     if(device_in_connection_state == 0)
     {
         blt_soft_timer_delete(app_terminate_timeout);
         blt_soft_timer_add(app_enable_adv_timeout, 3000000);
         ble_sts_t status = app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN);
         printf("status=%x\r\n",status);
         if(google_voice_ctl & FLAG_GOOGLE_CAPS_RESP)
             google_voice_ctl &= ~FLAG_GOOGLE_CAPS_RESP;
     }
}

int app_ota_terminate_timeoutcb(void)
{
    printf("app_ota_terminate_timeoutcb\r\n");
    app_ota_terminate(OTA_USER_TERMINATE);

    return -1;
}

int app_terminate_to_direct_timeoutcb(void)
{
    printf("app_terminate_to_direct_timeoutcb\r\n");
    app_adv_direct();

    return -1;
}

void ble_remote_terminate(u8 e,u8 *p, int n) //*p is terminate reason
{

    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    printf("ble_remote_terminate  bond_number=%x,*p=%x\r\n",bond_number,*p);

    if(app_repairing() == 0)
    {
        if(ota_is_working)
        {
            app_pairing_error();
            blt_soft_timer_add(app_ota_terminate_timeoutcb, 1000000);
            return;
        }
        if(*p == HCI_ERR_CONN_TIMEOUT){
            printf("HCI_ERR_CONN_TIMEOUT\r\n");
            device_timeout_state=0x55;
            is_pairing_mode = 0;
            is_reconn_mode = 0;
            if((flag_schedule_ota == 0x55) || (bond_number == 0))
            {
                enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
                bls_ll_setAdvEnable(0);
            }
            else
            {
                enterdeep_time = ADV_RECONN_ENTER_DEEP_COUNT;
                app_adv_direct();
            }
        }
        else if(*p == HCI_ERR_REMOTE_USER_TERM_CONN){  //0x13

             printf("remote  user terminate\r\n");
             device_timeout_state=0x55;
             if(is_pairing_mode)
                 is_pairing_mode = 0;
             if(is_reconn_mode)
                 is_reconn_mode = 0;
             bls_ll_setAdvEnable(0);
             if(app_slave_terminate == 0x55)
             {
                  enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
                  app_slave_terminate = 0;
                  printf("slave terminate\r\n");
             }
             else
             {
                 if(device_in_connection_state == 0)
                 {
                     enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
                 }
                 else
                 {
                     if(flag_schedule_ota == 0x55)
                     {
                         enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
                         bls_ll_setAdvEnable(0);
                     }
                     else
                     {
                         u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
                         if(bond_number)
                         {
                              enterdeep_time = ADV_RECONN_ENTER_DEEP_COUNT;
                         }
                         else
                             enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
                         blt_soft_timer_add(app_terminate_to_direct_timeoutcb, 600000);
                     }
                 }
             }
        }
        else if(*p == HCI_ERR_CONN_TERM_MIC_FAILURE){
              is_pairing_mode = 0;
              is_reconn_mode = 0;
              app_pairing_error();
              enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
              bls_ll_setAdvEnable(0);
              printf("HCI_ERR_CONN_TERM_MIC_FAILURE\r\n");
        }
        google_voice_ctl = 0;
    }

    device_in_connection_state = 0;
    mtuExchange_check_tick = 0;
    flag_dle = 0;

#if (BLE_REMOTE_PM_ENABLE)
     //user has push terminate pkt to ble TX buffer before deepsleep
    if(sendTerminate_before_enterDeep == 1){
        sendTerminate_before_enterDeep = 2;
    }
#endif

#if (BLE_AUDIO_ENABLE)
    if(ui_mic_enable){
        ui_enable_mic (0);
    }
#endif

    advertise_tick_retime();
}


_attribute_ram_code_ void user_set_rf_power (u8 e, u8 *p, int n)
{
#if (MP_TEST_MODE)
    if (gpio_read(DUT_INPUT_PORT) && test_get_mode() == MODE_TEST)
        rf_set_power_level_index(RF_POWER_P7p37dBm);
    else
#endif
    rf_set_power_level_index (MY_RF_POWER_INDEX);
}


#define PEER_MAC_FLASH_ERASE_TIMEOUT 600000

int app_peer_mac_flash_erase_timeoutcb(void)
{
    printf("app_peer_mac_flash_erase_timeoutcb\r\n");

    if(bls_ll_requestConnBrxEventDisable() > 120)
    {
        printf("app_peer_mac_flash_erase \r\n");
        bls_ll_disableConnBrxEvent();
        flash_erase_sector(MAC_DATA_SECT_ADDR);
        bls_ll_restoreConnBrxEvent();
        printf("app_peer_mac_flash_erase succ\r\n");
        write_peer_mac_info(peer_mac);
        return -1;
    }

    return 0;
}

void app_peer_mac_flash_erase(void)
{
    blt_soft_timer_add(app_peer_mac_flash_erase_timeoutcb, PEER_MAC_FLASH_ERASE_TIMEOUT);
}


void app_save_peer_mac_info(u8 *mac)
{
/*
    if((peer_mac[0] == 0) && (peer_mac[1] == 0))
    {
        memcpy(peer_mac,mac,6);
        write_peer_mac_info(peer_mac);
    }
    else
    */
    {
        printf("peer_mac =\r\n");
        for(u8 i=0;i<6;i++)
            printf(" %x",mac[i]);
        if(memcmp(mac,peer_mac,6))
        {
            printf("new mac\r\n");
            memcpy(peer_mac,mac,6);
            ir_table_init();
            write_ir_key_event_notify(0);
            flag_ccc_data = 0;
            atv_char_ctl_ccc = 0;
            atv_char_rx_ccc = 0;
            ir_flash_erase_flag = 0x55;
            write_ccc_info(&flag_ccc_data);
            if(is_peer_mac_flash_info_full() == 0)
               write_peer_mac_info(peer_mac);
            else
                  app_peer_mac_flash_erase();
        }
    }
}

void app_cmp_mac(void)
{
    smp_param_save_t  bondInfo;

    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();
    if(bond_number)
    {
        printf("app_cmp_mac\r\n");
        bls_smp_param_loadByIndex(0, &bondInfo);
        app_save_peer_mac_info(bondInfo.peer_addr);
    }
}

int app_host_event_callback (u32 h, u8 *para, int n)
{

    u8 event = h & 0xFF;

    switch(event)
    {
        case GAP_EVT_SMP_PARING_BEAGIN:
        {
            printf("----- Pairing begin -----\n");
        }
        break;

        case GAP_EVT_SMP_PARING_SUCCESS:
        {
            gap_smp_paringSuccessEvt_t* p = (gap_smp_paringSuccessEvt_t*)para;
            printf("Pairing success:bond flg %s\n", p->bonding ?"true":"false");

            if(p->bonding_result){
                app_connect_led();
                app_cmp_mac();
                printf("save smp key succ\n");
            }
            else{
                printf("save smp key failed\n");
            }

        }
        break;

        case GAP_EVT_SMP_PARING_FAIL:
        {
            gap_smp_paringFailEvt_t* p = (gap_smp_paringFailEvt_t*)para;
            printf("----- Pairing failed:rsn:0x%x -----\n", p->reason);
            app_pairing_error();
        }
        break;

        case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
        {
            gap_smp_connEncDoneEvt_t* p = (gap_smp_connEncDoneEvt_t*)para;

            printf("----- Connection encryption done -----\n");

            blt_soft_timer_delete(app_terminate_timeout);

            if(p->re_connect == SMP_STANDARD_PAIR){  //first paring
                printf("first pairing\r\n");
                app_is_set_mtusize = 0;
                mtuExchange_check_timer = 12000000;
                if(ir_flash_erase_flag == 0x55)
                {
                    ir_flash_set_flag(0);
                    ir_flash_erase_flag = 0;
                }
            }
            else if(p->re_connect == SMP_FAST_CONNECT){  //auto connect
                if(app_mtu_size < MTU_SIZE_SETTING)
                    blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, MTU_SIZE_SETTING);
                app_connect_led();
                mtuExchange_check_timer = 4000000;
                app_is_set_mtusize = 1;
                printf("auto connect \r\n");
            }

            device_in_connection_state = 1;
            is_pairing_mode = 0;
            is_reconn_mode = 0;

            mtuExchange_check_tick = clock_time() | 1;
            notify_get_rsp_tick = clock_time()|1;
            battery_refresh();
            blt_soft_timer_add(app_cachekey_send_timer,30000);
        }
        break;

        case GAP_EVT_ATT_EXCHANGE_MTU:
        {
           // flag_mtu = 1;
            gap_gatt_mtuSizeExchangeEvt_t *pEvt = (gap_gatt_mtuSizeExchangeEvt_t *)para;
            printf("MTU Peer MTU(%d)/Effect ATT MTU(%d).\n", pEvt->peer_MTU, pEvt->effective_MTU);
            app_mtu_size = U16_LO(pEvt->effective_MTU);
            flag_dle |= 0x10;
            /*
            if(!flag_dle){
                blc_ll_exchangeDataLength(LL_LENGTH_REQ,pEvt->effective_MTU);
            }
            */
        }
        break;


        default:
        break;
    }

    return 0;
}

void task_connect (u8 e, u8 *p, int n)
{
    printf("connect\r\n");
    app_mtu_size = 23;
    bls_l2cap_setMinimalUpdateReqSendingTime_after_connCreate(1000);
    enterdeep_time = ADV_RECONN_ENTER_DEEP_COUNT;

    latest_user_event_tick = clock_time();
    blt_soft_timer_add(app_terminate_timeout, 2000000);
    bls_ota_set_random(p+12);
}

void task_conn_update_req (u8 e, u8 *p, int n)
{
    printf("task_conn_update_req\r\n");
}

void task_conn_update_done (u8 e, u8 *p, int n)
{
    printf("task_conn_update_done\n");
}

int app_conn_param_update_response(u8 id, u16  result)
{
    if(result == CONN_PARAM_UPDATE_ACCEPT){
       // printf("CONN_PARAM_UPDATE_ACCEPT\n");
    }
    else if(result == CONN_PARAM_UPDATE_REJECT){
        //printf("CONN_PARAM_UPDATE_REJECT\n");
    }

    return 0;
}

#if (AUDIO_TRANS_USE_2M_PHY_ENABLE)
int app_host_event_callback (u32 h, u8 *para, int n)
{
    u8 event = h & 0xFF;
    if(event==GAP_EVT_ATT_EXCHANGE_MTU)
    {
        blc_ll_setPhy(BLS_CONN_HANDLE, PHY_TRX_PREFER, PHY_PREFER_2M,      PHY_PREFER_2M,    CODED_PHY_PREFER_NONE);
    }
    return 0;
}
void app_phy_update_complete_event(u8 e,u8 *p, int n)
{
}
#endif

extern u32 drive_pins[];
void blt_pm_proc(void)
{
    u8 time=0;
#if(BLE_REMOTE_PM_ENABLE)
#if (STUCK_KEY_PROCESS_ENABLE)
    extern u32 stuckkey_keypresstimer;
    if(key_not_released && stuckkey_keypresstimer && clock_time_exceed(stuckkey_keypresstimer, STUCK_KEY_ENTERDEEP_TIME*1000000))
    {
         printf("stuck key\r\n");
         stuckkey_keypresstimer = 0;
         u8 len = app_custom_get_drive_len();
         {
            for(int i=0; i<len;i++)
            {
                 extern u8 stuckKeyPress[];
                 if(stuckKeyPress[i])
                 {
                     printf("stuck key i=%x\r\n",i);
                     cpu_set_gpio_wakeup(drive_pins[i], KB_LINE_HIGH_VALID, 1);
                 }
                 else
                 {
                    cpu_set_gpio_wakeup(drive_pins[i], KB_LINE_HIGH_VALID, 0);
                 }
            }

            app_release_hid_key();

            printf("deep\r\n");
            cpu_sleep_wakeup(DEEPSLEEP_MODE,PM_WAKEUP_PAD,0);
            printf("into deep fail\r\n");
         }
    }
#endif

    if(device_led_busy())
    {
        bls_pm_setSuspendMask (SUSPEND_DISABLE);
        return;
    }
    if((ui_mic_enable || google_voice_ctl || ota_is_working) && (device_in_connection_state))
    {
        //printf("ota_is_working ui_mic_enable=%x google_voice_ctl=%x ota_is_working=%x,\r\n",ui_mic_enable,google_voice_ctl,ota_is_working);
        bls_pm_setSuspendMask (SUSPEND_DISABLE);
    }
    #if(REMOTE_IR_ENABLE)
        else if( ir_send_ctrl.is_sending){
            //printf(" ir_send_ctrl.is_sending\r\n");
            bls_pm_setSuspendMask(SUSPEND_DISABLE);
        }
    #endif

    #if (BLE_PHYTEST_MODE != PHYTEST_MODE_DISABLE)
        else if( blc_phy_isPhyTestEnable() )
        {
            bls_pm_setSuspendMask(SUSPEND_DISABLE);  //phy test can not enter suspend
        }
    #endif
    
#if (MP_TEST_MODE)
    else if (test_get_mode() == MODE_TEST)
    {
        if (test_get_state() == TEST_STAT_LED)
            cpu_sleep_wakeup(SUSPEND_MODE, PM_WAKEUP_PAD, 0);
        else if (test_get_state() == TEST_STAT_CURRENT)
            cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);
        else bls_pm_setSuspendMask(SUSPEND_DISABLE);
    }
#endif
    else
    {
       
        #if (PM_DEEPSLEEP_RETENTION_ENABLE)
            bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
        #else
            bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
        #endif

        int user_task_flg = ota_is_working || scan_pin_need || key_not_released;

        if(user_task_flg){
              // printf("ota_is_working scan_pin_need=%x google_voice_ctl=%x key_not_released=%x,device_led_busy()=%x\r\n",ui_mic_enable,scan_pin_need,key_not_released,device_led_busy());
            bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);

            #if (LONG_PRESS_KEY_POWER_OPTIMIZE)
                extern int key_matrix_same_as_last_cnt;
                if(!ota_is_working && key_matrix_same_as_last_cnt > 5){  //key matrix stable can optize
                    bls_pm_setManualLatency(2);
                }
                else{
                    bls_pm_setManualLatency(0);  //latency off: 0
                }
            #else
                bls_pm_setManualLatency(0);
            #endif
        }
      

    #if 1 //deepsleep
        if(sendTerminate_before_enterDeep == 1){ //sending Terminate and wait for ack before enter deepsleep
            if(user_task_flg){  //detect key Press again,  can not enter deep now
                sendTerminate_before_enterDeep = 0;
                bls_ll_setAdvEnable(1);   //enable adv again
            }
        }
        else if(sendTerminate_before_enterDeep == 2){  //Terminate OK
            analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) | CONN_DEEP_FLG);
            cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);  //deepsleep
        }


        if(  !blc_ll_isControllerEventPending() ){  //no controller event pending
            //adv 30s interval downgraded
            if( blc_ll_getCurrentState() == BLS_LINK_STATE_ADV && !sendTerminate_before_enterDeep && \
                clock_time_exceed(advertise_begin_tick , ADV_INTERNAL_SWITCH_TIME * 1000000))
            {
                //interval downgraded
                if(advertise_downgrade_flag == 0)
                {
                    advertise_downgrade_flag = 0xaa;
                    if(app_adv_interval_downgradeCb)
                        app_adv_interval_downgradeCb();
                }
            }
            //adv 2min, deepsleep
            if( blc_ll_getCurrentState() == BLS_LINK_STATE_ADV && !sendTerminate_before_enterDeep && \
                clock_time_exceed(advertise_begin_tick , ADV_IDLE_ENTER_DEEP_TIME * 1000000))
            {
                enterdeep_time_count++;
                printf("enterdeep_time_count=%x\r\n",enterdeep_time_count);
                if(enterdeep_time_count == enterdeep_time)
                {
                    if(enterdeep_time >1)
                        printf("adv 10 mins\r\n");
                    else
                        printf("adv 2 mins\r\n");
                    delay3s_to_sleep_tick = clock_time()|1;
                    device_led_off(APP_LED_GREEN);
                    app_pairing_error();
                }
                else
                {
                    advertise_begin_tick = clock_time();
                }
            }
            if(blc_ll_getCurrentState() == BLS_LINK_STATE_ADV && delay3s_to_sleep_tick && clock_time_exceed(delay3s_to_sleep_tick,3000*1000))
            {
                 printf("adv timeout\r\n");
                 app_schedule_ota_enter_deep();
            }
            if((is_pairing_mode == 0) && (is_reconn_mode == 0) && (device_in_connection_state == 0) &&  !user_task_flg )
            {
                 if(enterdeep_time == ADV_RECONN_ENTER_DEEP_COUNT)
                        time = ADV_IDLE_ENTER_DEEP_TIME;
                 else
                        time = 3;
                 if(clock_time_exceed(advertise_begin_tick ,  time* 1000000))
                 {
                     printf("idle to sleep\r\n");
                     app_schedule_ota_enter_deep();
                 }
            }
        }
    #endif
    }

#endif  //END of  BLE_REMOTE_PM_ENABLE
}



void ble_remote_set_sleep_wakeup (u8 e, u8 *p, int n)
{
    if( blc_ll_getCurrentState() == BLS_LINK_STATE_CONN && ((u32)(bls_pm_getSystemWakeupTick() - clock_time())) > 80 * CLOCK_16M_SYS_TIMER_CLK_1MS){  //suspend time > 30ms.add gpio wakeup
        bls_pm_setWakeupSource(PM_WAKEUP_PAD);  //gpio pad wakeup suspend/deepsleep
    }
}

_attribute_ram_code_ void user_init_battery_power_check(void)
{
    /*****************************************************************************************
     Note: battery check must do before any flash write/erase operation, cause flash write/erase
           under a low or unstable power supply will lead to error flash operation

           Some module initialization may involve flash write/erase, include: OTA initialization,
                SMP initialization, ..
                So these initialization must be done after  battery check
    *****************************************************************************************/
    #if (BATT_CHECK_ENABLE)  //battery check must do before OTA relative operation
        #if(VBAT_LEAKAGE_PROTECT_EN)
        do{
            u8 analog_deep = analog_read(USED_DEEP_ANA_REG);
            u16 vbat_deep_thres = VBAT_DEEP_THRES_MV;
            u16 vbat_suspend_thres = VBAT_SUSPEND_THRES_MV;
            if(analog_deep & LOW_BATT_FLG){
                if(analog_deep & LOW_BATT_SUSPEND_FLG){//<1.8v
                    vbat_deep_thres += 200;
                    vbat_suspend_thres += 100;
                }
                else{//1.8--2.0v
                    vbat_deep_thres += 200;
                }
            }
            app_battery_power_check(vbat_deep_thres,vbat_suspend_thres);
            #if (MODULE_WATCHDOG_ENABLE)
            wd_clear(); //clear watch dog
            #endif
            if(analog_deep & LOW_BATT_SUSPEND_FLG){
                sleep_us(100000);
            }
        }while(analog_read(USED_DEEP_ANA_REG) & LOW_BATT_SUSPEND_FLG);
        #else
            if(analog_read(USED_DEEP_ANA_REG) & LOW_BATT_FLG){
                app_battery_power_check(VBAT_DEEP_THRES_MV + 200);
            }
            else{
                app_battery_power_check(VBAT_DEEP_THRES_MV);
            }
        #endif

    #endif
}

_attribute_data_retention_ u8 wakeup_src=1;               //1:wakeup for pad  2:wake up for timer
_attribute_data_retention_ u8 power_down_flag=1;          //1: the rcu previous state is power down 2: the rcu previous state is power down
_attribute_data_retention_ u8 wakeup_key_send_flag=1;     //2: already send wake up packet

u8 app_wakeup_src(void)
{
    u8 flag = analog_read(0X44);
    if(flag & 0x08)
        return 1;  //wakeup for pad
    else
        return 2;  //wakeup for not pad
}

typedef struct{
    u32 dma_len;            //won't be a fixed number as previous, should adjust with the mouse package number

    u8    type;               //RA(1)_TA(1)_RFU(2)_TYPE(4)
    u8    rf_len;             //LEN(6)_RFU(2)
    u8    advA[6];            //address
    u8    data[31];           //0-31 byte
}rf_packet_adv_ind_module_t;

void app_tx_power_wakeup(u8 keyid)
{
    rf_packet_adv_ind_module_t tx_power_wakeup;
    smp_param_save_t  bondInfo;
    u8 buffer[40];
    u8 index=0,i;
    u8 pos_index[3]={0};

    printf("app_tx_power_wakeup\r\n");
    rf_set_ble_access_code_adv();
    rf_set_ble_crc_adv();
    bls_smp_param_loadByIndex(0, &bondInfo);  //get the latest bonding device (index: bond_number-1 )

    flash_read_page(0x78000+wakeup_key_pos, 40, buffer);

    //google format
    {
        tx_power_wakeup.rf_len = 19;
        tx_power_wakeup.dma_len = tx_power_wakeup.rf_len + 2;
        tx_power_wakeup.type = ADV_TYPE_CONNECTABLE_UNDIRECTED;

        memcpy(&(tx_power_wakeup.advA[0]),mac_public,6);
        tx_power_wakeup.data[0] = 0x0c;
        tx_power_wakeup.data[1] = 0x16;
        tx_power_wakeup.data[2] = 0x36;
        tx_power_wakeup.data[3] = 0xfd;
        tx_power_wakeup.data[4] = 0x01;
        if(keyid == 0xff)
        {
            tx_power_wakeup.data[5] = 0x02;  //schedule ota wakeup packet
        }
        else
        {
            tx_power_wakeup.data[5] = app_custom_wakeupkey_packet_index(keyid);
        }
        if(read_wakeup_keyindex_info(&index) == 0)
        {
            if(keyid == 0xff)
            {
                if(index)
                  tx_power_wakeup.data[6] = index-1;
                else
                  tx_power_wakeup.data[6] = 0;
            }
            else
                tx_power_wakeup.data[6] = index;
        }
        else
            tx_power_wakeup.data[6] = 0;

        memcpy(&(tx_power_wakeup.data[7]), bondInfo.peer_addr, 6);
    }

    for(u8 i=0;i<3;i++)
    {
       rf_set_ble_channel(37+i);

       rf_start_stx (&tx_power_wakeup, 100);

       while(rf_tx_finish()!=0x01);

       rf_tx_finish_clear_flag();
    }

    //custom format
    if(buffer[0] != 0xff)
    {
        tx_power_wakeup.rf_len = buffer[0]+6;
        tx_power_wakeup.dma_len = tx_power_wakeup.rf_len + 2;
        tx_power_wakeup.type = 0;
        memcpy(&(tx_power_wakeup.advA[0]),mac_public,6);
        for(i=0;i<buffer[0];i++)
            tx_power_wakeup.data[i] = buffer[4+i];
        printf("raw packet =\r\n");
        for(i=0;i<buffer[0];i++)
            printf(" %x",tx_power_wakeup.data[i]);
        for(i=0;i<3;i++)
        {
            if(buffer[i])
            {
                pos_index[i] = buffer[i+1];
            }
            else
                pos_index[i] = 0xff;
        }
        printf("\r\npos_index[0]=%x,pos_index[1]=%x,pos_index[2]=%x\r\n",pos_index[0],pos_index[1],pos_index[2]);

        for(i=0;i<buffer[0];i++)
        {
            if(i == pos_index[0])
            {
                if(keyid == 0xff)
                    tx_power_wakeup.data[i] = 2;
                else
                    tx_power_wakeup.data[i] = app_custom_wakeupkey_packet_index(keyid);
            }
            else if(i == pos_index[1])
            {
                 if(read_wakeup_keyindex_info(&index) == 0)
                 {
                    if(keyid == 0xff)
                    {
                        if(index)
                            tx_power_wakeup.data[i] = index-1;
                        else
                            tx_power_wakeup.data[i] = 0;
                    }
                    else
                        tx_power_wakeup.data[i] = index;
                 }
                 else
                     tx_power_wakeup.data[i] = 0;
                 printf("tx_power_wakeup.data[i]=%x\r\n",tx_power_wakeup.data[i]);
            }
            else if(i == pos_index[2])
            {
                 memcpy(&(tx_power_wakeup.data[i]), bondInfo.peer_addr, 6);
                 i += 5;
            }
        }

        printf("wakeup packet =\r\n");
        for(i=0;i<buffer[0];i++)
            printf(" %x",tx_power_wakeup.data[i]);
        for(u8 i=0;i<3;i++)
        {
           rf_set_ble_channel(37+i);

           rf_start_stx (&tx_power_wakeup, 100);

           while(rf_tx_finish()!=0x01);

           rf_tx_finish_clear_flag();
        }

    }

    if(keyid != 0xff)
    {
        index++;
        write_wakeup_keyindex_info(index);
    }
}

void app_delay_set(void)
{
    printf("app_delay_set\r\n");
    google_get_rsp_delay();
    if((flag_dle & 0x80) == 0)
    {
        mtuExchange_check_tick = clock_time() | 1;
        mtuExchange_check_timer = 500000;
    }
}

void app_set_mtusize(void)
{
    if(app_is_set_mtusize == 0)
    {
        app_is_set_mtusize = 1;
        printf("app_mtu_size=%x\r\n",app_mtu_size);
        if(app_mtu_size < MTU_SIZE_SETTING)
           blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, MTU_SIZE_SETTING);
    }
}

u8 is_mic_enable(void)
{
    return ui_mic_enable;
}

void user_init_normal(void)
{
    //random number generator must be initiated here( in the beginning of user_init_nromal)
    //when deepSleep retention wakeUp, no need initialize again
    random_generator_init();  //this is must

////////////////// BLE stack initialization ////////////////////////////////////

    u8  mac_random_static[6];
    //for 512K Flash, flash_sector_mac_address equals to 0x76000
    //for 1M   Flash, flash_sector_mac_address equals to 0xFF000
    blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);

    #if(BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_PUBLIC)
        app_own_address_type = OWN_ADDRESS_PUBLIC;
    #elif(BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_RANDOM_STATIC)
        app_own_address_type = OWN_ADDRESS_RANDOM;
        blc_ll_setRandomAddr(mac_random_static);
    #endif

    ////// Controller Initialization  //////////
    blc_ll_initBasicMCU();                      //mandatory
    blc_ll_initStandby_module(mac_public);                //mandatory
    blc_ll_initAdvertising_module(mac_public);     //adv module:          mandatory for BLE slave,
    blc_ll_initConnection_module();                //connection module  mandatory for BLE slave/master
    blc_ll_initSlaveRole_module();                //slave module:      mandatory for BLE slave,
    blc_ll_initPowerManagement_module();        //pm module:           optional
    blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW32K);

    #if(AUDIO_TRANS_USE_2M_PHY_ENABLE)
        blc_ll_init2MPhyCodedPhy_feature();            //enable 2M/coded PHY
    #endif

    ////// Host Initialization  //////////
    blc_gap_peripheral_init();    //gap initialization
    extern void my_att_init ();
    my_att_init (); //gatt initialization
    blc_l2cap_register_handler (blc_l2cap_packet_receive);      //l2cap initialization

    blc_att_setRxMtuSize(MTU_SIZE_SETTING);

    //Smp Initialization may involve flash write/erase(when one sector stores too much information,
    //   is about to exceed the sector threshold, this sector must be erased, and all useful information
    //   should re_stored) , so it must be done after battery check
    #if (BLE_REMOTE_SECURITY_ENABLE)
        blc_smp_peripheral_init();

        //Hid device on android7.0/7.1 or later version
        // New paring: send security_request immediately after connection complete
        // reConnect:  send security_request 1000mS after connection complete. If master start paring or encryption before 1000mS timeout, slave do not send security_request.
        //blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )
        blc_smp_configSecurityRequestSending(SecReq_NOT_SEND, SecReq_NOT_SEND, 1000);
    #else
        blc_smp_setSecurityLevel(No_Security);
    #endif

///////////////////// USER application initialization ///////////////////
    app_set_advdata_rspdata();

    if(analog_read(USED_DEEP_ANA_REG) & POWER_ON_FLG)
    {
        printf("power on deep\r\n");
        power_down_flag = 2;
    }
    else
    {
        power_down_flag = 1;
        printf("power on for power down\r\n");
        if(kb_powerdet())
        {
            printf("press and hold back key\r\n");
            flash_erase_sector(IR_DATA_SECT_0_ADDR);
            flash_erase_sector(IR_DATA_SECT_1_ADDR);
            flash_erase_sector(IR_DATA_SECT_2_ADDR);
            flash_erase_sector(IR_DATA_SECT_3_ADDR);
            u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
            if(bond_number)
                 bls_smp_param_deleteByIndex(0);
            bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
            flash_erase_sector(MAC_DATA_SECT_ADDR);
            flash_erase_sector(IR_KEY_EVENT_NOTIFY_SECT_ADDR);
            flash_erase_sector(CCC_DATA_AREA);
        }
    }
    u8 data = analog_read(DEEP_ANA_REG2);
    if(data == 0x0f)
        analog_write(DEEP_ANA_REG2, 0);
    app_schedule_ota_wakeup_host();

    is_pairing_mode = 0;
    device_in_connection_state = 0;
    is_reconn_mode = 0;

    ////////////////// config adv packet /////////////////////

    blc_smp_param_setBondingDeviceMaxNumber(1);

    if (test_get_mode() != MODE_TEST)
    {
        u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
        smp_param_save_t  bondInfo;
        if(bond_number)   //at least 1 bonding device exist
        {
           bls_smp_param_loadByIndex( bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )
        }

        if(bond_number)   //set direct adv
        {
            app_set_adv_interval_downgrade_direct();
            printf("\r\npeer_addr=%x %x %x %x %x %x\r\n",bondInfo.peer_addr[0],bondInfo.peer_addr[1],bondInfo.peer_addr[2],bondInfo.peer_addr[3],bondInfo.peer_addr[4],bondInfo.peer_addr[5]);
            if((analog_read(USED_DEEP_ANA_REG) & OTA_FLG) == OTA_FLG)
            {
                printf("ana read = ota succ\r\n");
                service_change = 0x55;
            }
            enterdeep_time = ADV_RECONN_ENTER_DEEP_COUNT;
            //set direct adv
            u8 status = bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
                                            ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, app_own_address_type,
                                            bondInfo.peer_addr_type,  bondInfo.peer_addr,
                                            MY_APP_ADV_CHANNEL,
                                            ADV_FP_NONE);
            if(status != BLE_SUCCESS) {      while(1); }  //debug: adv setting err
            //it is recommended that direct adv only last for several seconds, then switch to indirect adv
            bls_ll_setAdvDuration(MY_DIRECT_ADV_TMIE, 1);
            bls_app_registerEventCallback (BLT_EV_FLAG_ADV_DURATION_TIMEOUT, &direct_adv);

            bls_ll_setAdvEnable(1);  //adv enable
            printf("direct adv\r\n");
        }
        else   //set indirect adv
        {
            app_set_adv_interval_downgrade_indirect();
            enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
            u8 status = bls_ll_setAdvParam(  MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
                                             ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
                                             0,  NULL,
                                             MY_APP_ADV_CHANNEL,
                                             ADV_FP_NONE);
            if(status != BLE_SUCCESS) {      while(1); }  //debug: adv setting err
            printf("inderict adv\r\n");
        }
    }
    else
    {
        printf("test mode\r\n");
        app_set_adv_interval_downgrade_indirect();
        enterdeep_time = ADV_IDLE_ENTER_DEEP_COUNT;
        u8 status = bls_ll_setAdvParam(  MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
                                         ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
                                         0,  NULL,
                                         MY_APP_ADV_CHANNEL,
                                         ADV_FP_NONE);
        if(status != BLE_SUCCESS) {      while(1); }  //debug: adv setting err
        printf("inderict adv\r\n");
    }

    //set rf power index, user must set it after every suspend wakeup, cause relative setting will be reset in suspend
    user_set_rf_power(0, 0, 0);
    bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_EXIT, &user_set_rf_power);

    blc_gap_registerHostEventHandler( app_host_event_callback );
    blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN             |  \
                          GAP_EVT_MASK_SMP_PARING_SUCCESS           |  \
                          GAP_EVT_MASK_SMP_PARING_FAIL                |  \
                          GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE     |  \
                          GAP_EVT_MASK_ATT_EXCHANGE_MTU 
                          );

    //ble event call back
    bls_app_registerEventCallback (BLT_EV_FLAG_CONNECT, &task_connect);
    bls_app_registerEventCallback (BLT_EV_FLAG_TERMINATE, &ble_remote_terminate);

    bls_app_registerEventCallback (BLT_EV_FLAG_LL_REJECT_IND, &ble_ll_reject);

    bls_app_registerEventCallback (BLT_EV_FLAG_CONN_PARA_REQ, &task_conn_update_req);
    bls_app_registerEventCallback (BLT_EV_FLAG_CONN_PARA_UPDATE, &task_conn_update_done);

    bls_app_registerEventCallback (BLT_EV_FLAG_DATA_LENGTH_EXCHANGE, &task_datalengthexchange);
    blc_l2cap_registerConnUpdateRspCb(app_conn_param_update_response);
    #if(AUDIO_TRANS_USE_2M_PHY_ENABLE)
        blc_gap_setEventMask(GAP_EVT_MASK_ATT_EXCHANGE_MTU);
        blc_gap_registerHostEventHandler( app_host_event_callback );
        bls_app_registerEventCallback (BLT_EV_FLAG_PHY_UPDATE, &app_phy_update_complete_event);
    #endif

    ///////////////////// Power Management initialization///////////////////
#if(BLE_REMOTE_PM_ENABLE)
    blc_ll_initPowerManagement_module();

    #if (PM_DEEPSLEEP_RETENTION_ENABLE)
        bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
        blc_pm_setDeepsleepRetentionThreshold(50, 30);
        #if (__PROJECT_8278_BLE_REMOTE__)
            blc_pm_setDeepsleepRetentionEarlyWakeupTiming(480);
        #else
            blc_pm_setDeepsleepRetentionEarlyWakeupTiming(400);
        #endif
    #else
        bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
    #endif

    bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_ENTER, &ble_remote_set_sleep_wakeup);
#else
    bls_pm_setSuspendMask (SUSPEND_DISABLE);
#endif

#if    (TL_AUDIO_MODE & TL_AUDIO_MASK_SBC_MODE | TL_AUDIO_MASK_MSBC_MODE)
    blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW32K);
#else

#endif

#if (BLE_REMOTE_OTA_ENABLE)
    ////////////////// OTA relative ////////////////////////
    bls_ota_clearNewFwDataArea(); //must

    bls_ota_init_handle(OTA_CMD_OUT_DP_H);

#endif
    app_ui_init_normal();
    advertise_tick_retime();

    //user data init
    init_ccc_value();
    printf("flag_ccc_data:0x%x\n",flag_ccc_data);
    app_ir_programming_end_register(app_delay_set);
    is_flash_info_full();
    init_peer_mac();
    ir_param_init();
    u8 ccc = init_ir_key_event_notify();
    if(ccc != 0xff)
        ir_init_key_event_notify(ccc);
    init_wakeup_keyindex();
    extern void init_device_info (void);
    init_device_info();
}


_attribute_ram_code_ void user_init_deepRetn(void)
{
#if (PM_DEEPSLEEP_RETENTION_ENABLE)
    blc_ll_initBasicMCU();   //mandatory
#if (MP_TEST_MODE)
    if (gpio_read(DUT_INPUT_PORT) && test_get_mode() == MODE_TEST)
        rf_set_power_level_index(RF_POWER_P7p37dBm);
    else
#endif
    rf_set_power_level_index (MY_RF_POWER_INDEX);

    blc_ll_recoverDeepRetention();

    DBG_CHN0_HIGH;    //debug

    irq_enable();

    app_ui_init_deepRetn();
#endif
}

/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////

void ir_flash_erase_loop(void)
{
    if(ir_flash_erase_tick && clock_time_exceed(ir_flash_erase_tick, 8000000 ))
    {
        ir_flash_erase_tick = 0;
        printf("ir_flash_erase_loop\r\n");
        ir_flash_erase();
    }
}

void main_loop (void)
{
    mtu_exchange_loop();
    ir_flash_erase_loop();

#if (BLT_TEST_SOFT_TIMER_ENABLE)
    blt_soft_timer_process(MAINLOOP_ENTRY);
#endif
    ////////////////////////////////////// BLE entry /////////////////////////////////
    blt_sdk_main_loop();

    ////////////////////////////////////// UI entry /////////////////////////////////
    #if (BLE_AUDIO_ENABLE)
        app_audio_task();
    #endif

    app_ota_proc();

    #if (BATT_CHECK_ENABLE)
        if(battery_get_detect_enable() && clock_time_exceed(lowBattDet_tick, 300000) ){
            lowBattDet_tick = clock_time();
            #if(VBAT_LEAKAGE_PROTECT_EN)

            u8 analog_deep = analog_read(USED_DEEP_ANA_REG);
            u16 vbat_deep_thres = VBAT_DEEP_THRES_MV;
            u16 vbat_suspend_thres = VBAT_SUSPEND_THRES_MV;
            if(analog_deep & LOW_BATT_FLG){
                if(analog_deep & LOW_BATT_SUSPEND_FLG){//<1.8v
                    vbat_deep_thres += 200;
                    vbat_suspend_thres += 100;
                }
                else{//1.8--2.0v
                    vbat_deep_thres += 200;
                }
            }
            app_battery_power_check(vbat_deep_thres,vbat_suspend_thres);

            #else
                app_battery_power_check(VBAT_DEEP_THRES_MV);  //2000 mV low battery
            #endif
        }
    #endif
    proc_keyboard (0,0, 0);

#if (BLT_APP_LED_ENABLE)
    device_led_process();
#endif

    blt_pm_proc();
}




#endif  //end of __PROJECT_8267_BLE_REMOTE__ || __PROJECT_8261_BLE_REMOTE__ || __PROJECT_8269_BLE_REMOTE__
