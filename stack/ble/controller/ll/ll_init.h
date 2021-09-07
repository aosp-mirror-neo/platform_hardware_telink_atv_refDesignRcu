/******************************************************************************
 * @file     ll_init.h
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
#ifndef LL_INIT_H_
#define LL_INIT_H_


extern int blm_create_connection;


/******************************* User Interface  ************************************/
void         blc_ll_initInitiating_module(void);


ble_sts_t     blc_ll_createConnection (u16 scan_interval, u16 scan_window, init_fp_type_t initiator_filter_policy,
                              u8 adr_type, u8 *mac, u8 own_adr_type,
                              u16 conn_min, u16 conn_max, u16 conn_latency, u16 timeout,
                              u16 ce_min, u16 ce_max );

ble_sts_t     blc_ll_createConnectionCancel (void);


ble_sts_t   blc_ll_setCreateConnectionTimeout (u32 timeout_ms);



#endif /* LL_INIT_H_ */
