/********************************************************************************************************
 * @file     app_audio.c
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
#include "blt_common.h"
#include "adpcm_encode.h"

#include "app_audio.h"
#include "blt_led.h"

#if (BLE_AUDIO_ENABLE)

#define APP_AUDIO_GOOGLE_TIMEOUT1         3000000 // 3s
#define APP_AUDIO_GOOGLE_TIMEOUT2         15000000 // 15s

s16 audio_raw_buffer[AUDIO_BUFFER_SAMPLE];

_attribute_data_retention_ u32 app_audio_timer = 0;
_attribute_data_retention_ u32 app_audio_waitack_timer = 0;
_attribute_data_retention_ static u8 audio_send_index = 0;

_attribute_data_retention_ u8 app_audio_key_flags = 0; 	//bit[7]:1-press,0-release;
													   //bit[0]:1-report press; bit[1]:1-report release
                          	  	  	  	  	  	  	  //bit[2]:1-report search for google
_attribute_data_retention_ u16 app_audio_sync_serial = 0;
_attribute_data_retention_ u8 htt_audio_model_enable_flags = 0;
_attribute_data_retention_ u8 htt_audio_model_key_press_flags = 0;
_attribute_data_retention_ u8 g_stream_id = 1;
_attribute_data_retention_ u32 audio_stick = 0;

extern const 	led_cfg_t led_cfg[];
extern u8		ui_mic_enable;
extern int 		device_in_connection_state;
extern int 		ui_mtu_size_exchange_req;
extern u8       ui_mic_enable;
extern u32 		latest_user_event_tick;
extern u8  		audio_start;

extern void 	battery_set_detect_enable (int en);

void app_audio_init(void)
{
	app_audio_waitack_timer = 0;
	app_audio_key_flags = 0;
	htt_audio_model_enable_flags = 0;
	htt_audio_model_key_press_flags = 0;
#if BLE_DMIC_ENABLE
	gpio_set_func(GPIO_DMIC_DI, AS_GPIO);
	gpio_set_func(GPIO_DMIC_CK, AS_GPIO);
	gpio_set_input_en(GPIO_DMIC_DI, 0);
	gpio_set_output_en(GPIO_DMIC_DI, 0);
#else
	gpio_set_func(GPIO_AMIC_BIAS, AS_GPIO);
	gpio_set_input_en(GPIO_AMIC_BIAS, 0);
	gpio_set_output_en(GPIO_AMIC_BIAS, 1);
	gpio_write(GPIO_AMIC_BIAS, 0);
	gpio_set_func(GPIO_PC1, AS_GPIO);
#endif
	audio_config_mic_buf((u16 *)audio_raw_buffer, AUDIO_BUFFER_SIZE);

}

void app_audio_key_start(u8 isPress)
{
	u8 sendBuff[20]  = {0};

	if(isPress)
	{
		if(ui_mic_enable) return;

		if(htt_audio_model_enable_flags)
		{
			// htt model
			audio_stick = clock_time()|1;

			htt_audio_model_key_press_flags = 1;

			//htt audio start
			sendBuff[0] = ATV_MIC_CHAR_RSP_OPEN;
			sendBuff[1] = REASON_HTT; //reason :HTT Audio transfer is triggered by ¡°Assistant¡± button press and will stop once the button is released.
			sendBuff[2] = CS_16K16B; //codec used :ADPCM 16kHz/16bit
            if(g_stream_id > 0x80)
            {
            	g_stream_id = 1;
            }
			sendBuff[3] = g_stream_id; //streamid :1 - 0x80;

			g_stream_id ++;

			if(BLE_SUCCESS != bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 4))
			{
				return ;
			}
			//enable audio
			app_audio_enable();

		}
		else
		{
			//on request model
			if(app_audio_key_flags & APP_AUDIO_KEY_FLAG_PRESS) return;
			app_audio_key_flags |= APP_AUDIO_KEY_FLAG_PRESS;

			app_audio_waitack_timer = clock_time() | 1;

			app_audio_key_flags |= APP_AUDIO_KEY_FLAG_REPORT_PRESS;

			if((app_audio_key_flags & APP_AUDIO_KEY_FLAG_REPORT_PRESS))
			{
				ble_sts_t ret;
				u16 search_key = 0x0221;
				ret = bls_att_pushNotifyData(HID_CONSUME_REPORT_INPUT_DP_H, (u8*)&search_key, 2);
				if(ret != BLE_SUCCESS) return;
				app_audio_key_flags &= ~APP_AUDIO_KEY_FLAG_REPORT_PRESS;
				app_audio_key_flags |= APP_AUDIO_KEY_FLAG_REPORT_RELEASE;

			}
			if(app_audio_key_flags & APP_AUDIO_KEY_FLAG_REPORT_RELEASE)
			{
				ble_sts_t ret;
				u16 consumerKey_release = 0x0000;
				ret = bls_att_pushNotifyData(HID_CONSUME_REPORT_INPUT_DP_H, (u8*)&consumerKey_release, 2);
				if(ret != BLE_SUCCESS) return;
				app_audio_key_flags &= ~APP_AUDIO_KEY_FLAG_REPORT_RELEASE;
				app_audio_key_flags |= APP_AUDIO_KEY_FLAG_REPORT_START;

			}

			if((app_audio_key_flags & APP_AUDIO_KEY_FLAG_REPORT_START))
			{
				ble_sts_t ret;
				u8 search = 0x08;
				ret = bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, &search, 1);
				if(ret != BLE_SUCCESS) return;
				app_audio_key_flags &= ~APP_AUDIO_KEY_FLAG_REPORT_START;

			}
		}

	}
	else
	{
		if(htt_audio_model_enable_flags)
		{
			htt_audio_model_key_press_flags = 0;
			//disable audio
			app_audio_disable();
			//send audio stop to  master
			ble_sts_t ret;
			sendBuff[0] = ATV_MIC_CHAR_RSP_CLOSE;
			sendBuff[1] = AS_RELEASE_BUTTON; //triggered by releasing an Assistant button during HTT interaction;
			ret = bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 2);
			if(ret != BLE_SUCCESS) return;
		}
		else
		{
			app_audio_key_flags &= ~APP_AUDIO_KEY_FLAG_PRESS;
		}
	}
}

void app_audio_set_ble_conn(u8 isConn)
{
	if(isConn)
	{
		//do nothing
	}
	else
	{
        //disconnect
		htt_audio_model_enable_flags = 0;
		g_stream_id = 1;
		if(ui_mic_enable)
		{
			app_audio_disable();
		}
		app_audio_waitack_timer = 0;
		app_audio_key_flags = 0;
	}
}



void app_audio_enable(void)
{
	if(!device_in_connection_state) return;
	if(ui_mic_enable) return;
	ui_mic_enable = 1;
	app_audio_sync_serial = 0;
#if 0
	if( ui_mtu_size_exchange_req && (blc_ll_getCurrentState() == BLS_LINK_STATE_CONN))
	{
		ui_mtu_size_exchange_req =0;
		blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, 0x009e);
	}
#endif

	bls_pm_setManualLatency(0);
	bls_pm_setSuspendMask(SUSPEND_DISABLE);
	app_audio_timer = clock_time() | 1;
	adpcm_encode_init();
	audio_send_index = 0;
	#if (BLT_APP_LED_ENABLE)
		device_led_setup(led_cfg[1]);
	#endif

	audio_config_mic_buf((u16*)audio_raw_buffer, AUDIO_BUFFER_SIZE);

#if BLE_DMIC_ENABLE

	gpio_set_func(GPIO_DMIC_DI, AS_DMIC);
	gpio_set_func(GPIO_DMIC_CK, AS_DMIC);
	gpio_set_input_en(GPIO_DMIC_DI, 1);

	audio_dmic_init(AUDIO_16K);
#else
	gpio_set_output_en(GPIO_AMIC_BIAS, 1);
	gpio_write(GPIO_AMIC_BIAS, 1);
	gpio_set_func(GPIO_PC1, AS_AMIC);
	audio_amic_init(AUDIO_16K);
#endif
#if (IIR_FILTER_ENABLE)
	//only used for debugging EQ Filter parameters, removed after mass production
	extern void filter_setting();
	filter_setting();
#endif

#if ((!BLE_DMIC_ENABLE) && (BATT_CHECK_ENABLE))
	battery_set_detect_enable(0);
#endif
	latest_user_event_tick = clock_time() | 1;  //record latest key change time
}



void app_audio_disable(void)
{
	if(!ui_mic_enable) return;
	ui_mic_enable = 0;
	audio_start = 0;
	audio_stick = 0;
	#if (BLT_APP_LED_ENABLE)
		device_led_setup(led_cfg[2]);
	#endif
		app_audio_timer = 0;

#if BLE_DMIC_ENABLE

	gpio_set_func(GPIO_DMIC_DI, AS_GPIO);
	gpio_set_func(GPIO_DMIC_CK, AS_GPIO);
	gpio_set_input_en(GPIO_DMIC_DI, 0);
	gpio_set_output_en(GPIO_DMIC_DI, 0);

#else
	gpio_set_input_en(GPIO_AMIC_BIAS, 0);
	gpio_set_output_en(GPIO_AMIC_BIAS, 1);
	gpio_write(GPIO_AMIC_BIAS, 0);
	gpio_set_func(GPIO_PC1, AS_GPIO);

#endif
#if ((!BLE_DMIC_ENABLE) && (BATT_CHECK_ENABLE))
	battery_set_detect_enable(1);
#endif
}


void app_audio_handler(void)
{
	u8 i;
	u8 audio_send_length;
	u8 *p_audio_data;

	if(!device_in_connection_state) return;

	static u32 audioProcTick = 0;
	if(clock_time_exceed(audioProcTick, 500)){
		audioProcTick = clock_time();
	}
	else{
		return;
	}


	if(app_audio_timer != 0 && clock_time_exceed(app_audio_timer, APP_AUDIO_GOOGLE_TIMEOUT2))
	{
		app_audio_disable();

		ble_sts_t ret;
		u8 sendBuff[2] = {0};
		sendBuff[0] = ATV_MIC_CHAR_RSP_CLOSE;
		sendBuff[1] = AS_TIMEOUT; //triggered by ¡° Audio Transfer Timeout ¡±;
		ret = bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 2);
		if(ret != BLE_SUCCESS) return;
	}

	if(!htt_audio_model_enable_flags)
	{
		if(app_audio_waitack_timer != 0 && clock_time_exceed(app_audio_waitack_timer, APP_AUDIO_GOOGLE_TIMEOUT1))
		{
			app_audio_waitack_timer = 0;
			app_audio_key_flags = 0;
			app_audio_disable();
			return;
		}
	}

	adpcm_gol_encode_handler(0);

	if(blc_ll_getTxFifoNumber() < 7)
	{
		p_audio_data = adpcm_encode_get_data();

		if(p_audio_data)
		{
			for(i=0; i<7; i++)
			{
				if(audio_send_index < 6)
				{
					audio_send_length = 20;

				}
				else if(audio_send_index == 6)
				{
					audio_send_length = 14;
				}
				else
				{
					audio_send_length = 0;
				}
				if(BLE_SUCCESS == bls_att_pushNotifyData(AUDIO_GOOGLE_RX_DP_H, (u8*)p_audio_data+audio_send_index*20, audio_send_length))
				{
					audio_send_index++;
				}
				else
				{
					return ;
				}
				if(audio_send_index == 7)
				{
					audio_send_index = 0;
					adpcm_encode_inc_data();
				}
			}

		}

	}

}



int app_auido_google_callback(void* p)
{
	u8 sendBuff[20]  = {0};
	rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;

	if(pw->dat[0] == AUDIO_GOOGLE_CMD_OPEN)
	{
		if(htt_audio_model_enable_flags)
		{
			//when htt model ongoing mic open comes send mic already open error (0x0f01) and continue
			sendBuff[0] = ATV_MIC_CHAR_RSP_MIC_OPEN_ERROR;
			sendBuff[1] = MIC_OPEN_ERROR_HIGH;
			sendBuff[2] = MIC_ALREADY_OPEN_LOW;
			if(BLE_SUCCESS != bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 3))
			{
				return 0;
			}
		}
		else
		{
			//on request model -- audio start
			sendBuff[0] = ATV_MIC_CHAR_RSP_OPEN;
			sendBuff[1] = REASON_MICOPEN; //reason :Audio transfer is triggered by MIC_OPEN request;
			sendBuff[2] = CS_16K16B; //codec used :ADPCM 16kHz/16bit
			sendBuff[3] = 0x00; //streamid = 0;
			if(BLE_SUCCESS != bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 4))
			{
				return 0;
			}
			audio_stick = clock_time()|1;

			app_audio_enable();

			app_audio_waitack_timer = 0;

		}

	}
	else if(pw->dat[0] == AUDIO_GOOGLE_CMD_CLOSE)
	{
		u8 streamid = pw->dat[1];

		if(htt_audio_model_enable_flags)
		{
			//htt model
			if(streamid != 0)
			{
				app_audio_disable();
				sendBuff[0] = ATV_MIC_CHAR_RSP_CLOSE;
				sendBuff[1] = AS_MICCLOSE; //triggered by MIC_CLOSE message;
				if(BLE_SUCCESS != bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 2))
				{
					return 0;
				}
			}
		}
		else
		{
			//on request model
			app_audio_disable();
			sendBuff[0] = ATV_MIC_CHAR_RSP_CLOSE;
			sendBuff[1] = AS_MICCLOSE; //triggered by MIC_CLOSE message;
			if(BLE_SUCCESS != bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 2))
			{
				return 0;
			}
		}
	}
	else if(pw->dat[0] == AUDIO_GOOGLE_CMD_CAP)
	{

        //u16 version = pw->dat[1];
        //u16 legacy_0x0003 = pw->dat[3];

        u8 model = pw->dat[5];

        if(ON_REQUEST == model)
        {
        	// on request model
        	htt_audio_model_enable_flags = 0;
        }
        else if (HTT == model)
        {
        	//htt model
        	htt_audio_model_enable_flags = 1;
        }

		//htt_audio_model_enable_flags = 1;


		if(htt_audio_model_enable_flags)
		{
			sendBuff[0] = ATV_MIC_CHAR_RSP_CAP;
			sendBuff[1] = VERSION_10_HIGH; // version high
			sendBuff[2] = VERSION_10_LOW; // version low 1.0
			sendBuff[3] = CS_16K16B; //codecs_supported 16Khz(ADPCM,16bits)
			sendBuff[4] = HTT; //assistant interaction model ON_Request(0x00)HTT(0x03)
			sendBuff[5] = BYTE_20_HIGH; //frame lengths high
			sendBuff[6] = BYTE_20_LOW; //frame lengths low -- 20byte
			sendBuff[7] = EcReserved; // not used
			sendBuff[8] = 0x00; //not used
		}
		else
		{
			sendBuff[0] = ATV_MIC_CHAR_RSP_CAP;
			sendBuff[1] = VERSION_04_HIGH; //major version
			sendBuff[2] = VERSION_04_LOW; //minor version
			sendBuff[3] = v4CS_BOTH_HIGH; //codecs_supported high
			sendBuff[4] = v4CS_BOTH_LOW; //codecs_supported low:  0x0001-8Khz(ADPCM,16bits), 0x0003-8K/16Khz(ADPCM,16bits)
			sendBuff[5] = v4BF_HIGH; //frame lengths high
			sendBuff[6] = v4BF_LOW; //frame lengths low -- 134Byts
			sendBuff[7] = BYTE_20_HIGH; //packet lengths high
			sendBuff[8] = BYTE_20_LOW; //packet lengths low -- 20 Bytes

		}

		if(bls_att_pushNotifyData(AUDIO_GOOGLE_CTL_DP_H, sendBuff, 9))
		{
			return 0;
		}

	}
	else if(pw->dat[0] == AUDIO_GOOGLE_CMD_EXTEND)
	{
		//fresh the timeout
		app_audio_timer = clock_time() | 1;
	}

	return 0;
}


#endif
