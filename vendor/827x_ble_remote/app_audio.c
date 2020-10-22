/******************************************************************************
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
 *****************************************************************************/

#include "tl_common.h"
#include "drivers.h"
#include "app_config.h"
#include "application/audio/gl_audio.h"
#include "application/audio/tl_audio.h"
#include "application/audio/audio_config.h"
#include "stack/ble/ble.h"
#include "app_audio.h"
#include "vendor/common/blt_led.h"
#include "stack/ble/ble.h"
#include "battery_check.h"

_attribute_data_retention_    u8        ui_mic_enable = 0;
_attribute_data_retention_    u8        key_on_request_press = 0;
_attribute_data_retention_    u8        key_ptt_press = 0;
_attribute_data_retention_    u8        key_htt_press = 0;

#if (BLE_AUDIO_ENABLE)

#include "application/audio/gl_audio.h"
    u32     key_voice_pressTick = 0;

    #if BLE_DMIC_ENABLE
        void dmic_gpio_reset (void)
        {
            gpio_set_func(GPIO_DMIC_BIAS, AS_GPIO);
            gpio_set_input_en(GPIO_DMIC_BIAS, 0);
            gpio_set_output_en(GPIO_DMIC_BIAS, 1);
            gpio_write(GPIO_DMIC_BIAS, 0);

            gpio_set_func(GPIO_DMIC_DI, AS_GPIO);
            gpio_set_input_en(GPIO_DMIC_DI, 1);
            gpio_set_output_en(GPIO_DMIC_DI, 0);
            gpio_setup_up_down_resistor(GPIO_DMIC_DI, PM_PIN_PULLDOWN_100K);

            gpio_set_func(GPIO_DMIC_CK, AS_GPIO);
            gpio_set_input_en(GPIO_DMIC_CK, 0);
            gpio_set_output_en(GPIO_DMIC_CK, 1);
            gpio_write(GPIO_DMIC_CK, 0);
        }
    #else
        void amic_gpio_reset (void)
        {
            gpio_set_func(GPIO_AMIC_BIAS, AS_GPIO);
            gpio_set_input_en(GPIO_AMIC_BIAS, 0);
            gpio_set_output_en(GPIO_AMIC_BIAS, 1);
            gpio_write(GPIO_AMIC_BIAS, 0);

            gpio_set_func(GPIO_AMIC_C1, AS_GPIO);
            gpio_set_input_en(GPIO_AMIC_C1, 0);
            gpio_set_output_en(GPIO_AMIC_C1, 1);
            gpio_write(GPIO_AMIC_C1, 0);

        }
    #endif
    extern u8  buffer_mic_pkt_wptr;
    extern u8  buffer_mic_pkt_rptr;
    extern u16 app_audio_sync_serial;
    extern u32 app_audio_transfer_timer;
    extern void app_ota_status(u8 status);

    u32   audio_stick = 0;
    _attribute_data_retention_    u8  audio_start = 0;
    void ui_enable_mic (int en)
    {
        ui_mic_enable = en;
        printf("ui_mic_enable=%x\r\n",ui_mic_enable);
        if(ui_mic_enable)
        {
            app_ota_status(0);
        }
        else
        {
            app_ota_status(1);
        }
#if BLE_DMIC_ENABLE
        //DMIC Bias output
        gpio_set_output_en (GPIO_DMIC_BIAS, en);
        gpio_write (GPIO_DMIC_BIAS, en);
#else
        //AMIC Bias output
        gpio_set_output_en (GPIO_AMIC_BIAS, en);
        #if (MCU_CORE_TYPE == MCU_CORE_8278)
            gpio_set_data_strength (GPIO_AMIC_BIAS, en);
        #endif
        gpio_write (GPIO_AMIC_BIAS, en);
#endif
        extern void app_mic_led(u8 en);
        app_mic_led(en);
        if(en){  //audio on
            app_audio_sync_serial = 0;
            bls_pm_setManualLatency(0);
            bls_pm_setSuspendMask(SUSPEND_DISABLE);

            app_audio_transfer_timer = clock_time() | 1;
//            extern u16    adpcm_serial_num;
//            adpcm_serial_num = 0;
//            audio_send_index = 0;

            ///////////////////// AUDIO initialization///////////////////
            //buffer_mic set must before audio_init !!!
            //audio_config_mic_buf ( (unsigned short*)buffer_mic, TL_MIC_BUFFER_SIZE);
            printf("buffer_mic:%d\n",(google_voice_ver==10)?VOICE_V1P0_ADPCM_UNIT_SIZE*8:VOICE_V0P4_ADPCM_UNIT_SIZE*8);
            audio_config_mic_buf ( (unsigned short*)buffer_mic, (google_voice_ver==10)?VOICE_V1P0_ADPCM_UNIT_SIZE*8:VOICE_V0P4_ADPCM_UNIT_SIZE*8);
            buffer_mic_pkt_rptr = buffer_mic_pkt_wptr = 0;
            #if (BLE_DMIC_ENABLE)  //Dmic config
                gpio_set_func(GPIO_DMIC_DI, AS_DMIC);
                gpio_set_func(GPIO_DMIC_CK, AS_DMIC);
                gpio_set_input_en(GPIO_DMIC_DI, 1);
                audio_dmic_init(AUDIO_16K);

            #else  //Amic config
                gpio_set_output_en(GPIO_AMIC_C1, 0);
                audio_set_mute_pga(0);  ////enable audio need follow this step: 1 enable bias; 2 disable mute_pga;
                                          //3 init; 4 delay about 17ms; 5 enable mute_pga.
                audio_amic_init(AUDIO_16K);
            #endif
                extern    u32     latest_user_event_tick;
                latest_user_event_tick = clock_time() | 1;
            #if (IIR_FILTER_ENABLE)
                //only used for debugging EQ Filter parameters, removed after mass production
                extern void filter_setting();
                filter_setting();
            #endif
        }
        else{  //audio off
            #if BLE_DMIC_ENABLE
                dmic_gpio_reset();
            #else
                #if (MCU_CORE_TYPE == MCU_CORE_8278)
                    audio_codec_and_pga_disable();    //power off codec and pga
                #elif (MCU_CORE_TYPE == MCU_CORE_8258)
                    adc_power_on_sar_adc(0);   //power off sar adc
                #endif
                    amic_gpio_reset();
            #endif
            audio_stick = 0;
            app_audio_transfer_timer = 0;
            audio_start = 0;
            buffer_mic_pkt_rptr = buffer_mic_pkt_wptr = 0;
        }
        #if ((!BLE_DMIC_ENABLE) && (BATT_CHECK_ENABLE))
            battery_set_detect_enable(!en);
        #endif
    }

    void voice_key_press(void)
    {
//        if(!ui_mic_enable && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){
//
//            if(app_audio_key_start(1) == APP_AUDIO_ENABLE){
//                audio_stick = clock_time()|1;
//                ui_enable_mic(1);
//            }
//        }
    }

    void voice_key_release(void)
    {
//        if(ui_mic_enable){
//            if(app_audio_key_start(0) == APP_AUDIO_DISABLE){
//                ui_enable_mic(0);
//            }
//        }
    }

//    _attribute_ram_code_ void task_audio (void)
//    {
//        static u32 audioProcTick = 0;
//        if(clock_time_exceed(audioProcTick, 500)){
//            audioProcTick = clock_time();
//        }
//        else{
//            return;
//        }
//
//        ///////////////////////////////////////////////////////////////
//        log_event(TR_T_audioTask);
//
//        if(app_audio_timeout_proc()){
//            return;
//        }
//        proc_mic_encoder ();
//
//
//
//        //send voice data
//        if(google_voice_ver == 0x04){
//            u8 audio_send_length;
//            //////////////////////////////////////////////////////////////////
//            if (blc_ll_getTxFifoNumber() < 7)
//            {
//                int *p = mic_encoder_data_buffer ();
//                if(p)
//                {
//                    for(u8 i=0; i<7; i++)
//                    {
//                        if(audio_send_index < 6)
//                        {
//                            audio_send_length = 20;
//
//                        }
//                        else if(audio_send_index == 6)
//                        {
//                            audio_send_length = 14;
//                        }
//                        else
//                        {
//                            audio_send_length = 0;
//                        }
//                        if(BLE_SUCCESS == bls_att_pushNotifyData(AUDIO_GOOGLE_RX_DP_H, (u8*)p+audio_send_index*20, audio_send_length))
//                        {
//                            audio_send_index++;
//                        }
//                        else
//                        {
//                            return ;
//                        }
//                        if(audio_send_index == 7)
//                        {
//                            audio_send_index = 0;
//                            mic_encoder_data_read_ok();
//                        }
//                    }
//
//                }
//            }
//        }else if(google_voice_ver == 10){
//
//        }else{
//            //google voice version error
//        }
//
//
//    }

    void proc_audio(void){

        if(ui_mic_enable){
            #if (MCU_CORE_TYPE == MCU_CORE_8278)
            if(audio_start || (audio_stick && clock_time_exceed(audio_stick, 380*1000))){// for 8278
            #elif (MCU_CORE_TYPE == MCU_CORE_8258)
            if(audio_start || (audio_stick && clock_time_exceed(audio_stick, 1*1000))){// for 8258
            #endif
                audio_start = 1;
                //task_audio();
                app_audio_task();
            }
        }
        else{
            audio_start = 0;
        }

    }

#endif

