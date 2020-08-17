/******************************************************************************
 * @file     blt_fw_signature.c
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
#include "blt_fw_sign.h"
#include "stack/ble/blt_config.h"
#include "blt_common.h"
#include "proj_lib/firmware_encrypt.h"

#if FIRMWARES_SIGNATURE_ENABLE
void blt_firmware_signature_check(void)
{
        unsigned int flash_mid;
        unsigned char flash_uid[16];
        unsigned char signature_enc_key[16];
        int flag = flash_read_mid_uid_with_check(&flash_mid, flash_uid);

        if(flag==0){  //reading flash UID error
            while(1);
        }

        firmware_encrypt_based_on_uid (flash_uid, signature_enc_key);

        if(memcmp(signature_enc_key, (u8*)(flash_sector_calibration + CALIB_OFFSET_FIRMWARE_SIGNKEY), 16)){  //signature not match
            while(1);   //user can change the code here to stop firmware running
        }
}
#endif
