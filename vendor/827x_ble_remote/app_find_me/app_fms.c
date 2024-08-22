/******************************************************************************
 * @file     app_fms.c
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

extern own_addr_type_t   app_own_address_type;

/**********************************************************************
 * LOCAL MARCO
 */

//#define s_p_app_fms_ctrl     ((app_buzzer_ctrl_t *)(app_fms_buf+0x10))


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





int app_fms_att_ctl_cb(void * p){
    rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;

    u16 len = pw->l2cap-3;//pw->rf_len-7;
    array_printf(pw->dat,len);

    return 0;
}

int app_fms_att_data_cb(void * p){
    rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;

    u16 len = pw->l2cap-3;//pw->rf_len-7;
    array_printf(pw->dat,len);

    return 0;
}





