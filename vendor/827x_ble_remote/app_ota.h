/******************************************************************************
 * @file     app_ota.h
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

#ifndef APP_OTA_H_
#define APP_OTA_H_

//slave to master
#define CMD_REPORT_VERSION                    0xfe00
#define CMD_CONTROL                           0xfe01
#define CMD_OTA_RESULT                        0xfe02

#define OTA_PROTECT_VOLTAGE                   2300    //mv


#define SLAVE_REPORT_CMD                      0xfe

//report type
enum{
    TYPE_VERSION = 0,
     TYPE_SINATURE_DATA,
     TYPE_CONTROL_DATA,
     TYPE_OTA_RESULT,
};

// control data
enum{
     OTA_PAUSE = 0,
     OTA_CONTINUE,
};


typedef enum{
     FW_VER_CHECK     = BIT(0),    //just send version to Master
     BATT_CHECK_OK    = BIT(1),
     OTA_SIGNATURE    = BIT(2),
     OTA_START        = BIT(3),
}OTA_CtlFlagDef;

typedef void (*ota_signatureCb_t)(void);
typedef void (*ota_FWdecryptionCb_t)(void *data);
typedef void (*ota_newStartCb_t)(void *data);

void google_ota_pause();
void bls_ota_registerotaSignatureCmdCb(ota_signatureCb_t cb);
void bls_ota_registerotaFWdecryptionCb(ota_FWdecryptionCb_t cb);
void bls_ota_registerotaNewStartCb(ota_newStartCb_t cb);
void google_ota_continue();
void google_ota_result(int result);
int google_ota_write(void * p);

extern u8  ota_ctl_flag;

#endif /* APP_OTA_H_ */
