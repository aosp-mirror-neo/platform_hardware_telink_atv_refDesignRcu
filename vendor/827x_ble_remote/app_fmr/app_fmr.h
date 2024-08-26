/********************************************************************************************************
 * @file    app_fmr.h
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
#ifndef APP_FMR_H_
#define APP_FMR_H_


/**********************************************************************
 * GLOBAL DEFINES
 */

#define APP_FMR_HANDLE                          \
            APP_FMR_PS_H,                       \
            APP_FMR_CMD_OUT_CD_H,               \
            APP_FMR_CMD_OUT_DP_H,               \
            APP_FMR_CMD_OUT_CCC_H,


#define APP_FMR_SERVICE                                                                                                                             \
    {4,ATT_PERMISSIONS_READ, 2, 16,                         (u8*)(&my_primaryServiceUUID),  (u8*)(&app_fmr_service_uuid),   0},                     \
    {0,ATT_PERMISSIONS_READ, 2, sizeof(app_fmr_char_val),   (u8*)(&my_characterUUID),       (u8*)(app_fmr_char_val),        0},                     \
    {0,ATT_PERMISSIONS_RDWR, 16,sizeof(app_fmr_data),       (u8*)(&app_fmr_char_uuid),      (u8*)(app_fmr_data),            &app_fmr_write_cb, 0},  \
    {0,ATT_PERMISSIONS_RDWR, 2, sizeof(app_fmr_ccc),        (u8*)(&clientCharacterCfgUUID), (u8*)(&app_fmr_ccc),            0},                     \


/**********************************************************************
 * ATT SERVICE VARIABLES
 */

/* FMR UUID */
extern const unsigned char app_fmr_service_uuid[16];
extern const unsigned char app_fmr_char_uuid[16];

/* FMR  attribute values */
extern const unsigned char app_fmr_char_val[19];

/* FMR data */
extern volatile unsigned char app_fmr_data[3];
extern unsigned short app_fmr_ccc;


/**********************************************************************
 * GLOBAL FUNCTIONS
 */

/**
 * @brief This function is used to get whether FMR is enabled
 *
 * @param p the pointer of buffer
 * @return unsigned char 1:enable 0:disable
 */
unsigned char app_fmr_is_fmr_en(void);

/**
 * @brief       This function is used to set whether FMR is enabled or disabled
 *
 * @param       en 1:enable 0:disable
 * @return      none
 */

void app_fmr_set_fmr_en(unsigned char en);

/**
 * @brief       This function is the write callback of the FMR service. Registered in FMR ATT table.
 *
 * @param       p the pointer of buffer
 * @return      int Always returns 0
 */
int app_fmr_write_cb(void * p);

/**
 * @brief       This function is used to clear all parameters related to FMR
 * @param[in]   none
 * @return      none
 */
void app_fmr_parameter_init(void);

/**
 * @brief       This function is used to confirm whether FMR is busy
 * @param[in]   none
 * @return      unsigned char 1:busy 0:idle
 */
unsigned char app_fmr_is_busy(void);

/**
 * @brief       This function is almost equal to FMR stop. Only used after keystroke discovery.
 * @param[in]   none
 * @return      none
 */
void app_fmr_on_button_event(void);

/**
 * @brief      The FMR task. It needs to be executed regularly when the buzzer is working,
 *             so this function needs to be placed in the main loop.
 * @param[in]  none
 * @return     none
 */
void app_fmr_task(void);

void app_fmr_set_periodic_wakeup(unsigned char en, unsigned short interval);

unsigned char app_fmr_is_periodic_wakeup_en(void);


unsigned char app_fmr_is_adv_busy(void);

void app_fmr_try_connect(void);

void app_fmr_periodic_wake_up(void);

void app_fmr_delete_adv_task(void);

#endif /* APP_FMR_H_ */
