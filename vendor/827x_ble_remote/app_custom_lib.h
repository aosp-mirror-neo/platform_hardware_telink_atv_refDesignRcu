/******************************************************************************
 * @file     app_custom_lib.h
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

#ifndef APP_CUSTOM_LIB_H_
#define APP_CUSTOM_LIB_H_

#define GOOGLE_SPEC  1
#define MTK_SPEC     2

typedef struct{
    u8    rf_len;             //LEN(6)_RFU(2)
    u8    data[31];           //0-31 byte
}rf_packet_adv_ind_module_t;

extern void app_send_wakeup_packet(u8 keyid);
extern u32 app_custom_wakeup_tick(void);

//void app_woble_refresh_tick(void);
void app_woble_buffer_init(void);
void app_woboe_set_para(u8 key_id,u8 seq_no);
void app_woble_schedule_ota(u8 seq_no);
void app_woble_clear_para(void);
void app_woble_init_settings(u8 *buf,u8 adv_type);
u8 app_woble_send_task(void);
#endif
