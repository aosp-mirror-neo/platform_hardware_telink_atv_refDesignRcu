/******************************************************************************
 * @file     app_buzzer.c
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

#include "../app_config.h"
#include "tl_common.h"
#include "drivers.h"
#include "app_fms.h"


/**********************************************************************
 * LOCAL MARCO
 */

#define s_p_app_buzzer_ctrl     ((volatile app_buzzer_ctrl_t *)app_fms_buf)


/**********************************************************************
 * LOCAL TYPES
 */

/**********************************************************************
 * GLOBAL VARIABLES
 */

/**********************************************************************
 * LOCAL VARIABLES
 */

const app_buzzer_symbol_t app_song[] = {
//    {B_N_M5, NT/2}, {B_N_M5, NT/2},
//    {B_N_M6, NT},   {B_N_M5, NT},   {B_N_M1, NT},
//    {B_N_M7, 2*NT}, {B_N_M5, NT/2}, {B_N_M5, NT/2},
//    {B_N_M6, NT},   {B_N_M5, NT},   {B_N_H2, NT},
//    {B_N_H1, 2*NT}, {B_N_M5, NT/2}, {B_N_M5, NT/2},
//    {B_N_H5, NT},   {B_N_H3, NT},   {B_N_H1, NT},
//    {B_N_M7, NT},   {B_N_M6, NT},   {B_N_H4, NT/2},    {B_N_H4, NT/2},
//
//    {B_N_H3, NT},   {B_N_H1, NT},   {B_N_H2, NT},
//    {B_N_H1, 2*NT}, {B_N_M5, NT/2}, {B_N_M5, NT/2},
//    {B_N_H1, 3*NT},
//    {B_N_H1, 3*NT},

    {B_N_M1, NT/2},
    {B_N_M2, NT/2},
    {B_N_M3, NT/2},
    {B_N_M4, NT/2},
    {B_N_M5, NT/2},
    {B_N_M6, NT/2},
    {B_N_M7, NT/2},
};

//const static app_buzzer_ctrl_t s_app_buzzer_ctrl = {
//        status:APP_BUZZER_STATUS_IDLE,
//        song_index:0,
//        song_size:0,
//        note_duration:0,
//        note_timer_tick:0,
//};

/**********************************************************************
 * LOCAL FUNCTIONS
 */


void app_buzzer_buffer_init(void){
    /* clear buffer data */
//    memcpy((u8 *)s_p_app_buzzer_ctrl, &s_app_buzzer_ctrl, sizeof(app_buzzer_ctrl_t) );
    memset((u8 *)s_p_app_buzzer_ctrl, 0x00, sizeof(app_buzzer_ctrl_t) );
}

void app_buzzer_pwm_init(void){

    pwm_set_clk(CLOCK_SYS_CLOCK_HZ, BUZZER_CLOCK);
    gpio_set_func(BUZZER_GPIO, AS_PWM4);
    pwm_set_mode(PWM4_ID, PWM_NORMAL_MODE);
}


u8 app_buzzer_is_buzy(){
    return s_p_app_buzzer_ctrl->status;
}

void app_buzzer_play(APP_BUZZER_STATUS_T reason, u8 sound_level){

    if(s_p_app_buzzer_ctrl->status == reason){
        /* Dynamically modify the volume of the song */
        s_p_app_buzzer_ctrl->sound_level = sound_level;
        return;
    }

    app_buzzer_pwm_init();

    s_p_app_buzzer_ctrl->status = reason;
    s_p_app_buzzer_ctrl->sound_level = sound_level;


    s_p_app_buzzer_ctrl->song_index = 0;
    s_p_app_buzzer_ctrl->song_size = sizeof(app_song)/sizeof(app_buzzer_symbol_t);

    printf("Max index: %d  song size: %d \n", s_p_app_buzzer_ctrl->song_size, sizeof(app_song));

    /* set 1st note */
    u16 note = app_song[0].note;
    u16 duration = app_song[0].duration;

    s_p_app_buzzer_ctrl->song_start_tick =  s_p_app_buzzer_ctrl->note_timer_tick = clock_time()|1;
    s_p_app_buzzer_ctrl->note_duration = duration;

    u16 counter = BUZZER_CLOCK/note;
    pwm_set_cycle_and_duty(PWM4_ID, counter, counter*s_p_app_buzzer_ctrl->sound_level/100);

    pwm_start(PWM4_ID);

    bls_pm_setSuspendMask (SUSPEND_DISABLE);
}

void app_buzzer_stop(void){
    pwm_stop(PWM4_ID);
    s_p_app_buzzer_ctrl->status = APP_BUZZER_STATUS_IDLE;

}

void app_buzzer_task(void){

    if(!s_p_app_buzzer_ctrl->status) return;

//    if(s_p_app_buzzer_ctrl->song_index >= s_p_app_buzzer_ctrl->song_size)    return;

    if(clock_time_exceed(s_p_app_buzzer_ctrl->note_timer_tick,s_p_app_buzzer_ctrl->note_duration*1000)){

        /* 220707. Make sure to play the complete note before stopping the song */
        if(clock_time_exceed(s_p_app_buzzer_ctrl->song_start_tick, APP_FMS_SONG_MAX_PLAY_TIME*1000000)){
            app_imc_stop_alert();
            printf("Maximum play time reached !!!!!!!! \n");
        }

        printf("pwm_state: 0x%02X song level: %d  index: %d  \n",
                reg_pwm_enable&BIT(PWM4_ID),
                s_p_app_buzzer_ctrl->sound_level,
                s_p_app_buzzer_ctrl->song_index );

        s_p_app_buzzer_ctrl->song_index++;

        if(s_p_app_buzzer_ctrl->song_index >= s_p_app_buzzer_ctrl->song_size){

            if(APP_BUZZER_STATUS_FIND_ME == s_p_app_buzzer_ctrl->status){
                /* If the buzzer is started by the find me command, it will loop infinitely */
                s_p_app_buzzer_ctrl->song_index = 0;
            }else{
                app_buzzer_stop();
                printf("play done !!!!!!!! \n");
                return;
            }
        }

        u16 note = app_song[s_p_app_buzzer_ctrl->song_index].note;
        u16 duration = app_song[s_p_app_buzzer_ctrl->song_index].duration;
        u16 counter = BUZZER_CLOCK/note;

//        printf("cycel: %d duty: %d \n",counter,counter*s_p_app_buzzer_ctrl->sound_level/100);
        pwm_set_cycle_and_duty(PWM4_ID, counter, counter*s_p_app_buzzer_ctrl->sound_level/100);

        s_p_app_buzzer_ctrl->note_timer_tick = clock_time()|1;
        s_p_app_buzzer_ctrl->note_duration = duration;
    }
}

