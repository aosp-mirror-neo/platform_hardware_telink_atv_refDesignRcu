/******************************************************************************
 * @file     blt_led.h
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

#ifndef BLT_LED_H_
#define BLT_LED_H_

#include "tl_common.h"

#define APP_LED_GREEN          0
#define APP_LED_RED            1

#define APP_DEVICE_LED_COUNT   2


//led management
typedef struct{
    u8 led;
    u8 firston;
    u16 onTime_ms;
    u16 offTime_ms;
	u8 repeatCount; 
	u8  iskeepon;
} led_cfg_t;

typedef struct{
    u8 isEnable;
    u8 isOn;
    u8 onLevel;
    u8 isKeepOn;
    u16 count;
    u16 onTimer;
    u16 offTimer;
    u32 timer;
    u32 gpio;
}app_device_led_ctrl_t;

extern void led_proc(void);
extern void device_led_init(void);
extern int device_led_busy(void);
extern void device_led_setup(led_cfg_t led_cfg);
extern void device_led_off(u8 led);
extern void device_led_on(u8 led);
inline void device_led_process(void)
{
    led_proc();
}




#endif /* BLT_LED_H_ */
