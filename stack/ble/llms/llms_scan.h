/******************************************************************************
 * @file     llms_scan.h
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

#ifndef LLMS_SCAN_H_
#define LLMS_SCAN_H_


/********************* Macro & Enumeration & Structure Definition for Stack, user can not use!!!!  **********************/

#define            BLMS_SCAN_SLOT                                      32  //32*0.625 = 20mS
#define            BLMS_SCAN_INTERVL                                ( BLMS_SCAN_SLOT * CLOCK_16M_SYS_TIMER_CLK_625US)


typedef struct {
    u8        scan_type;
    u8        scan_filterPolicy;
    u8        filter_dup;

    u8        scanDevice_num;
    u8        scanRspDevice_num;
    u8        scan_extension_mask;
    u8         rsvd;
//    s8        T_SCAN_REQ_INTVL;



    //u32        scan_interval;
}st_llms_scan_t;

extern    _attribute_aligned_(4) st_llms_scan_t  blmsScn;





/************************************ User Interface  ******************************************************/
void         blc_llms_initScanning_module(void);

ble_sts_t     blc_llms_setScanParameter (scan_type_t scan_type, u16 scan_interval, u16 scan_window, own_addr_type_t  ownAddrType, scan_fp_type_t scanFilter_policy);
ble_sts_t     blc_llms_setScanEnable (scan_en_t scan_enable, dupFilter_en_t filter_duplicate);



/*********************************** Stack Interface, user can not use!!! **********************************/

void     blt_llms_switchScanChannel (int set_chn);
int      blt_llms_procScanPkt(u8 *raw_pkt, u8 *new_pkt, u32 tick_now);
int     blt_llms_procScanData(u8 *raw_pkt);

#endif /* LLMS_SCAN_H_ */
