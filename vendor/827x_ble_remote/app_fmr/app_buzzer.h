/********************************************************************************************************
 * @file    app_buzzer.h
 *
 * @brief   This is the header file for b85m
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
#ifndef APP_BUZZER_H_
#define APP_BUZZER_H_

#include "../app_config.h"

/**********************************************************************
 * GLOBAL DEFINE
 */

#define APP_BUZZER_SILENCE_TIME_MS          (500)       //500ms



typedef enum{
    APP_BUZZER_STATUS_IDLE      = 0,
    APP_BUZZER_STATUS_PLAY_ONCE,
    APP_BUZZER_STATUS_PLAY_LOOP,
}APP_BUZZER_STATUS_T;

typedef enum{
    APP_BUZZER_PLAY_INDEX_RINGTONE_0,
}APP_BUZZER_INDEX_T;


/**********************************************************************
 * GLOBAL FUNCTIONS
 */

/**
 * @brief      Clear parameters of app_buzzer
 * @param[in]  none
 * @return     none
 */
void app_buzzer_buffer_init(void);

/**
 * @brief      Get buzzer status
 * @param[in]  none
 * @return     unsigned char  0: Idle other: busy
 */
unsigned char app_buzzer_is_buzy(void);

/**
 * @brief      Get buzzer status
 * @param[in]  index        This parameter can be specified when the module provides multiple songs.
 * @param[in]  status       1:play the song only once  2:play the same song in loop  other:illegal value
 * @param[in]  sound_level  PWM duty value. The range is 0-100
 * @return     none
 */
void app_buzzer_play(APP_BUZZER_INDEX_T index, APP_BUZZER_STATUS_T status, unsigned char sound_level);

/**
 * @brief      This function is used to stop the buzzer
 * @param[in]  none
 * @return     none
 */
void app_buzzer_stop(void);

/**
 * @brief      This function is used to obtain the starting system tick of the buzzer's silence.
 * @param[in]  none
 * @return     unsigned int 0:Non-silence state other:tick of silence
 */
unsigned int app_buzzer_get_silence_tick(void);

/**
 * @brief      This function is used to clear the start tick of silence.
 * @param[in]  none
 * @return     none
 */
void app_buzzer_clear_silence_tick(void);

/**
 * @brief      This function is used to dynamically modify the PWM duty according to the battery voltage.
 * @param[in]  none
 * @return     unsigned char PWM duty set according to voltage
 */
unsigned char app_buzzer_set_and_get_pwm_duty_by_voltage(void);

/**
 * @brief      The buzzer task. It needs to be executed regularly when the buzzer is working,
 *             so this function needs to be placed in the main loop.
 * @param[in]  none
 * @return     none
 */
void app_buzzer_task(void);

#endif /* APP_BUZZER_H_ */
