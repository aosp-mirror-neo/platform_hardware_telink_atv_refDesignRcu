/******************************************************************************
 * @file     ble.h
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

#ifndef BLE_H_
#define BLE_H_


#include "blt_config.h"
#include "ble_common.h"
#include "ble_format.h"

#include "l2cap/l2cap.h"
#include "phy/phy.h"
#include "phy/phy_test.h"


#include "attr/att.h"
#include "attr/gatt.h"
#include "attr/gatt_uuid.h"


#include "smp/smp.h"
#include "smp/smp_const.h"
#include "smp/smp_central.h"
#include "smp/smp_peripheral.h"
#include "smp/smp_storage.h"

#include "gap/gap.h"
#include "gap/gap_event.h"

#include "crypt/aes_ccm.h"
#include "crypt/le_crypto.h"
#include "crypt/aes/aes_att.h"

#include "hci/hci.h"
#include "hci/hci_const.h"
#include "hci/hci_event.h"
#include "hci/usb_desc.h"

#include "service/ble_ll_ota.h"
#include "service/device_information.h"
#include "service/hids.h"

#include "ll/ll.h"
#include "ll/ll_adv.h"
#include "ll/ll_encrypt.h"
#include "ll/ll_init.h"
#include "ll/ll_pm.h"
#include "ll/ll_scan.h"
#include "ll/ll_whitelist.h"
#include "ll/ll_conn/ll_conn.h"
#include "ll/ll_conn/ll_slave.h"
#include "ll/ll_conn/ll_master.h"
#include "ll/ll_conn/ll_conn_phy.h"
#include "ll/ll_conn/ll_conn_csa.h"

#include "ll/ll_ext.h"
#include "ll/ll_ext_adv.h"
#include "ll/ll_ext_scan.h"


#include "bqb/bqb_ll.h"





volatile void  smemset(register char * dest,register int val,register unsigned int len);
volatile void * smemcpy(register char * out, register char * in, register unsigned int len);



#endif /* BLE_H_ */
