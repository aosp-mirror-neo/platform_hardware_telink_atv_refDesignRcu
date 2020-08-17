/******************************************************************************
 * @file     llms_conn.h
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

#ifndef LLMS_CONN_H_
#define LLMS_CONN_H_



/********************* Macro & Enumeration & Structure Definition for Stack, user can not use!!!!  **********************/

#define            BLMS_EARLY_CAL_MAP_TO_SAVE_TIME_EN                1   //when calculate new channel map in BRX/BTX start, 70uS is used

//////////////////////////////// LL ENC ///////////////////////////////////
#define            MS_LL_ENC_OFF                                    0
#define            MS_LL_ENC_REQ                                    1
#define            MS_LL_ENC_RSP_T                                  2
#define            MS_LL_ENC_START_REQ_T                            3
#define            MS_LL_ENC_START_RSP                              4
#define            MS_LL_ENC_START_RSP_T                            5
#define            MS_LL_ENC_PAUSE_REQ                              6
#define            MS_LL_ENC_PAUSE_RSP_T                            7
#define            MS_LL_ENC_PAUSE_RSP                              8
#define            MS_LL_REJECT_IND_T                               9
#define            MS_LL_ENC_SMP_INFO_S                             10
#define            MS_LL_ENC_SMP_INFO_E                             11

#define            MS_CONN_ENC_CHANGE                               BIT(16) //offset 2byte
#define            MS_CONN_ENC_REFRESH                              BIT(17)
#define            MS_CONN_ENC_REFRESH_T                            BIT(18)



#define            BLMS_CONN_HANDLE                                (BLM_CONN_HANDLE | BLS_CONN_HANDLE)



#define            CONN_STATUS_DISCONNECT                           0   //disconnect must be "0"
#define            CONN_STATUS_COMPLETE                             1
#define            CONN_STATUS_ESTABLISH                            2


#define         BLMS_CONN_RX_MAX_TRY_NUM                            4
#define         BLMS_CONN_UPDATE_RX_MAX_TRY_NUM                     4


#define         BLMS_TX_FIFO_NUM                                    8
#define         BLMS_STACK_USED_TX_FIFO_NUM                         1
#define         BLMS_TX_FIFO_SIZE                                   40

#define            CONN_UPDATE_CHN_MAP                              BIT(0)
#define            CONN_UPDATE_CONN_PARAM                           BIT(1)
#define            CONN_UPDATE_PHY_MODE                             BIT(2)

typedef struct {
    u8        save_flg;
    u8        sn_nesn;
    u8        dma_tx_rptr;
    u8        reg_core_f03;
}bb_ms_sts_t;

typedef struct {

    u8        tx_wptr;
    u8        tx_rptr;
    u8        tx_num;
    u8        conn_rx_num;  //rx number in a new interval   //TODO: all connection can share one

    u8        role;
    u8        ll_enc_busy;
    u8        connState;             // 0/Conn_Complete/Conn_Establish
    u8        conn_peer_sn;

    u8        conn_snnesn;
    u8        rsvd11;
    u8        rsvd22;
    u8        rsvd33;

    u8       conn_dma_tx_rptr;
    u8       smp_empty_pkt_entry; //slave-role empty pkt entry: ble main_loop entry enable flg
    u8       blt_tx_pkt_hold;
    u8       last_rf_len;

    u8        chn_idx;
    u8        conn_chn;
    u8        conn_para_update;
    u8        conn_map_update;

    u8        conn_receive_packet;
    u8        conn_established;
    u8        peer_adr_type;
    u8        conn_sca;

    u8         conn_receive_new_packet;
    u8         conn_peer_terminate;
    u8         conn_local_terminate;
    u8         conn_terminate_reason;


    u8        conn_chn_hop;
    u8        conn_winsize_next;        //u8 is enough
    u8        conn_interval_next;        //pay attention: we do not support too big interval, so u8  enough
    u8        conn_offset_next;        //pay attention: we do not support too big interval, so winOffset u8  enough



    u16        conn_inst_next;
    u16        conn_map_inst_next;        //update conn_param & update_map should separate, update map may delay to process due to BRX slot dropped
    u16        conn_latency_next;
    u16        conn_timeout_next;     //Note: unit 10mS



    //0x20, 32


    u16        updateReq_conn_min;
    u16        updateReq_conn_max;
    u16        updateReq_conn_latency;
    u16        updateReq_conn_timeout;

    u16        conn_nesn;//local nesn
    u16        conn_sn;  //local sn

    u16        connHandle;
    u16        conn_interval;
    u16        conn_latency;
    u16        conn_inst;
    u16        conn_inst_backup;
    u16        enc_ediv;       //EDIV

    //0x40, 64

    u32        conn_paraUpt_tick;
    u32        conn_terminate_tick;
    u32        conn_interval_tick;
    u32        conn_timeout;
    u32        conn_established_tick;

    u32        btxbrx_start_tick;
    u32        conn_tick;
    u32        conn_access_code;
    u32        conn_crc;

    //0x60, 96

    u32     conn_crc_revert; //revert crc24 init value
    u32     ll_remoteFeature;
    u32     conn_inst_u32;

    bb_ms_sts_t blms_bb;// BaseBand info: SN/NESN/TX_FIFO_RPTR

    u8        peer_adr[6];
    u8        conn_chn_map[5];
    u8        conn_chn_map_next[5];

    //0x80, 128

    u8        enc_random[8];  //RANDOM

    u32        enc_ivm;       //master IVm
    u8         enc_skdm[8];   //master SKDm
    u32        enc_ivs;       //slave  IVs
    u8         enc_skds[8];   //slave  SKDs

    //0xA0, 160

    u8        chn_tbl[40];
#if (BLMS_EARLY_CAL_MAP_TO_SAVE_TIME_EN)
    u8        chn_new_tbl[40];
#endif
    ble_crypt_para_t    crypt;   //40 Byte
    u8        rsvd16[16];


    //0x100, 256

    u32        tx_fifo[BLMS_TX_FIFO_NUM][BLMS_TX_FIFO_SIZE>>2];  // 320, 0x140

    // 0x240

} st_llms_conn_t;

//extern _attribute_aligned_(4)    st_llms_conn_t    blms[];

extern _attribute_aligned_(4)    st_llms_conn_t    AA_blms[];   // AA_ just for debug
#define blms    AA_blms


extern st_llms_conn_t  *blms_pconn;
extern int              blms_conn_sel;



typedef int (*blms_LTK_req_callback_t)(u16 handle, u8* rand, u16 ediv);
//extern blms_LTK_req_callback_t blt_llms_ltk_request;




/*********************************** Stack Interface Begin, user can not use!!! **********************************/
int         blms_connect_common(st_llms_conn_t *pc, rf_packet_connect_t * pInit);
int         blms_start_common  (st_llms_conn_t *pc);
int         blms_post_common   (st_llms_conn_t *pc);

bool         blms_ll_pushTxfifo (u16 connHandle, u8 *p);
ble_sts_t    blt_llms_unknown_rsp(u16 connHandle, u8 op_code );
int          blt_llms_main_loop_data (u16 connHandle, u8 *raw_pkt);


void         blt_llms_registerLtkReqEvtCb(blms_LTK_req_callback_t evtCbFunc);

/*********************************** Stack Interface End *********************************************************/














/************************************ User Interface  Begin  ******************************************************/
ble_sts_t    blc_llms_setMaxConnectionNumber(int max_master_num, int max_slave_num);
ble_sts_t    blc_llms_setMasterConnectionInterval(u16 conn_interval);   //This API master called after "blc_llms_setMaxConnectionNumber"


ble_sts_t    blc_llms_disconnect (u16 connHandle, u8 reason);

/************************************ User Interface  End  ********************************************************/

#endif /* LLMS_CONN_H_ */
