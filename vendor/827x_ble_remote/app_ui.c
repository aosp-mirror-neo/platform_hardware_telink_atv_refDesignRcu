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
#include "../common/blt_soft_timer.h"

#include "rc_ir.h"
#include "battery_check.h"
#include "app_audio.h"
#include "app_flash_write.h"
#include "app_ir.h"
#include "app_ui.h"

#if (MP_TEST_MODE)
#include "app_test.h"
#endif

#include "app_custom.h"
#include "app_custom_lib.h"
#include "./app_find_me/app_fms.h"
#include "./app_fmr/app_fmr.h"



//////////////////// key type ///////////////////////
#define IDLE_KEY                   0
#define CONSUMER_KEY               1
#define KEYBOARD_KEY               2
#define IR_KEY                     3
#define VOICE_KEY                  4

u8 ir_hw_initialed = 0;  //note: can not be retention variable
_attribute_data_retention_ u8  key_type;
_attribute_data_retention_ int key_not_released;
_attribute_data_retention_ u8  ota_is_working = 0;
_attribute_data_retention_ u8  app_mtu_size = 23;

_attribute_data_retention_ int ir_not_released=0;
_attribute_data_retention_ u8  user_key_mode;
_attribute_data_retention_ u8  repairing_flag = 0;
_attribute_data_retention_ u8  is_pairing_mode=0;
_attribute_data_retention_ u8  is_reconn_mode=0;
_attribute_data_retention_ u8  ota_flag = 0;  //0:pause 1:continue
_attribute_data_retention_ u8  testmode_key_seq[5]={0};
_attribute_data_retention_ u8  testmode_key_seq_count=0;

/* 221116 by ken.  This variable is used to wait for google voice mic open cmd */
_attribute_data_retention_ u32 g_delay_en_pm = 0;


/* 230525 by ken.  Add a second pairing key combo key */
_attribute_data_retention_ u8 g_2ndPairKey[2] = {0xff, 0xff};


extern u8 wakeup_key_keycount;
extern u8  comb_key_keyid[9];
extern u32 latest_user_event_tick;
extern u8  lowbat;
extern u8  device_in_connection_state;
extern u8  flag_schedule_ota;
extern u8  device_timeout_state;
extern u8  wakeup_src;
extern u8  wakeup_key_keyid;
//extern u8  wakeup_key_send_flag;
//extern u8  wakeup_packet_format;
extern u32 advertise_begin_tick;
extern u8  en_powerkey_cache;
extern u8  detpowerkey;
extern unsigned char Kb_Map_Testmode_Seq_Correspend_Index_G10[];
extern unsigned char Kb_Map_Testmode_Seq_Correspend_Index_G20[];

extern int ir_fallback_send(u8 key_down);
extern void device_led_off(u8 led);
extern void app_enter_deep_timeset(void);
extern void app_set_adv_interval_downgrade_indirect(void);
extern u8  app_custom_is_wakeup_key(u8 keyid);
extern ble_sts_t app_terminate(u8 reason);
extern int bls_smp_param_deleteByIndex(u8 index);
extern void app_stop_wakeup_loop(void);

static const u16 vk_consumer_map[] = {
    /* 230529 by ken. add magic key code */
    GOOGLE_MKEY_MAGIC,


    GOOGLE_MKEY_NOTIFICATION,
    GOOGLE_MKEY_LIVETV,
    GOOGLE_MKEY_TELETEXT,
    GOOGLE_MKEY_USERPROFILE,
    GOOGLE_MKEY_ALLAPPS,
    GOOGLE_MKEY_FASTREWIND,
    GOOGLE_MKEY_RECORD,
    GOOGLE_MKEY_PLAYPAUSE,
    GOOGLE_MKEY_FASTFORWARD,
    GOOGLE_MKEY_INFO,
    GOOGLE_MKEY_SUBTITLE,
    GOOGLE_MKEY_RED,
    GOOGLE_MKEY_GREEN,
    GOOGLE_MKEY_YELLOW,
    GOOGLE_MKEY_BLUE,
    GOOGLE_MKEY_YOUTUBE,
    GOOGLE_MKEY_NETFLIX,
    GOOGLE_MKEY_DISNEY,
    GOOGLE_MKEY_HBOMAX,
    GOOGLE_MKEY_UP,
    GOOGLE_MKEY_DN,
    GOOGLE_MKEY_LEFT,
    GOOGLE_MKEY_RIGHT,
    GOOGLE_MKEY_CENTER,
    GOOGLE_MKEY_HOME,
    GOOGLE_MKEY_BACK,
    GOOGLE_MKEY_POWER,
    GOOGLE_MKEY_VOL_MUTE,
    GOOGLE_MKEY_CHN_UP,
    GOOGLE_MKEY_CHN_DN,
    GOOGLE_MKEY_GUIDE,
    GOOGLE_MKEY_BOOKMARK,
    GOOGLE_MKEY_ASSIST,
    GOOGLE_MKEY_INPUT,
    GOOGLE_MKEY_DASHBOARD,
};


/////////////////////////// led management /////////////////////
#if (BLT_APP_LED_ENABLE)



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
        {APP_LED_RED,     1,   1000,     1000 ,  15,     0,  },    //5Hz for 3 seconds
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
        ir_send_ctrl.is_full_sequence = 0;
        ir_nec_send(syscode,~(syscode),ircode);

    }
    else if(type == TYPE_IR_RELEASE){
        ir_send_release();
    }
}



void ir_dispatch_ext(u8 type, u8 keyindex){

    u8 data[4];
    if(!ir_hw_initialed){
        ir_hw_initialed = 1;
        rc_ir_init();
    }
    if(type == TYPE_IR_SEND){
        ir_send_ctrl.is_full_sequence = 0;

        if(keyindex == VOICE)
        {
         if(app_custom_get_device_type() == REMOTE_G10)
             keyindex = 9;
         else
             keyindex = 18;
        }

        if(nec_ir_addr1_table[keyindex] == 0xff)
        {
            data[0] = 0x88;
        }
        else
            data[0] = nec_ir_addr1_table[keyindex];

        if(nec_ir_addr2_table[keyindex] == 0xff)
        {
            data[1] = ~data[0];
        }
        else
            data[1] = nec_ir_addr2_table[keyindex];

        data[2] = nec_ir_cmd1_table[keyindex];
        if(nec_ir_cmd2_table[keyindex] == 0xff)
        {
            data[3] = ~nec_ir_cmd1_table[keyindex];
        }
        else
            data[3] = nec_ir_cmd2_table[keyindex];

        //printf("\r\nir_dispatch_ext ir_cm1_value=%x,ir_cm2_value=%x,\r\n",data[2],data[3]);
        ir_nec_send_ext(data);

    }
    else if(type == TYPE_IR_RELEASE){
        ir_send_release();
    }
}


#endif

#if (BLE_REMOTE_OTA_ENABLE)
#include "app_ota.h"
extern u8  mac_public[6];
u8 app_enter_ota_mode(u8 *data)
{
    printf("start data:");
    array_printf(data,4);
    extern u32 ota_program_offset;
    extern int ota_program_bootAddr;
    printf("ota_program_offset=%x\r\n",ota_program_offset);
    printf("ota_program_offset=%x\r\n",ota_program_bootAddr);

    u8 check_data[4];
    bls_ota_get_random(check_data);

    if(memcmp(check_data,data,4 )){

        u8 report_data[3] ={CLIENT_REPORT_CMD, TYPE_OTA_RESULT, GOOGLE_OTA_START_KEY_ERR};

        bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, report_data, 3);
        printf("OTA_START_KEY_ERR\n");

        return 1;
    }
    bls_pm_setSuspendMask (SUSPEND_DISABLE);
    ota_flag = 1;

    //add user code
    printf("ota start red led on\r\n");
    device_led_setup(led_cfg[LED_OTA_START]);
    ota_is_working = 1;
    bls_ota_setTimeout(90 * 1000 * 1000); //set OTA timeout  90 seconds
    return 0;
}

#if (CUSTOM_OTA_SET)
/***************************************************************************
 ***     Use the following functions to modify various types of keys     ***
 *** These 3 keys can be applied to google or configured by yourselves   ***
 **************************************************************************/


/**
 * change authentication plaintext and plaintext key.
 * This key needs to be consistent with the OTA key(app_AuthKey.txt) selected in the demo ota app.
 * Otherwise it will trigger an authentication error
 **/
void app_ota_get_plaintext_key(u8 *plaintext,u8 *cus_auth_key){

    u8 auth_plaintext[16] = {'D','e','v','i','c','e','M','o','d','e','l','0',0x00,0x00,0x00,0x00,};
    u8 auth_key[16] = {'C','u','s','t','o','m','e','r','A','u','t','h','K','e','y','0',};  // APP key;

    memcpy(plaintext,auth_plaintext,16);
    memcpy(cus_auth_key,auth_key,16);
}


/**
 * Set FW encryption key
 * This key needs to be consistent with the FW_ENC_TEST_KEY in the 8271_ble_remote folder.
 * Otherwise, the OTA will fail due to decryption errors
 **/
void app_ota_get_fw_decryption_key(u8 *dec_key){

    u8 key[16] = {'F','w','_','E','n','c','_','S','a','m','p','l','e','K','e','y',};
    memcpy(dec_key,key,16);
}

/**
 * change ECDSA public key
 * The public key and PrivateSample.pem here are a pair.
 * Users can use OpenSSL to generate public and private keys by themselves
 **/
void app_ota_get_ecdsa_pub_key(u8 *pub_key){
    u8 key[64] = {	0x9f, 0xbb, 0x58, 0xd1, 0x8b, 0x31, 0xc2, 0x41, 0xa0, 0x99, 0x78, 0x10, 0xdb, 0x1a, 0x54, 0x67, 
					0x11, 0x11, 0xce, 0x42, 0x7c, 0xef, 0x1d, 0x4f, 0x5a, 0x86, 0x8c, 0x88, 0xa9, 0x76, 0x9c, 0x1b, 
					0x78, 0x4a, 0xe1, 0x25, 0x0e, 0xc6, 0xed, 0x2d, 0x57, 0xec, 0x9f, 0xf8, 0x4f, 0x08, 0x93, 0x8a, 
					0x7f, 0xd2, 0xd4, 0x08, 0x6f, 0x64, 0xd0, 0x48, 0xd7, 0xe5, 0x54, 0xfa, 0xf9, 0xbe, 0xce, 0x05, };
    memcpy(pub_key,key,64);
}
#endif

void app_ota_result(u8 result)
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
    app_ota_terminate(GOOGLE_OTA_TIMEOUT);
    return -1;
}

extern u8  google_ota_start_flag;
void app_ota_timeout(void)
{
    app_pairing_error();
    blt_soft_timer_add(app_ota_timeoutcb, 1000000);
}

void app_ota_status(u8 status)
{
    if(device_in_connection_state == 0) return;
    if(ota_is_working == 0) return;
    ble_sts_t stat;
    if((status == 0) && (ota_flag == 1))
    {
        stat = app_ota_pause();
        ota_flag = 0;
        printf("app_ota_pause stat=%x\r\n",stat);
    }

    if((status == 1) && (ota_flag == 0))
    {
        ble_sts_t stat;
        stat = app_ota_continue();
        ota_flag = 1;
        printf("app_ota_continue stat=%x\r\n",stat);
    }
}

void app_ota_slave_terminate(void)
{
    printf("app_ota_slave_terminate\n");
    u8 report_data[3] = {CLIENT_REPORT_CMD, TYPE_OTA_RESULT, GOOGLE_OTA_USER_TERMINATE};

    u16 i = 0;
    while(bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, report_data, 3) &&  i < 2000){
       sleep_us(100);
       i++;
    }
}


void app_schedule_ota_enter_deep(void)
{
    u16 my_sch_ota_time[7]= {0,90,120,180,300,480,780};
    u8 my_index = analog_read(DEEP_ANA_REG2);

    u16 my_entry_deep_time = 0;

    /* restart schedule ota*/
    if((0x55 == flag_schedule_ota) && (0x55 == device_timeout_state)){
        printf("[NOTICE] Restart schedule OTA !!! \n");
        my_entry_deep_time = count_schedule_ota;
        my_index = 1;
    }else if(my_index){
        printf("[NOTICE] Current schedule OTA index: %d !!! \n", my_index);
        my_entry_deep_time = (my_index<7)?my_sch_ota_time[my_index]:0;
        if(APP_WAKEUP_SRC_DEEP_TIMER == app_ui_get_wakeup_src()){
            my_index++;
            /* Cannot be set to >=7 here. After waking up, it is necessary to judge
             * whether it is due to schedule OTA according to whether my_index is 0 or not. */
            if(my_index > 7){
                my_index=0;
                my_entry_deep_time = 0;
                analog_write(DEEP_ANA_REG2, my_index);
                printf("[NOTICE] Clear schedule ota Task !!! \n");
            }
        }
    }

    if(my_entry_deep_time){
        printf("Entry schedule OTA index: %d time: %d min \n", my_index-1,my_entry_deep_time );

        analog_write(DEEP_ANA_REG2, my_index);
        cpu_long_sleep_wakeup_32k_rc(DEEPSLEEP_MODE,PM_WAKEUP_TIMER|PM_WAKEUP_PAD,my_entry_deep_time*CLOCK_32K_CLOCK_1MIN);
//        cpu_long_sleep_wakeup_32k_rc(DEEPSLEEP_MODE,PM_WAKEUP_TIMER|PM_WAKEUP_PAD,my_entry_deep_time*CLOCK_32K_CLOCK_1S/2);  // for testing
    }else{

//        if(s_p_app_fms_flag_adv[0]) return;

        /* If the schedule OTA is not working, the value of my_entry_deep_time should be 0 */
        printf("[NOTICE] Check periodic wakeup for FMS !!! \n");
        app_fmr_periodic_wake_up();

        printf("[NOTICE] Idle to sleep \n");
        cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);
    }
}

void app_schedule_ota_send_notify_to_continue_ota(void)
{
    u8 dat[2] = {0};
    u8 report_data[4] = {CLIENT_REPORT_CMD,0x04,0x73,0x6f};
    u8 data = analog_read(DEEP_ANA_REG2);

    dat[0] = 0x73;
    dat[1] = 0x6f;
    if((APP_WAKEUP_SRC_DEEP_TIMER == app_ui_get_wakeup_src()) && (data))//wake up for timer
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
    u8 flag=0;

    if((APP_WAKEUP_SRC_DEEP_TIMER == app_ui_get_wakeup_src()) && (data) )
    {
        flag = app_custom_is_enable_wakeup_key();
        if(flag)
        {
            printf("app_schedule_ota_wakeup_host\r\n");
//            wakeup_key_send_flag = 2;
            app_woble_schedule_ota(wakeup_key_keycount);
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

#include "application/audio/gl_audio.h"

extern u8 app_audio_key_flags;
extern u8 htt_audio_model_key_press_flags;

typedef enum{
    APP_ACCESSIBILITY_SHORTCUT_NONE=0,
    APP_ACCESSIBILITY_SHORTCUT_PAIRING,
    APP_ACCESSIBILITY_SHORTCUT_FACTORYRESET,
    APP_ACCESSIBILITY_SHORTCUT_BUGREPORT,
    APP_ACCESSIBILITY_SHORTCUT_DOWNBACK,
    APP_ACCESSIBILITY_SHORTCUT_SINGLEKEY
}APP_Accessibility_Shortcut_Mode;


_attribute_data_retention_     u32 keyScanLongPressTick = 0;
_attribute_data_retention_     u32 LongPressTick = 0;
_attribute_data_retention_     APP_Accessibility_Shortcut_Mode  Accessibility_Shortcut_Mode = APP_ACCESSIBILITY_SHORTCUT_NONE;
_attribute_data_retention_     u8 Accessibility_Shortcut_Flag = 0;


#if (STUCK_KEY_PROCESS_ENABLE)
     extern u32 stuckkey_keypresstimer;
#endif

extern void app_adv_indirect(void);
u8 app_repairing(void)
{
    if(repairing_flag == 0x55)
    {
         //printf("app_repairing\r\n");
         app_adv_indirect();
         repairing_flag = 0;
#if BLE_AUDIO_ENABLE
         notify_get_rsp_en = 0;
#endif
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


    printf("%s en_cache:[%d] key_id:[%d] \n", __FUNCTION__, en_powerkey_cache, keyindex);

    if(en_powerkey_cache == 0)
    {
        printf("[NOTICE] check key ID !!! ");
        if(keyindex == 0) return 0;
    }else{
        printf("[NOTICE] push key to buffer !!! ");
    }


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

int app_cachekey_send_timer(void)
{
    u8 i;
    u8 key0;
    u8 key_value;
    u8 key_buf[8] = {0,0,0,0,0,0,0,0};
    u16 consumer_key[2]={0};

    if(device_in_connection_state == 0) return 0;
    if(app_key_store_count == 0) return -1;
    printf("app_key_send_store,app_key_store_count=%x\r\n",app_key_store_count);

    key0 = app_key_store_buffer[0];
    printf("send cache key key=%x\r\n",key0);
    {
#if APP_IR_OVER_BLE
        programming_key_set(0);
#endif
        key_value = p_kb_map_ble[key0];
        if(key_value >= CR_MEDIA_KEY_INDEX )
        {
            consumer_key[0] = vk_consumer_map[key_value-CR_MEDIA_KEY_INDEX];
            consumer_key[1] = 0;
            printf("consume key=%x \r\n",consumer_key[0]);     //down
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
    if(app_key_store_count == 0)
        return -1;
    else
        return 0;
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
#if APP_IR_OVER_BLE
        programming_key_set(0);
#endif
        key_value = p_kb_map_ble[key0];
        if(key_value >= CR_MEDIA_KEY_INDEX )
        {
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


u8 app_is_navi_key(u8 data0,u8 data1)
{
    u8 flag[2]={5,5};

    if(data0 == data1) return 0;
    for(u8 i=0;i<4;i++)
    {
        if(app_custom_get_device_type() == REMOTE_G10)
        {
            if(data0 == navi_key_index_g10[i])
                flag[0] = i;
            if(data1 == navi_key_index_g10[i])
                flag[1] = i;
        }
        else
        {
            if(data0 == navi_key_index_g20[i])
                flag[0] = i;
            if(data1 == navi_key_index_g20[i])
                flag[1] = i;
        }
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
    if(device_in_connection_state == 0)
    {
        printf("comb_key_stop_ir_1\r\n");
        if(ir_not_released)ir_not_released = 0;
        if( ir_send_ctrl.is_sending)
        {
             printf("comb_key_stop_ir_2\r\n");
#if APP_IR_OVER_BLE
             programming_key_set(0);
#endif
             ir_send_release();
        }
    }
    else
    {
        //factory reset and key mute is ir programmed
        if(ir_send_ctrl.is_sending)
        {
             printf("comb_key_stop_ir and ir release\r\n");
#if APP_IR_OVER_BLE
             programming_key_set(0);
#endif
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
    //printf("app_factory_reset_timer\r\n");
    if(device_in_connection_state)
    {
#if BLE_AUDIO_ENABLE
        printf("app_factory_reset_timer_google_voice_ctl=%x\r\n",google_voice_ctl);
        u16 i=0;
        if(((google_voice_ctl & FLAG_AUDIO_CLOSE) == 0) || (ota_is_working))
        {
            if(ota_is_working)
            {
                app_ota_slave_terminate();
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
#endif
        {
            factory_reset_timer_count++;
            if(factory_reset_timer_count < 20)
            {
                return 0;
            }
        }
    }

#if APP_IR_OVER_BLE
    ir_flash_factory();
#endif
    flash_erase_sector(MAC_DATA_SECT_ADDR);
//    u8 data=0;
//    flash_read_page(APP_DATA_UPDATE_OVER_OTA_ADDR, 1, &data);
//    if(data != 0xff)
//        flash_erase_sector(APP_DATA_UPDATE_OVER_OTA_ADDR);

    /* Preserve NEC IR data during Factory rest in v2.10 */
//    flash_read_page(APP_NEC_IR_CODE_TABLE, 1, &data);
//    if(data != 0xff)
//        flash_erase_sector(APP_NEC_IR_CODE_TABLE);

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
#if BLE_AUDIO_ENABLE
    google_voice_ctl = 0;
#endif
    ota_is_working = 0;

    return -1;
}

#if BLE_AUDIO_ENABLE
void app_mic_led(u8 en)
{
    if(audio_start_reason == REASON_PTT)
    {
        if(en)
        {
            device_led_setup(led_cfg[LED_SHINE_PTT]);
        }
        else
        {
            if(factory_reset_flag == 0)
            {
                device_led_setup(led_cfg[LED_AUDIO_OFF]);
            }
        }
    }
    else
    {
        if(en)
        {
            device_led_setup(led_cfg[LED_AUDIO_ON]);
        }
        else
        {
            if(factory_reset_flag == 0)
            {
                device_led_setup(led_cfg[LED_AUDIO_OFF]);
            }
        }
    }
}
#endif

void app_keypress_led(u8         ir_key)
{
    if(lowbat == 0x55)
    {
        printf("low bat\r\n");
        device_led_setup(led_cfg[LED_SHINE_LOWBAT]);
        if((is_pairing_mode == 0) && (is_reconn_mode == 0) && (ui_mic_enable == 0))
        {
            if(device_in_connection_state == 1)
            {
                device_led_setup(led_cfg[LED_SHINE_FAST]);
            }
        }
        return;
    }
    else
    {
          if((is_pairing_mode == 0) && (is_reconn_mode == 0) && (ui_mic_enable == 0))
          {
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
        if((is_pairing_mode == 0) && (ota_is_working == 0) && (ui_mic_enable == 0))
        {
            printf("ir db key press\r\n");
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

void app_accessibility_short_key_to_single_key(void)
{
    if(Accessibility_Shortcut_Mode != APP_ACCESSIBILITY_SHORTCUT_NONE)
    {
        printf("app_accessibility_short_key_to_single_key\r\n");
        Accessibility_Shortcut_Mode = APP_ACCESSIBILITY_SHORTCUT_SINGLEKEY;
    }
    if((is_pairing_mode == 0))
    {
        printf("app_accessibility_short_key_to_single_key_close led\r\n");
        device_led_off(APP_LED_GREEN);
    }
}

int app_poweron_timeout(void)
{
     u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();
     printf("app_poweron_timeout\r\n");
     if((bond_number == 0) && (en_ble_adv))
     {
         is_pairing_mode = 0x55;
#if BLE_AUDIO_ENABLE
         notify_get_rsp_en = 1;
#endif
         app_pairing_led();
         app_adv_indirect();
     }

     return -1;
}

int app_usermode_to_testmode_timeout(void)
{
     printf("app_usermode_to_testmode_timeout\r\n");
     detpowerkey = 0;

     return -1;
}

void app_testmode_seq_record(u8 keyid)
{
    if(detpowerkey)
    {
        printf("app_testmode_seq_record\r\n");
        if(app_custom_get_device_type() == REMOTE_G10)
        {
             if(testmode_key_seq[0] == Kb_Map_Testmode_Seq_Correspend_Index_G10[0])
             {
                 if(testmode_key_seq_count<=4)
                 {
                     testmode_key_seq[testmode_key_seq_count] = keyid;
                     testmode_key_seq_count++;
                 }
             }
             else
             {
                 if(keyid == Kb_Map_Testmode_Seq_Correspend_Index_G10[0])
                 {
                     testmode_key_seq[0] = Kb_Map_Testmode_Seq_Correspend_Index_G10[0];
                     testmode_key_seq_count++;
                 }
             }
        }
        else
        {
             if(testmode_key_seq[0] == Kb_Map_Testmode_Seq_Correspend_Index_G20[0])
             {
                 if(testmode_key_seq_count<=4)
                 {
                     testmode_key_seq[testmode_key_seq_count] = keyid;
                     testmode_key_seq_count++;
                 }
             }
             else
             {
                 if(keyid == Kb_Map_Testmode_Seq_Correspend_Index_G20[0])
                 {
                     testmode_key_seq[0] = Kb_Map_Testmode_Seq_Correspend_Index_G20[0];
                     testmode_key_seq_count++;
                 }
             }
        }
        if(testmode_key_seq_count == 5)
        {
            printf("testmode_key_seq[0] =%d, %d, %d, %d, %d\r\n",testmode_key_seq[0],testmode_key_seq[1],testmode_key_seq[2],testmode_key_seq[3],testmode_key_seq[4]);
            if(app_custom_get_device_type() == REMOTE_G10)
            {
                if(memcmp(testmode_key_seq,Kb_Map_Testmode_Seq_Correspend_Index_G10,5) == 0)
                   {
                       //printf("user mode to test mode g10\r\n");
                    flash_erase_sector(APP_TEST_SECT_ADDR);
                    start_reboot();
                   }
            }
            else
            {
                if(memcmp(testmode_key_seq,Kb_Map_Testmode_Seq_Correspend_Index_G20,5) == 0)
                   {
                       //printf("user mode to test mode g20\r\n");
                    flash_erase_sector(APP_TEST_SECT_ADDR);
                    start_reboot();
                   }
            }
        }
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
    if(kb_event.cnt >= 3){

        printf("[NOTICE] More than 2 buttons were pressed. key_type: [0x%x] \n",key_type);
    }
    else if (kb_event.cnt == 2)   //two key press, do  not process
    {
        //retime
        advertise_tick_retime();
        if(is_pairing_mode)    return;
        Accessibility_Shortcut_Flag = 0x55;
#if (STUCK_KEY_PROCESS_ENABLE)
        stuckkey_keypresstimer = clock_time() | 1;
#endif
        if (((key0 == comb_key_keyid[0]) && (key1 == comb_key_keyid[1])) || ((key0 == comb_key_keyid[1]) && (key1 == comb_key_keyid[0]))) //Pairing Mode  back+home
        {
            printf("pair start\r\n");
            comb_key_stop_ir();
            //in voice mode need disable enter pairing mode
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
            Accessibility_Shortcut_Mode = APP_ACCESSIBILITY_SHORTCUT_PAIRING;
        }else if( ((key0 == g_2ndPairKey[0]) && (key1 == g_2ndPairKey[1])) || ((key0 == g_2ndPairKey[1]) && (key1 == g_2ndPairKey[0])) ){
            printf("pair start by 2nd pair key\r\n");
            comb_key_stop_ir();
            //in voice mode need disable enter pairing mode
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
            Accessibility_Shortcut_Mode = APP_ACCESSIBILITY_SHORTCUT_PAIRING;
        }
        else if (((key0 == comb_key_keyid[2]) && (key1 == comb_key_keyid[3])) || ((key0 == comb_key_keyid[3]) && (key1 == comb_key_keyid[2]))) //Factory Reset Mode center+mute
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
            Accessibility_Shortcut_Mode = APP_ACCESSIBILITY_SHORTCUT_FACTORYRESET;
        }
        else if (((key0 == comb_key_keyid[0]) && (key1 == comb_key_keyid[2])) || ((key0 == comb_key_keyid[2]) && (key1 == comb_key_keyid[0]))) //Bug report  back+center
        {
            printf("[NOTICE]  back+center: bug report start  !!!!!!!!!! \n");
            comb_key_stop_ir();

            //
//            device_led_setup(led_cfg[LED_AUDIO_ON]);

            device_led_off(APP_LED_GREEN);
            if(lowbat == 0)
            {
                device_led_setup(led_cfg[LED_AUDIO_ON]);
            }

            if(device_in_connection_state){

                consumer_key[0] = 0;
                consumer_key[1] = 0;
                bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);


                key_type = CONSUMER_KEY;

                consumer_key[0] = GOOGLE_MKEY_BACK;
                consumer_key[1] = GOOGLE_MKEY_CENTER;
                bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);
            }else{
//                keyScanLongPressTick = clock_time() | 1; //none zero
//                LongPressTick = 1000000;
//                Accessibility_Shortcut_Mode = APP_ACCESSIBILITY_SHORTCUT_BUGREPORT;
            }

            keyScanLongPressTick = clock_time() | 1; //none zero
            LongPressTick = 1000000;
            Accessibility_Shortcut_Mode = APP_ACCESSIBILITY_SHORTCUT_BUGREPORT;

        }
        else if (((key0 == comb_key_keyid[0]) && (key1 == comb_key_keyid[7])) || ((key0 == comb_key_keyid[7]) && (key1 == comb_key_keyid[0]))) //  back+down
        {
            // TODO
            printf("[NOTICE]  back+dwon: talk back start  !!!!!!!!!! \n");

            comb_key_stop_ir();


            device_led_off(APP_LED_GREEN);
            if(lowbat == 0)
            {
                device_led_setup(led_cfg[LED_AUDIO_ON]);
            }

            if(device_in_connection_state){

                consumer_key[0] = 0;
                consumer_key[1] = 0;
                bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);


                key_type = CONSUMER_KEY;

                consumer_key[0] = GOOGLE_MKEY_BACK;
                consumer_key[1] = GOOGLE_MKEY_DN;
                bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);
            }else{
//                keyScanLongPressTick = clock_time() | 1; //none zero
//                LongPressTick = 2000000;
//                Accessibility_Shortcut_Mode = APP_ACCESSIBILITY_SHORTCUT_DOWNBACK;
            }

            keyScanLongPressTick = clock_time() | 1; //none zero
            LongPressTick = 1000000;
            Accessibility_Shortcut_Mode = APP_ACCESSIBILITY_SHORTCUT_DOWNBACK;

        }
        else
        {
            if(device_in_connection_state == 0)
            {
                if(app_is_navi_key(key0,key1) == 0)
                {
                    //printf(" stop double key in ir mode\r\n");
                    //printf("ir_send_ctrl.is_sending =%x\r\n",ir_send_ctrl.is_sending);
                    if(ir_send_ctrl.is_sending)
                    {
                        key_type = IR_KEY;
                        ir_send_release();
                    }
                }
                else
                {
                    //printf("double key is navi in ir mode\r\n");
                    if(ir_send_ctrl.is_sending == IR_SENDING_NONE)
                    {
                        key_type = IR_KEY;
                        ir_dispatch_ext(TYPE_IR_SEND, key0);
                        if(ir_not_released == 0) ir_not_released = 1;
                    }
                }
            }
            else
            {
                if(app_is_navi_key(key0,key1) == 0)
                {
                    //printf(" stop double key in rf mode\r\n");
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
                    //printf("double key is navi in rf mode,key0=%x,key1=%x\r\n",key0,key1);
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
        //retime
        advertise_tick_retime();

        printf("key_id=%x\r\n",key0);

        app_testmode_seq_record(key0);

    #if (STUCK_KEY_PROCESS_ENABLE)
        stuckkey_keypresstimer = clock_time() | 1;
    #endif

        if(device_in_connection_state == 0)
        {
            u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();
            if(bond_number == 1)
            {
                if(app_custom_is_wakeup_key(key0) == 1)
                {
                    wakeup_key_keycount++;
                    if(is_wakeup_keyindex_flash_info_full())
                    {
                        flash_erase_sector(WAKEUP_KEYINDEX_DATA_SECT_ADDR);
                    }

                    write_wakeup_keyindex_info(wakeup_key_keycount);

                    app_woboe_set_para(key0,wakeup_key_keycount);

                    advertise_begin_tick = 0;

                }else{
                    app_woble_clear_para();
                }
            }
        }

        if(app_fmr_is_busy()){
            /* Do nothing
             * When FMR is ON, the remote should avoid sending key events
             * (neither BLE,nor IR, nor Assistant) while in buzz mode. */
        }else{
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

            active_mic_open();
        #endif
            key_type = IR_KEY;
#if (APP_IR_OVER_BLE)
            if(ir_key_is_suppress(key0) == 0)
            {
                //printf("is suppress\r\n");
                app_keypress_led(1);
                if(ir_fallback_send_key_code(key0,1))
                {
                  if(!ir_not_released)
                      ir_not_released = 1;
                  return;
                }
            }
#endif
            app_keypress_led(0);
            if(app_keycache(key0) == 1)
            {
              return;
            }
            #if APP_IR_OVER_BLE
                programming_key_set(0);
            #endif
        }

        if(app_fmr_is_busy()){
            /* Do nothing
             * When FMR is ON, the remote should avoid sending key events
             * (neither BLE,nor IR, nor Assistant) while in buzz mode. */
        }else if(device_in_connection_state){
            key_value = p_kb_map_ble[key0];
            printf("key_value=%x\r\n",key_value);

            if(key_value >= CR_MEDIA_KEY_INDEX )
            {
                key_type = CONSUMER_KEY;
                consumer_key[0] = vk_consumer_map[key_value-CR_MEDIA_KEY_INDEX];
                consumer_key[1] = 0;
                printf("consume key=%x \r\n",consumer_key[0]);
                bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);
            #if BLE_AUDIO_ENABLE
                if(consumer_key[0] == GOOGLE_MKEY_CENTER)
                {
                    google_voice_dpad_select();

                    /* 221116 by ken. Record delay PM Start Time. The delay condition is to press the ASSIST and CENTER buttons */
                    g_delay_en_pm = clock_time()|1;

                    printf("[NOTICE] Record delay PM tick ");
                }
            #endif
            }else{
                //printf("KEYBOARD_KEY_key_value=%x \r\n",key_value);

                key_type = KEYBOARD_KEY;
                key_buf[2] = key_value;
                bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);
            }

        }else{
            if((key0 == comb_key_keyid[0]) && (is_pairing_mode == 0x55))
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

            key_type = IR_KEY;
            if(!ir_not_released){
                printf("ir_dispatch send\r\n");
                ir_dispatch_ext(TYPE_IR_SEND, key0);
                ir_not_released = 1;
            }
        }

    #if FIND_ME_ENABLE
        // TODO stop
        app_fmr_on_button_event();
    #endif
    }
    else   //kb_event.cnt == 0,  key release
    {
        printf("key release\r\n");
        advertise_tick_retime();

        key_not_released = 0;
        keyScanLongPressTick = 0;
        LongPressTick = 0;

        app_key_release_action();

        Accessibility_Shortcut_Flag = 0;

    }
}

/* This function is implemented to achieve couch mode (1) */
void app_key_release_action(void){

#if    BLE_AUDIO_ENABLE
        if((audio_start_reason == REASON_HTT) && (google_voice_ctl & FLAG_NOTIFY_AUDIO_DATA)){
            printf("key release_audio_start_reason:0x%x\n",audio_start_reason);
            google_voice_ctl |= FLAG_AUDIO_CLOSE;
            audio_stop_reason = REASON_RELEASE_HTT;
            audio_start_reason = 0;
            ui_enable_mic(0);
            g_delay_send_audio_stop = clock_time();
        }
#endif  /* BLE_AUDIO_ENABLE */

#if (STUCK_KEY_PROCESS_ENABLE)
        stuckkey_keypresstimer = 0;
#endif

#if APP_IR_OVER_BLE
        if(is_programming_key_send() == 1)
        {
           printf("key up\r\n");
           ir_fallback_send(0);
        }
        programming_key_set(0);
#endif

    printf(" app_key_release_action. key_type=%x\r\n",key_type);
    if(key_type == CONSUMER_KEY)
    {
        u16 consumer_key[2]={0};
        bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);
    }
    else if(key_type == KEYBOARD_KEY)
    {
        u8 key_buf[8] = {0,0,0,0,0,0,0,0};
        bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
    }

#if (REMOTE_IR_ENABLE)
    else if(key_type == IR_KEY)
    {
        if(ir_not_released){
            ir_not_released = 0;
            if(ir_send_ctrl.is_sending)
            {
                ir_dispatch(TYPE_IR_RELEASE, 0, 0);  //release
            }
#if (APP_IR_OVER_BLE)
            ir_repeat_delay_release_time();
#endif  /* APP_IR_OVER_BLE */
        }
    }
#endif  /* REMOTE_IR_ENABLE */

    key_type = 0;

}


void app_factory_reset(void)
{
    u8 audio_open=0;

    //printf("Factory Reset Mode ongoning\r\n");
    if(lowbat == 0x55)
    {
        return;
    }

    //global vari
    factory_reset_flag = 0x55;
    is_reconn_mode = 0;
    is_pairing_mode = 0;

    flag_ccc_data = 0;
#if BLE_AUDIO_ENABLE
    atv_char_ctl_ccc = 0;
    atv_char_rx_ccc = 0;
#endif
    write_ccc_info(&flag_ccc_data);
    //led
    device_led_setup(led_cfg[LED_SHINE_SUCCESS]);

    //ir
    write_ir_key_event_notify(0);
#if APP_IR_OVER_BLE
    ir_table_init();
#endif
    analog_write(DEEP_ANA_REG2, 0);
    bls_ll_setAdvEnable(0);
    if(device_in_connection_state)
    {
        //printf("Accessibility_Shortcut_Mode=%x\r\n",Accessibility_Shortcut_Mode);
#if BLE_AUDIO_ENABLE
        if(ui_mic_enable)
        {
            audio_open = 1;
            extern u8 app_audio_key_stop(u8 reason);
            app_audio_key_stop(REASON_OTHERS);
        }
#endif
    }
    extern void app_enter_deep_timeset(void);
    app_enter_deep_timeset();
    if(audio_open)
        blt_soft_timer_add(app_factory_reset_timer, FACTORY_RESET_TIMEOUT);
    else
        app_factory_reset_timer();

}

#define GPIO_WAKEUP_KEYPROC_CNT                3


_attribute_data_retention_    static int gpioWakeup_keyProc_cnt = 0;
_attribute_data_retention_    static u32 keyScanTick = 0;

void proc_keyboard (u8 e, u8 *p, int n)
{
//    u16 consumer_key[2];
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

    if((keyScanLongPressTick == 0) && (Accessibility_Shortcut_Mode != APP_ACCESSIBILITY_SHORTCUT_NONE))
    {
        //printf("stop led\r\n");
        device_led_off(APP_LED_GREEN);
        Accessibility_Shortcut_Mode = APP_ACCESSIBILITY_SHORTCUT_NONE;
    }

    if(keyScanLongPressTick && clock_time_exceed(keyScanLongPressTick, LongPressTick))
    {
        keyScanLongPressTick = 0;
        advertise_tick_retime();
        //printf("Accessibility_Shortcut timeout and Accessibility_Shortcut_Mode=%x\r\n",Accessibility_Shortcut_Mode);
        if(Accessibility_Shortcut_Mode == APP_ACCESSIBILITY_SHORTCUT_SINGLEKEY) return;
        if(Accessibility_Shortcut_Mode == APP_ACCESSIBILITY_SHORTCUT_PAIRING)
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
#if BLE_AUDIO_ENABLE
            notify_get_rsp_en = 1;
#endif
            if(device_in_connection_state == 0)
            {
//                app_stop_wakeup_loop();
                app_woble_clear_para();
                ll_resolvingList_reset();
#if APP_IR_OVER_BLE
                ir_table_init();
                ir_flash_set_flag(0);
#endif
                app_set_adv_interval_downgrade_indirect();
                extern void app_adv_indirect(void);
                app_adv_indirect();
            }
            else
            {
                if(ota_is_working)
                {
                    app_ota_slave_terminate();
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
        else if(Accessibility_Shortcut_Mode == APP_ACCESSIBILITY_SHORTCUT_FACTORYRESET)
        {
            app_factory_reset();
        }
        else if(Accessibility_Shortcut_Mode == APP_ACCESSIBILITY_SHORTCUT_BUGREPORT)
        {
            printf("APP_ACCESSIBILITY_SHORTCUT_BUGREPORT ongoning\r\n");
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
//                key_type = CONSUMER_KEY;
//
//                consumer_key[0] = GOOGLE_MKEY_BACK;
//                consumer_key[1] = GOOGLE_MKEY_CENTER;
//                bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);
            }
        }
        else if(Accessibility_Shortcut_Mode == APP_ACCESSIBILITY_SHORTCUT_DOWNBACK)
        {
            printf("APP_ACCESSIBILITY_SHORTCUT_DOWNBACK ongoning\r\n");
            device_led_setup(led_cfg[LED_SHINE_SUCCESS]);
            if(device_in_connection_state == 0)
            {
                key_type = IR_KEY;
                if(!ir_not_released){
                    ir_dispatch(TYPE_IR_SEND, 0x88, 0xd0);
                    ir_not_released = 1;
                }
            }
            else
            {
//                key_type = CONSUMER_KEY;
//                consumer_key[0] = GOOGLE_MKEY_BACK;
//                consumer_key[1] = GOOGLE_MKEY_DN;
//                bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 4);
            }
        }
        Accessibility_Shortcut_Mode = APP_ACCESSIBILITY_SHORTCUT_NONE;
    }

    //app_key_send_store();
}

extern u32    scan_pin_need;

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

    // TODO
    if(APP_WAKEUP_SRC_POWER_ON == app_ui_get_wakeup_src())
    {
        device_led_setup(led_cfg[LED_POWER_ON]);  //only poweron
    }
#endif

#if (REMOTE_IR_ENABLE)
    user_key_mode = analog_read(USED_DEEP_ANA_REG) & IR_MODE_DEEP_FLG ? KEY_MODE_IR : KEY_MODE_BLE;
#endif


#if (BLT_TEST_SOFT_TIMER_ENABLE)
    blt_soft_timer_init();

#if (MP_TEST_MODE)
    if((APP_WAKEUP_SRC_POWER_ON == app_ui_get_wakeup_src()) && (test_get_mode() != MODE_TEST))
#else
    if((APP_WAKEUP_SRC_POWER_ON == app_ui_get_wakeup_src()))
#endif
    {
        blt_soft_timer_add(app_poweron_timeout, 1000000);
    }
    if(detpowerkey == 1)
    {
        blt_soft_timer_add(app_usermode_to_testmode_timeout, 20000000);
    }
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
    u16 handle = pw->handle;

    u16 data = (pw->dat[1] << 8) + pw->dat[0];

    array_printf(pw->dat,(pw->l2cap - 3));

    u8 save_to_flash = 1;

    switch(handle){
#if  BLE_AUDIO_ENABLE

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
 #endif
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

int app_is_key_released(void)
{
    return key_not_released;
}


/* Wake up source related functions */
_attribute_data_retention_ static u8 s_app_ui_wakeup_src = APP_WAKEUP_SRC_NONE;
static void app_ui_set_wakeup_src(APP_UI_WAKEUP_SRC_T wakeup_src){

    if(wakeup_src >= APP_WAKEUP_SRC_NOT_USE) return;
    s_app_ui_wakeup_src = wakeup_src;

    printf("wakeup source: {%s} !!!! \n",
                (APP_WAKEUP_SRC_POWER_ON==s_app_ui_wakeup_src)?"POWER_ON":
                (APP_WAKEUP_SRC_DEEP_PAD==s_app_ui_wakeup_src)?"DEEP_PAD":
                ((APP_WAKEUP_SRC_DEEP_TIMER==s_app_ui_wakeup_src)?"DEEP_TIMER":"SOFT_RESET"));
}

APP_UI_WAKEUP_SRC_T app_ui_get_wakeup_src(void){

    return s_app_ui_wakeup_src;
}

void app_ui_check_wakeup_src(u8 deepRetWakeUp){
    if((analog_read(USED_DEEP_ANA_REG) & POWER_ON_FLG) == 0){
        app_ui_set_wakeup_src(APP_WAKEUP_SRC_POWER_ON);
    }else if(!deepRetWakeUp){

        u8 my_power_flags = analog_read(0X44);

        printf(" my_power_flags: 0x%x \n", my_power_flags);

        if(WAKEUP_STATUS_PAD == (my_power_flags & (WAKEUP_STATUS_TIMER|WAKEUP_STATUS_PAD))){
            app_ui_set_wakeup_src(APP_WAKEUP_SRC_DEEP_PAD);
        }else if(my_power_flags & WAKEUP_STATUS_TIMER){
            app_ui_set_wakeup_src(APP_WAKEUP_SRC_DEEP_TIMER);
        }else{
            app_ui_set_wakeup_src(APP_WAKEUP_SRC_SOFT_RESET);
        }
    }

    /* Keep the POWER_ON_FLG bit as 1. This bit will be cleared when power on */
    analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) | POWER_ON_FLG);
}


void app_ui_check_delay_en_pm(void){
    /* 221116 by ken. clear delay PM flag. The delay condition is to press the ASSIST and CENTER buttons */
    if(g_delay_en_pm && clock_time_exceed(g_delay_en_pm, 2000000)){
        g_delay_en_pm = 0;
        printf("[NOTICE] Clear delay PM flag ");
    }
}


void app_ui_set_2ndPairKey(u8 key0, u8 key1){

    if((0xFF == key0) || (0xFF == key1)){
        g_2ndPairKey[0] = 0xFF;
        g_2ndPairKey[1] = 0xFF;
        return;
    }

    g_2ndPairKey[0] = key0;
    g_2ndPairKey[1] = key1;
    printf("[NOTICE] Enable 2nd pair COMBO key. key0:[%d] key2:[%d]", g_2ndPairKey[0], g_2ndPairKey[1]);
}

