/******************************************************************************
 * @file     ll_master.h
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
#ifndef LL_MASTER_H_
#define LL_MASTER_H_



/******************************* User Interface  ************************************/
void        blc_ll_initMasterRoleSingleConn_module(void);
bool        blm_ll_isRfStateMachineBusy(void);

ble_sts_t    blm_ll_disconnect (u16 handle, u8 reason);
ble_sts_t    blm_ll_updateConnection (u16 connHandle,
                              u16 conn_min, u16 conn_max, u16 conn_latency, u16 timeout,
                              u16 ce_min, u16 ce_max );

ble_sts_t    blm_ll_setHostChannel (u16 handle, u8 * map);


rf_packet_l2cap_t *        blm_l2cap_packet_pack (u16 conn, u8 * raw_pkt);


#endif /* LL_MASTER_H_ */
