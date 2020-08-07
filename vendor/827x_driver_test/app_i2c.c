/******************************************************************************
 * @file     app_i2c.c
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


extern void i2c_master_test_init(void);
extern void i2c_slave_test_init(void);
extern void	i2c_master_mainloop(void);
extern void	i2c_slave_mainloop(void);


void app_i2c_test_init(void)
{
	WaitMs(2000);  //leave enough time for SWS_reset when power on


#if(I2C_DEMO_SELECT == I2C_DEMO_MASTER)

	i2c_master_test_init();

#elif(I2C_DEMO_SELECT == I2C_DEMO_SLAVE)

	i2c_slave_test_init();

#endif
}



void app_i2c_test_start(void)
{
#if (I2C_DEMO_SELECT == I2C_DEMO_MASTER)   //master demo mainloop


	i2c_master_mainloop();


#elif(I2C_DEMO_SELECT == I2C_DEMO_SLAVE)  //slave demo mainloop

	i2c_slave_mainloop();

#endif
}
