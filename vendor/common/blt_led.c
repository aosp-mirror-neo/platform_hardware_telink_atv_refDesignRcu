/******************************************************************************
 * @file     blt_led.c
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
/*
 * blt_led.c
 *
 *  Created on: 2016-1-29
 *      Author: Administrator
 */

#include "tl_common.h"
#include "drivers.h"
#include "../common/blt_led.h"



app_device_led_ctrl_t gAppDeviceLedCtrl[APP_DEVICE_LED_COUNT];

static void device_led_config(u8 led, u8 onLevel, u32 gpio)
{
    if(led >= APP_DEVICE_LED_COUNT) return;

    gpio_set_func(gpio,  AS_GPIO);
    gpio_set_input_en(gpio, 0);
    gpio_set_output_en(gpio, 1);

    gAppDeviceLedCtrl[led].isEnable = false;
    gAppDeviceLedCtrl[led].isOn = false;
    gAppDeviceLedCtrl[led].onLevel = onLevel;
    gAppDeviceLedCtrl[led].gpio = gpio;
    gpio_write(gAppDeviceLedCtrl[led].gpio, !gAppDeviceLedCtrl[led].onLevel);
}

void device_led_on(u8 led)
{
    if(led >= APP_DEVICE_LED_COUNT) return;
    gAppDeviceLedCtrl[led].isEnable = false;
    gAppDeviceLedCtrl[led].isOn = true;
    gpio_write(gAppDeviceLedCtrl[led].gpio, gAppDeviceLedCtrl[led].onLevel);
}

void device_led_off(u8 led)
{
    if(led >= APP_DEVICE_LED_COUNT) return;
    gAppDeviceLedCtrl[led].isEnable = false;
    gAppDeviceLedCtrl[led].isOn = false;
    gpio_write(gAppDeviceLedCtrl[led].gpio, !gAppDeviceLedCtrl[led].onLevel);
}

void device_led_init(void){  
	device_led_config(APP_LED_GREEN, 1,  GPIO_LED);
    device_led_config(APP_LED_RED, 1, GPIO_LED2);
}

int device_led_busy(void)
{
    u8 j=0;
    for(u8 i=0;i<APP_DEVICE_LED_COUNT;i++)
    {
        if(gAppDeviceLedCtrl[i].count != 0)
		    j++;
			
    }
    if(j) return 1;
	else  return 0;
}

void device_led_setup(led_cfg_t led_cfg)
{
    u8 led = led_cfg.led;

    if(led >= APP_DEVICE_LED_COUNT) return;
	device_led_off(led);
	if(led_cfg.firston)
	{
	    device_led_on(led);
	}
	if((led_cfg.repeatCount == 0) && !led_cfg.firston)
	{
	    if(led_cfg.offTime_ms == 0)
	    {
	        device_led_on(led);
	    }
	}
    gAppDeviceLedCtrl[led].isEnable = true;
    gAppDeviceLedCtrl[led].isKeepOn = led_cfg.iskeepon;
    gAppDeviceLedCtrl[led].count = led_cfg.repeatCount;
    gAppDeviceLedCtrl[led].onTimer = led_cfg.onTime_ms;
    gAppDeviceLedCtrl[led].offTimer = led_cfg.offTime_ms;
    gAppDeviceLedCtrl[led].timer = (clock_time()-led_cfg.onTime_ms) | 1;
}

void led_proc(void)
{
    int i;

    for(i=0; i<APP_DEVICE_LED_COUNT; i++){
        if(!gAppDeviceLedCtrl[i].isEnable) continue;
        if(gAppDeviceLedCtrl[i].isOn 
            && clock_time_exceed(gAppDeviceLedCtrl[i].timer, gAppDeviceLedCtrl[i].onTimer*1000)) {
            gAppDeviceLedCtrl[i].timer = clock_time() | 1;
            if(gAppDeviceLedCtrl[i].offTimer != 0){
                gAppDeviceLedCtrl[i].isOn = false;
                gpio_write(gAppDeviceLedCtrl[i].gpio, !gAppDeviceLedCtrl[i].onLevel);
            }
            if(gAppDeviceLedCtrl[i].count != 0){
                gAppDeviceLedCtrl[i].count--;
                if(gAppDeviceLedCtrl[i].count != 0) continue;
                if(!gAppDeviceLedCtrl[i].isKeepOn){
                    gpio_write(gAppDeviceLedCtrl[i].gpio, !gAppDeviceLedCtrl[i].onLevel);
                    gAppDeviceLedCtrl[i].isOn = false;
                }else{
                    gpio_write(gAppDeviceLedCtrl[i].gpio, gAppDeviceLedCtrl[i].onLevel);
                    gAppDeviceLedCtrl[i].isOn = true;
                }
                gAppDeviceLedCtrl[i].isEnable = false;
                return ;
            }
		}
        if(!gAppDeviceLedCtrl[i].isOn && gAppDeviceLedCtrl[i].onTimer != 0
            && clock_time_exceed(gAppDeviceLedCtrl[i].timer, gAppDeviceLedCtrl[i].offTimer*1000)){
            gAppDeviceLedCtrl[i].timer = clock_time() | 1;
            gAppDeviceLedCtrl[i].isOn = true;
            gpio_write(gAppDeviceLedCtrl[i].gpio, gAppDeviceLedCtrl[i].onLevel);
        }
    }
}



