/********************************************************************************************************
 * @file     battery_check.h 
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
 *******************************************************************************************************/

#ifndef BATTERY_CHECK_H_
#define BATTERY_CHECK_H_

#define MANNUAL_MODE_GET_ADC_SAMPLE_RESULT  0
#define VBAT_LEAKAGE_PROTECT_EN				1
#define VBAT_DEEP_THRES_MV				2000   // 2000 mV low battery alarm
#define VBAT_SUSPEND_THRES_MV			1800   // 1800 mV low battery alarm



void battery_set_detect_enable (int en);
int  battery_get_detect_enable (void);

#if(VBAT_LEAKAGE_PROTECT_EN)
_attribute_ram_code_ int app_battery_power_check(u16 threshold_deep_vol_mv, u16 threshold_suspend_vol_mv);
#else
int app_battery_power_check(u16 alram_vol_mv);
#endif


#endif /* APP_BATTDET_H_ */
