/******************************************************************************
 * @file     app.c
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


extern void app_timer_test_init(void);

extern void app_i2c_test_init(void);
extern void app_i2c_test_start(void);

extern void app_spi_test_init(void);
extern void app_spi_test_start(void);

extern void app_uart_test_init(void);
extern void app_uart_test_start(void);

extern void app_adc_test_init(void);
extern void app_adc_test_start(void);

extern void app_pwm_test(void);

extern void app_gpio_irq_test_init(void);

extern void app_led_init(void);

extern void test_low_power(void);

extern void app_emi_init(void);
extern void app_rf_emi_test_start(void);

void user_init()
{

#if (DRIVER_TEST_MODE == TEST_HW_TIMER)

	app_timer_test_init();

#elif (DRIVER_TEST_MODE == TEST_GPIO_IRQ)

	app_gpio_irq_test_init();
	app_led_init();

#elif (DRIVER_TEST_MODE == TEST_UART)

	app_uart_test_init();

#elif (DRIVER_TEST_MODE == TEST_IIC)

	app_i2c_test_init();

#elif (DRIVER_TEST_MODE == TEST_SPI)

	app_spi_test_init();

#elif (DRIVER_TEST_MODE == TEST_ADC)

	app_adc_test_init();

#elif (DRIVER_TEST_MODE == TEST_PWM)

	app_pwm_test();

#elif (DRIVER_TEST_MODE == TEST_LOW_POWER)

	test_low_power();

#elif (DRIVER_TEST_MODE == TEST_RF_EMI)

	app_emi_init();

#else


#endif

}


/////////////////////////////////////////////////////////////////////
// main loop flow
/////////////////////////////////////////////////////////////////////
u32 tick_wakeup;
void main_loop (void)
{
#if (DRIVER_TEST_MODE == TEST_UART)

	app_uart_test_start();

#elif (DRIVER_TEST_MODE == TEST_IIC)

	app_i2c_test_start();

#elif (DRIVER_TEST_MODE == TEST_SPI)

	app_spi_test_start();

#elif (DRIVER_TEST_MODE == TEST_ADC)
	app_adc_test_start();

#elif (DRIVER_TEST_MODE == TEST_RF_EMI)

	app_rf_emi_test_start();

#else

#endif

}




