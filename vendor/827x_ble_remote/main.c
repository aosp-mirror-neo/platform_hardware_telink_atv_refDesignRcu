/******************************************************************************
 * @file     main.c
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
#include "stack/ble/ble.h"
#include "vendor/common/blt_common.h"
#include "vendor/common/blt_fw_sign.h"
#include "vendor/common/flash_fw_check.h"
#include "../../vendor/common/user_config.h"
#if (MP_TEST_MODE)
#include "../common/blt_led.h"

#include "app_flash_write.h"
#include "app_test.h"
#endif
#include "app_custom.h"


extern void user_init_normal();
extern void user_init_deepRetn();

extern void main_loop (void);

extern void deep_wakeup_proc(void);

extern void rc_ir_irq_prc(void);
extern _attribute_ram_code_ void user_init_battery_power_check(void);
_attribute_ram_code_ void irq_handler(void)
{
#if (REMOTE_IR_ENABLE)
    rc_ir_irq_prc();
#endif

    irq_blt_sdk_handler ();

#if (BLE_PHYTEST_MODE != PHYTEST_MODE_DISABLE)
    extern void irq_phyTest_handler(void);
    irq_phyTest_handler();
#endif

}

_attribute_ram_code_ int main (void)    //must run in ramcode
{

    DBG_CHN0_LOW;   //debug

    blc_pm_select_internal_32k_crystal();

#if(MCU_CORE_TYPE == MCU_CORE_8258)
    cpu_wakeup_init();
#elif(MCU_CORE_TYPE == MCU_CORE_8278)
    cpu_wakeup_init(LDO_MODE,EXTERNAL_XTAL_24M);
#endif
    int deepRetWakeUp = pm_is_MCU_deepRetentionWakeup();  //MCU deep retention wakeUp

    rf_drv_init(RF_MODE_BLE_1M);
   // blt_extended_reject_enable(0);
    gpio_init( !deepRetWakeUp );  //analog resistance will keep available in deepSleep mode, so no need initialize again

#if (CLOCK_SYS_CLOCK_HZ == 16000000)
    clock_init(SYS_CLK_16M_Crystal);
#elif (CLOCK_SYS_CLOCK_HZ == 24000000)
    clock_init(SYS_CLK_24M_Crystal);
#elif (CLOCK_SYS_CLOCK_HZ == 32000000)
    clock_init(SYS_CLK_32M_Crystal);
#elif (CLOCK_SYS_CLOCK_HZ == 48000000)
    clock_init(SYS_CLK_48M_Crystal);
#endif

#if (UART_PRINT_ENABLE)
    extern void app_uart_tx(void);
    app_uart_tx();
#endif

#if (MP_TEST_MODE)
    GPIO_PB2_PULL_DOWN_100K;
    if (gpio_read(DUT_INPUT_PORT) && test_get_mode() == MODE_TEST)
    {
        extern void app_phytest_init(void);
        app_phytest_init();

        extern void app_trigger_phytest_mode(void);
        app_trigger_phytest_mode();
    }
#endif

    if(!deepRetWakeUp){//read flash size
        //app_custom_test();
        app_custom_init();
        user_init_battery_power_check(); //battery check must do before flash code
        blc_readFlashSize_autoConfigCustomFlashSector();
        #if FIRMWARE_CHECK_ENABLE
            //Execution time is in ms.such as:48k fw,16M crystal clock,need about 290ms.
            if(flash_fw_check(0xffffffff)){ //return 0, flash fw crc check ok. return 1, flash fw crc check fail
                while(1);                    //Users can process according to the actual application.
            }
        #endif
    }

    blc_app_loadCustomizedParameters();  //load customized freq_offset cap value

    set_disBrxRequest_thresold(2);

    if( deepRetWakeUp ){
        //printf("user_init_deepRetn\n");
        user_init_deepRetn ();
    }
    else{
        extern u8 wakeup_src;  //1:wakeup for pad  2:wake up for timer
        extern u8 app_wakeup_src(void);

        wakeup_src = app_wakeup_src();
        printf("wakeup_src=%x\r\n",wakeup_src);
        #if FIRMWARES_SIGNATURE_ENABLE
            blt_firmware_signature_check();
        #endif
        printf("user_init_normal\n");

        user_init_normal ();
    }

    irq_enable();
   
#if (MP_TEST_MODE)
    if (test_get_mode() == MODE_TEST) test_entry_mode();
#endif

    while (1) {
#if (MODULE_WATCHDOG_ENABLE)
        wd_clear(); //clear watch dog
#endif
        main_loop ();
    }

}

