/******************************************************************************
 * @file     llms.h
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

#ifndef LLMS_H_
#define LLMS_H_

#include "llms_slot.h"


//TODO:  all debug MACRO should remove at last
#define         DBG_SLOT_CALCULTE_TIMING_EN                        1
#define         DBG_DECRYPTION_ERR_EN                              1




#define            CONNECT_COMPLETE_CALLBACK_IN_MAINLOOP            1

/********************* Macro & Enumeration & Structure Definition for Stack, user can not use!!!!  **********************/
#define            BLMS_STATE_NONE                                 0
#define            BLMS_STATE_ADV                                  BIT(1)
#define            BLMS_STATE_SCAN                                 BIT(2)
#define            BLMS_STATE_SCAN_POST                            BIT(3)
#define            BLMS_STATE_BTX_S                                BIT(4)
#define            BLMS_STATE_BTX_E                                BIT(5)
#define            BLMS_STATE_BRX_S                                BIT(6)
#define            BLMS_STATE_BRX_E                                BIT(7)




#define         BLMS_FLG_RF_CONN_DONE                              (FLD_RF_IRQ_CMD_DONE  | FLD_RF_IRQ_FIRST_TIMEOUT | FLD_RF_IRQ_RX_TIMEOUT | FLD_RF_IRQ_RX_CRC_2)

//TODO: delete all debug codes after functions are tested OK
#define         BLMS_ERR_DEBUG(x)                                do{irq_disable();write_reg32(0x40000, (x));while(1);}while(0)
//#define         BLMS_ERR_DEBUG(x)





#define            SYS_IRQ_TRIG_SLOT_TASK                           0         //Scan/ ADV /BRX start /BTX start
#define            SYS_IRQ_TRIG_BTX_POST                            BIT(0)
#define            SYS_IRQ_TRIG_BRX_POST                            BIT(1)
#define            SYS_IRQ_TRIG_SCAN_POST                           BIT(2)

#define            SYS_IRQ_TRIG_BRX_START                           BIT(3)   //only slave high duty mode use



extern u32                 Crc24Lookup[16];


typedef struct {
    u8        adv_en;
    u8        scan_en;
    u8        adv_scanReq_connReq;
    u8        slot_process_en;

    u8        max_master_num;
    u8        max_slave_num;
    u8        cur_master_num;
    u8        cur_slave_num;


    u8        connSync;                //if more than 8 connections, u8 -> u16
    u8        connUpdate_nearby;        //if more than 8 connections, u8 -> u16
    u8        connUpdate_cmd;            //if more than 8 connections, u8 -> u16
    u8        new_conn_forbidden;

    u8        master_connInter;       // "u8" is enough, 320 mS most
    u8        half_connInter;
    u8        quarter_connInter;
    u8        slotNum_whole_inter;

    u8        slotNum_half_inter;
    u8        slotNum_quarter_inter;
    u8        drop_rx_data;
    u8        rsvd;



    u32        slot_idx_1st_btx_mark;   // slot_type
    u8        slot_offset_mark[BLMS_MAX_CONN_MASTER_NUM]; //maybe 1 or 2, so need lay it at last

} st_llms_para_t;

extern _attribute_aligned_(4)    st_llms_para_t  blmsParam;


extern    volatile    u8    blms_state;
extern                u8    blms_tx_empty_packet[];

extern    volatile    u32 rx_tick_now;

extern    volatile    int                blms_disconnect;
extern    volatile    int             blms_encryption_st;




typedef     int (*llms_host_mainloop_callback_t)(void);
typedef     int (*llms_enc_done_callback_t)(u16 connHandle);
typedef     int (*llms_conn_complete_handler_t)(u16 conn, u8 *p);
typedef     int (*llms_conn_terminate_handler_t)(u16 conn, u8 *p);

extern llms_host_mainloop_callback_t     llms_host_main_loop_cb;
extern llms_enc_done_callback_t          llms_encryption_done_cb;
extern llms_conn_complete_handler_t     llms_connComplete_handler;
extern llms_conn_terminate_handler_t     llms_connTerminate_handler;





/******************************* Macro & Enumeration variables for User ************************************/












/*********************************** Stack Interface Begin, user can not use!!! **********************************/
void         irq_ble_master_slave_handler(void);
void         blt_llms_channelTable_calc(u8* chm, u8 hop, u8 *ptbl);
int         blt_llms_procPendingEvent(void);


void        smemset4(int * dest, int val, int len);



void         blt_llms_registerHostMainloopCallback (llms_host_mainloop_callback_t cb);
void         blt_llms_registerConnectionEncryptionDoneCallback(llms_enc_done_callback_t  cb);
void         blt_llms_registerConnectionCompleteHandler(llms_conn_complete_handler_t  handler);
void         blt_llms_registerConnectionTerminateHandler(llms_conn_terminate_handler_t  handler);


/*********************************** Stack Interface End *********************************************************/










/************************************ User Interface  Begin  ******************************************************/

void     blc_llms_initBasicMCU (void);
void     blc_llms_initStandby_module (u8 *public_adr);
void     blc_llms_initMultiMasterSingleSlave_module(void);

u8 * blms_l2cap_packet_pack(u16 connHandle, u8 * raw_pkt);

ble_sts_t  blms_att_pushNotifyData (u16 connHandle, u16 attHandle, u8 *p, int len);
void     blms_sdk_main_loop (void);

u8 blms_getCurrConnRole(u16 connHandle);
u8 blms_fifo_num (u16 connHandle);
u8  blms_ll_getTxFifoNumber (u16 connHandle);
bool blms_ll_isTxFifoAvailableForApp(u16 connHandle);

int blms_is_handle_valid (u16 handle);

void  blms_ll_setEncryptionBusy(u16 connHandle, u8 enc_busy);
int  blms_ll_isEncryptionBusy(u16 connHandle);
u8 blms_ll_pushTxfifo_hold (u16 connHandle, u8 *p);



/************************************ User Interface  End  ********************************************************/







#endif /* LLMS_H_ */
