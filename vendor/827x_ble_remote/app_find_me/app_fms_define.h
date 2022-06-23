/******************************************************************************
 * @file     app_fms_define.h
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

#ifndef APP_FMS_DEFINE_H_
#define APP_FMS_DEFINE_H_

typedef enum{
    APP_FMS_MODE_ALL_DISABLE                        = 0,
    APP_FMS_MODE_ALERT                              = 0xE100,

}APP_FMS_MODE_T;

typedef enum{
    APP_FMS_ALERT_CMD_STOP                          = 0,
    APP_FMS_ALERT_CMD_LVL_LOW                       = 0xE101,
    APP_FMS_ALERT_CMD_LVL_MEDIUM                    = 0xE102,
    APP_FMS_ALERT_CMD_LVL_HIGH                      = 0xE103,

}APP_FMS_ALERT_CMD_T;


typedef enum{
    APP_FMS_ALERT_STATUS_STOP                       = 0,
    APP_FMS_ALERT_STATUS_LVL_LOW                    = 0xE181,
    APP_FMS_ALERT_STATUS_LVL_MEDIUM                 = 0xE182,
    APP_FMS_ALERT_STATUS_LVL_HIGH                   = 0xE183,

}APP_FMS_ALERT_STATUS_T;

typedef enum{
    APP_FMS_NOTIFY_DATA_TYPE_ALERT_STATUS           = 0xE180,

    APP_FMS_NOTIFY_DATA_TYPE_ERROR_CODE             = 0x0F00,

}APP_FMS_NOTIFY_DATA_TYPE_T;



typedef enum{
    APP_FMS_ERROR_CODE_NO_ERROR = 0,
    APP_FMS_ERROR_CODE_ALL_MODES_DISABLED           = 0x0F01,
    APP_FMS_ERROR_CODE_UNKNOWN_CMD                  = 0x0F02,
    APP_FMS_ERROR_CODE_ALERT_MODE_NOT_SUPPORTED     = 0x0F03,

}APP_FMS_ERROR_CODE_T;

typedef int (*app_fms_mode_callback_t)(u8 *buf, u8 len);

typedef struct {
    u8 en_periodic_wakeup;
    u8 reserved;
    u16 periodic_wakeup_interval;
}app_fms_ctrl_t;

#endif /* APP_FMS_DEFINE_H_ */
