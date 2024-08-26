/********************************************************************************************************
 * @file    app_fmr.c
 *
 * @brief   This is the source file for b85m
 *
 * @author  ROW GROUP
 * @date    06,2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *          All rights reserved.
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "../app_config.h"
#include "../app_att.h"
#include "../battery_check.h"

#include "app_fmr.h"
#include "app_buzzer.h"

/**********************************************************************
 * LOCAL MARCO
 */

#define GOOGLE_FMR_SERVICE_UUID        0x64,0xB6,0x17,0xF6,0x01,0xAF,0x7D,0xBC,0x05,0x4F,0x21,0x5A,0x01,0x00,0x03,0x18
#define GOOGL_FMR_CHAR_UUID            0x64,0xB6,0x17,0xF6,0x01,0xAF,0x7D,0xBC,0x05,0x4F,0x21,0x5A,0x02,0x00,0x03,0x18

#define GOOGL_FMR_MAX_DURATION          (0x12C)     // 30 sec

#define FMR_DEBUG_LOG(...)

#define fmr_buzzer_play(duty)     app_buzzer_play(APP_BUZZER_PLAY_INDEX_RINGTONE_0, APP_BUZZER_STATUS_PLAY_LOOP, duty);


/**********************************************************************
 * LOCAL TYPES
 */

typedef enum{
    GOOGLE_FMR_EVENT_REQUEST = 0,
    GOOGLE_FMR_EVENT_DURATION_ELAPSED,
    GOOGLE_FMR_EVENT_RCU_FOUND,
    GOOGLE_FMR_EVENT_DURATION_TOO_LONG,
    GOOGLE_FMR_EVENT_LOW_BATTERY,

    GOOGLE_FMR_EVENT_OTHER = 0x7F,

    GOOGLE_FMR_EVENT_INVALID_REQUEST = 0xFF,
}fmr_event_t;


typedef struct {
    u8 en_fmr;
    u8 en_periodic_wakeup;
    u16 periodic_wakeup_interval;
}app_fmr_ctrl_t;

/**********************************************************************
 * ATT SERVICE
 */
#define WRAPPING_BRACES(__DATAS__)          { __DATAS__ }

const u8 app_fmr_service_uuid[16]   = WRAPPING_BRACES(GOOGLE_FMR_SERVICE_UUID);
const u8 app_fmr_char_uuid[16]      = WRAPPING_BRACES(GOOGL_FMR_CHAR_UUID);

const u8 app_fmr_char_val[19] = {
    CHAR_PROP_READ | CHAR_PROP_NOTIFY | CHAR_PROP_WRITE_WITHOUT_RSP,
    U16_LO(APP_FMR_CMD_OUT_DP_H), U16_HI(APP_FMR_CMD_OUT_DP_H),
    GOOGL_FMR_CHAR_UUID,
};

_attribute_data_retention_ volatile u8 app_fmr_data[3]  = {0x00};
_attribute_data_retention_ u16 app_fmr_ccc  = 0;

/**********************************************************************
 * GLOBAL VARIABLES
 */

/**********************************************************************
 * LOCAL VARIABLES
 */
_attribute_data_retention_ u32 s_fmr_counter_tick  = 0;

_attribute_data_retention_ app_fmr_ctrl_t s_fmr_ctrl = {0};

/**********************************************************************
 * LOCAL FUNCTIONS
 */



fmr_event_t app_fmr_get_event(void){

    return app_fmr_data[0];
}

void app_fmr_set_event(fmr_event_t event){

    fmr_event_t my_fmr_event = event;
    switch (event) {
        case      GOOGLE_FMR_EVENT_REQUEST:
        case      GOOGLE_FMR_EVENT_DURATION_ELAPSED:
        case      GOOGLE_FMR_EVENT_RCU_FOUND:
        case      GOOGLE_FMR_EVENT_DURATION_TOO_LONG:
        case      GOOGLE_FMR_EVENT_LOW_BATTERY:
        case      GOOGLE_FMR_EVENT_OTHER:
        case      GOOGLE_FMR_EVENT_INVALID_REQUEST:
            break;
        default:
            my_fmr_event = GOOGLE_FMR_EVENT_OTHER;
            break;
    }

    app_fmr_data[0] = my_fmr_event;
    printf("Set FMR event to %d\n", app_fmr_data[0]);
}

u16 app_fmr_get_duration(void){

    u16 my_duration = MAKE_U16(app_fmr_data[1], app_fmr_data[2]);
    return my_duration;
}

void app_fmr_set_duration(u16 duration){
    app_fmr_data[1] = U16_HI(duration);
    app_fmr_data[2] = U16_LO(duration);

    FMR_DEBUG_LOG("Set FMR duration to DEC:[%d] HEX:[0x%02X%02X]" , duration, app_fmr_data[1], app_fmr_data[2]);
}

void app_fmr_decrease_duration(void){

    u16 my_duration = app_fmr_get_duration();

    if(my_duration){
        my_duration--;
        app_fmr_set_duration(my_duration);
    }
}

void app_fmr_set_fmr_para(fmr_event_t event, u16 duration){

    /* Is set in the response to write operation when the provided remaining buzz duration is too large.
     * Remaining buzz duration is adjusted to a reasonable value at the same time. */
    if(duration > GOOGL_FMR_MAX_DURATION){
        app_fmr_set_event(GOOGLE_FMR_EVENT_DURATION_TOO_LONG);
        app_fmr_set_duration(GOOGL_FMR_MAX_DURATION);
    }else{
        app_fmr_set_event(event);
        app_fmr_set_duration(duration);
    }


    bls_att_pushNotifyData(APP_FMR_CMD_OUT_DP_H, (u8 *)app_fmr_data, sizeof(app_fmr_data));
    printf("NOTIFY FMR DATA:");
    array_printf((u8 *)app_fmr_data, sizeof(app_fmr_data));
}

/**********************************************************************
 * GLOBAL FUNCTIONS
 */

u8 app_fmr_is_fmr_en(void){
    return s_fmr_ctrl.en_fmr;
}

void app_fmr_set_fmr_en(u8 en){
    s_fmr_ctrl.en_fmr = en;
}


int app_fmr_write_cb(void * p){

    rf_packet_att_data_t *req = (rf_packet_att_data_t*)p;
    u16 my_data_len = req->l2cap - 3;

    // TODO

    #define FMR_LOW_VOLTABE     (2250)      //mv

    if(get_battery_voltage() < FMR_LOW_VOLTABE){
        app_fmr_set_fmr_para(GOOGLE_FMR_EVENT_LOW_BATTERY, 0);
        return 0;
    }

    /* Any write operation of length less than 3 bytes or where the last event field is not equal to PEER_REQUEST shall not be accepted
     * and shall trigger INVALID_REQUEST notification (see Invalid request example). The failed write operations shall not alter
     * the value of Remaining buzz duration field; */
    if((my_data_len < 3) || (req->dat[0] !=  GOOGLE_FMR_EVENT_REQUEST)){
        app_fmr_set_fmr_para(GOOGLE_FMR_EVENT_INVALID_REQUEST, app_fmr_get_duration());
        return 0;
    }

    /* The writes of 3 bytes or more are accepted if the first byte of the last event field is set to PEER_REQUEST.
     * The following 2 bytes shall be treated as the new value of remaining buzz duration (16-bit integer), all remaining bytes are ignored.
     * Writing a non-zero value shall start the buzz mode for the provided duration. Writing 0 shall stop the buzz mode if active. */
    u16 my_duration = MAKE_U16(req->dat[1], req->dat[2]);
    if(my_duration){
        /* buzzer ON. The value that is implied by every successful write operation.*/
        s_fmr_counter_tick = clock_time()|1;

        fmr_buzzer_play(app_buzzer_set_and_get_pwm_duty_by_voltage());

//        app_led_pwm_enable(ALP_CFG_INDEX_FMR, 0);
        app_fmr_set_fmr_para(GOOGLE_FMR_EVENT_REQUEST, my_duration);
    }else{
        /* buzzer OFF */
        app_buzzer_stop();
//        app_led_pwm_disable(ALP_CFG_INDEX_FMR);
        app_fmr_set_fmr_para(GOOGLE_FMR_EVENT_REQUEST, 0);
    }

    return 0;
}

void app_fmr_parameter_init(void){

    app_fmr_data[0] = 0x00;
    app_fmr_data[1] = 0x00;
    app_fmr_data[2] = 0x00;

    s_fmr_counter_tick  = 0;
    app_buzzer_buffer_init();
}

u8 app_fmr_is_busy(void){

    u8 my_event = app_fmr_get_event();
    u16 my_duration = app_fmr_get_duration();

    u8 my_flag_busy = 1;
    if( (GOOGLE_FMR_EVENT_RCU_FOUND == my_event) || (0 == my_duration) ){
        my_flag_busy = 0;
    }

    return my_flag_busy;
}

void app_fmr_on_button_event(void){

    if(app_fmr_is_busy() == 0) return;

    /* Set when the user picks up the remote or presses any button while in the buzz mode. */
    app_buzzer_stop();
//    app_led_pwm_disable(ALP_CFG_INDEX_FMR);
    app_fmr_set_fmr_para(GOOGLE_FMR_EVENT_RCU_FOUND, 0);
}

void app_fmr_task(void){

    if(0 == app_fmr_is_busy()){
        if(app_buzzer_is_buzy()){
            app_buzzer_stop();
        }
//        app_led_pwm_disable(ALP_CFG_INDEX_FMR);
        return;
    }

    if(clock_time_exceed(s_fmr_counter_tick, 100*1000)){
        s_fmr_counter_tick = clock_time()|1;
        app_fmr_decrease_duration();
    }

    /* Set if and only if the remaining buzz duration is counted down to 0 */
    if(app_fmr_get_duration() == 0){
        app_buzzer_stop();
//        app_led_pwm_disable(ALP_CFG_INDEX_FMR);
        app_fmr_set_fmr_para(GOOGLE_FMR_EVENT_DURATION_ELAPSED, 0);
    }
}


/**
 * periodic wake-up ADV
 */

typedef struct {
    u16 count;
    u16 adv_interval_min;
    u16 adv_interval_max;

    u8 adv_type;
    u8 reserved;
}app_fmr_adv_timer_table_t;


_attribute_data_retention_ u8 s_flag_fmr_adv = 0;
_attribute_data_retention_ u8 s_fmr_adv_table_index = 0;
_attribute_data_retention_ u16 s_fmr_adv_send_count = 0;



const app_fmr_adv_timer_table_t app_fmr_adv_timer_table[] = {
/*  count adv_interval_min    adv_interval_max    adv_type                                reserved  */
    {15,  ADV_INTERVAL_20MS,  ADV_INTERVAL_25MS,  ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, 0},
    {45,  ADV_INTERVAL_100MS, ADV_INTERVAL_150MS, ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, 0},
    {120, ADV_INTERVAL_1S,    ADV_INTERVAL_1S5,   ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, 0},
};


void app_fmr_set_periodic_wakeup(u8 en, u16 interval){

    if(0 == en){
        s_fmr_ctrl.en_periodic_wakeup = 0;
        return;
    }

    if((interval <= 1440) && (interval != 0)){
        /* 220707. The valid range is changed to 1~1440(min) */
        s_fmr_ctrl.periodic_wakeup_interval = interval;
    }else{
        /* If the interval data in the flash is wrong, use 30 */
        s_fmr_ctrl.periodic_wakeup_interval = 30;
    }

    s_fmr_ctrl.en_periodic_wakeup = 1;

    printf("[SET] periodic wake-up:[%d] interval:[%d]", s_fmr_ctrl.en_periodic_wakeup, s_fmr_ctrl.periodic_wakeup_interval);
}

u8 app_fmr_is_periodic_wakeup_en(void){
    return s_fmr_ctrl.en_periodic_wakeup;
}

u8 app_fmr_is_adv_busy(void){
    return s_flag_fmr_adv;
}

void app_fmr_adv_settings(u16 interval_min, u16 interval_max){

    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    bls_ll_setAdvEnable(0);  // ADV disable

    if(bond_number){

        smp_param_save_t  bondInfo;
        bls_smp_param_loadByIndex( bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )
        ll_resolvingList_reset();
        ll_resolvingList_add(bondInfo.peer_id_adrType,bondInfo.peer_id_addr,bondInfo.peer_irk,bondInfo.local_irk);
        ll_whiteList_reset();

        u8 status=0,rpa_flag=0;
        if (memcmp(bondInfo.peer_id_addr, bondInfo.peer_addr, 6) == 0)
        {
           rpa_flag = 0;
        }
        else
        {
           extern u8 en_slave_rpa;
           if(en_slave_rpa == 0)
               rpa_flag = 0;
           else
               rpa_flag = 1;
        }

        // TODO check setting host address
        if (0 == rpa_flag){
            ll_whiteList_add(bondInfo.peer_addr_type,bondInfo.peer_addr);
            status = bls_ll_setAdvParam(interval_min, interval_max,
                                        ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, OWN_ADDRESS_PUBLIC,
                                        bondInfo.peer_addr_type,  bondInfo.peer_addr,
                                        BLT_ENABLE_ADV_ALL,
                                        ADV_FP_NONE);
        }else{
            status = bls_ll_setAdvParam(interval_min, interval_max,
                                        ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, OWN_ADDRESS_RESOLVE_PRIVATE_PUBLIC,
                                        bondInfo.peer_id_adrType,  bondInfo.peer_id_addr,
                                        BLT_ENABLE_ADV_ALL,
                                        ADV_FP_NONE);
        }

        if(status != BLE_SUCCESS) {      while(1); }  //debug: adv setting err

        bls_ll_setAdvEnable(1);  //adv enable
        s_flag_fmr_adv = 1;
    }
}



int app_fmr_adv_timer(void){

    if(!s_flag_fmr_adv){
        printf("[NOTICE] FMS ADV flag is disabled !!! \n");
        return -1;
    }

    s_fmr_adv_send_count++;

//    printf("%s count: %d  s_fmr_adv_count: %d \n",__FUNCTION__, s_fmr_adv_count, my_table_cnt);

    if(s_fmr_adv_send_count >= app_fmr_adv_timer_table[s_fmr_adv_table_index].count){
        s_fmr_adv_table_index++;

        u8 my_table_cnt = sizeof(app_fmr_adv_timer_table)/sizeof(app_fmr_adv_timer_table_t);
        if(s_fmr_adv_table_index >= my_table_cnt ){
            bls_ll_setAdvEnable(0);
            printf("End fmr direct adv \n");
            s_flag_fmr_adv = 0;
            return -1;
        }

        printf("FMS ADV parameter table_index: %d adv_interval_IDX MIN: %d  MAX: %d \n",
                                s_fmr_adv_table_index,
                                app_fmr_adv_timer_table[s_fmr_adv_table_index].adv_interval_min,
                                app_fmr_adv_timer_table[s_fmr_adv_table_index].adv_interval_max);

        app_fmr_adv_settings(   app_fmr_adv_timer_table[s_fmr_adv_table_index].adv_interval_min,
                                app_fmr_adv_timer_table[s_fmr_adv_table_index].adv_interval_max);
    }
    return 0;
}

void app_fmr_try_connect(void){

    if(!s_fmr_ctrl.en_periodic_wakeup) return;

    printf("%s \n", __FUNCTION__ );

    s_fmr_adv_send_count = 0;
    s_fmr_adv_table_index = 0;

    printf("FMS ADV parameter table_index: %d adv_interval_IDX MIN: %d  MAX: %d \n",
                            s_fmr_adv_table_index,
                            app_fmr_adv_timer_table[s_fmr_adv_table_index].adv_interval_min,
                            app_fmr_adv_timer_table[s_fmr_adv_table_index].adv_interval_max);

    app_fmr_adv_settings(   app_fmr_adv_timer_table[s_fmr_adv_table_index].adv_interval_min,
                            app_fmr_adv_timer_table[s_fmr_adv_table_index].adv_interval_max);
    blt_soft_timer_add(app_fmr_adv_timer, 1000000);

}

void app_fmr_delete_adv_task(void){
    s_flag_fmr_adv = 0;
}


void app_fmr_periodic_wake_up(void){
    /* FMS periodic wake up disabled */

    printf("%s. %d \n", __FUNCTION__, s_fmr_ctrl.en_periodic_wakeup);
    if(!s_fmr_ctrl.en_periodic_wakeup) return;

    /* periodic wakeup interval out of range */
    if( (0 == s_fmr_ctrl.periodic_wakeup_interval) || (s_fmr_ctrl.periodic_wakeup_interval > 1440) ) return;

    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number

    if(bond_number){
        printf("[NOTICE] FMS periodic wake up time: %d min \n", s_fmr_ctrl.periodic_wakeup_interval);

//        analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) | (PERIODIC_WAKE_UP));
        cpu_long_sleep_wakeup_32k_rc(DEEPSLEEP_MODE,PM_WAKEUP_TIMER|PM_WAKEUP_PAD, s_fmr_ctrl.periodic_wakeup_interval*CLOCK_32K_CLOCK_1MIN);
//        cpu_long_sleep_wakeup_32k_rc(DEEPSLEEP_MODE,PM_WAKEUP_TIMER|PM_WAKEUP_PAD,g_p_app_fmr_ctrl->periodic_wakeup_interval*CLOCK_32K_CLOCK_1S);   // for testing
    }
}


