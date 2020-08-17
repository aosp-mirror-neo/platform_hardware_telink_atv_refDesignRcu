/******************************************************************************
 * @file     llms_slot.h
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

#ifndef LLMS_SLOT_H_
#define LLMS_SLOT_H_



#include "llms_conn.h"








#define            SLOT_MAX_NUM                                    128


/*******************************************************************************
 *
 * BIT(7)    BIT(6)    BIT(5)    BIT(4)       (BIT(3) BIT(2)  BIT(1) BIT(0) )
 *   M          S         SCAN     ADV                  conn_handle_idx
 ******************************************************************************/
#define            SLOT_TASK_ADV                                    BIT(4)
#define            SLOT_TASK_SCAN                                   BIT(5)
#define            SLOT_TASK_SLAVE                                  BIT(6)   //TODO, task_mask  SLOT_TASK_SLAVE ERROR
#define            SLOT_TASK_MASTER                                 BIT(7)

#define         SLOT_TASK_CONN                                    (SLOT_TASK_SLAVE | SLOT_TASK_MASTER)


#define            CONN_IDX_MASK                                    15    //15 is very safe. cause total connection number will not exceed 15





#define            CONN_MAX_NUM_M1_S1                                    0
#define            CONN_MAX_NUM_M2_S2                                    1
#define            CONN_MAX_NUM_M4_S3                                    2

#ifndef            CONN_MAX_NUM_CONFIG
#define            CONN_MAX_NUM_CONFIG                                    CONN_MAX_NUM_M4_S3
#endif



#if (CONN_MAX_NUM_CONFIG == CONN_MAX_NUM_M1_S1)
    // connHandle:
    // Master:    0x80  Slave:  0x41
    #define            BLMS_MAX_CONN_NUM                                2
    #define            BLMS_MAX_CONN_MASTER_NUM                         1
    #define            BLMS_MAX_CONN_SLAVE_NUM                          1


    #define            CONN_IDX_MASTER0                                 0
    #define            CONN_IDX_SLAVE0                                  1

#elif (CONN_MAX_NUM_CONFIG == CONN_MAX_NUM_M2_S2)
    // connHandle:
    // Master:    0x80/0x81   Slave:  0x42/0x43
    #define            BLMS_MAX_CONN_NUM                                4
    #define            BLMS_MAX_CONN_MASTER_NUM                         2
    #define            BLMS_MAX_CONN_SLAVE_NUM                          2


    #define            CONN_IDX_MASTER0                                 0
    #define            CONN_IDX_SLAVE0                                  2

#elif (CONN_MAX_NUM_CONFIG == CONN_MAX_NUM_M4_S3)
    // connHandle:
    // Master:    0x80/0x81/0x82/0x83   Slave:  0x44/0x45/0x46
    #define            BLMS_MAX_CONN_NUM                                7
    #define            BLMS_MAX_CONN_MASTER_NUM                         4
    #define            BLMS_MAX_CONN_SLAVE_NUM                          3


    #define            CONN_IDX_MASTER0                                 0
    #define            CONN_IDX_SLAVE0                                  4

#else

#endif





    #define         SLOT_TASK_MSK_CONN_0                             BIT(0)

#if (CONN_MAX_NUM_CONFIG == CONN_MAX_NUM_M1_S1)

    #define         SLOT_TASK_MSK_MASTER0                            BIT(0)
    #define         SLOT_TASK_MSK_SLAVE0                             BIT(1)

    #define         SLOT_TASK_MSK_MASTER                             SLOT_TASK_MSK_MASTER0                                                              SLOT_TASK_MSK_SLAVE0  | SLOT_TASK_MSK_SLAVE1    )
    #define         SLOT_TASK_MSK_SLAVE                              SLOT_TASK_MSK_SLAVE0

    #define         SLOT_TASK_MSK_CONN                               ( SLOT_TASK_MSK_MASTER0 | SLOT_TASK_MSK_SLAVE0 )

#elif (CONN_MAX_NUM_CONFIG == CONN_MAX_NUM_M2_S2)

    #define         SLOT_TASK_MSK_MASTER0                            BIT(0)
    #define         SLOT_TASK_MSK_MASTER1                            BIT(1)
    #define         SLOT_TASK_MSK_SLAVE0                             BIT(2)
    #define         SLOT_TASK_MSK_SLAVE1                             BIT(3)


    #define         SLOT_TASK_MSK_MASTER                             ( SLOT_TASK_MSK_MASTER0 | SLOT_TASK_MSK_MASTER1 )                                                              SLOT_TASK_MSK_SLAVE0  | SLOT_TASK_MSK_SLAVE1    )
    #define         SLOT_TASK_MSK_SLAVE                              ( SLOT_TASK_MSK_SLAVE0  | SLOT_TASK_MSK_SLAVE1 )


    #define         SLOT_TASK_MSK_CONN                               ( SLOT_TASK_MSK_MASTER0 | SLOT_TASK_MSK_MASTER1 | \
                                                                      SLOT_TASK_MSK_SLAVE0  | SLOT_TASK_MSK_SLAVE1 )
#elif (CONN_MAX_NUM_CONFIG == CONN_MAX_NUM_M4_S3)

    #define         SLOT_TASK_MSK_MASTER0                            BIT(0)
    #define         SLOT_TASK_MSK_MASTER1                            BIT(1)
    #define         SLOT_TASK_MSK_MASTER2                            BIT(2)
    #define         SLOT_TASK_MSK_MASTER3                            BIT(3)
    #define         SLOT_TASK_MSK_SLAVE0                             BIT(4)
    #define         SLOT_TASK_MSK_SLAVE1                             BIT(5)
    #define         SLOT_TASK_MSK_SLAVE2                             BIT(6)


    #define         SLOT_TASK_MSK_MASTER                            ( SLOT_TASK_MSK_MASTER0 | SLOT_TASK_MSK_MASTER1 |  \
                                                                      SLOT_TASK_MSK_MASTER2 | SLOT_TASK_MSK_MASTER3 )

    #define         SLOT_TASK_MSK_SLAVE                             ( SLOT_TASK_MSK_SLAVE0  | SLOT_TASK_MSK_SLAVE1    |  SLOT_TASK_MSK_SLAVE2    )

    #define         SLOT_TASK_MSK_CONN                              ( SLOT_TASK_MSK_MASTER0 | SLOT_TASK_MSK_MASTER1 |  \
                                                                      SLOT_TASK_MSK_MASTER2 | SLOT_TASK_MSK_MASTER3 | \
                                                                      SLOT_TASK_MSK_SLAVE0  | SLOT_TASK_MSK_SLAVE1    |  SLOT_TASK_MSK_SLAVE2 )
#endif


#define            SLOT_TASK_MSK_ADV                                 BIT(12)
#define            SLOT_TASK_MSK_SCAN                                BIT(13)






#define            SLOT_UPDT_SLAVE_SYNC                             BIT(0)
#define            SLOT_UPDT_SLAVE_SYNC_DONE                        BIT(1)
#define            SLOT_UPDT_SLAVE_TERMINATE                        BIT(2)
#define            SLOT_UPDT_ADV_RECOVER                            BIT(3)

#define            SLOT_UPDT_MASTER_CONN                            BIT(4)
#define            SLOT_UPDT_MASTER_TERMINATE                       BIT(5)
#define            SLOT_UPDT_SCAN_RECOVER                           BIT(6)

#define            SLOT_UPDT_SLAVE_SLOT_FINE_TUNE                   BIT(7)







#define            SLOT_DURATION_SET_SCAN                            1

/****************************************************************************************************************************************
ADV data 37 bytes, Scan_Rsp 16 bytes, 32M clock,  all code running in SRAM, no Scan_Req/Conn_Req, cost 2353 uS
ADV data 37 bytes, Scan_Rsp 16 bytes, 32M clock,  all code running in SRAM, no Scan_Req, Conn_Req on chn39, 2700 uS (2019.09.03 SiHui test data)

ADV data 37 bytes, Scan_Rsp 37 bytes, 32M clock,  all code running in SRAM, no Scan_Req on chn37/38,
            Scan_Req&Scan_Rsp on chn39, 2940 uS -> 2890 uS (400->375 optimize) (2019.09.17 SiHui test data)

Worst Case:
PDU 37 bytes, 32M clock,  all code running in SRAM, receive scan_req/conn_req on chn37 failed,
              receive scan_req/conn_req on chn38 failed, receive scan_req success and send scan_rsp
                this timing is about: 2890+300*2 = 3490

***************************************************************************************************************************************/
//TODO, optimize if ADV channel is not 37/38/39, or if ADV data & Scan_Rsp data not longest
//TODO, if none connecTable ADV or driect_adv, duration can be smaller
#define            SLOT_DURATION_ADV                                 6   // 625*4 = 2500uS,  625*5= 3125 uS,  625*6= 3750 uS


#define            SLOT_DURATION_SLAVE_COMMON                        4
#define            SLOT_DURATION_MASTER_COMMON                       3





// 150us(T_ifs) + 352us(conn_req) = 502 us,  sub some margin 22(RX IRQ delay/irq_handler delay)
// real test data: 470 uS,  beginning is "u32 tick_now = rx_tick_now = clock_time();" in irq_blms_rx
//                            ending is    "while( !(reg_rf_irq_status & FLD_RF_IRQ_TX));" in
//                          "irq_handler" to "u32 tick_now = rx_tick_now = clock_time();" is 4 uS
#define            PKT_INIT_AFTER_RX_TICK                           ( 480 *CLOCK_16M_SYS_TIMER_CLK_1US)

#define            SLOT_PROCESS_MAX_TICK                            ( 300 *CLOCK_16M_SYS_TIMER_CLK_1US)

#define            BOUNDARY_MARGIN_COMMON_TICK                      ( SLOT_PROCESS_MAX_TICK + 20 *CLOCK_16M_SYS_TIMER_CLK_1US)
#define            BOUNDARY_MARGIN_INIT_TICK                        ( PKT_INIT_AFTER_RX_TICK + BOUNDARY_MARGIN_COMMON_TICK)      // initiate timing + slot_update_rebuild_allocate running potential biggest time




#define            BOUNDARY_RX_RELOAD_TICK                          ( 30  *CLOCK_16M_SYS_TIMER_CLK_1US)

enum{
    CLOCK_16M_SYS_TIMER_CLK_312US  = 5000,   //312.5*16
    CLOCK_16M_SYS_TIMER_CLK_625US  = 10000,  //625*16
    CLOCK_16M_SYS_TIMER_CLK_1250US = 20000,  //1250*16
};


typedef enum{
    SLOT_PRIORITY_LOW            =    0,

    SLOT_PRIORITY_MID            =    100,

    SLOT_PRIORITY_HIGH_THRES     =    199,

    SLOT_PRIORITY_HIGH           =    200,
    SLOT_PRIORITY_SUPER_HIGH     =    240,    //only for slave connection establish or connection update
}slot_pri_t;




typedef unsigned int slot_type ;
#define SLOT_TYPE_THRES                  256

typedef struct{

    u8    priority     [BLMS_MAX_CONN_NUM == 7 ? 8 : BLMS_MAX_CONN_NUM];


    u8     num_cur;
    u8     update;
    u8     rebuild;
    u8     slot_task_irq;                     //only used for insert Scan in slot gap

    u32    task_mask;
    u32    slot_tick_irq;
    u32    slot_tick_next;



    u32    slot_start_adv;
    u32    slot_start_scan;
    u32    slot_tick_irq_real;  //used for timing calculate when initiate, see the explanations in blms_m_connect
    slot_type     slot_idx_irq;



    slot_type     slot_mark_adv;
    slot_type     slot_mark_scan;
    slot_type     slot_idx_next;
    slot_type     slot_idx_start;   // start of a new slot timing


    slot_type     slot_mark_conn[BLMS_MAX_CONN_NUM == 7 ? 8 : BLMS_MAX_CONN_NUM];   //slot_idx_mark_conn



    u8    slot_duration[BLMS_MAX_CONN_NUM == 7 ? 8 : BLMS_MAX_CONN_NUM];
    u8    slot_interval[BLMS_MAX_CONN_NUM == 7 ? 8 : BLMS_MAX_CONN_NUM];
    u8    slot_drop_num[BLMS_MAX_CONN_NUM == 7 ? 8 : BLMS_MAX_CONN_NUM];



    u32    slot_start_conn[BLMS_MAX_CONN_NUM == 7 ? 8 : BLMS_MAX_CONN_NUM];
    u32    slot_tick_conn[BLMS_MAX_CONN_NUM == 7 ? 8 : BLMS_MAX_CONN_NUM];



    slot_type     slot_idx_irq_real; //used for timing calculate when initiate, see the explanations in blms_m_connect
}slot_st_t;

extern _attribute_aligned_(4)    slot_st_t    AA_bltSlot;   // AA_ just for debug

#define bltSlot    AA_bltSlot



extern u8    AA_slot_map[];    // AA_ just for debug

#define slot_map    AA_slot_map
/************************************ User Interface  ******************************************************/



















/*********************************** Stack Interface, user can not use!!! **********************************/
int        blt_llms_slotUpdateRebuildAllocate(void);

int        blt_llms_startSlotTiming(void);
int     blt_llms_rebuildSlotTimingAdv(void);
int     blt_llms_rebuildSlotTimingScan(void);
int     blt_llms_findAdvSlotAvailable(void);

u32     blt_llms_findFirstAvailableStartSlot(slot_type mark, u16 interval);

static inline void blt_slot_setConnSlotPriority(u8 conn_idx, slot_pri_t pri){
    bltSlot.priority[conn_idx] = pri;
}

static inline u8 blt_slot_getConnSlotPriority(u8 conn_idx){
    return bltSlot.priority[conn_idx];
}


static inline void blt_slot_clearConnSlotPriority(u8 conn_idx){
    bltSlot.priority[conn_idx] = 0;
}

void blt_slot_increaseConnSlotPriority(u8 conn_idx, u8 inc);

static inline void blt_slot_addTaskMask(u32 task){
    bltSlot.task_mask |= (task);
}

static inline void blt_slot_removeTaskMask(u32 task){
    bltSlot.task_mask &= ~(task);
}

static inline void blt_slot_clearTaskMask(void){
    bltSlot.task_mask = 0;
}

static inline void blt_slot_addUpdate(u8 updt){
    bltSlot.update |= (updt);
}

static inline void blt_slot_removeUpdate(u8 updt){
    bltSlot.update &= ~(updt);
}




#endif /* LLMS_SLOT_H_ */
