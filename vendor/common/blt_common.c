/******************************************************************************
 * @file     blt_common.c
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
#include "blt_common.h"
#include "stack/ble/ble.h"



_attribute_data_retention_    u32 flash_sector_mac_address = CFG_ADR_MAC_512K_FLASH;            //default flash is 512k
_attribute_data_retention_    u32 flash_sector_calibration = CFG_ADR_CALIBRATION_512K_FLASH;    //default flash is 512k





void blc_readFlashSize_autoConfigCustomFlashSector(void)
{


    u8    flash_cap = ((get_flash_mid&0x00ff0000)>>16);
    unsigned char adc_vref_calib_value_rd[4] = {0};

    if(flash_cap == FLASH_SIZE_512K){
        flash_sector_mac_address = CFG_ADR_MAC_512K_FLASH;
        flash_sector_calibration = CFG_ADR_CALIBRATION_512K_FLASH;
    }
    else if(flash_cap == FLASH_SIZE_1M){
        flash_sector_mac_address = CFG_ADR_MAC_1M_FLASH;
        flash_sector_calibration = CFG_ADR_CALIBRATION_1M_FLASH;
    }
    else{
        //This SDK do not support flash size other than 512K/1M
        //If code stop here, please check your Flash
        while(1);
    }

    //load adc vref value from flash
    if(adc_vref_cfg.adc_calib_en)
    {
        flash_read_page(flash_sector_calibration+CALIB_OFFSET_ADC_VREF, 4, adc_vref_calib_value_rd);
        if((adc_vref_calib_value_rd[2] != 0xff) || (adc_vref_calib_value_rd[3]  != 0xff ))
        {
            /******Method of calculating calibration Flash_vbat_Vref value: ********/
            /******Vref = [1175 +First_Byte-255+Second_Byte] mV =  [920 + First_Byte + Second_Byte] mV  ********/
            adc_vref_cfg.adc_vref = 920 + adc_vref_calib_value_rd[2] + adc_vref_calib_value_rd[3];
        }
        //else use the value init in efuse
    }

    flash_set_capacity(flash_cap);
}




/*
 *Kite:     VVWWXX38C1A4YYZZ
 *Vulture:  VVWWXXD119C4YYZZ
 * public_mac:
 *                 Kite     : VVWWXX 38C1A4
 *                 Vulture : VVWWXX D119C4
 * random_static_mac: VVWWXXYYZZ C0
 */

void blc_initMacAddress(int flash_addr, u8 *mac_public, u8 *mac_random_static)
{
    if(flash_sector_mac_address == 0){
        return;
    }


    u8 mac_read[8];
    flash_read_page(flash_addr, 8, mac_read);

    u8 value_rand[5];
    generateRandomNum(5, value_rand);

    u8 ff_six_byte[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    if ( memcmp(mac_read, ff_six_byte, 6) ) {
        memcpy(mac_public, mac_read, 6);  //copy public address from flash
    }
    else{  //no public address on flash
        mac_public[0] = value_rand[0];
        mac_public[1] = value_rand[1];
        mac_public[2] = value_rand[2];

        #if(MCU_CORE_TYPE == MCU_CORE_825x)
            mac_public[3] = 0x38;             //company id: 0xA4C138
            mac_public[4] = 0xC1;
            mac_public[5] = 0xA4;
        #elif(MCU_CORE_TYPE == MCU_CORE_827x)
            mac_public[3] = 0xD1;             //company id: 0xC119D1
            mac_public[4] = 0x19;
            mac_public[5] = 0xC4;
        #endif


        flash_write_page (flash_addr, 6, mac_public);
    }





    mac_random_static[0] = mac_public[0];
    mac_random_static[1] = mac_public[1];
    mac_random_static[2] = mac_public[2];
    mac_random_static[5] = 0xC0;             //for random static

    u16 high_2_byte = (mac_read[6] | mac_read[7]<<8);
    if(high_2_byte != 0xFFFF){
        memcpy( (u8 *)(mac_random_static + 3), (u8 *)(mac_read + 6), 2);
    }
    else{
        mac_random_static[3] = value_rand[3];
        mac_random_static[4] = value_rand[4];

        flash_write_page (flash_addr + 6, 2, (u8 *)(mac_random_static + 3) );
    }
}

