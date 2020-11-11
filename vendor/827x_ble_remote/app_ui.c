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

#if (MP_TEST_MODE)
#include "app_test.h"
#endif

#include "app_custom.h"

//////////////////// key type ///////////////////////
#define IDLE_KEY                   0
#define CONSUMER_KEY               1
#define KEYBOARD_KEY               2
#define IR_KEY                     3
#define VOICE_KEY                  4

_attribute_data_retention_    u8      key_type;
_attribute_data_retention_    int     key_not_released;


_attribute_data_retention_    u8       ota_is_working = 0;




_attribute_data_retention_    int     ir_not_released=0;
_attribute_data_retention_    u8      user_key_mode;
u8      ir_hw_initialed = 0;   //note: can not be retention variable
_attribute_data_retention_ u8 repairing_flag = 0;
_attribute_data_retention_ u8 is_pairing_mode=0;
_attribute_data_retention_ u8 is_reconn_mode=0;
_attribute_data_retention_    u8  ota_flag = 0;  //0:pause 1:continue

extern    u32     latest_user_event_tick;
extern u8 lowbat;
extern int device_in_connection_state;
extern u8 flag_schedule_ota;
extern u8  device_timeout_state;
extern u8 wakeup_src;
extern u8 wakeup_key[15];
extern u8 power_down_flag;
extern u8 wakeup_key_send_flag;
extern int ir_fallback_send(u8 key_down);
extern void device_led_off(u8 led);
extern void app_enter_deep_timeset(void);
extern void app_set_adv_interval_downgrade_indirect(void);
extern void app_tx_power_wakeup(u8 keyid);
extern u8  app_custom_is_wakeup_key(u8 keyid);
extern ble_sts_t app_terminate(u8 reason);
static const u16 vk_consumer_map[] = {
    G00GLE_MKEY_INFO,
    G00GLE_MKEY_SUBTITLE,
    G00GLE_MKEY_RED,
    G00GLE_MKEY_GREEN,
    G00GLE_MKEY_YELLOW,
    G00GLE_MKEY_BLUE,
    G00GLE_MKEY_YOUTUBE,
    G00GLE_MKEY_NETFLIX,
    G00GLE_MKEY_DISNEY,
    G00GLE_MKEY_HBOMAX,
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
        LED_SHINE_SUCCESS, //5
        LED_SHINE_PAIRING, //6
        LED_SHINE_PTT, //7
        LED_SHINE_LOWBAT,
        LED_PAIR_ERROR,
        LED_OTA_START,
        LED_OTA_END,
        LED_KEYPRESS
    };

    const led_cfg_t led_cfg[] = {
            {APP_LED_GREEN,   1,   1000,     0,      1,      0,  },    //power-on, 1s on
            {APP_LED_GREEN,   1,   100,      0 ,     0,      1,  },    //audio on, long on
            {APP_LED_GREEN,   0,    0,       100 ,   0,      1,  },    //audio off, long off
            {APP_LED_GREEN,   1,   500,      500 ,   2,      0,  },    //1Hz for 3 seconds
            {APP_LED_GREEN,   1,   100,      100 ,   1,      0,  },    //5Hz for 3 seconds
            {APP_LED_GREEN,   0,   100,      100 ,   2,      0,  },    //2Hz for 50 seconds
            {APP_LED_GREEN,   1,   250,      250 ,   0,      0,  },    //2Hz
            {APP_LED_GREEN,   1,   500,      500 ,   0,      0,  },    //1Hz
            {APP_LED_RED,     1,   100,      100 ,   5,      0,  },    //5Hz for 5 seconds
            {APP_LED_RED,     1,   50,       50,     4,      0,  },    
            {APP_LED_RED,     1,   100,      0,      0,      1,  },    
            {APP_LED_RED,     1,   0,        1000,   3,      1,  },
            {APP_LED_RED,     1,   100,      100 ,   1,      0,  },    //5Hz for 3 seconds

    };

#endif

void app_pairing_error(void)
{
    device_led_setup(led_cfg[LED_PAIR_ERROR]);
}

extern u32 keyScanLongPressTick;


#if (REMOTE_IR_ENABLE)
    //ir key
    #define TYPE_IR_SEND               1
    #define TYPE_IR_RELEASE            2

    ///////////////////// key mode //////////////////////
    #define KEY_MODE_BLE               0    //ble key
    #define KEY_MODE_IR                1    //ir  key


    _attribute_data_retention_ u8 *p_kb_map_ble;
    _attribute_data_retention_ u8 *p_kb_map_ir;
    _attribute_data_retention_ u8 *p_kb_map_normal;


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
    #include "app_ota.h"
    extern u8  mac_public[6];
    u8 FW_dec_flag = 0;

    void app_ota_ver_check(void){
        printf("CMD_OTA_FW_VERSION\n");
        ota_ctl_flag = 0;

        u8 FW_version[6] = {0};
        FW_version[0] = CLIENT_REPORT_CMD;
        FW_version[1] = TYPE_VERSION;
        FW_version[2] = *(volatile u8 *)0x02;
        FW_version[3] = *(volatile u8 *)0x03;
        FW_version[4] = *(volatile u8 *)0x04;
        FW_version[5] = *(volatile u8 *)0x05;

        bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, FW_version, 6);
        printf("notify_ver:");
        array_printf(FW_version,6);

        ota_ctl_flag |= FW_VER_CHECK;

        //check battery voltage
        u16 ota_battery_voltage = get_battery_voltage();
        printf("get_battery_voltage:%d\n",ota_battery_voltage);

        if(ota_battery_voltage < OTA_PROTECT_VOLTAGE){

            u8 report_data[3];
            report_data[0] = CLIENT_REPORT_CMD;
            report_data[1] = TYPE_OTA_RESULT;
            report_data[2] = OTA_LOW_VOLTAGE;

            bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, report_data, 3);
            printf("OTA_LOW_VOLTAGE\n");

            ota_ctl_flag = 0;
        }else{
            ota_ctl_flag |= BATT_CHECK_OK;
            printf("OTA_VOLTAGE_OK. 0xota_ctl_flag:%x \n",ota_ctl_flag);
        }
    }

    void app_ota_signature(){
        u8 aes_plaintext[16] = {'g','o','o','g','l','e','G','1','0','R','C','U',0x00,0x00,0x00,0x00,};
        u8 signature_key[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',};  //OTA key;
        u8 notify_data[20] = {0};

        aes_plaintext[12] = mac_public[0] ^ mac_public[1];
        aes_plaintext[13] = mac_public[1] ^ mac_public[2];
        aes_plaintext[14] = mac_public[2] ^ mac_public[3];
        aes_plaintext[15] = mac_public[3] ^ mac_public[4];

//        printf("aes_plaintext:");
//        array_printf(aes_plaintext,16);
//        printf("signature_key:");
//        array_printf(signature_key,16);

        aes_ll_encryption(signature_key,aes_plaintext,notify_data+4);
        notify_data[2] = CLIENT_REPORT_CMD;
        notify_data[3] = TYPE_SINATURE_DATA;

        printf("notify_data:");
        array_printf(notify_data+2,18);
        bls_att_pushNotifyData (OTA_CMD_OUT_DP_H, notify_data+2, 18);

        ota_ctl_flag |= OTA_SIGNATURE;
    }
    void app_enter_ota_mode(void *data)
    {
        printf("start data:");
        array_printf(data,4);
        bls_pm_setSuspendMask (SUSPEND_DISABLE);
        u8 check_data[4];

        check_data[0] = mac_public[0] ^ mac_public[1];
        check_data[1] = mac_public[1] ^ mac_public[2];
        check_data[2] = mac_public[2] ^ mac_public[3];
        check_data[3] = mac_public[3] ^ mac_public[4];

        if(memcmp(check_data,data,4 )){

            u8 report_data[3];
            report_data[0] = CLIENT_REPORT_CMD;
            report_data[1] = TYPE_OTA_RESULT;
            report_data[2] = OTA_START_KEY_ERR;

            bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, report_data, 3);
            printf("OTA_START_KEY_ERR\n");

            ota_ctl_flag = 0;
            return;
        }
        ota_ctl_flag |= OTA_START;
        ota_flag = 1;

        FW_dec_flag = 0;    //clear firmware decryption flag

        //add user code
        printf("ota start red led on\r\n");
        device_led_setup(led_cfg[LED_OTA_START]);
        ota_is_working = 1;
        bls_ota_setTimeout(90 * 1000 * 1000); //set OTA timeout  90 seconds

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
     gpio_set_output_en(GPIO_LED, 0);
     #endif
 }

 void app_ota_result(int result)
 {
     printf("google_ota_result result=%x\r\n",result);
     u8 report_data[3];
     report_data[0] = CLIENT_REPORT_CMD;
     report_data[1] = TYPE_OTA_RESULT;
     report_data[2] = (u8)result;
     if(device_in_connection_state)
     {
         u16 i = 0;
         while(bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, report_data, 3) &&  i < 2000){
         sleep_us(100);
         i++;
         }

         i = 0;
         while(app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN) &&  i < 2000){
         sleep_us(100);
         i++;
         }

         i = 0;
         while(blc_ll_getTxFifoNumber() &&  i < 400){
         sleep_us(10000);//printf("waiting TX fifo is empty\n"); //waiting TX fifo is empty
         i++;
         }
     }
     if(result == 0)
     {
         printf("ota end red led off\r\n");
         device_led_setup(led_cfg[LED_OTA_END]);
         printf("ota_suc and write_ana\r\n");
         analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) | OTA_FLG);
         printf("analog_read(USED_DEEP_ANA_REG)=%x\r\n",analog_read(USED_DEEP_ANA_REG));
     }
}

extern u16 count_schedule_ota;
void app_schedule_ota(u8 *data)
{
    //array_printf(data,4);
    u16 count = (data[0]<<8) + data[1];

    if(count ==0)
    {
        count_schedule_ota = 90;

    }else if(count >= 1440){    //24 hour
        count_schedule_ota = 1440;
    }else{
        count_schedule_ota = count;
    }
    printf("app_schedule_ota:0x%d\n",count);

    flag_schedule_ota = 0x55;
    analog_write(DEEP_ANA_REG2, 0);
}

int app_ota_timeoutcb(void)
{
    printf("app_ota_timeoutcb\r\n");
    terminate_google_ota_proc(OTA_TIMEOUT);
    return -1;
}

void app_ota_timeout(void)
{
    extern u8 google_ota_start_flag;
    google_ota_start_flag = 0;
    app_pairing_error();
    blt_soft_timer_add(&app_ota_timeoutcb, 1000000);
}

void app_ota_status(u8 status)
{
    if(device_in_connection_state == 0) return;
    if(ota_is_working == 0) return;
    ble_sts_t stat;
    if((status == 0) && (ota_flag == 1))
    {
        stat = google_ota_pause();
        ota_flag = 0;
        printf("google_ota_pause stat=%x\r\n",stat);
    }
    
    if((status == 1) && (ota_flag == 0))
    {
        ble_sts_t stat;
        stat = google_ota_continue();
        ota_flag = 1;
        printf("google_ota_continue stat=%x\r\n",stat);
    }
}

void app_ota_terminate(void)
{
    printf("app_ota_terminate\n");
    u8 report_data[3];
    report_data[0] = CLIENT_REPORT_CMD;
    report_data[1] = TYPE_OTA_RESULT;
    report_data[2] = OTA_USER_TERMINATE;
    u16 i = 0;
    while(bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, report_data, 3) &&  i < 2000){
       sleep_us(100);
       i++;
    }
}

#define SCHE_OTA_1MIN    32000*60
#define SCHE_OTA_1HOUR   32000*60*60
void app_schedule_ota_enter_deep(void)
{
    u8 sch_ota_timer[7]= {0,90,2,3,5,8,13};
    u8 data = analog_read(DEEP_ANA_REG2);
    if(data == 0)
    {
        if((flag_schedule_ota == 0x55) && (device_timeout_state == 0x55))
        {
            printf("sch ota first\r\n");
            analog_write(DEEP_ANA_REG2, 1);
            cpu_long_sleep_wakeup_32k_rc(DEEPSLEEP_MODE,PM_WAKEUP_TIMER|PM_WAKEUP_PAD,count_schedule_ota*SCHE_OTA_1MIN);
        }
        else
        {
            printf("idle to sleep\r\n");
            cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);
        }
    }
    else
    {
        if(wakeup_src == 2)
        {
            data++;
            printf("sch ota timer ++,data=%x\r\n",data);
        }
        else
            printf("sch ota timer maintain\r\n");
        if(data >= 7)
        {
            printf("idle to sleep\r\n");
            analog_write(DEEP_ANA_REG2, 0);
            cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0); 
        }
        analog_write(DEEP_ANA_REG2, data);
        if(data<7)
        printf("sch ota data =%x,time =%x\r\n",data,sch_ota_timer[data]);
        if(data == 1)
            cpu_long_sleep_wakeup_32k_rc(DEEPSLEEP_MODE,PM_WAKEUP_TIMER|PM_WAKEUP_PAD,sch_ota_timer[data]*SCHE_OTA_1MIN);
        else
            cpu_long_sleep_wakeup_32k_rc(DEEPSLEEP_MODE,PM_WAKEUP_TIMER|PM_WAKEUP_PAD,sch_ota_timer[data]*SCHE_OTA_1HOUR);
    }
}

void app_schedule_ota_send_notify_to_continue_ota(void)
{ 
    u8 dat[2] = {0};
    u8 report_data[4] = {CLIENT_REPORT_CMD,0x04,0x73,0x6f};
    u8 data = analog_read(DEEP_ANA_REG2);

    dat[0] = 0x73;
    dat[1] = 0x6f;
    if((wakeup_src == 2) && (data))//wake up for timer
    {
        printf("app_schedule_ota_send_notify_to_continue_ota\r\n");
        bls_att_pushNotifyData(HID_CONSUME_REPORT_INPUT_DP_H, dat, 2); 
        bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, report_data, 4);
        analog_write(DEEP_ANA_REG2, 0);
    }
}

void app_schedule_ota_wakeup_host(void)
{
    u8 data = analog_read(DEEP_ANA_REG2);
    u8 i;
    u8 flag=0;

    if((wakeup_src == 2) && (data) && (power_down_flag == 2))
    {
        for(i=0;i<15;i++)
        {
            if(wakeup_key[i] != 0xff)
                flag++;
        }
        if(flag)
        {
            printf("app_schedule_ota_wakeup_host\r\n");
            app_tx_power_wakeup(0xff);
        }
    }
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
    APP_TWOKEYPRESS_NONE=0,
    APP_TWOKEYPRESS_PAIRING,
    APP_TWOKEYPRESS_FACTORYRESET,
    APP_TWOKEYPRESS_BUGREPORT,
    APP_TWOKEYPRESS_ONEKEY
}APP_TwoKeyPressMode;


_attribute_data_retention_     u32 keyScanLongPressTick = 0;
_attribute_data_retention_     u32 LongPressTick = 0;
_attribute_data_retention_     APP_TwoKeyPressMode  TwoKeyPressMode = APP_TWOKEYPRESS_NONE;
_attribute_data_retention_     u8 TwoKeyPressFlag = 0;
_attribute_data_retention_     u16 SpecialKey = 0;


int app_special_key_delay_timer(void)
{
    printf("app_special_key_delay_timer\r\n");
    bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&SpecialKey, 2);
    return -1;
}

int app_ir_bugreport_key_delay_timer(void)
{
    printf("app_ir_bugreport_key_delay_timer\r\n");
    printf("\r\nir_key_value=%x\r\n",(u8)SpecialKey);
    key_type = IR_KEY;
    if(!ir_not_released){
        ir_dispatch(TYPE_IR_SEND, 0x88, (u8)SpecialKey);
        ir_not_released = 1;
    }

    if(key_not_released == 0)
    {
        if(ir_not_released)ir_not_released = 0;
        if( ir_send_ctrl.is_sending)
        {
            ir_send_release();
        }
        return -1;
    }
    if(key_not_released == 1)
        return -1;
    return -1;
}

#if (STUCK_KEY_PROCESS_ENABLE)
     extern u32 stuckkey_keypresstimer;
#endif

extern void app_adv_indirect(void);
u8 app_repairing(void)
{
    if(repairing_flag == 0x55)
    {
         printf("app_repairing\r\n");
         app_adv_indirect();
         repairing_flag = 0;
         notify_get_rsp_en = 0;
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

    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    if(bond_number == 0) return 0;   
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
        return 0;
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
    /*
    if(ir_key_is_suppress(key0) == 0)
    {
        printf("is suppress\r\n");
        if(ir_fallback_send_key_code(key0,1))
        {
            ir_fallback_send(0);   //release
        }
    }
    else
    */
    {
        programming_key_set(0);
        key_value = p_kb_map_ble[key0];
        if(key_value >= CR_MEDIA_KEY_INDEX )
        {
            if(key_value == CR_DASHBOARD)
            {
                consumer_key[0] = key_setting_or_noti[1];
            }
            else if(key_value == CR_GUIDE)
            {
                consumer_key[0] = key_guide_or_livetv[1];
            }
            else
                consumer_key[0] = vk_consumer_map[key_value-CR_MEDIA_KEY_INDEX];
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

void app_release_hid_key(void)
{
    u8 key_buf[8] = {0,0,0,0,0,0,0,0};
    u16 consumer_key[2]={0};
    u16 i=0;

    if(device_in_connection_state)
    {
        while(bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4) && i<2000)
        {
            sleep_us(100);
            i++;
        }
        i = 0;
        while(bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8) && i<2000)
        {
            sleep_us(100);
            i++;
        }
        i = 0;
        while(app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN) &&  i < 2000)
        {
            sleep_us(100);
            i++;
        }
        i =0;
        while(blc_ll_getTxFifoNumber() && i<400)
        {
            sleep_us(1000);
            i++;
        }
        printf("app_release_hid_key\r\n");
    }

}
void comb_key_stop_ir(void)
{
    blt_soft_timer_delete(app_ir_bugreport_key_delay_timer);
    if(device_in_connection_state == 0)
    {
        if(ir_not_released)ir_not_released = 0;
        if( ir_send_ctrl.is_sending)
        {
             ir_send_release();
        }
    }
    else
    {
        //factory reset and key mute is ir programmed
        if(ir_send_ctrl.is_sending)
        {
             printf("comb_key_stop_ir and ir release\r\n");
             programming_key_set(0);
             ir_send_release();
        }
    }
}

#define FACTORY_RESET_TIMEOUT 100000

_attribute_data_retention_     u8 factory_reset_flag = 0;
_attribute_data_retention_     u8 factory_reset_temi_conn_flag = 0;
_attribute_data_retention_     u8 factory_reset_timer_count = 0;

int app_factory_reset_timer(void)
{
    u8 bond_number;
    u16 i=0;
    printf("app_factory_reset_timer\r\n");
    if(device_in_connection_state)
    {
        printf("app_factory_reset_timer_google_voice_ctl=%x\r\n",google_voice_ctl);
        if(((google_voice_ctl & FLAG_AUDIO_CLOSE) == 0) || (ota_is_working))
        {
            if(ota_is_working)
            {
                app_ota_terminate();
                device_led_off(APP_LED_RED);
            }
            if(factory_reset_temi_conn_flag == 0)
            {
                i = 0;
                while(app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN) &&  i < 2000){
                sleep_us(100);
                i++;
                }
            }
            else
            {
                if(blc_ll_getTxFifoNumber())
                {
                    factory_reset_temi_conn_flag++;
                    if(factory_reset_temi_conn_flag < 20)
                    {
                        return 0;
                    }
                }
            }
        }
        else
        {
            factory_reset_timer_count++;
            if(factory_reset_timer_count < 20)
            {
                return 0;
            }
        }
    }

    ir_flash_factory();
    flash_erase_sector(MAC_DATA_SECT_ADDR);
    bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
    printf("bond_number=%x\r\n",bond_number);
    if(bond_number)
    {
        printf("delete bond info\r\n");
        bls_smp_param_deleteByIndex(0);
    }
    factory_reset_flag = 0;
    factory_reset_temi_conn_flag = 0;
    factory_reset_timer_count = 0;
    google_voice_ctl = 0;
    ota_is_working = 0;
    return -1;
}

void app_mic_led(u8 en)
{
    if(audio_start_reason == REASON_PTT)
    {
        if(en)
        {
            printf("app_mic_led\r\n");
            device_led_setup(led_cfg[LED_SHINE_PTT]);
        }
        else
        {
            if(factory_reset_flag == 0)
            {
                printf("app_mic_led close led");
                device_led_setup(led_cfg[en ? 1 : 2]);
            }
        }
    }
    else
    {
        if(en)
        {
            device_led_setup(led_cfg[en ? 1 : 2]);
            printf("app_mic_led\r\n");
        }
        else
        {
            if(factory_reset_flag == 0)
            {
                printf("app_mic_led close led");
                device_led_setup(led_cfg[en ? 1 : 2]);
            }
        }
    }
}

void app_keypress_led(u8         ir_key)
{
    printf("app_keypress_led_lowbat=%x\r\n",lowbat);
    if(lowbat == 0x55)
    {
        printf("low bat\r\n");
        device_led_setup(led_cfg[LED_SHINE_LOWBAT]);
        return;
    }
    else
    {
          if((is_pairing_mode == 0) && (is_reconn_mode == 0) && (ui_mic_enable == 0))
          {
              printf("is_pairing_mode=%x,is_reconn_mode=%x\r\n",is_pairing_mode,is_reconn_mode);
              if(device_in_connection_state == 1)
              {
                  device_led_setup(led_cfg[LED_SHINE_FAST]);
              }
          }
          if((ota_is_working == 0) && (device_in_connection_state == 0))
          {
              device_led_setup(led_cfg[LED_KEYPRESS]);
          }
    }
    if(ir_key)
    {
        if((is_pairing_mode == 0) && (is_reconn_mode == 0) && (ota_is_working == 0) && (ui_mic_enable == 0))
        {
            device_led_setup(led_cfg[LED_SHINE_FAST]);
            device_led_setup(led_cfg[LED_KEYPRESS]);
        }
    }
}

void app_connect_led(void)
{  
    device_led_setup(led_cfg[LED_SHINE_SUCCESS]);
}

void app_pairing_led(void)
{
    device_led_setup(led_cfg[LED_SHINE_PAIRING]);
}

void app_twokey_to_onekey(void)
{
    if(TwoKeyPressMode != APP_TWOKEYPRESS_NONE)
    {
        printf("app_twokey_to_onekey\r\n");
        TwoKeyPressMode = APP_TWOKEYPRESS_ONEKEY; 
    }
    if((is_pairing_mode == 0) && (is_reconn_mode == 0))
    {
        printf("app_twokey_to_onekey_close led\r\n");
        device_led_off(APP_LED_GREEN);
    }
}

extern void advertise_tick_retime(void);
void key_change_proc(void)
{
    latest_user_event_tick = clock_time();  //record latest key change time

    u8 key0 = kb_event.keycode[0];
    u8 key1 = kb_event.keycode[1];
    u8 key_value;
    u8 key_buf[8] = {0,0,0,0,0,0,0,0};
    u16 consumer_key[2]={0};

    key_not_released = 1;
    if (kb_event.cnt == 2)   //two key press, do  not process
    {
        //retime
        printf("kb_event.cnt == 2_step1\r\n");
        advertise_tick_retime();
        if(wakeup_key_send_flag == 1)
        {
            wakeup_key_send_flag = 2;
        }
        if(is_pairing_mode)    return;
        TwoKeyPressFlag = 0x55;
#if (STUCK_KEY_PROCESS_ENABLE)
        stuckkey_keypresstimer = clock_time() | 1;
#endif
        printf("kb_event.cnt == 2_step2\r\n");
        if ( (key0 == 0x0a && key1 == 0x07) || (key0 == 0x07 && key1 == 0x0a)) //Pairing Mode
        {
            printf("pair start\r\n");
            comb_key_stop_ir();
            blt_soft_timer_delete(app_special_key_delay_timer);
            //in voice mode need disable enter pairing mode
            printf("twokey ui_mic_enable=%x\r\n",ui_mic_enable);
            if(ui_mic_enable)
            {
               return;
            }
            if(lowbat == 0)
            {
                device_led_setup(led_cfg[LED_AUDIO_ON]);
            }
            keyScanLongPressTick = clock_time() | 1; //none zero
            LongPressTick = 4000000;
            TwoKeyPressMode = APP_TWOKEYPRESS_PAIRING;
        }
        else if ( (key0 == 0x15 && key1 == 0x16) || (key0 == 0x16 && key1 == 0x15)) //Factory Reset Mode
        {
            printf("Factory Reset Mode start\r\n");
            comb_key_stop_ir();
            device_led_off(APP_LED_GREEN);
            if(lowbat == 0)
            {
                device_led_setup(led_cfg[LED_AUDIO_ON]);
            }
            keyScanLongPressTick = clock_time() | 1; //none zero
            LongPressTick = 4000000;
            TwoKeyPressMode = APP_TWOKEYPRESS_FACTORYRESET;
        }
        else if ( (key0 == 0x07 && key1 == 0x15) || (key0 == 0x15 && key1 == 0x07)) //Bug report
        {
            printf("bug report start\r\n");
            comb_key_stop_ir();
            device_led_setup(led_cfg[LED_AUDIO_ON]);
            keyScanLongPressTick = clock_time() | 1; //none zero
            LongPressTick = 1000000;
            TwoKeyPressMode = APP_TWOKEYPRESS_BUGREPORT;
        }
        else
        {
            if(device_in_connection_state == 0)
            {
                if(app_is_navi_key(key0,key1) == 0)
                {
                    printf(" stop double key in ir mode\r\n");
                    blt_soft_timer_delete(app_ir_bugreport_key_delay_timer);
                    printf("ir_send_ctrl.is_sending =%x\r\n",ir_send_ctrl.is_sending);
                    if(ir_send_ctrl.is_sending)
                    {
                        key_type = IR_KEY;
                        ir_send_release();
                    }
                }
                else
                {
                    printf("double key is navi in ir mode\r\n");
                    if(ir_send_ctrl.is_sending == IR_SENDING_NONE)
                    {
                        if(key0 == 12)
                            key_value = key_setting_or_noti[0];
                        else if(key0 == 13)
                            key_value = key_guide_or_livetv[0];
                        else
                            key_value = p_kb_map_ir[key0];
                        key_type = IR_KEY;
                        ir_dispatch(TYPE_IR_SEND, 0x88, key_value);
                        if(ir_not_released == 0) ir_not_released = 1;
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
                    comb_key[0] = vk_consumer_map[p_kb_map_ble[key0]-CR_MEDIA_KEY_INDEX];
                    comb_key[1] = vk_consumer_map[p_kb_map_ble[key1]-CR_MEDIA_KEY_INDEX];
                    bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&comb_key, 4);
                }
            }
        }
    }
    else if(kb_event.cnt == 1)
    {
          printf("ir_not_released=%x\r\n",ir_not_released);
          //retime
          advertise_tick_retime();

          printf("key_id=%x\r\n",key0);

#if (STUCK_KEY_PROCESS_ENABLE)
          stuckkey_keypresstimer = clock_time() | 1;
#endif

          if(device_in_connection_state == 0)
          {
              u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();
              if(bond_number == 1)
              {
                  if((power_down_flag == 2) || (wakeup_src == 1))
                  {
                      if(wakeup_key_send_flag == 1)
                      {
                          if(app_custom_is_wakeup_key(key0) == 1)
                          {
                              app_tx_power_wakeup(key0);
                              wakeup_key_send_flag = 2;
                          }
                      }
                  }
              }
          }
          if(wakeup_key_send_flag == 1)
          {
              wakeup_key_send_flag = 2;
          }

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
          active_mic_open();
          key_type = IR_KEY;
          
          if(ir_key_is_suppress(key0) == 0)
          {
          
              printf("is suppress\r\n");
              app_keypress_led(1);
              if(ir_fallback_send_key_code(key0,1))
              {
                  if(!ir_not_released)
                      ir_not_released = 1;
                  return;
              }
          }
          app_keypress_led(0);
          if(app_keycache(key0) == 1)
          {
              return;
          }
          programming_key_set(0);
          if(device_in_connection_state)
          {
                key_value = p_kb_map_ble[key0];
                printf("key_value=%x\r\n",key_value);

                if(key_value >= CR_MEDIA_KEY_INDEX )
                {
                    key_type = CONSUMER_KEY;
                    if(key_value == CR_DASHBOARD)
                    {
                        consumer_key[0] = key_setting_or_noti[1];
                    }
                    else if(key_value == CR_GUIDE)
                    {
                        consumer_key[0] = key_guide_or_livetv[1];
                    }
                    else
                        consumer_key[0] = vk_consumer_map[key_value-CR_MEDIA_KEY_INDEX];
                    consumer_key[1] = 0;
                    printf("consume key=%x \r\n",consumer_key[0]);
                    if((key0 == 0x07) || (key0 == 0x0a))
                    {
                         SpecialKey = consumer_key[0];
                         blt_soft_timer_add(&app_special_key_delay_timer, 80000);
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
             else if(key0 == 12)
                 key_value = key_setting_or_noti[0];
             else if(key0 == 13)
                 key_value = key_guide_or_livetv[0];
             else
                 key_value = p_kb_map_ir[key0];
             if((key_value == IR_BACK) &&(is_pairing_mode == 0x55))
             {
                 printf("back key, stop adv\r\n");
                 device_led_off(APP_LED_GREEN);
                 bls_ll_setAdvEnable(0);
                 is_pairing_mode = 0;
                 is_reconn_mode = 0;
                 app_enter_deep_timeset();
                 u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
                  printf("bond_number_1=%x\r\n",bond_number);
             }
             if((key0 == 0x07) || (key0 == 0x15))
             {
                 SpecialKey = key_value;
                 blt_soft_timer_add(&app_ir_bugreport_key_delay_timer, 80000);
                 return;
             }
             printf("\r\nir_key_value=%x\r\n",key_value);
             key_type = IR_KEY;
             if(!ir_not_released){
                  printf("ir_dispatch send\r\n");
                 ir_dispatch(TYPE_IR_SEND, 0x88, key_value);
                 ir_not_released = 1;
             }
          }

    }
    else   //kb_event.cnt == 0,  key release
    {
        printf("key release\r\n");
        advertise_tick_retime();
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
            printf("ir_not_released=%x\r\n",ir_not_released);
            if(ir_not_released){
                ir_not_released = 0;
                if(ir_send_ctrl.is_sending)
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

void app_factory_reset(void)
{
    u8 audio_open=0;

    printf("Factory Reset Mode ongoning\r\n");
    if(lowbat == 0x55)
    {
        return;
    }

    //global vari
    factory_reset_flag = 0x55;
    is_reconn_mode = 0;
    is_pairing_mode = 0;

    //led
    device_led_setup(led_cfg[LED_SHINE_SUCCESS]);

    //ir
    ir_table_init();
    analog_write(DEEP_ANA_REG2, 0);
    bls_ll_setAdvEnable(0);
    if(device_in_connection_state)
    {
        printf("TwoKeyPressMode=%x\r\n",TwoKeyPressMode);
        if(ui_mic_enable)
        {
            audio_open = 1;
            extern u8 app_audio_key_stop(u8 reason);
            app_audio_key_stop(REASON_OTHERS);
        }
    }
    extern void app_enter_deep_timeset(void);
    app_enter_deep_timeset();
    if(audio_open)
        blt_soft_timer_add(&app_factory_reset_timer, FACTORY_RESET_TIMEOUT);
    else
        app_factory_reset_timer();

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
#if (MP_TEST_MODE)
        if (test_get_mode() == MODE_TEST) test_proc_key();
        else
#endif
        key_change_proc();
    }

#if(DEEPBACK_FAST_KEYSCAN_ENABLE)
    if(deepback_key_state != DEEPBACK_KEY_IDLE){
        deepback_post_proc();
    }
#endif

    if((keyScanLongPressTick == 0) && (TwoKeyPressMode != APP_TWOKEYPRESS_NONE)) //&& (TwoKeyPressMode != APP_TWOKEYPRESS_ONEKEY))
    {
        printf("stop led\r\n");
        device_led_off(APP_LED_GREEN);
        TwoKeyPressMode = APP_TWOKEYPRESS_NONE;
    }

    if(keyScanLongPressTick && clock_time_exceed(keyScanLongPressTick, LongPressTick))
    {
        keyScanLongPressTick = 0;
        advertise_tick_retime();
        printf("twokey timeout and TwoKeyPressMode=%x\r\n",TwoKeyPressMode);
        if(TwoKeyPressMode == APP_TWOKEYPRESS_ONEKEY) return;
        if(TwoKeyPressMode == APP_TWOKEYPRESS_PAIRING)
        {
            printf("trigger pair\r\n");
            if(lowbat == 0x55)
            {
                return;
            }
            device_led_setup(led_cfg[LED_SHINE_PAIRING]);
            is_pairing_mode = 0x55;
            if(is_reconn_mode)
                is_reconn_mode = 0;
            notify_get_rsp_en = 1;
            if( device_in_connection_state == 0)
            {
                ir_table_init();
                ir_flash_set_flag(0);
                app_set_adv_interval_downgrade_indirect();
                extern void app_adv_indirect(void);
                app_adv_indirect();
            }
            else
            {
                if(ota_is_working)
                {
                    app_ota_terminate();
                    device_led_off(APP_LED_RED);
                }
                printf("re-pairing \r\n");
                u16 i=0;
                while(app_terminate(HCI_ERR_REMOTE_USER_TERM_CONN) &&  i < 2000){
                sleep_us(100);
                i++;
                }
                repairing_flag = 0x55;
            }
        }
        else if(TwoKeyPressMode == APP_TWOKEYPRESS_FACTORYRESET)
        {
            app_factory_reset();
        }
        else if(TwoKeyPressMode == APP_TWOKEYPRESS_BUGREPORT)
        {
            printf("APP_TWOKEYPRESS_BUGREPORT ongoning\r\n");
            device_led_setup(led_cfg[LED_SHINE_SUCCESS]);
            if(device_in_connection_state == 0)
            {
                key_type = IR_KEY;
                if(!ir_not_released){
                    ir_dispatch(TYPE_IR_SEND, 0x88, 0x96);
                    ir_not_released = 1;
                }
            }
            else
            {
                u16 consumer_key[2];
                consumer_key[0] = G00GLE_MKEY_BACK;
                consumer_key[1] = G00GLE_MKEY_CENTER;
                bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);
            }
        }
        TwoKeyPressMode = APP_TWOKEYPRESS_NONE;
    }

    app_key_send_store();
}
#endif

extern u32    scan_pin_need;

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
    kb_wake_config();

    bls_app_registerEventCallback (BLT_EV_FLAG_GPIO_EARLY_WAKEUP, &proc_keyboard);

    #if (BLE_AUDIO_ENABLE)
        #if (BLE_DMIC_ENABLE)
            dmic_gpio_reset();
        #else
            amic_gpio_reset();
        #endif
    #endif


#if (BLT_APP_LED_ENABLE)
    device_led_init();  //LED initialization
    device_led_off(APP_LED_RED);
    if((analog_read(USED_DEEP_ANA_REG) & POWER_ON_FLG) == 0)
    {
        printf("led on 1s\r\n");
        device_led_setup(led_cfg[LED_POWER_ON]);  //only poweron
    }
#endif
    analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) | POWER_ON_FLG);

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
    kb_wake_config();
    
    #if (BLE_AUDIO_ENABLE)
        #if (BLE_DMIC_ENABLE)
            dmic_gpio_reset();
        #else
            amic_gpio_reset();
        #endif
    #endif

#if (BLT_APP_LED_ENABLE)
    device_led_init();  //LED initialization
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

