/******************************************************************************
 * @file     app_imc.c
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
#include "../app_config.h"
#include "stack/ble/ble.h"

#include "app_fms.h"



#if FIND_ME_ENABLE
extern u8  device_in_connection_state;

/**********************************************************************
 * LOCAL MARCO
 */

/**********************************************************************
 * LOCAL TYPES
 */

/**********************************************************************
 * GLOBAL VARIABLES
 */

/**********************************************************************
 * LOCAL VARIABLES
 */



/**********************************************************************
 * LOCAL FUNCTIONS
 */


void app_imc_start_alert(APP_FMS_ALERT_CMD_T alert_cmd){

    switch (alert_cmd) {
        case APP_FMS_ALERT_CMD_LVL_LOW:
            printf("APP_FMS_ALERT_CMD_LVL_LOW \n");
            app_buzzer_play(APP_BUZZER_STATUS_FIND_ME,BUZZER_VOLUME_LOW);
            app_fms_notify_data_type(APP_FMS_NOTIFY_DATA_TYPE_ALERT_STATUS, APP_FMS_ALERT_STATUS_LVL_LOW);
            break;
        case APP_FMS_ALERT_CMD_LVL_MEDIUM:
            printf("APP_FMS_ALERT_CMD_LVL_MEDIUM \n");
            app_buzzer_play(APP_BUZZER_STATUS_FIND_ME,BUZZER_VOLUME_MEDIUM);
            app_fms_notify_data_type(APP_FMS_NOTIFY_DATA_TYPE_ALERT_STATUS, APP_FMS_ALERT_STATUS_LVL_MEDIUM);
            break;
        case APP_FMS_ALERT_CMD_LVL_HIGH:
            printf("APP_FMS_ALERT_CMD_LVL_HIGH \n");
            app_buzzer_play(APP_BUZZER_STATUS_FIND_ME,BUZZER_VOLUME_HIGH);
            app_fms_notify_data_type(APP_FMS_NOTIFY_DATA_TYPE_ALERT_STATUS, APP_FMS_ALERT_STATUS_LVL_HIGH);
            break;
        default:
            break;
    }
}

void app_imc_stop_alert(void){
    app_fms_notify_data_type(APP_FMS_NOTIFY_DATA_TYPE_ALERT_STATUS, APP_FMS_ALERT_STATUS_STOP);
    app_buzzer_stop();
}


int app_imc_alert_cb(u8 *buf, u8 len){

    u16 my_alert_cmd = (buf[0]<<8) | (buf[1]);

    switch (my_alert_cmd) {
        case APP_FMS_ALERT_CMD_STOP:
            printf("APP_FMS_ALERT_STOP \n");
            app_imc_stop_alert();
            break;
        case APP_FMS_ALERT_CMD_LVL_LOW:
        case APP_FMS_ALERT_CMD_LVL_MEDIUM:
        case APP_FMS_ALERT_CMD_LVL_HIGH:
            app_imc_start_alert(my_alert_cmd);
            break;
        default:
        {
            app_fms_notify_data_type(APP_FMS_NOTIFY_DATA_TYPE_ERROR_CODE, APP_FMS_ERROR_CODE_UNKNOWN_CMD);
        }
            break;
    }
    return 0;
}


#endif /* FIND_ME_ENABLE */
