/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef APP_BUZZER_SONG_H_
#define APP_BUZZER_SONG_H_

typedef enum{

    B_N_SILENCE   = 0,

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

    B_R_N0        = 1568,
    B_R_N1        = 2093,
    B_R_N2        = 2637,
    B_R_N3        = 3136,
    B_R_N4        = 4186,

}APP_BUZZER_NOTE_T;

typedef enum{

    B_NT_COMMON        = 400,
    B_NT_50            = 50,
    B_NT_130           = 130,
    B_NT_500           = 500,


}APP_BUZZER_NOTE_TIME_T;

typedef struct {
    unsigned int            note;        // musical notes. refer APP_BUZZER_NOTE_T
    unsigned short          duration;    // note duration
}app_buzzer_symbol_t;

const app_buzzer_symbol_t app_song_loop[] =
{
    {B_R_N0,    B_NT_50},
    {B_R_N1,    B_NT_50},
    {B_R_N2,    B_NT_50},
    {B_R_N3,    B_NT_50},
    {B_R_N4,    B_NT_130},
    {B_R_N0,    B_NT_50},
    {B_R_N1,    B_NT_50},
    {B_R_N2,    B_NT_50},
    {B_R_N3,    B_NT_50},
    {B_R_N4,    B_NT_130},
    {B_R_N0,    B_NT_50},
    {B_R_N1,    B_NT_50},
    {B_R_N2,    B_NT_50},
    {B_R_N3,    B_NT_50},
    {B_R_N4,    B_NT_130},
    {B_R_N0,    B_NT_50},
    {B_R_N1,    B_NT_50},
    {B_R_N2,    B_NT_50},
    {B_R_N3,    B_NT_50},
    {B_R_N4,    B_NT_130},

    /* 500ms silence, then repeat */
    {B_N_SILENCE,   B_NT_500},
};

#endif /* APP_BUZZER_SONG_H_ */
