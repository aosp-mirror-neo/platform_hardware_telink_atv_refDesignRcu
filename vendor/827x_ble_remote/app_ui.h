/******************************************************************************
 * @file     app_ui.h
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

#ifndef APP_UI_H_
#define APP_UI_H_

#include "app_config.h"

void app_ui_init_normal(void);
void app_ui_init_deepRetn(void);

void proc_keyboard (u8 e, u8 *p, int n);





#if (BLE_REMOTE_OTA_ENABLE)
    //void app_debug_ota_result(int result);
    void app_ota_ver_check(void);
    void app_ota_signature();
    void app_enter_ota_mode(void *data);
    void app_ota_result(int result);
    void app_schedule_ota(u8 *data);

    //void app_ota_decryption(u8 *data);
#endif
    int att_ccc_control(void* p);


extern u8   key_type;
extern int  key_not_released;
extern int  ir_not_released;
extern u8   user_key_mode;
extern u8   ir_hw_initialed;
extern u8   ota_is_working;
extern int  lowBatt_alarmFlag;

extern u8  app_mtu_size;

extern void app_pairing_led(void);
extern void app_pairing_error(void);
extern u8 app_repairing(void);
extern void app_release_hid_key(void);
extern void app_schedule_ota_enter_deep(void);
extern void app_schedule_ota_wakeup_host(void);
extern void app_schedule_ota_send_notify_to_continue_ota(void);
extern void app_ota_timeout(void);
extern int app_cachekey_send_timer(void);
extern u8 app_is_key_released(void);

#endif /* APP_UI_H_ */
