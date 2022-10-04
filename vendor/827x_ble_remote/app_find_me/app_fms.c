/******************************************************************************
 * @file     app_fms.c
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
#include "../app_config.h"
#include "stack/ble/ble.h"

#include "app_fms.h"

extern own_addr_type_t   app_own_address_type;

/**********************************************************************
 * LOCAL MARCO
 */

#define s_p_app_fms_ctrl     ((app_buzzer_ctrl_t *)(app_fms_buf+0x10))



/**********************************************************************
 * LOCAL TYPES
 */

typedef struct {
    u16 count;
    u16 adv_interval_min;
    u16 adv_interval_max;

    u8 adv_type;
    u8 reserved;
}app_fms_adv_timer_table_t;


/**********************************************************************
 * GLOBAL VARIABLES
 */

_attribute_data_retention_ u8 g_p_app_fms_ctrl_val[2] = {0};

//const app_fms_ctrl_t g_app_fms_ctrl = {
//    en_periodic_wakeup:0,
//    reserved:0,
//    periodic_wakeup_interval:0,
//};

/**********************************************************************
 * LOCAL VARIABLES
 */

_attribute_data_retention_ static app_fms_mode_callback_t s_fms_mode_cb = app_imc_alert_cb;
//
//_attribute_data_retention_ u8 s_fms_adv_table_index = 0;
//_attribute_data_retention_ u16 s_fms_adv_count = 0;


const app_fms_adv_timer_table_t app_fms_adv_timer_table[] = {
/*  count adv_interval_min    adv_interval_max    adv_type                                reserved  */
    {15,  ADV_INTERVAL_20MS,  ADV_INTERVAL_25MS,  ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, 0},
    {45,  ADV_INTERVAL_100MS, ADV_INTERVAL_150MS, ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, 0},
    {120, ADV_INTERVAL_1S,    ADV_INTERVAL_1S5,   ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, 0},
};



/**********************************************************************
 * LOCAL FUNCTIONS
 */

void app_fms_buffer_init(void){
//    memcpy((u8 *)g_p_app_fms_ctrl, &g_app_fms_ctrl, sizeof(app_fms_ctrl_t));
//    memset((u8 *)g_p_app_fms_ctrl_val, 0x00 ,2);

    memset((u8 *)g_p_app_fms_ctrl, 0x00, sizeof(app_fms_ctrl_t));
    memset((u8 *)g_p_app_fms_data_val, 0x00 ,2);

    s_p_app_fms_flag_adv[0] = 0;
    s_p_app_fms_adv_table_index[0] = 0;
    s_p_app_fms_adv_count[0] = 0;

    app_buzzer_buffer_init();
}


void app_fms_set_mode(APP_FMS_MODE_T mode, app_fms_mode_callback_t fms_cb){

    g_p_app_fms_ctrl_val[0] = U16_HI(mode);
    g_p_app_fms_ctrl_val[1] = U16_LO(mode);
    s_fms_mode_cb = fms_cb;
}

void app_fms_notify_error_code(APP_FMS_ERROR_CODE_T error_code){

    u8 my_error_code[] = {  U16_HI(APP_FMS_NOTIFY_DATA_TYPE_ERROR_CODE),
                            U16_LO(APP_FMS_NOTIFY_DATA_TYPE_ERROR_CODE),
                            U16_HI(error_code),
                            U16_LO(error_code),};

    bls_att_pushNotifyData(FMS_GOOGLE_DATA_DP_H, my_error_code, sizeof(my_error_code));

}

void app_fms_notify_data_type(APP_FMS_NOTIFY_DATA_TYPE_T data_type, APP_FMS_ERROR_CODE_T error_code){
    u8 my_error_code[] = {  U16_HI(data_type),
                            U16_LO(data_type),
                            U16_HI(error_code),
                            U16_LO(error_code),};

    bls_att_pushNotifyData(FMS_GOOGLE_DATA_DP_H, my_error_code, sizeof(my_error_code));
}


void app_fms_init(void){
    if(g_p_app_fms_ctrl->en_fms){
        app_fms_set_mode(APP_FMS_MODE_ALERT, app_imc_alert_cb);
    }else{
        app_fms_set_mode(APP_FMS_MODE_ALL_DISABLE, NULL);
    }
}



int app_fms_att_ctl_cb(void * p){
    rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;

    u16 len = pw->l2cap-3;//pw->rf_len-7;
    array_printf(pw->dat,len);

    u16 my_fms_mode = (pw->dat[0]<<8) | (pw->dat[1]);

    switch (my_fms_mode) {
        case APP_FMS_MODE_ALL_DISABLE:
            app_fms_set_mode(APP_FMS_MODE_ALL_DISABLE, NULL);

            break;
        case APP_FMS_MODE_ALERT:
            printf(" APP_FMS_MODE_ALERT en_fms: 0x%x \n", g_p_app_fms_ctrl->en_fms);
            if(g_p_app_fms_ctrl->en_fms){
                app_fms_set_mode(APP_FMS_MODE_ALERT, app_imc_alert_cb);
            }else{
                app_fms_notify_data_type(APP_FMS_NOTIFY_DATA_TYPE_ERROR_CODE, APP_FMS_ERROR_CODE_ALERT_MODE_NOT_SUPPORTED);
            }
            break;
        default:
            app_fms_set_mode(APP_FMS_MODE_ALL_DISABLE, NULL);
            break;
    }
    return 0;
}

int app_fms_att_data_cb(void * p){
    rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;

    u16 len = pw->l2cap-3;//pw->rf_len-7;
    array_printf(pw->dat,len);

    if(!s_fms_mode_cb){
        app_fms_notify_data_type(APP_FMS_NOTIFY_DATA_TYPE_ERROR_CODE, APP_FMS_ERROR_CODE_ALL_MODES_DISABLED);
    }else{
        s_fms_mode_cb(pw->dat, len);
    }

    return 0;
}





void app_fms_adv_settings(u16 interval_min, u16 interval_max){

    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    bls_ll_setAdvEnable(0);  // ADV disable

    if(bond_number){

        smp_param_save_t  bondInfo;
        bls_smp_param_loadByIndex( bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )
        ll_resolvingList_add(bondInfo.peer_id_adrType,bondInfo.peer_id_addr,bondInfo.peer_irk,bondInfo.local_irk);
        ll_whiteList_reset();

        u8 status = 0;

        // TODO check setting host address
        if (memcmp(bondInfo.peer_id_addr, bondInfo.peer_addr, 6) == 0){
            status = bls_ll_setAdvParam(interval_min, interval_max,
                                        ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, OWN_ADDRESS_PUBLIC,
                                        bondInfo.peer_addr_type,  bondInfo.peer_addr,
                                        BLT_ENABLE_ADV_ALL,
                                        ADV_FP_NONE);
        }else{
            ll_resolvingList_reset();
            status = ll_resolvingList_add(bondInfo.peer_id_adrType, bondInfo.peer_id_addr, bondInfo.peer_irk, bondInfo.local_irk);
            status = ll_resolvingList_setAddrResolutionEnable(1);

            extern u8 en_slave_rpa;
            if(0 == en_slave_rpa) {
                app_own_address_type = OWN_ADDRESS_RESOLVE_PRIVATE_PUBLIC;
            }else{
                app_own_address_type = OWN_ADDRESS_PUBLIC;
            }

            status = bls_ll_setAdvParam(interval_min, interval_max,
                                        ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, app_own_address_type,
                                        bondInfo.peer_id_adrType,  bondInfo.peer_id_addr,
                                        BLT_ENABLE_ADV_ALL,
                                        ADV_FP_NONE);
        }

        if(status != BLE_SUCCESS) {      while(1); }  //debug: adv setting err

        bls_ll_setAdvEnable(1);  //adv enable
        s_p_app_fms_flag_adv[0] = 1;
    }
}



int app_fms_adv_timer(void){

    if(!s_p_app_fms_flag_adv[0]){
        printf("[NOTICE] FMS ADV flag is disabled !!! \n");
        return -1;
    }

    s_p_app_fms_adv_count[0]++;

//    printf("%s count: %d  s_fms_adv_count: %d \n",__FUNCTION__, s_fms_adv_count, my_table_cnt);

    if(s_p_app_fms_adv_count[0] >= app_fms_adv_timer_table[s_p_app_fms_adv_table_index[0]].count){
        s_p_app_fms_adv_table_index[0]++;

        u8 my_table_cnt = sizeof(app_fms_adv_timer_table)/sizeof(app_fms_adv_timer_table_t);
        if(s_p_app_fms_adv_table_index[0] >= my_table_cnt ){
            bls_ll_setAdvEnable(0);
            printf("End fms direct adv \n");
            s_p_app_fms_flag_adv[0] = 0;
            return -1;
        }

        printf("FMS ADV parameter table_index: %d adv_interval_IDX MIN: %d  MAX: %d \n",
                                s_p_app_fms_adv_table_index[0],
                                app_fms_adv_timer_table[s_p_app_fms_adv_table_index[0]].adv_interval_min,
                                app_fms_adv_timer_table[s_p_app_fms_adv_table_index[0]].adv_interval_max);

        app_fms_adv_settings(   app_fms_adv_timer_table[s_p_app_fms_adv_table_index[0]].adv_interval_min,
                                app_fms_adv_timer_table[s_p_app_fms_adv_table_index[0]].adv_interval_max);
    }
    return 0;
}

void app_fms_try_connect(void){

    printf("%s \n", __FUNCTION__ );

    s_p_app_fms_adv_count[0] = 0;
    s_p_app_fms_adv_table_index[0] = 0;

    printf("FMS ADV parameter table_index: %d adv_interval_IDX MIN: %d  MAX: %d \n",
                            s_p_app_fms_adv_table_index[0],
                            app_fms_adv_timer_table[s_p_app_fms_adv_table_index[0]].adv_interval_min,
                            app_fms_adv_timer_table[s_p_app_fms_adv_table_index[0]].adv_interval_max);

    app_fms_adv_settings(   app_fms_adv_timer_table[s_p_app_fms_adv_table_index[0]].adv_interval_min,
                            app_fms_adv_timer_table[s_p_app_fms_adv_table_index[0]].adv_interval_max);
    blt_soft_timer_add(app_fms_adv_timer, 1000000);

}

void app_fms_delete_adv_task(void){
    s_p_app_fms_flag_adv[0] = 0;
}


void app_fms_periodic_wake_up(void){
    /* FMS periodic wake up disabled */

    if(!g_p_app_fms_ctrl->en_periodic_wakeup) return;

    /* periodic wakeup interval out of range */
    if(0 == g_p_app_fms_ctrl->periodic_wakeup_interval  || g_p_app_fms_ctrl->periodic_wakeup_interval > 1440) return;

    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number

    if(bond_number){
        printf("[NOTICE] FMS periodic wake up time: %d min \n", g_p_app_fms_ctrl->periodic_wakeup_interval);

//        analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) | (PERIODIC_WAKE_UP));
        cpu_long_sleep_wakeup_32k_rc(DEEPSLEEP_MODE,PM_WAKEUP_TIMER|PM_WAKEUP_PAD,g_p_app_fms_ctrl->periodic_wakeup_interval*CLOCK_32K_CLOCK_1MIN);
//        cpu_long_sleep_wakeup_32k_rc(DEEPSLEEP_MODE,PM_WAKEUP_TIMER|PM_WAKEUP_PAD,g_p_app_fms_ctrl->periodic_wakeup_interval*CLOCK_32K_CLOCK_1S);   // for testing
    }
}
