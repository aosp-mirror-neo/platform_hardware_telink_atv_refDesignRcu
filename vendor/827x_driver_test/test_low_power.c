/******************************************************************************
 * @file     test_low_power.c
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

#include "tl_common.h"
#include "drivers.h"



#define TEST_LP_SUSPEND_NO_WAKEUP						1 //ext32k+suspend+none: 45.09uA
#define TEST_LP_SUSPEND_GPIO_PAD_WAKEUP					2
#define TEST_LP_SUSPEND_TIMER_WAKEUP					3

#define TEST_LP_SUSPEND_TIMER_GPIO_PAD_WAKEUP			4

#define TEST_LP_DEEPSLEEP_NO_WAKEUP						5 //ext32k+deep+none: 0.43uA    //int32k+deep+none: 0.41uA
#define TEST_LP_DEEPSLEEP_GPIO_PAD_WAKEUP				6 //ext32k+deep+pad:  0.43uA    //int32k+deep+pad:  0.41uA
#define TEST_LP_DEEPSLEEP_TIMER_WAKEUP					7 //ext32k+deep+tmr:  2.03uA    //int32k+deep+tmr:  0.92uA

#define TEST_LP_DEEPSLEEP_RET16K_NO_WAKEUP				8 //ext32k+deepret16k+none(tmr should open: 3.00uA    //int32k+deepret16k+none: 1.41uA
#define TEST_LP_DEEPSLEEP_RET16K_GPIO_PAD_WAKEUP		9 //ext32k+deepret16k+pad(tmr should open:  3.00uA    //int32k+deepret16k+pad:  1.41uA
#define TEST_LP_DEEPSLEEP_RET16K_TIMER_WAKEUP			10//ext32k+deepret16k+tmr:  3.00uA                    //int32k+deepret16k+tmr:  1.87uA


#define TEST_LP_MODE									5



int first_power_on = 0;

u8 AA_wakeupSrc[256];
u8 AA_wkp_index;

void test_low_power(void)
{
	if(analog_read(0x3a) == 0x5a){  //read flag
		first_power_on = 0;
	}
	else{
		first_power_on = 1;
	}


	if(first_power_on){
		DBG_CHN0_TOGGLE;
		sleep_us(500000);
		DBG_CHN0_TOGGLE;
		sleep_us(500000);
		DBG_CHN0_TOGGLE;
		sleep_us(500000);
		DBG_CHN0_TOGGLE;
		sleep_us(500000);
	}
	else{
		DBG_CHN0_TOGGLE;
		sleep_us(500000);
		DBG_CHN0_TOGGLE;
		sleep_us(500000);
		DBG_CHN0_TOGGLE;
		sleep_us(500000);
		DBG_CHN0_TOGGLE;
		sleep_us(500000);
	}

	analog_write(0x3a, 0x5a); //set flag



#if(TEST_LP_MODE == TEST_LP_SUSPEND_NO_WAKEUP)

	gpio_shutdown(GPIO_ALL);  //all GPIO high_Z state, no current leakage

	cpu_sleep_wakeup(SUSPEND_MODE , 0, 0);

#elif(TEST_LP_MODE == TEST_LP_SUSPEND_GPIO_PAD_WAKEUP)

	gpio_setup_up_down_resistor(GPIO_PB6, PM_PIN_PULLDOWN_100K);
	cpu_set_gpio_wakeup(GPIO_PB6, Level_High, 1);


	while(1){

		u32 wakeup_src = cpu_sleep_wakeup(SUSPEND_MODE, PM_WAKEUP_PAD, 0);  //PM_WAKEUP_PAD

		AA_wakeupSrc[AA_wkp_index++] = wakeup_src;

		DBG_CHN1_TOGGLE;
		sleep_us(300000);
		DBG_CHN1_TOGGLE;
		sleep_us(100000);
		DBG_CHN1_TOGGLE;
		sleep_us(600000);
		DBG_CHN1_TOGGLE;
		sleep_us(100000);
	}

#elif(TEST_LP_MODE == TEST_LP_SUSPEND_TIMER_WAKEUP)


	u32 now_tick;
	while(1){

		now_tick = clock_time();

		u32 wakeup_src = cpu_sleep_wakeup(SUSPEND_MODE, PM_WAKEUP_TIMER,  now_tick + 20 * CLOCK_16M_SYS_TIMER_CLK_1S);

		AA_wakeupSrc[AA_wkp_index++] = wakeup_src;

		DBG_CHN1_TOGGLE;
		sleep_us(20000);
		DBG_CHN1_TOGGLE;
		sleep_us(10000);
		DBG_CHN1_TOGGLE;
		sleep_us(60000);
		DBG_CHN1_TOGGLE;
		sleep_us(10000);
	}

#elif(TEST_LP_MODE == TEST_LP_SUSPEND_TIMER_GPIO_PAD_WAKEUP)

	gpio_setup_up_down_resistor(GPIO_PB6, PM_PIN_PULLDOWN_100K);
	cpu_set_gpio_wakeup(GPIO_PB6, Level_High, 1);


	u32 now_tick;
	while(1){

		now_tick = clock_time();

		u32 wakeup_src = cpu_sleep_wakeup(SUSPEND_MODE, PM_WAKEUP_TIMER | PM_WAKEUP_PAD,  now_tick + 20 * CLOCK_16M_SYS_TIMER_CLK_1S);

		AA_wakeupSrc[AA_wkp_index++] = wakeup_src;

		DBG_CHN1_TOGGLE;
		sleep_us(300000);
		DBG_CHN1_TOGGLE;
		sleep_us(100000);
		DBG_CHN1_TOGGLE;
		sleep_us(600000);
		DBG_CHN1_TOGGLE;
		sleep_us(100000);
	}

#elif(TEST_LP_MODE == TEST_LP_DEEPSLEEP_NO_WAKEUP)

	gpio_shutdown(GPIO_ALL); //all GPIO high_Z state, no current leakage

	cpu_sleep_wakeup(DEEPSLEEP_MODE , 0, 0);

#elif(TEST_LP_MODE == TEST_LP_DEEPSLEEP_GPIO_PAD_WAKEUP)


	gpio_setup_up_down_resistor(GPIO_PB6, PM_PIN_PULLDOWN_100K);
	cpu_set_gpio_wakeup(GPIO_PB6, Level_High, 1);

	while(1){
		cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);  //PM_WAKEUP_PAD
	}


#elif(TEST_LP_MODE == TEST_LP_DEEPSLEEP_TIMER_WAKEUP)

	gpio_shutdown(GPIO_ALL);  //all GPIO high_Z state, no current leakage

	while(1){

		cpu_sleep_wakeup(DEEPSLEEP_MODE , PM_WAKEUP_TIMER, clock_time() + 20*CLOCK_16M_SYS_TIMER_CLK_1S);

		DBG_CHN1_TOGGLE;
		sleep_us(700000);
		DBG_CHN1_TOGGLE;
		sleep_us(300000);
		DBG_CHN1_TOGGLE;
		sleep_us(700000);
		DBG_CHN1_TOGGLE;
		sleep_us(300000);
	}

#elif(TEST_LP_MODE == TEST_LP_DEEPSLEEP_RET16K_NO_WAKEUP)

	gpio_shutdown(GPIO_ALL);  //all GPIO high_Z state, no current leakage
	cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW16K , 0, 0);

#elif(TEST_LP_MODE == TEST_LP_DEEPSLEEP_RET16K_GPIO_PAD_WAKEUP)

	gpio_setup_up_down_resistor(GPIO_PB6, PM_PIN_PULLDOWN_100K);
	cpu_set_gpio_wakeup(GPIO_PB6, Level_High, 1);

	while(1){
		cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW16K, PM_WAKEUP_PAD, 0);  //PM_WAKEUP_PAD
	}

#elif(TEST_LP_MODE == TEST_LP_DEEPSLEEP_RET16K_TIMER_WAKEUP)

	gpio_shutdown(GPIO_ALL);  //all GPIO high_Z state, no current leakage

	while(1){

		cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW16K , PM_WAKEUP_TIMER, clock_time() + 20*CLOCK_16M_SYS_TIMER_CLK_1S);

		DBG_CHN1_TOGGLE;
		sleep_us(700000);
		DBG_CHN1_TOGGLE;
		sleep_us(300000);
		DBG_CHN1_TOGGLE;
		sleep_us(700000);
		DBG_CHN1_TOGGLE;
		sleep_us(300000);
	}
#else


#endif


}
