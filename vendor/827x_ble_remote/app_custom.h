/******************************************************************************
 * @file     app_custom.h
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

#ifndef APP_CUSTOM_H_
#define APP_CUSTOM_H_

#define APP_CUSTOM_WAKEUPKEY_NUM 12

extern const unsigned char Kb_Map_Normal[8][3];
extern u8 nec_ir_addr1_table[48];
extern u8 nec_ir_addr2_table[48];
extern u8 nec_ir_cmd1_table[48];
extern u8 nec_ir_cmd2_table[48];
extern u8 wakeup_key1[APP_CUSTOM_WAKEUPKEY_NUM];
extern u8 wakeup_key2[APP_CUSTOM_WAKEUPKEY_NUM];
extern u8 en_google_wakeuppack;
extern u8 en_ble_adv;
extern u8 en_slave_rpa;
extern const unsigned char navi_key_index_g10[4];
extern const unsigned char navi_key_index_g20[4];

extern void app_custom_init(void);
extern u8 app_custom_get_drive_len(void);
extern u8 app_custom_get_device_type(void);
extern void app_custom_param_init(u8 device_layout);
extern u8 app_custom_is_enable_wakeup_key(void);
extern u8  app_custom_wakeupkey_packet_index(u8 keyid);
extern void app_custom_data_update_process(u8 *data,u16 len);
extern void app_custom_data_update_sector_erase_loop(void);
extern void app_custom_write_conditional(void);
#endif
