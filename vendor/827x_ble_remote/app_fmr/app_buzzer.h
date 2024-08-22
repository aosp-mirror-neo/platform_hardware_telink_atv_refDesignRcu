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

typedef enum{
    APP_BUZZER_STATUS_IDLE      = 0,
    APP_BUZZER_STATUS_PLAY_ONCE,
    APP_BUZZER_STATUS_PLAY_LOOP,
}APP_BUZZER_STATUS_T;

typedef enum{
    APP_BUZZER_PLAY_INDEX_RINGTONE_0,
}APP_BUZZER_INDEX_T;

typedef struct {
    unsigned int     note;        // musical notes. refer APP_BUZZER_NOTE_T
    unsigned short        duration;    // note duration
}app_buzzer_symbol_t;


typedef struct {
    APP_BUZZER_STATUS_T   status;
    unsigned char         sound_level;

    unsigned char         song_index;
    unsigned char         song_note_index;
    unsigned short        song_size;

    unsigned short        note_duration;
    unsigned int          note_timer_tick;
    unsigned int          song_start_tick;
}app_buzzer_ctrl_t;


/**********************************************************************
 * GLOBAL FUNCTIONS
 */

void app_buzzer_buffer_init(void);
void app_buzzer_play(APP_BUZZER_INDEX_T index, APP_BUZZER_STATUS_T status, unsigned char sound_level);
void app_buzzer_stop(void);
void app_buzzer_task(void);


unsigned char app_buzzer_is_buzy();

#endif /* APP_BUZZER_H_ */
