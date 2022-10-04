/******************************************************************************
 * @file     app_fms.h
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

#ifndef APP_FMS_H_
#define APP_FMS_H_

#include "app_fms_define.h"
#include "app_imc.h"
#include "app_buzzer.h"
#include "stack/ble/ble.h"



/**********************************************************************
 * LOCAL MARCO
 */

#define reserve_buf4                    (u8 *)((u32)phy_test_2_wire_rx_from_uart + 3 + 0x840000)    // 0x3c
#define app_fms_buf                     (reserve_buf4)

//#define g_p_app_fms_ctrl_val          ((volatile u8 *)(app_fms_buf+0x20))
#define g_p_app_fms_data_val            ((volatile u8 *)(app_fms_buf+0x22))
#define g_p_app_fms_ctrl                ((volatile app_fms_ctrl_t *)(app_fms_buf+0x24))
//#define s_p_app_fms_mode_cb           ((app_fms_mode_callback_t)(app_fms_buf+0x18))
#define s_p_app_fms_flag_adv            ((volatile u8 *)(app_fms_buf+0x2c))
#define s_p_app_fms_adv_table_index     ((volatile u8 *)(app_fms_buf+0x2d))
#define s_p_app_fms_adv_count           ((volatile u16 *)(app_fms_buf+0x2e))


extern u8 g_p_app_fms_ctrl_val[2];


int app_fms_att_ctl_cb(void * p);
int app_fms_att_data_cb(void * p);


void app_fms_buffer_init(void);
void app_fms_init(void);

void app_fms_notify_data_type(APP_FMS_NOTIFY_DATA_TYPE_T data_type, APP_FMS_ERROR_CODE_T error_code);

void app_fms_try_connect(void);
void app_fms_delete_adv_task(void);
void app_fms_periodic_wake_up(void);

#endif /* APP_FMS_H_ */
