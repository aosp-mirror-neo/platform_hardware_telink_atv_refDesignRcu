/******************************************************************************
 * @file     ble_controller.h
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
#ifndef BLE_CONTROLLER_H_
#define BLE_CONTROLLER_H_


#include "stack/ble/ble_common.h"
#include "stack/ble/ble_format.h"


#include "stack/ble/hci/hci.h"
#include "stack/ble/hci/hci_const.h"
#include "stack/ble/hci/hci_cmd.h"
#include "stack/ble/hci/hci_event.h"


#include "stack/ble/controller/ll/ll.h"
#include "stack/ble/controller/ll/ll_adv.h"
#include "stack/ble/controller/ll/ll_pm.h"
#include "stack/ble/controller/ll/ll_scan.h"
#include "stack/ble/controller/ll/ll_whitelist.h"
#include "stack/ble/controller/ll/ll_resolvlist.h"
#include "stack/ble/controller/ll/ll_conn/ll_conn.h"
#include "stack/ble/controller/ll/ll_conn/ll_slave.h"
#include "stack/ble/controller/ll/ll_conn/ll_conn_csa.h"

#if(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
#include "stack/ble/controller/ll/ll_init.h"
#include "stack/ble/controller/ll/ll_conn/ll_master.h"
#endif

#include "stack/ble/controller/ll/ll_ext_adv.h"


#include "phy/phy.h"
#include "phy/phy_test.h"


#include "algorithm/ecc/ecc_ll.h"
#include "algorithm/aes_ccm/aes_ccm.h"


/*********************************************************/
//Remove when file merge to SDK //
#include "stack/ble/ble_config.h"
#include "stack/ble/trace.h"

/*********************************************************/

#endif /* BLE_H_ */
