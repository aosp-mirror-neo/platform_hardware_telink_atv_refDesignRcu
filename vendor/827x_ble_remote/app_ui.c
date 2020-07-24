/******************************************************************************
 * @file     app_ui.c
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
#include "app_config.h"

#include "application/keyboard/keyboard.h"
#include "application/usbstd/usbkeycode.h"
#include "../common/blt_led.h"
#include "../common/blt_soft_timer.h"

#include "rc_ir.h"
#include "battery_check.h"
#include "app_audio.h"
#include "app_flash_write.h"
#include "app_ir.h"

//////////////////// key type ///////////////////////
#define IDLE_KEY                   0
#define CONSUMER_KEY                  1
#define KEYBOARD_KEY                  2
#define IR_KEY                      3
#define VOICE_KEY                  4

_attribute_data_retention_    u8         key_type;
_attribute_data_retention_    int     key_not_released;


_attribute_data_retention_    u8         ota_is_working = 0;




_attribute_data_retention_    int     ir_not_released;
_attribute_data_retention_    u8         user_key_mode;
u8      ir_hw_initialed = 0;   //note: can not be retention variable
_attribute_data_retention_ u8 repairing_flag = 0;




extern    u32     latest_user_event_tick;

_attribute_data_retention_ u8 is_pairing_mode=0;

static const u16 vk_consumer_map[16] = {
    G00GLE_MKEY_UP,
    G00GLE_MKEY_DN,
    G00GLE_MKEY_LEFT,
    G00GLE_MKEY_RIGHT,
    G00GLE_MKEY_CENTER,
    G00GLE_MKEY_HOME,
    G00GLE_MKEY_BACK,
    G00GLE_MKEY_POWER,
    G00GLE_MKEY_VOL_MUTE,
    G00GLE_MKEY_CHN_UP,
    G00GLE_MKEY_CHN_DN,
    G00GLE_MKEY_GUIDE,
    G00GLE_MKEY_BOOKMARK,
    G00GLE_MKEY_ASSIST,
    G00GLE_MKEY_INPUT,
    G00GLE_MKEY_DASHBOARD,
};


/////////////////////////// led management /////////////////////
#if (BLT_APP_LED_ENABLE)

    enum{
        LED_POWER_ON = 0,
        LED_AUDIO_ON,    //1
        LED_AUDIO_OFF,    //2
        LED_SHINE_SLOW, //3
        LED_SHINE_FAST, //4
        LED_SHINE_OTA, //5
        LED_SHINE_PAIRING, //6
        LED_SHINE_FACTORY_RESET, //7
        LED_SHINE_BUG_REPORT,    //8
    };

    const led_cfg_t led_cfg[] = {
            {1000,     0,      1,      0x00,  },    //power-on, 1s on
            {100,      0 ,     0xff,   0x05,  },    //audio on, long on
            {0,        100 ,   0xff,   0x02,  },    //audio off, long off
            {500,      500 ,   2,      0x04,  },    //1Hz for 3 seconds
            {100,      100 ,   3,      0x04,  },    //5Hz for 3 seconds
            {250,      250 ,   200,    0x08,  },    //2Hz for 50 seconds
            {250,      250 ,   0xfe,   0x08,  },    //2Hz
            {250,      250 ,   4,      0x08,  },    //2Hz for 3 times
            {100,      100 ,   5,      0x08,  },    //5Hz for 1 seconds

    };

#endif


extern u32 keyScanLongPressTick;





#if (REMOTE_IR_ENABLE)
    //ir key
    #define TYPE_IR_SEND               1
    #define TYPE_IR_RELEASE            2

    ///////////////////// key mode //////////////////////
    #define KEY_MODE_BLE               0    //ble key
    #define KEY_MODE_IR                1    //ir  key


    static const u8 kb_map_ble[30] =    KB_MAP_BLE;
    static const u8 kb_map_ir[30] =     KB_MAP_IR;


    void ir_dispatch(u8 type, u8 syscode ,u8 ircode){

        if(!ir_hw_initialed){
            ir_hw_initialed = 1;
            rc_ir_init();
        }

        if(type == TYPE_IR_SEND){
            ir_nec_send(syscode,~(syscode),ircode);

        }
        else if(type == TYPE_IR_RELEASE){
            ir_send_release();
        }
    }


#endif



#if (BLE_REMOTE_OTA_ENABLE)
    void app_enter_ota_mode(void)
    {
        ota_is_working = 1;
        #if (BLT_APP_LED_ENABLE)
            //device_led_setup(led_cfg[LED_SHINE_OTA]);
        #endif
        bls_ota_setTimeout(90 * 1000 * 1000); //set OTA timeout  15 seconds
    }



    void app_debug_ota_result(int result)
    {

        #if(0 && BLT_APP_LED_ENABLE)  //this is only for debug

            gpio_set_output_en(GPIO_LED, 1);

            if(result == OTA_SUCCESS){  //led for debug: OTA success
                gpio_write(GPIO_LED, 1);
                sleep_us(500000);
                gpio_write(GPIO_LED, 0);
                sleep_us(500000);
                gpio_write(GPIO_LED, 1);
                sleep_us(500000);
                gpio_write(GPIO_LED, 0);
                sleep_us(500000);
            }
            else{  //OTA fail

                #if 0 //this is only for debug,  can not use this in application code
                    irq_disable();
                    WATCHDOG_DISABLE;



                    while(1){
                        gpio_write(GPIO_LED, 1);
                        sleep_us(200000);
                        gpio_write(GPIO_LED, 0);
                        sleep_us(200000);
                    }

                #endif

            }

            gpio_set_output_en(GPIO_LED, 0);
        #endif
    }
#endif






/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////


//This function process ...
void deep_wakeup_proc(void)
{

#if(DEEPBACK_FAST_KEYSCAN_ENABLE)
    //if deepsleep wakeup is wakeup by GPIO(key press), we must quickly scan this
    //press, hold this data to the cache, when connection established OK, send to master
    //deepsleep_wakeup_fast_keyscan
    if(analog_read(USED_DEEP_ANA_REG) & CONN_DEEP_FLG){
        if(kb_scan_key (KB_NUMLOCK_STATUS_POWERON, 1) && kb_event.cnt){
            deepback_key_state = DEEPBACK_KEY_CACHE;
            key_not_released = 1;
            memcpy(&kb_event_cache,&kb_event,sizeof(kb_event));
        }

        analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) & (~CONN_DEEP_FLG));
    }
#endif
}





void deepback_pre_proc(int *det_key)
{
#if (DEEPBACK_FAST_KEYSCAN_ENABLE)
    // to handle deepback key cache
    if(!(*det_key) && deepback_key_state == DEEPBACK_KEY_CACHE && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN \
            && clock_time_exceed(bls_ll_getConnectionCreateTime(), 25000)){

        memcpy(&kb_event,&kb_event_cache,sizeof(kb_event));
        *det_key = 1;

        if(key_not_released || kb_event_cache.keycode[0] == VOICE){  //no need manual release
            deepback_key_state = DEEPBACK_KEY_IDLE;
        }
        else{  //need manual release
            deepback_key_tick = clock_time();
            deepback_key_state = DEEPBACK_KEY_WAIT_RELEASE;
        }
    }
#endif
}

void deepback_post_proc(void)
{
#if (DEEPBACK_FAST_KEYSCAN_ENABLE)
    //manual key release
    if(deepback_key_state == DEEPBACK_KEY_WAIT_RELEASE && clock_time_exceed(deepback_key_tick,150000)){
        key_not_released = 0;
        u8         key_buf[8] = {0};
        key_buf[2] = 0;
        bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
        deepback_key_state = DEEPBACK_KEY_IDLE;
    }
#endif
}
#if BLE_AUDIO_ENABLE

#include "application/audio/gl_audio.h"

extern u8 app_audio_key_flags;
extern u8 htt_audio_model_key_press_flags;

typedef enum{
    APP_TWOKEYPRESS_NONE,
    APP_TWOKEYPRESS_PAIRING,
    APP_TWOKEYPRESS_FACTORYRESET,
    APP_TWOKEYPRESS_BUGREPORT,
}APP_TwoKeyPressMode;

extern int device_in_connection_state;

_attribute_data_retention_     u32 keyScanLongPressTick = 0;
_attribute_data_retention_     u32 LongPressTick = 0;
_attribute_data_retention_     APP_TwoKeyPressMode  TwoKeyPressMode = APP_TWOKEYPRESS_NONE;
_attribute_data_retention_     u8 TwoKeyPressFlag = 0;
_attribute_data_retention_     u16 SpecialKey = 0;

int app_special_key_delay_timer(void)
{
    printf("app_special_key_delay_timer\r\n");
    bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&SpecialKey, 2);
    blt_soft_timer_delete(app_special_key_delay_timer);
    return 0;
}


#if (STUCK_KEY_PROCESS_ENABLE)
     extern u32 stuckkey_keypresstimer;
#endif


u8 app_repairing(void)
{
    if(repairing_flag == 0x55)
    {
         printf("app_repairing\r\n");
         bls_ll_setAdvEnable(1);
         repairing_flag = 0;
         return 1;
    }
    return 0;
}

#define  KEY_CACHE_TIMEOUT   8000000
#define  KEY_CACHE_MAX       22

_attribute_data_retention_ u8  app_key_store_count=0;
_attribute_data_retention_ u8  app_key_store_buffer[KEY_CACHE_MAX]={0};
u8 app_keycache(u8 keyindex)
{
    u8 i;

    if(device_in_connection_state == 0)
    {
        if(app_key_store_count < KEY_CACHE_MAX)
        {
            for(i=0;i<app_key_store_count;i++)
            {
                if(app_key_store_buffer[i] == keyindex)
                {
                    i = 0x55;
                    break;
                }
            }
            if(i != 0x55)
                app_key_store_buffer[app_key_store_count++] = keyindex;
            return 0;
        }
        else
        {
            app_key_store_count = KEY_CACHE_MAX;
            for(i=0; i<KEY_CACHE_MAX-1; i++)
            {
                app_key_store_buffer[i] = app_key_store_buffer[i+1];
            }
            app_key_store_buffer[i] = keyindex;
        }
        printf("app_keycache = \r\n");
        for(i=0;i<app_key_store_count;i++)
            printf(" %x",app_key_store_buffer[i]);
    }
    else if(device_in_connection_state && app_key_store_count != 0)
    {
        if(app_key_store_count < KEY_CACHE_MAX)
        {
            app_key_store_buffer[app_key_store_count++] = keyindex;
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

void app_key_send_store(void)
{
    u8 i;
    u8 key0;
    u8 key_value;
    u8 key_buf[8] = {0,0,0,0,0,0,0,0};
    u16 consumer_key[2]={0};

    if(app_key_store_count == 0) return;
    if(device_in_connection_state == 0) return;
    printf("app_key_send_store,app_key_store_count=%x\r\n",app_key_store_count);

    key0 = app_key_store_buffer[0];
    printf("send cache key key=%x\r\n",key0);
    if(ir_key_is_suppress(key0) == 0)
    {
        printf("is suppress\r\n");
        if(ir_fallback_send_key_code(key0,1))
        {
            ir_fallback_send(0);   //release
        }
    }
    else
    {
        programming_key_set(0);
        key_value = kb_map_ble[key0];
        if(key_value >= 0xf0 )
        {
            consumer_key[0] = vk_consumer_map[key_value & 0x0f];
            consumer_key[1] = 0;
            printf("consume key=%x \r\n",consumer_key[0]);   //down
            bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);
            consumer_key[0] = 0;                             //release
            consumer_key[1] = 0;
            bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);

        }
        else
        {
            printf("KEYBOARD_KEY_key_value=%x \r\n",key_value);
            key_buf[2] = key_value;                       //down
            bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);

            key_buf[2] = 0;                               //release
            bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
        }
    }

    for(i=1; i<app_key_store_count; i++)
    {
        app_key_store_buffer[i-1] = app_key_store_buffer[i];
    }
    app_key_store_count --;
}
const u8 navi_key_index[4] = {1,4,0x0f,0x12};


u8 app_is_navi_key(u8 data0,u8 data1)
{
    u8 flag[2]={5,5};

    if(data0 == data1) return 0;
    for(u8 i=0;i<4;i++)
    {
        if(data0 == navi_key_index[i])
            flag[0] = i;
        if(data1 == navi_key_index[i])
            flag[1] = i;
    }
    if((flag[0]!=5) && (flag[1]!=5))
        return 1;
    return 0;
}

void key_change_proc(void)
{

    latest_user_event_tick = clock_time();  //record latest key change time
    ir_test_init();

    u8 key0 = kb_event.keycode[0];
    u8 key1 = kb_event.keycode[1];
    u8 key_value;
    u8 key_buf[8] = {0,0,0,0,0,0,0,0};
    u16 consumer_key[2]={0};

    key_not_released = 1;
    if (kb_event.cnt == 2)   //two key press, do  not process
    {
        //retime
        advertise_tick_retime();
#if (STUCK_KEY_PROCESS_ENABLE)
        stuckkey_keypresstimer = clock_time() | 1;
#endif
        if ( (key0 == 0x0a && key1 == 0x07) || (key0 == 0x07 && key1 == 0x0a)) //Pairing Mode
        {
            printf("pair start\r\n");
            blt_soft_timer_delete(app_special_key_delay_timer);
            device_led_setup(led_cfg[LED_AUDIO_ON]);

            keyScanLongPressTick = clock_time() | 1; //none zero
            LongPressTick = 4000000;
            TwoKeyPressMode = APP_TWOKEYPRESS_PAIRING;
            TwoKeyPressFlag = 0x55;
        }
        else if ( (key0 == 0x16 && key1 == 0xc0) || (key0 == 0xc0 && key1 == 0x16)) //Factory Reset Mode
        {
            printf("Factory Reset Mode start\r\n");
            device_led_off();
            device_led_setup(led_cfg[LED_AUDIO_ON]);
            keyScanLongPressTick = clock_time() | 1; //none zero
            LongPressTick = 4000000;
            TwoKeyPressMode = APP_TWOKEYPRESS_FACTORYRESET;
            TwoKeyPressFlag = 0x55;
        }
        else if ( (key0 == 0x17 && key1 == 0x15) || (key0 == 0x15 && key1 == 0x07)) //Bug report
        {
            printf("bug report start\r\n");
            device_led_setup(led_cfg[LED_AUDIO_ON]);
            keyScanLongPressTick = clock_time() | 1; //none zero
            LongPressTick = 1000000;
            TwoKeyPressMode = APP_TWOKEYPRESS_BUGREPORT;
            TwoKeyPressFlag = 0x55;
        }
        else
        {
            if(device_in_connection_state == 0)
            {
                if(app_is_navi_key(key0,key1) == 0)
                {
                    printf(" stop double key in ir mode\r\n");
                    TwoKeyPressFlag = 0xaa;
                    ir_send_release();
                }
                else
                {
                    printf("double key is navi in ir mode\r\n");
                    if(ir_send_ctrl.is_sending == IR_SENDING_NONE)
                    {
                        key_value = kb_map_ir[key0];
                        key_type = IR_KEY;
                        ir_dispatch(TYPE_IR_SEND, 0x88, key_value);
                    }
                }
            }
            else
            {
                if(app_is_navi_key(key0,key1) == 0)
                {
                    printf(" stop double key in rf mode\r\n");
                    if(key_type == CONSUMER_KEY)
                    {
                        consumer_key[0] = 0;
                        consumer_key[1] = 0;
                        bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);
                    }
                    else if(key_type == KEYBOARD_KEY)
                    {
                        key_buf[2] = 0;
                        bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
                    }
                }
                else
                {
                    printf("double key is navi in rf mode,key0=%x,key1=%x\r\n",key0,key1);
                    u16 comb_key[2];
                    comb_key[0] = vk_consumer_map[kb_map_ble[key0] & 0x0f];
                    comb_key[1] = vk_consumer_map[kb_map_ble[key1] & 0x0f];
                    bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&comb_key, 4);
                }
            }
        }
    }
    else if(kb_event.cnt == 1)
    {
          //retime
          advertise_tick_retime();

          #if (BLT_APP_LED_ENABLE)
            device_led_setup(led_cfg[LED_SHINE_FAST]);
          #endif
          printf("key_id=%x\r\n",key0);

#if (STUCK_KEY_PROCESS_ENABLE)
          stuckkey_keypresstimer = clock_time() | 1;
#endif

#if (BLE_AUDIO_ENABLE)
          if(device_in_connection_state)
          {
            if (key0 == VOICE)
            {
                if(google_voice_ctl & FLAG_NOTIFY_AUDIO_DATA) return;
                printf("CONSUMER_KEY\n");
                key_type = VOICE_KEY;
                google_voice_start();
                return;
            }
         }
#endif
          key_type = IR_KEY;
          if(app_keycache(key0) == 1)
          {
              return;
          }
          if(ir_key_is_suppress(key0) == 0)
          {
              printf("is suppress\r\n");
              if(ir_fallback_send_key_code(key0,1))
              {
                  if(!ir_not_released)
                      ir_not_released = 1;
                  return;
              }
          }
          programming_key_set(0);
          if(device_in_connection_state)
          {

                key_value = kb_map_ble[key0];

                if(key_value >= 0xf0 ){

                    key_type = CONSUMER_KEY;
                    consumer_key[0] = vk_consumer_map[key_value & 0x0f];
                    consumer_key[1] = 0;
                    printf("consume key=%x \r\n",consumer_key[0]);
                    if((key0 == 0x07) || (key0 == 0x0a))
                    {
                         SpecialKey = consumer_key[0];
                         blt_soft_timer_add(&app_special_key_delay_timer, 60000);
                         return;
                    }
                    bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);
                    if(consumer_key[0] == G00GLE_MKEY_CENTER)
                    {
                        google_voice_dpad_select();
                    }
                }
                else
                {
                    printf("KEYBOARD_KEY_key_value=%x \r\n",key_value);

                    key_type = KEYBOARD_KEY;
                    key_buf[2] = key_value;
                    bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);
                }

          }
          else
          {
             if(key0 == VOICE)
                 key_value = 0x46;
             else
                key_value = kb_map_ir[key0];
             if((key_value == IR_BACK) &&(is_pairing_mode == 0x55))
             {
                 device_led_off();
                 bls_ll_setAdvEnable(0);
             }
             printf("\r\nir_key_value=%x\r\n",key_value);
             key_type = IR_KEY;
             if(!ir_not_released){
                 ir_dispatch(TYPE_IR_SEND, 0x88, key_value);
                 ir_not_released = 1;
             }
          }

    }
    else   //kb_event.cnt == 0,  key release
    {
        printf("key release\r\n");
        if(audio_start_reason == REASON_HTT){
            printf("key release_audio_start_reason:0x%x\n",audio_start_reason);
            google_voice_ctl |= FLAG_AUDIO_CLOSE;
            audio_stop_reason = REASON_RELEASE_HTT;
        }
        key_not_released = 0;
        keyScanLongPressTick = 0;
        LongPressTick = 0;
        #if (STUCK_KEY_PROCESS_ENABLE)
        stuckkey_keypresstimer = 0;
        #endif
        if(is_programming_key_send() == 1)
        {
           printf("notify up\r\n");
           ir_fallback_send(0);
        }
        programming_key_set(0);
        printf("key_type=%x\r\n",key_type);
        if(key_type == CONSUMER_KEY)
        {
            consumer_key[0] = 0;
            consumer_key[1] = 0;
            bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);
        }
        else if(key_type == KEYBOARD_KEY)
        {
            key_buf[2] = 0;
            bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
        }

#if (REMOTE_IR_ENABLE)
        else if(key_type == IR_KEY)
        {
            if(ir_not_released){
                ir_not_released = 0;
                if( TwoKeyPressFlag != 0xaa)
                {
                    ir_dispatch(TYPE_IR_RELEASE, 0, 0);  //release
                }
                ir_repeat_delay_release_time();
            }
        }
#endif
        TwoKeyPressFlag = 0;
    }


}



#define GPIO_WAKEUP_KEYPROC_CNT                3


_attribute_data_retention_    static int gpioWakeup_keyProc_cnt = 0;
_attribute_data_retention_    static u32 keyScanTick = 0;


void proc_keyboard (u8 e, u8 *p, int n)
{

    //when key press gpio wakeup suspend, proc keyscan at least GPIO_WAKEUP_KEYPROC_CNT times
    //regardless of 8000 us interval
    if(e == BLT_EV_FLAG_GPIO_EARLY_WAKEUP){
        gpioWakeup_keyProc_cnt = GPIO_WAKEUP_KEYPROC_CNT;
    }
    else if(gpioWakeup_keyProc_cnt){
        gpioWakeup_keyProc_cnt --;
    }


    if(gpioWakeup_keyProc_cnt || clock_time_exceed(keyScanTick, 8000)){
        keyScanTick = clock_time();
    }
    else{
        return;
    }

    kb_event.keycode[0] = 0;
    int det_key = kb_scan_key (0, 1);


#if(DEEPBACK_FAST_KEYSCAN_ENABLE)
    if(deepback_key_state != DEEPBACK_KEY_IDLE){
        deepback_pre_proc(&det_key);
    }
#endif


    if (det_key){
        key_change_proc();
    }

#if(DEEPBACK_FAST_KEYSCAN_ENABLE)
    if(deepback_key_state != DEEPBACK_KEY_IDLE){
        deepback_post_proc();
    }
#endif

    if(keyScanLongPressTick && clock_time_exceed(keyScanLongPressTick, LongPressTick))
    {
        keyScanLongPressTick = 0;
        if(TwoKeyPressMode == APP_TWOKEYPRESS_PAIRING)
        {
            printf("trigger pair\r\n");
            printf("ir_send_ctrl.is_sending=%x",ir_send_ctrl.is_sending);
            device_led_setup(led_cfg[LED_SHINE_PAIRING]);
            is_pairing_mode = 0x55;
            if( device_in_connection_state == 0)
            {
                bls_ll_setAdvEnable(1);  //adv enable
            }
            else
            {
                printf("re-pairing \r\n");
                bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN); //push terminate cmd into ble TX buffer
                repairing_flag = 0x55;
            }
        }
        else if(TwoKeyPressMode == APP_TWOKEYPRESS_FACTORYRESET)
        {
            printf("Factory Reset Mode ongoning\r\n");
            device_led_setup(led_cfg[LED_SHINE_FACTORY_RESET]);
            ir_table_init();
            if(device_in_connection_state)
            {
                bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN); //push terminate cmd into ble TX buffer
                bls_ll_setAdvEnable(0);  //adv enable
            }
            else
            {
                u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
                if(bond_number)
                     bls_smp_param_deleteByIndex(0);
            }
        }
        else if(TwoKeyPressMode == APP_TWOKEYPRESS_BUGREPORT)
        {
            printf("APP_TWOKEYPRESS_BUGREPORT ongoning\r\n");
            device_led_setup(led_cfg[LED_SHINE_BUG_REPORT]);
            if(device_in_connection_state == 0)
            {
                 ir_dispatch(TYPE_IR_SEND, 0x88, 0x96);
                 ir_dispatch(TYPE_IR_RELEASE, 0, 0);  //release
            }
            else
            {
                u16 consumer_key[2];
                consumer_key[0] = G00GLE_MKEY_BACK;
                consumer_key[1] = G00GLE_MKEY_CENTER;
                bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);
            }
        }
    }
    app_key_send_store();
}
#endif



extern u32    scan_pin_need;



void app_pairing_led(void)
{
    device_led_setup(led_cfg[LED_SHINE_PAIRING]);
}

int gpio_test0(void)
{
    //gpio 0 toggle to see the effect
    DBG_CHN4_TOGGLE;

    return 0;
}


int gpio_test1(void)
{
    //gpio 1 toggle to see the effect
    DBG_CHN5_TOGGLE;


    return 0;

}

int gpio_test2(void)
{
    DBG_CHN6_TOGGLE;

    return 0;
}

int gpio_test3(void)
{
    DBG_CHN7_TOGGLE;

    return 0;
}






void app_ui_init_normal(void)
{



    /////////// keyboard gpio wakeup init ////////
    u32 pin[] = KB_DRIVE_PINS;
    for (int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
    {
        cpu_set_gpio_wakeup (pin[i], Level_High,1);  //drive pin pad high wakeup deepsleep
    }

    bls_app_registerEventCallback (BLT_EV_FLAG_GPIO_EARLY_WAKEUP, &proc_keyboard);

    #if (BLE_AUDIO_ENABLE)
        #if (BLE_DMIC_ENABLE)
            dmic_gpio_reset();
        #else
            amic_gpio_reset();
        #endif
    #endif


#if (BLT_APP_LED_ENABLE)
    printf("led on 1s\r\n");
    device_led_init(GPIO_LED, LED_ON_LEVAL);  //LED initialization
    device_led_setup(led_cfg[LED_POWER_ON]);
#endif


#if (REMOTE_IR_ENABLE)
    user_key_mode = analog_read(USED_DEEP_ANA_REG) & IR_MODE_DEEP_FLG ? KEY_MODE_IR : KEY_MODE_BLE;
#endif



#if (BLT_TEST_SOFT_TIMER_ENABLE)
    blt_soft_timer_init();
    //blt_soft_timer_add(&gpio_test0, 35000);
    //blt_soft_timer_add(&gpio_test1, 23000);
    //blt_soft_timer_add(&gpio_test2, 27000);
    //blt_soft_timer_add(&gpio_test3, 33000);
#endif

}



void app_ui_init_deepRetn(void)
{
    /////////// keyboard gpio wakeup init ////////
    u32 pin[] = KB_DRIVE_PINS;
    for (int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
    {
        cpu_set_gpio_wakeup (pin[i], Level_High, 1);  //drive pin pad high wakeup deepsleep
    }

    #if (BLE_AUDIO_ENABLE)
        #if (BLE_DMIC_ENABLE)
            dmic_gpio_reset();
        #else
            amic_gpio_reset();
        #endif
    #endif

#if (BLT_APP_LED_ENABLE)
    device_led_init(GPIO_LED, 1);  //LED initialization
#endif

#if (REMOTE_IR_ENABLE)
    user_key_mode = analog_read(USED_DEEP_ANA_REG) & IR_MODE_DEEP_FLG ? KEY_MODE_IR : KEY_MODE_BLE;
#endif
}



extern u16 atv_char_ctl_ccc;
extern u16 atv_char_rx_ccc;

int att_ccc_control(void* p){
    printf("att_ccc_control\n");
    rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;
    u16 handle = (pw->hh << 8) + pw->hl;

    u16 data = (pw->dat[1] << 8) + pw->dat[0];

    array_printf(pw->dat,(pw->l2cap - 3));

    u8 save_to_flash = 1;

    switch(handle){
        case AUDIO_GOOGLE_RX_CCC_H:
        {
            printf("AUDIO_GOOGLE_RX_CCC_H\n");

            atv_char_rx_ccc = data;

            if(data){
                if(flag_ccc_data & FLAG_GOOGLE_RX_CCC){
                    save_to_flash = 0;
                }else{
                    flag_ccc_data |= FLAG_GOOGLE_RX_CCC;
                }

            }else{
                if(flag_ccc_data & FLAG_GOOGLE_RX_CCC){
                    flag_ccc_data &= ~FLAG_GOOGLE_RX_CCC;
                }else{
                    save_to_flash = 0;
                }
            }
            break;
        }
        case AUDIO_GOOGLE_CTL_CCC_H:
        {
            printf("AUDIO_GOOGLE_CTL_CCC_H\n");

            atv_char_ctl_ccc = data;

            if(data){
                if(flag_ccc_data & FLAG_GOOGLE_CTL_CCC){
                    save_to_flash = 0;
                }else{
                    flag_ccc_data |= FLAG_GOOGLE_CTL_CCC;
                }

            }else{
                if(flag_ccc_data & FLAG_GOOGLE_CTL_CCC){
                    flag_ccc_data &= ~FLAG_GOOGLE_CTL_CCC;
                }else{
                    save_to_flash = 0;
                }
            }
            break;
        }
        case OTA_CMD_OUT_CCC_H:
        {
            printf("OTA_CMD_OUT_CCC_H\n");

            extern u16 my_Otaccc;
            my_Otaccc = data;

            save_to_flash = 0;
            break;
        }
        default:
        {
            save_to_flash = 0;
            break;
        }
    }

    if(save_to_flash){
        //save ccc data to flash
        write_ccc_info(&flag_ccc_data);
    }

    return 0;
}
