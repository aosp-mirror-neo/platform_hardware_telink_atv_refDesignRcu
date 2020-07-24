/******************************************************************************
 * @file     llms_adv.h
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

#ifndef LLMS_ADV_H_
#define LLMS_ADV_H_



/********************* Macro & Enumeration & Structure Definition for Stack, user can not use!!!!  **********************/

typedef struct {
    u8        adv_chn_mask;
    u8        adv_duration_en;
    u8        adv_type;
    u8         adv_filterPolicy;


    u16        advInt_min;
    u16     advInt_max;

    u32        adv_duration_us;
    u32        adv_begin_tick;

    u8        T_SCAN_RSP_INTVL;
    u8      own_addr_type;


}st_llms_adv_t;



extern _attribute_aligned_(4) st_llms_adv_t  blmsAdv;







/************************************ User Interface  ******************************************************/
void         blc_llms_initAdvertising_module(void);

ble_sts_t      blc_llms_setAdvData(u8 *data, u8 len);
ble_sts_t     blc_llms_setScanRspData(u8 *data, u8 len);
ble_sts_t   blc_llms_setAdvEnable(int adv_enable);
ble_sts_t   blc_llms_setAdvParam( u16 intervalMin,  u16 intervalMax,  adv_type_t advType,           own_addr_type_t ownAddrType,  \
                                  u8 peerAddrType, u8  *peerAddr,    u8            adv_channelMap,   adv_fp_type_t   advFilterPolicy);



void         blc_llms_setAdvCustomedChannel (u8 chn0, u8 chn1, u8 chn2);



/*********************************** Stack Interface, user can not use!!! **********************************/
int     blt_llms_send_adv(void);









#endif /* LLMS_ADV_H_ */
