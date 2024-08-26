/********************************************************************************************************
 * @file    app_buzzer.c
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
#include "../app_ui.h"
#include "../app_ota.h"

#include "app_buzzer.h"
#include "app_buzzer_song.h"

/**********************************************************************
 * LOCAL MARCO
 */



#define BUZZER_GPIO               GPIO_PB4
#define BUZZER_PWM_ID             PWM4_ID
#define BUZZER_CLOCK              CLOCK_SYS_CLOCK_HZ

#define BUZZER_DEBUG_LOG(...)


/**********************************************************************
 * LOCAL TYPES
 */









typedef struct{
    app_buzzer_symbol_t*    song;
    int                     size;
}app_buzzer_song_t;

typedef struct {
    APP_BUZZER_STATUS_T     status;
    unsigned char           sound_level;

    unsigned char           song_index;
    unsigned char           song_note_index;
    unsigned short          song_size;

    unsigned short          note_duration;
    unsigned int            note_timer_tick;
    unsigned int            song_start_tick;
}app_buzzer_ctrl_t;

enum{
    BUZZER_DUTY_MAX = 100,
    BUZZER_DUTY_50  = 50,
    BUZZER_DUTY_30  = 30,
    BUZZER_DUTY_15  = 15,
    BUZZER_DUTY_5   = 5,
    BUZZER_DUTY_MIN = 0,
};

/**********************************************************************
 * GLOBAL VARIABLES
 */

/**********************************************************************
 * LOCAL VARIABLES
 */



const app_buzzer_song_t app_song[] =
{
    {(app_buzzer_symbol_t*)app_song_loop, sizeof(app_song_loop)/sizeof(app_buzzer_symbol_t)},
};

_attribute_data_retention_ static app_buzzer_ctrl_t s_app_buzzer_ctrl={0};
_attribute_data_retention_ static u32 s_buzzer_silence_tick = 0;


/**********************************************************************
 * LOCAL FUNCTIONS
 */

void app_buzzer_pwm_init(void){

//    pwm_set_clk(CLOCK_SYS_CLOCK_HZ, BUZZER_CLOCK);
    app_ui_init_pwm_clock();
    gpio_set_func(BUZZER_GPIO, AS_PWM4);
    pwm_set_mode(BUZZER_PWM_ID, PWM_NORMAL_MODE);
}


/**********************************************************************
 * GLOBAL FUNCTIONS
 */

void app_buzzer_buffer_init(void){
    /* clear buffer data */
    memset(&s_app_buzzer_ctrl, 0x00, sizeof(app_buzzer_ctrl_t) );
}

u8 app_buzzer_is_buzy(void){
    return s_app_buzzer_ctrl.status;
}

void app_buzzer_play(APP_BUZZER_INDEX_T index, APP_BUZZER_STATUS_T status, u8 sound_level){

    if(s_app_buzzer_ctrl.song_index == index && s_app_buzzer_ctrl.status == status){
        /* Dynamically modify the volume of the song */
        s_app_buzzer_ctrl.sound_level = min(sound_level, BUZZER_DUTY_MAX);
        return;
    }

    /* Using a buzzer will cause the battery voltage to drop a lot. So stop OTA first */
    if(ota_is_working){
        app_ota_terminate(GOOGLE_OTA_USER_TERMINATE);
        printf("play buzzer during OTA\n");
    }
    app_ota_ClearDelayCount();

    device_led_setup(led_cfg[LED_BUZZER]);

    /* Prevent index parameter errors */
    s_app_buzzer_ctrl.song_index = min(index, APP_BUZZER_PLAY_INDEX_RINGTONE_0);

    app_buzzer_clear_silence_tick();
    app_buzzer_pwm_init();

    s_app_buzzer_ctrl.status = min(status, APP_BUZZER_STATUS_PLAY_LOOP);
    s_app_buzzer_ctrl.sound_level = min(sound_level, BUZZER_DUTY_MAX);

    s_app_buzzer_ctrl.song_note_index = 0;
    s_app_buzzer_ctrl.song_size = app_song[s_app_buzzer_ctrl.song_index].size;


    printf("song index: %d  song size: %d\n", s_app_buzzer_ctrl.song_index, s_app_buzzer_ctrl.song_size);

    /* set 1st note */
    u16 note = app_song[s_app_buzzer_ctrl.song_index].song[0].note;
    u16 duration = app_song[s_app_buzzer_ctrl.song_index].song[0].duration;

    s_app_buzzer_ctrl.song_start_tick =  s_app_buzzer_ctrl.note_timer_tick = clock_time()|1;
    s_app_buzzer_ctrl.note_duration = duration;

    u16 counter = BUZZER_CLOCK/note;
    pwm_set_cycle_and_duty(BUZZER_PWM_ID, counter, counter*s_app_buzzer_ctrl.sound_level/100);

    printf("song_note_index: %d cycle: %d counter: %d duty: %d\n",s_app_buzzer_ctrl.song_note_index, note ,counter, s_app_buzzer_ctrl.sound_level);
    pwm_start(BUZZER_PWM_ID);

    bls_pm_setSuspendMask (SUSPEND_DISABLE);
}

void app_buzzer_stop(void){

    device_led_off(APP_LED_RED);

    pwm_stop(BUZZER_PWM_ID);
    app_buzzer_buffer_init();
}

u32 app_buzzer_get_silence_tick(void){
    return s_buzzer_silence_tick;
}

void app_buzzer_clear_silence_tick(void){
    s_buzzer_silence_tick = 0;
}

u8 app_buzzer_set_and_get_pwm_duty_by_voltage(void){

    enum{ MAX_BAT_VOL = 3200 };

    extern u16 get_battery_voltage(void);
    u16 my_bat_vol = get_battery_voltage();
    my_bat_vol = min(my_bat_vol, MAX_BAT_VOL);

    u8 my_buzzer_duty = BUZZER_DUTY_50;

    switch (my_bat_vol)
    {
    case 2600 ... 3300:     /*  When the battery voltage is between 2.6V to 3.2V, keep the default PWM duty. */
        break;
    case 2500 ... 2599:     /*  When the battery voltage is between 2.5V to 2.59V, change PWM duty to 30%.   */
        my_buzzer_duty = BUZZER_DUTY_30;
        break;
    case 2350 ... 2499:     /*  When the battery voltage is between 2.35V to 2.49V, change PWM duty to 15%.   */
        my_buzzer_duty = BUZZER_DUTY_15;
        break;
    case 2250 ... 2349:     /*  When the battery voltage is between 2.25V to 2.24V, change PWM duty to 5%.    */
        my_buzzer_duty = BUZZER_DUTY_5;
        break;
    default:
        my_buzzer_duty = BUZZER_DUTY_MIN;
        break;
    }

    s_app_buzzer_ctrl.sound_level = min(my_buzzer_duty, BUZZER_DUTY_MAX);

    return s_app_buzzer_ctrl.sound_level;
}

void app_buzzer_task(void){

    if(!s_app_buzzer_ctrl.status) return;

    if(clock_time_exceed(s_app_buzzer_ctrl.note_timer_tick,s_app_buzzer_ctrl.note_duration*1000)){

        BUZZER_DEBUG_LOG("pwm_state: 0x%02X song level: %d  index: %d",
                reg_pwm_enable&BIT(BUZZER_PWM_ID),
                s_app_buzzer_ctrl.sound_level,
                s_app_buzzer_ctrl.song_note_index );

        s_app_buzzer_ctrl.song_note_index++;

        if(s_app_buzzer_ctrl.song_note_index >=s_app_buzzer_ctrl.song_size){

            if(APP_BUZZER_STATUS_PLAY_LOOP == s_app_buzzer_ctrl.status){
                /* If the buzzer is started by the find me command, it will loop infinitely */
                s_app_buzzer_ctrl.song_note_index = 0;

            }else{
                app_buzzer_stop();
                printf("play done !!!!!!!! \n");
                return;
            }
        }

        u16 note = app_song[s_app_buzzer_ctrl.song_index].song[s_app_buzzer_ctrl.song_note_index].note;
        u16 duration = app_song[s_app_buzzer_ctrl.song_index].song[s_app_buzzer_ctrl.song_note_index].duration;

        if(note){
            u16 counter = BUZZER_CLOCK/note;

            /* The value of s_app_buzzer_ctrl.sound_level will not exceed 100 when assigned. If this happens, set duty to the lowest value */
            if(s_app_buzzer_ctrl.sound_level > 100){
                s_app_buzzer_ctrl.sound_level = BUZZER_DUTY_MIN;
            }

            pwm_set_cycle_and_duty(BUZZER_PWM_ID, counter, counter*s_app_buzzer_ctrl.sound_level/100);
            app_buzzer_clear_silence_tick();
        }else{
            /* A cmp_tick of 0 will cause the PWM to continuously output low level. cycle_tick only requires non-0. */
            pwm_set_cycle_and_duty(BUZZER_PWM_ID, /* any non-zero value */32000, 0);
            s_buzzer_silence_tick = clock_time()|1;
        }

        BUZZER_DEBUG_LOG("song_note_index: [%d] FREQ:[%d] duty:[%d] counter:[%d]", s_app_buzzer_ctrl.song_note_index, note, s_app_buzzer_ctrl.sound_level, counter);

        s_app_buzzer_ctrl.note_timer_tick = clock_time()|1;
        s_app_buzzer_ctrl.note_duration = duration;
    }
}
