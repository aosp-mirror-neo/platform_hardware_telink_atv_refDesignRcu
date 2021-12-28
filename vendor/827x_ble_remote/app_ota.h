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


#define CMD_REPORT_VERSION                    0xfe00
#define CMD_CONTROL                           0xfe01

#define OTA_PROTECT_VOLTAGE                   2400    //mv

#define CLIENT_REPORT_CMD                     0xfe


//report type
enum{
    TYPE_VERSION = 0,
    TYPE_SINATURE_DATA,
    TYPE_CONTROL_DATA,
    TYPE_OTA_RESULT,
};

// control data
enum{
    OTA_CTRL_PAUSE = 0,
    OTA_CTRL_CONTINUE,
};

typedef enum{
    IDLE_MODE = 0,
    OTA_VER_CHECK,    //Includes battery voltage detection
    OTA_AUTHENTICATION,
    OTA_PROCESS,
}ota_status_type_t;

enum{
     GOOGLE_OTA_SUCCESS = 0,      //success
     GOOGLE_OTA_PACKET_LOSS,      //lost one or more OTA PDU
     GOOGLE_OTA_DATA_CRC_ERR,     //data CRC err
     GOOGLE_OTA_WRITE_FLASH_ERR,  //write OTA data to flash ERR
     GOOGLE_OTA_DATA_UNCOMPLETE,  //lost last one or more OTA PDU
     GOOGLE_OTA_TIMEOUT,           //
     GOOGLE_OTA_FW_CHECK_ERR,
     GOOGLE_OTA_LOW_VOLTAGE,
     GOOGLE_OTA_START_KEY_ERR,    //8

     //app not
     GOOGLE_OTA_USER_TERMINATE,   //9
     GOOGLE_OTA_SIGNATURE_ERR,   //

};

void bls_ota_init_handle(u16 handle);

ota_status_type_t app_get_ota_status();

void app_ota_set_en_ecdsa(u8 en);

void bls_ota_set_random(u8 *random);
void bls_ota_get_random(u8 *random);
char app_ota_pause();
char app_ota_continue();

void app_ota_terminate(u8 reason);

int app_ota_write(void * p);
void app_ota_proc(void);

//extern u16 ota_ctl_flag;

#endif /* APP_OTA_H_ */
