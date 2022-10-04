/******************************************************************************
 * @file     app_buzzer.h
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

#ifndef APP_BUZZER_H_
#define APP_BUZZER_H_


#define NT                        (400)           // note time

#define BUZZER_GPIO               GPIO_PB4
#define BUZZER_CLOCK              CLOCK_SYS_CLOCK_HZ // 2000000

#define BUZZER_VOLUME_LOW         (3)             // 3%
#define BUZZER_VOLUME_MEDIUM      (8)             // 8%
#define BUZZER_VOLUME_HIGH        (15)            // 15%

typedef enum{
    /* Bass  */
    B_N_L1        = 262,
    B_N_L1S       = 277,    // S: #  b: flat
    B_N_L2        = 294,
    B_N_L2S       = 311,
    B_N_L3        = 330,
    B_N_L4        = 349,
    B_N_L4S       = 370,
    B_N_L5        = 392,
    B_N_L5S       = 415,
    B_N_L6        = 440,
    B_N_L6S       = 466,
    B_N_L7        = 494,

    /* Alto  */
    B_N_M1        = 523,
    B_N_M1S       = 554,
    B_N_M2        = 587,
    B_N_M2S       = 622,
    B_N_M3        = 659,
    B_N_M4        = 698,
    B_N_M4S       = 740,
    B_N_M5        = 784,
    B_N_M5S       = 831,
    B_N_M6        = 880,
    B_N_M6S       = 932,
    B_N_M7        = 988,

    /* Treble  */
    B_N_H1        = 1046,
    B_N_H1S       = 1109,
    B_N_H2        = 1175,
    B_N_H2S       = 1245,
    B_N_H3        = 1318,
    B_N_H4        = 1397,
    B_N_H4S       = 1480,
    B_N_H5        = 1568,
    B_N_H5S       = 1661,
    B_N_H6        = 1760,
    B_N_H6S       = 1865,
    B_N_H7        = 1976,

}APP_BUZZER_NOTE_T;

typedef enum{
    APP_BUZZER_STATUS_IDLE      = 0,
    APP_BUZZER_STATUS_FIND_ME,        // start by find me cmd
    APP_BUZZER_STATUS_POWER_ON,        // start by power on

}APP_BUZZER_STATUS_T;

typedef struct {
    APP_BUZZER_NOTE_T     note;        // musical notes. refer APP_BUZZER_NOTE_T
    unsigned short        duration;    // note duration
}app_buzzer_symbol_t;


typedef struct {
    APP_BUZZER_STATUS_T   status;
    unsigned char         sound_level;

    unsigned short        song_index;
    unsigned short        song_size;

    unsigned short        note_duration;
    unsigned int          note_timer_tick;
    unsigned int          song_start_tick;
}app_buzzer_ctrl_t;

void app_buzzer_buffer_init(void);
void app_buzzer_pwm_init(void);
void app_buzzer_play(APP_BUZZER_STATUS_T reason, unsigned char sound_level);
void app_buzzer_stop(void);
void app_buzzer_task(void);

unsigned char app_buzzer_is_buzy();

#endif /* APP_BUZZER_H_ */
