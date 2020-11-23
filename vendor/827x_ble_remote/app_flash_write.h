/******************************************************************************
 * @file     app_info.h
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

#ifndef APP_INFO_H_
#define APP_INFO_H_

///////////////////////
#define FLASH_FLAG                   0xFFA5


#define IR_FLASH_LENGTH              0x500  //(256*5)

//ccc data flag
#define FLAG_GOOGLE_RX_CCC           BIT(0)
#define FLAG_GOOGLE_CTL_CCC          BIT(1)

extern u8 flag_ccc_data;
extern u8 peer_mac[6];
void write_ccc_info(u8 *ccc);
u8 read_ccc_info(u8 *ccc);
void init_ccc_value();

u8 flash_write_info(unsigned long addr, u8 used_sector, u8 used_block, unsigned long len, u8 *wbuf );
u8 flash_read_info(unsigned long addr, u8 used_sector, u8 used_block, unsigned long len, u8 *rbuf);
void flash_check_area(unsigned long addr, u8 used_sector, u8 used_block, unsigned long len);
extern void init_peer_mac(void);
extern u8 is_peer_mac_flash_info_full(void);
extern void write_peer_mac_info(u8 *mac);
extern void write_wakeup_keyindex_info(u8 index);
extern u8 read_wakeup_keyindex_info(u8 *index);
extern u8 is_wakeup_keyindex_flash_info_full(void);
extern void init_wakeup_keyindex(void);
extern u8 init_ir_key_event_notify(void);
extern u8 is_ir_key_event_notify_flash_info_full(void);
extern void write_ir_key_event_notify(u8 index);

#endif /* APP_INFO_H_ */
