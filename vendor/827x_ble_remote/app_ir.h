/******************************************************************************
 * @file     app_ir.h
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

#ifndef APP_IR_H_
#define APP_IR_H_


typedef enum{
    ATV_IR_SERVICE_IDX      = IR_PS_H,
    ATV_IR_PROG_CONTROL_IDX = ATV_IR_SERVICE_IDX+2,
    ATV_IR_KEY_ID_IDX       = ATV_IR_PROG_CONTROL_IDX+2,
    ATV_IR_CODE_IDX         = ATV_IR_KEY_ID_IDX+2,
    ATV_IR_SUPPRESS_IDX     = ATV_IR_CODE_IDX+2,
    ATV_IR_KEY_EVENT_IDX    = ATV_IR_SUPPRESS_IDX+2,
    ATV_IR_KEY_EVENT_CCCD_IDX,
}ATV_IR_IDX;



typedef enum{
    KEY_IDX_VOLUP,
    KEY_IDX_VOLDN,
    KEY_IDX_MUTE,
    KEY_IDX_POWER,
    KEY_IDX_INPUT
}ATV_IR_KEY_IDX;

typedef void (*app_ir_programming_end_Cb_t)(void);
extern void ir_flash_set_flag(u8 flag);
extern void programming_key_set(u8 data);
extern u8 ir_key_is_suppress(u8 key_idx);
extern u8 ir_fallback_send_key_code(u8 button_idx, bool key_down);
extern u8 ir_key_is_suppress(u8 key_idx);
extern int ir_repeat_delay_release_time(void);
extern u8 is_programming_key_send(void);
extern int ir_type_read(void);
extern void ir_table_init(void);
extern void ir_flash_factory(void);
extern void ir_param_init(void);
extern void ir_flash_erase(void);
extern void ir_flash_eraseall(void);
extern void app_ir_programming_end_register(app_ir_programming_end_Cb_t cb);
#endif
