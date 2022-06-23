/********************************************************************************************************
 * @file     app_test.h
 *
 * @brief    Test mode for TLSR chips
 *
 * @author   webadmin@ohsunghq.co.kr;
 * @date     Aug. 20, 2020
 *
 * @attention
 *
 *  Copyright (C) 2020 Ohsung Electronics Co., Ltd.
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
 *******************************************************************************************************/

#ifndef APP_TEST_H_
#define APP_TEST_H_


typedef struct
{
    u8 device_type;
    u8 pnp_id[7];
    u8 device_name_length;
    u8 device_name[29];
    u8 device_ui_layout;
    u8 wakeup_key_1[2];
    u8 wakeup_key_2[2];
    u8 wakeup_packet[35];
} _tCustom_Data;

extern _tCustom_Data _custom_data;

#if (MP_TEST_MODE)


typedef enum
{
    TEST_STAT_ROOT,
    TEST_STAT_IR_KEY,
    TEST_STAT_CURRENT,
    TEST_STAT_LED,
    TEST_STAT_USERMODE,
} tTest_State;

void test_entry_mode(void);
tTest_State test_get_state(void);
u8 test_get_mode(void);
void test_set_mode(u8 mode);
void test_proc_key(void);
int test_write_cb(void *p);
#endif /// MP_TEST_MODE


#endif /* APP_TEST_H_ */

