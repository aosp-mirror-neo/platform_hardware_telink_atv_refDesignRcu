/******************************************************************************
 * @file     app_adc.c
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


#if (DRIVER_TEST_MODE == TEST_ADC)


#define  TEST_ADC_GPIO						1   //test voltage come from adc gpio
#define  TEST_ADC_VBAT						2   //test power supply


#define  TEST_ADC_SELECT					TEST_ADC_GPIO







void adc_gpio_ain_init(void)
{
	//set misc channel en,  and adc state machine state cnt 2( "set" stage and "capture" state for misc channel)
	adc_set_chn_enable_and_max_state_cnt(ADC_MISC_CHN, 2);  	//set total length for sampling state machine and channel

	//set "capture state" length for misc channel: 240
	//set "set state" length for misc channel: 10
	//adc state machine  period  = 24M/250 = 96K, T = 10.4 uS
	#if (MCU_CORE_TYPE == MCU_CORE_8278)
		adc_set_state_length(240,10);  	//set R_max_mc,R_max_c,R_max_s
	#else
		adc_set_state_length(240, 0, 10); 
	#endif

	//set misc channel use differential_mode (telink advice: only differential mode is available)
	//single mode adc source, PB4 for example: PB4 positive channel, GND negative channel
	gpio_set_func(GPIO_PB4, AS_GPIO);
	gpio_set_input_en(GPIO_PB4, 0);
	gpio_set_output_en(GPIO_PB4, 0);
	gpio_write(GPIO_PB4, 0);

	adc_set_ain_channel_differential_mode(B4P, GND);


	//set misc channel resolution 14 bit
	//notice that: in differential_mode MSB is sign bit, rest are data,  here BIT(13) is sign bit
	adc_set_resolution(RES14);  //set resolution


	//set misc channel vref 1.2V
	adc_set_ref_voltage(ADC_VREF_1P2V);  					//set channel Vref



	//set misc t_sample 6 cycle of adc clock:  6 * 1/4M
	adc_set_tsample_cycle(SAMPLING_CYCLES_6);  	//Number of ADC clock cycles in sampling phase

	//set Analog input pre-scaling 1/8
	adc_set_ain_pre_scaler(ADC_PRESCALER_1F8);
}




void adc_vbat_detect_init(void)
{


	//set misc channel en,  and adc state machine state cnt 2( "set" stage and "capture" state for misc channel)
	adc_set_chn_enable_and_max_state_cnt(ADC_MISC_CHN, 2);  	//set total length for sampling state machine and channel


	//set "capture state" length for misc channel: 240
	//set "set state" length for misc channel: 10
	//adc state machine  period  = 24M/250 = 96K, T = 10.4 uS
	#if (MCU_CORE_TYPE == MCU_CORE_8278)
		adc_set_state_length(240,10);  	//set R_max_mc,R_max_c,R_max_s
	#else
		adc_set_state_length(240, 0, 10);
	#endif

	//telink advice: you must choose one gpio with adc function to output high level(voltage will equal to vbat), then use adc to measure high level voltage
	gpio_set_func(GPIO_PB0, AS_GPIO);
	gpio_set_input_en(GPIO_PB0, 0);
	gpio_set_output_en(GPIO_PB0, 1);
	gpio_write(GPIO_PB0, 1);

	//set misc channel use differential_mode(telink advice: only differential mode is available)
	adc_set_ain_channel_differential_mode(B0P, GND);

	//set misc channel resolution 14 bit
	//notice that: in differential_mode MSB is sign bit, rest are data,  here BIT(13) is sign bit
	adc_set_resolution(RES14);   //set resolution



	//set misc channel vref 1.2V
	adc_set_ref_voltage(ADC_VREF_1P2V);  					//set channel Vref



	//set misc t_sample 6 cycle of adc clock:  6 * 1/4M
	adc_set_tsample_cycle(SAMPLING_CYCLES_6);  	//Number of ADC clock cycles in sampling phase

	//set Analog input pre-scaling 1/8
	adc_set_ain_pre_scaler(ADC_PRESCALER_1F8);


}


void app_adc_test_init(void)
{
////Step 1: power off sar adc/////////////////////////////////////////////////////////
	/******power off sar adc********/
	adc_power_on_sar_adc(0);
//////////////////////////////////////////////////////////////////////////////////////





////Step 2: Config some common adc settings(user can not change these)/////////////////
	/******enable signal of 24M clock to sar adc********/
	adc_enable_clk_24m_to_sar_adc(1);

	/******set adc sample clk as 4MHz******/
	adc_set_sample_clk(5); //adc sample clk= 24M/(1+5)=4M

	/******set adc L R channel Gain Stage bias current trimming******/
	#if (MCU_CORE_TYPE == MCU_CORE_8258)
		adc_set_left_gain_bias(GAIN_STAGE_BIAS_PER100);
		adc_set_right_gain_bias(GAIN_STAGE_BIAS_PER100);
	#endif	
////////////////////////////////////////////////////////////////////////////////////////





////Step 3: Config adc settings  as needed /////////////////////////////////////////////
#if (TEST_ADC_SELECT == TEST_ADC_GPIO)
	adc_gpio_ain_init();

#elif (TEST_ADC_SELECT == TEST_ADC_VBAT)

	adc_vbat_detect_init();

#endif
////////////////////////////////////////////////////////////////////////////////////////



////Step 4: power on sar adc/////////////////////////////////////////////////////////
	/******power on sar adc********/
	adc_power_on_sar_adc(1);
////////////////////////////////////////////////////////////////////////////////////////
}





u16 Adc_cur_rawData;   //unit: m V
u16 Adc_raw_data[256];
u8  Adc_raw_datIndex = 0;



//just for display, fake data
u32 Adc_cur_vol_oct; //debug
u16 Adc_cal_vol_oct[256];
u8  Adc_cal_vol_octIndex= 0;



u32 tick_adc_sample = 0;
void app_adc_test_start(void)
{



	if(clock_time_exceed(tick_adc_sample, 200000)){
		tick_adc_sample = clock_time();


		Adc_cur_rawData =  adc_sample_and_get_result();

		Adc_raw_data[Adc_raw_datIndex ++]  = Adc_cur_rawData;


#if 1 //debug
		Adc_cur_vol_oct =   (Adc_cur_rawData/1000)<<12 | ((Adc_cur_rawData/100)%10)<<8 \
				                      | ((Adc_cur_rawData%100)/10)<<4  | (Adc_cur_rawData%10);
		Adc_cal_vol_oct[Adc_cal_vol_octIndex ++] = Adc_cur_vol_oct;
#endif

	}
}




#endif   //end of DRIVER_TEST_MODE == TEST_ADC
