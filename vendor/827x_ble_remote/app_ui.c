/********************************************************************************************************
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
 *******************************************************************************************************/

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

#include "../common/app_audio.h"



_attribute_data_retention_	u8 		key_type;
_attribute_data_retention_	int 	key_not_released;


_attribute_data_retention_	u8 		ota_is_working = 0;


_attribute_data_retention_	int     ui_mtu_size_exchange_req = 0;

_attribute_data_retention_	int 	ir_not_released;
_attribute_data_retention_	u8 		user_key_mode;
u8      ir_hw_initialed = 0;   //note: can not be retention variable



_attribute_data_retention_	u8		ui_mic_enable = 0;
_attribute_data_retention_	u8 		key_voice_press = 0;

//unpairing concerned
_attribute_data_retention_ u8 sendTerminate_before_checkSMPinfo = 0;
_attribute_data_retention_ u32 key_clr_smp_info_pressTick = 0;
_attribute_data_retention_ u8 key_clr_smp_info_press = 0;


//pairing start concerned
_attribute_data_retention_ static u8 pairing_success_flg;
_attribute_data_retention_ u8 sendTerminate_before_pairingStart = 0;
_attribute_data_retention_ u8 key_pairing_mode_press = 0;
_attribute_data_retention_ u32 key_pairing_pressTick = 0;

_attribute_data_retention_ u8 rcu_combined_2keys_flg = 0; //RCU two key combinations flg

_attribute_data_retention_	ble_sts_t 	push_Notifystatus = BLE_SUCCESS;

extern	u32 	latest_user_event_tick;
extern  u8 		htt_audio_model_key_press_flags;

static const u16 vk_consumer_map[16] = {
		MKEY_VOL_UP,
		MKEY_VOL_DN,
		MKEY_MUTE,
		MKEY_CHN_UP,

		MKEY_CHN_DN,
		MKEY_POWER,
		MKEY_AC_SEARCH,
		MKEY_RECORD,

		MKEY_PLAY,
		MKEY_PAUSE,
		MKEY_STOP,
		MKEY_FAST_FORWARD,  //can not find fast_backword in <<HID Usage Tables>>

		MKEY_FAST_FORWARD,
		MKEY_AC_HOME,
		MKEY_AC_BACK,
		MKEY_MENU,
};


/////////////////////////// led management /////////////////////
#if (BLT_APP_LED_ENABLE)

	enum{
		LED_POWER_ON = 0,
		LED_AUDIO_ON,	//1
		LED_AUDIO_OFF,	//2
		LED_SHINE_SLOW, //3
		LED_SHINE_FAST, //4
		LED_SHINE_OTA, //5
		LED_SHINE_160MS, //6
		LED_SHINE_125MS, //7
	};

	const led_cfg_t led_cfg[] = {
			{1000,    0,      1,      0x00,	 },    //power-on, 1s on
			{100,	  0 ,	  0xff,	  0x02,  },    //audio on, long on
			{0,	      100 ,   0xff,	  0x02,  },    //audio off, long off
			{500,	  500 ,   2,	  0x04,	 },    //1Hz for 3 seconds
			{250,	  250 ,   4,	  0x04,  },    //2Hz for 3 seconds
			{250,	  250 ,   200,	  0x08,  },    //2Hz for 50 seconds
			{160,     160,    200,    0x00,  }, 	//T = 320ms manul pair
			{125,     125,    3,      0x04,  },    //T = 250ms
	};

#endif





#if (STUCK_KEY_PROCESS_ENABLE)
	_attribute_data_retention_	u32 	stuckKey_keyPressTime;
	_attribute_data_retention_  u8      struckKey_occurred_flg = 0;
#endif
	_attribute_data_retention_  u8		ble_not_released = 0;



#if (REMOTE_IR_ENABLE)

	static const u8 kb_map_ble[30] = 	KB_MAP_BLE;
	static const u8 kb_map_ir[30] = 	KB_MAP_IR;


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
			device_led_setup(led_cfg[LED_SHINE_OTA]);
		#endif
		bls_ota_setTimeout(60 * 1000 * 1000); //set OTA timeout  15 seconds
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
	if(analog_read(USED_DEEP_ANA_REG) & CONN_DEEP_FLG)
	{
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
	#if 1
		u8 		key_buf[8] = {0};
		key_buf[2] = 0;
		bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
		deepback_key_state = DEEPBACK_KEY_IDLE;
	#else
		deepback_key_state = DEEPBACK_KEY_IDLE;
		//DEBUG("Deep back post: IR Key release\n");
		ir_dispatch(TYPE_IR_RELEASE, 0x00, 0x00);
	#endif

	}
#endif
}

void key_release_resend()
{
	if(push_Notifystatus != BLE_SUCCESS)
	{
		if(key_type == CONSUMER_KEY)
		{
			u16 consumer_key = 0;
			push_Notifystatus=bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
		}
		else if(key_type == KEYBOARD_KEY)
		{
			u8 KB_buf[8] = {0};
			push_Notifystatus=bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, KB_buf, 8); //release
		}
	}
}

void key_change_proc(void)
{


	latest_user_event_tick = clock_time() | 1;  //record latest key change time

	if(key_voice_press){  //clear voice key press flg
		key_voice_press = 0;
	}

	u8 key0 = kb_event.keycode[0];
	u8 key1 = kb_event.keycode[1];
	u16 key_value;
	u8 key_buf[8] = {0,0,0,0,0,0,0,0};

	key_not_released = 1;

	if(key_clr_smp_info_press){
		key_clr_smp_info_press = 0;
		rcu_combined_2keys_flg = 0;
	}

	if (kb_event.cnt == 2)   //two key press, do  not process
	{
#if REMOTE_IR_ENABLE
		if(ir_not_released)
		{
			//DEBUG(">= 2 keys are pressed at the same time, RCU don't transmit IR\n");

			ir_send_ctrl.is_sending = 0;
			ir_send_release();
		}
#endif
		if(ble_not_released)
		{
			ble_not_released = 0;
			//DEBUG("rcu don't support >= 2 keys are pressed at the same time, send release key");
			if(key_type == CONSUMER_KEY){
				u16 consumer_key = 0;
				//DEBUG("(consumer release)\n");
				bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);

			}
			if(key_type == KEYBOARD_KEY){
				key_buf[2] = 0;
				//DEBUG("(keyboard release)\n");
				bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);
			}
		}

#if (BLE_PHYTEST_MODE != PHYTEST_MODE_DISABLE)  //"enter + back" trigger PhyTest
		//notice that if IR enable, trigger keys must be defined in key map
		if ( (key0 == VK_ENTER && key1 == VK_0) || (key0 == VK_0 && key1 == VK_ENTER))
		{
			extern void app_phytest_init(void);
			extern void app_trigger_phytest_mode(void);
			app_phytest_init();
			app_trigger_phytest_mode();
#if BLT_APP_LED_ENABLE
			device_led_setup(led_cfg[LED_SHINE_FAST]);
#endif
		}
#endif
		/*
		 * After press [OK]+[RIGHT] key during 5 seconds, start pairing.RCU delete the pairing information and
		 * then after blink Green LED 3 times at interval of 250ms
		 */
#if (REMOTE_IR_ENABLE)
		if(!ui_mic_enable && !ota_is_working && ((key0 == 11 && key1 == 16) || (key0 == 16 && key1 == 11)))
#else
		if(!ui_mic_enable && !ota_is_working && ((key0 == VK_ENTER && key1 == VK_RIGHT) || (key0 == VK_RIGHT && key1 == VK_ENTER)))
#endif

		{
			key_pairing_mode_press = 1;
			key_pairing_pressTick = clock_time()|1;
			rcu_combined_2keys_flg = 1;

			ble_not_released = 0; //avoid key release send key code
			ir_not_released = 0;
		}
		/*
		 * After press [OK]+[DOWN] key during 5 seconds, start unpairing.RCU delete the pairing information and
		 * then after blink Green LED 3 times at interval of 250ms, exit unpairing.RCU can unpairing at anytime
		 */
		else if
#if (REMOTE_IR_ENABLE)
		(!ui_mic_enable && !ota_is_working && ((key0 == 16 && key1 == 21) || (key0 == 21 && key1 == 16)))
#else
		(!ui_mic_enable && !ota_is_working && ((key0 == VK_ENTER && key1 == VK_DOWN) || (key0 == VK_DOWN && key1 == VK_ENTER)))
#endif

		{
			key_clr_smp_info_press = 1;
			key_clr_smp_info_pressTick = clock_time()|1;
			rcu_combined_2keys_flg = 1;

			ble_not_released = 0; //avoid key release send key code
			ir_not_released = 0;
		}

	}
	else if(kb_event.cnt == 1)
	{
        u8 conn_state = blc_ll_getCurrentState();
        if(conn_state == BLS_LINK_STATE_CONN)
        {
            user_key_mode = KEY_MODE_BLE;
            analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG)&(~IR_MODE_DEEP_FLG));
        }
        else
        {
#if (REMOTE_IR_ENABLE)
            user_key_mode = KEY_MODE_IR;
            analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG)|IR_MODE_DEEP_FLG);
#endif
        }
		if(key0 == VOICE && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN)
		{
			#if (BLE_AUDIO_ENABLE)
				app_audio_key_start(true);
			#endif
		}

#if (REMOTE_IR_ENABLE)
		else if(user_key_mode == KEY_MODE_BLE)
		{

			key_value = kb_map_ble[key0];

			if(key_value >= 0xf0 )
			{
				if(!ble_not_released)
				{
					ble_not_released = 1;
					key_type = CONSUMER_KEY;
					u16 consumer_key = vk_consumer_map[key_value & 0x0f];
					bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);

				}

			}
			else
			{
				if(!ble_not_released)
				{
					ble_not_released = 1;
					key_type = KEYBOARD_KEY;
					key_buf[2] = key_value;
					bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);
				}
			}
		}
		else if(user_key_mode == KEY_MODE_IR)
		{  //IR mode

			if(!ir_not_released){
				key_value = kb_map_ir[key0];
				key_type = IR_KEY;
				ir_dispatch(TYPE_IR_SEND, 0x88, key_value);
				ir_not_released = 1;
			}
		}
		else
		{
			key_type = IDLE_KEY;
		}
#else
		else
		{
			key_value = key0;
			if(key_value >= 0xf0 ){
				key_type = CONSUMER_KEY;
				u16 consumer_key = vk_consumer_map[key_value & 0x0f];
				bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
			}
			else
			{
				key_type = KEYBOARD_KEY;
				key_buf[2] = key_value;
				bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);
			}
		}

#endif
	}
	else   //kb_event.cnt == 0,  key release
	{
		key_not_released = 0;
		if(key_pairing_mode_press)
		{
			key_pairing_mode_press = 0;
			rcu_combined_2keys_flg = 0;
		}
		if(key_type == CONSUMER_KEY)
		{
			if(ble_not_released)
			{
				ble_not_released = 0;
				u16 consumer_key = 0;
				push_Notifystatus = bls_att_pushNotifyData (HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
			}

		}
		else if(key_type == KEYBOARD_KEY)
		{
			if(ble_not_released)
			{
				ble_not_released = 0;
				key_buf[2] = 0;
				push_Notifystatus = bls_att_pushNotifyData (HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
			}
		}
#if (REMOTE_IR_ENABLE)
		else if(key_type == IR_KEY)
		{
			if(ir_not_released){
				ir_not_released = 0;
				ir_dispatch(TYPE_IR_RELEASE, 0, 0);  //release
			}
		}
#endif

#if (BLE_AUDIO_ENABLE)
		if(app_audio_key_flags & APP_AUDIO_KEY_FLAG_PRESS || htt_audio_model_key_press_flags)
		{
			app_audio_key_start(false);
		}
#endif

	}
}

void switch_to_origin_setting(u8 e, u8 *p, int n)
{
	//DEBUG("Adv TimeOut occur, disable adv\n");
	bls_ll_setAdvEnable(0);
	extern u32 advertise_begin_tick;

	advertise_begin_tick = 0;

#if BLT_APP_LED_ENABLE
	//DEBUG("Stop led blink\n");
	device_led_setup(led_cfg[LED_AUDIO_OFF]);
#endif

#if REMOTE_IR_ENABLE
	user_key_mode = KEY_MODE_IR;
#endif

}


#define GPIO_WAKEUP_KEYPROC_CNT				3

extern u32 advertise_begin_tick;
_attribute_data_retention_	static int gpioWakeup_keyProc_cnt = 0;
_attribute_data_retention_	static u32 keyScanTick = 0;

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
		#if (STUCK_KEY_PROCESS_ENABLE)
			if(kb_event.cnt){//key press
				stuckKey_keyPressTime = clock_time() | 1;
			}
		#endif
		key_change_proc();
	}

	key_release_resend();//check and resend  deepback key release send fail

#if(DEEPBACK_FAST_KEYSCAN_ENABLE)
	if(deepback_key_state != DEEPBACK_KEY_IDLE){
		deepback_post_proc();
	}
#endif
	static u8 start_pairing_adv_flg;
	static u8 start_check_smp_info_flg;
	//5s long press [OK] + [HOME] key - Pairing
	if(key_pairing_mode_press && clock_time_exceed(key_pairing_pressTick, 5000000))
	{
		user_key_mode = KEY_MODE_BLE;  // switch to BLE mode

		if(blc_ll_getCurrentState() == BLS_LINK_STATE_CONN)
		{
			//DEBUG("already in conn state\nTerminate the conn\n");
			bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN);
			sendTerminate_before_pairingStart = 1;
		}
		else
		{
			start_pairing_adv_flg = 1;
		}
	}
	//5s long press [OK] + [BACK] key - UnPairing (delete smp info)
	else if(key_clr_smp_info_press && clock_time_exceed(key_clr_smp_info_pressTick, 5000000))
	{
		key_clr_smp_info_press = 0;

		//After ble disconnect, switch to IR mode.
		if(blc_ll_getCurrentState() == BLS_LINK_STATE_CONN)
		{
			//DEBUG("Already in conn state, Terminate the conn\n");
			bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN);
			sendTerminate_before_checkSMPinfo = 1;
		}
		else
		{
			start_check_smp_info_flg = 1;
		}
	#if BLT_APP_LED_ENABLE
			device_led_setup(led_cfg[LED_SHINE_125MS]);
	#endif
	}


	if((sendTerminate_before_pairingStart == 2 || start_pairing_adv_flg) && !key_pairing_mode_press )
	{
		sendTerminate_before_pairingStart = 0;
		start_pairing_adv_flg = 0;
		key_pairing_mode_press = 0;
		/*
		 * If RC is not-virgin, upon user long press setting for 5s to initiate pairing,
		 * RC delete current pairing information hen start pairing as a Virgin RC.
		 */
		if(blc_smp_param_getCurrentBondingDeviceNumber())//get bonded device number
		{
			//DEBUG("RC is not-virgin, delete smp info\n");
			bls_smp_param_deleteByIndex(0);//because bonding only one device, delete index0's info.

		}
		//if do not disable adv the indirect adv time will not 30s
		if(blc_ll_getCurrentState() == BLS_LINK_STATE_ADV)
		{
			bls_ll_setAdvEnable(0);//disable adv start
		}

		//RCU transmit the Un-direct Advertising during 30 second
		//DEBUG("Set un-direct adv\n");

		bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
							ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
							0,  NULL, MY_APP_ADV_CHANNEL, ADV_FP_NONE);

		//DEBUG("BLE adv start, adv TO: 30s.\n");
		bls_ll_setAdvDuration(30*1000*1000, 1);
		bls_app_registerEventCallback (BLT_EV_FLAG_ADV_DURATION_TIMEOUT, &switch_to_origin_setting);
		bls_ll_setAdvEnable(1);//enable adv start
		user_set_rf_power(0, 0, 0);
		extern u32 advertise_begin_tick;
		advertise_begin_tick = clock_time();
	#if BLT_APP_LED_ENABLE
		device_led_setup(led_cfg[LED_SHINE_160MS]);
	#endif
	}
	else if(sendTerminate_before_checkSMPinfo == 2 || start_check_smp_info_flg)
	{
		sendTerminate_before_checkSMPinfo = 0;
		start_check_smp_info_flg = 0;

		if(blc_smp_param_getCurrentBondingDeviceNumber())
		{
			//Since only one set of binding information is required, remove the index0 here
			bls_smp_param_deleteByIndex(0);
		}
		else
		{
			//DEBUG("There's no bonding device info\n");
		}

		bls_ll_setAdvEnable(0);//disable adv
		advertise_begin_tick = 0;
	}

}


extern u32	scan_pin_need;




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




#if (BLT_APP_LED_ENABLE)
	device_led_init(GPIO_LED, LED_ON_LEVAL);  //LED initialization
	device_led_setup(led_cfg[LED_POWER_ON]);
#endif


#if (REMOTE_IR_ENABLE)
	user_key_mode = KEY_MODE_IR;
#endif



#if (BLT_TEST_SOFT_TIMER_ENABLE)
	blt_soft_timer_init();
	blt_soft_timer_add(&gpio_test0, 35000);
	blt_soft_timer_add(&gpio_test1, 23000);
	blt_soft_timer_add(&gpio_test2, 27000);
	blt_soft_timer_add(&gpio_test3, 33000);
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


#if (BLT_APP_LED_ENABLE)
	device_led_init(GPIO_LED, 1);  //LED initialization
#endif

#if (REMOTE_IR_ENABLE)
#endif

}


