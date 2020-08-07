/******************************************************************************
 * @file     app_spi.c
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




extern void spi_master_test_init(void);
extern void spi_slave_test_init(void);
extern void spi_master_mainloop(void);




void app_spi_test_init(void)
{
	WaitMs(2000);  //leave enough time for SWS_reset when power on
	//SPI:CK/CN/DO/DI   A4/D6/A2/A3, D7/D2/B7/B6
	#if (SPI_MODE==SPI_MASTER_MODE)
	 spi_master_test_init();

	#else
	 spi_slave_test_init();
	#endif
}






void app_spi_test_start(void)
{
#if (SPI_MODE==SPI_MASTER_MODE)

	spi_master_mainloop();

#else
		WaitMs(50);
#endif
}


void app_spi_test_irq_proc(void)
{

}
