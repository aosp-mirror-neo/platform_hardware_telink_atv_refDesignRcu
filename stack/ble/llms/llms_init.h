/******************************************************************************
 * @file     llms_init.h
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

#ifndef LLMS_INIT_H_
#define LLMS_INIT_H_


//can not be "1", will conflict with " blms_create_connection & CONN_IDX_MASK"
#define            BLMS_CONNECTION_REQ                        BIT(15)
#define            BLMS_CONNECTION_REQ_PENDING                BIT(14)

#define          BLMS_WINSIZE                            7













typedef struct {
    u8    conn_policy;
    u8    conn_advType;

    u8    conn_mac[6];

}st_llms_init_t;

extern    _attribute_aligned_(4) st_llms_init_t  blmsInit;

extern    volatile    int blms_create_connection;
extern    u32 blms_timeout_connectDevice;
extern    u32    blms_tick_connectDevice;


extern    rf_packet_ll_init_t    pkt_llms_init;


/************************************ User Interface  ******************************************************/
ble_sts_t     blc_llms_createConnection( u16 scan_interval, u16 scan_window, init_fp_type_t initiator_filter_policy,
                                        u8 adr_type,       u8  *mac,        u8               own_adr_type,
                                       u16 conn_min,       u16 conn_max,    u16               conn_latency,                 u16 timeout,
                                       u16 ce_min,           u16 ce_max );

ble_sts_t     blc_llms_setCreateConnectionTimeout (u32 timeout_ms);


/*********************************** Stack Interface, user can not use!!! **********************************/
int  blt_llms_procInitPkt(u8 *raw_pkt);




#endif /* LLMS_INIT_H_ */
