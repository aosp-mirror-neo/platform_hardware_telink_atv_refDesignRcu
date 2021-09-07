/******************************************************************************
 * @file     ble_host.h
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
#ifndef STACK_BLE_HOST_BLE_HOST_H_
#define STACK_BLE_HOST_BLE_HOST_H_

#include "stack/ble/ble_common.h"
#include "stack/ble/ble_format.h"

#include "stack/ble/host/l2cap/l2cap.h"


#include "stack/ble/host/attr/att.h"
#include "stack/ble/host/attr/gatt.h"


#include "stack/ble/host/smp/smp.h"
#include "stack/ble/host/smp/smp_alg.h"
#include "stack/ble/host/smp/smp_peripheral.h"
#if (MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
#include "stack/ble/host/smp/smp_central.h"
#endif
#include "stack/ble/host/smp/smp_storage.h"


#include "stack/ble/host/gap/gap.h"
#include "stack/ble/host/gap/gap_event.h"


/*********************************************************/
//Remove when file merge to SDK //
#include "stack/ble/ble_config.h"
#include "stack/ble/trace.h"

/*********************************************************/


#endif /* STACK_BLE_HOST_BLE_HOST_H_ */
