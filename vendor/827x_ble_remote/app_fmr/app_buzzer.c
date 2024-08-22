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
    app_buzzer_symbol_t* song;
    int size;
}app_buzzer_song_t;

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
/**********************************************************************
 * LOCAL FUNCTIONS
 */

void app_buzzer_buffer_init(void){
    /* clear buffer data */
    memset(&s_app_buzzer_ctrl, 0x00, sizeof(app_buzzer_ctrl_t) );
}

void app_buzzer_pwm_init(void){

    pwm_set_clk(CLOCK_SYS_CLOCK_HZ, BUZZER_CLOCK);
    gpio_set_func(BUZZER_GPIO, AS_PWM4);
    pwm_set_mode(BUZZER_PWM_ID, PWM_NORMAL_MODE);
}

u8 app_buzzer_is_buzy(){
    return s_app_buzzer_ctrl.status;
}

void app_buzzer_play(APP_BUZZER_INDEX_T index, APP_BUZZER_STATUS_T status, u8 sound_level){

    if(s_app_buzzer_ctrl.song_index == index && s_app_buzzer_ctrl.status == status){
        /* Dynamically modify the volume of the song */
        s_app_buzzer_ctrl.sound_level = sound_level;
        return;
    }

    device_led_setup(led_cfg[LED_BUZZER]);

    s_app_buzzer_ctrl.song_index = index;

    app_buzzer_pwm_init();

    s_app_buzzer_ctrl.status = status;
    s_app_buzzer_ctrl.sound_level = sound_level;

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
            pwm_set_cycle_and_duty(BUZZER_PWM_ID, counter, counter*s_app_buzzer_ctrl.sound_level/100);
        }else{
            /* A cmp_tick of 0 will cause the PWM to continuously output low level. cycle_tick only requires non-0. */
            pwm_set_cycle_and_duty(BUZZER_PWM_ID, /* any non-zero value */32000, 0);
        }

        BUZZER_DEBUG_LOG("song_note_index: [%d] FREQ:[%d] duty:[%d] counter:[%d]", s_app_buzzer_ctrl.song_note_index, note, s_app_buzzer_ctrl.sound_level, counter);

        s_app_buzzer_ctrl.note_timer_tick = clock_time()|1;
        s_app_buzzer_ctrl.note_duration = duration;
    }
}

