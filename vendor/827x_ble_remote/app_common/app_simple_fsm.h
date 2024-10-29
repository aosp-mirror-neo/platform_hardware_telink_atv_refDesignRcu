/********************************************************************************************************
 * @file	app_simple_fsm.h
 *
 * @brief	This is the header file for b85m
 *
 * @author	ROW GROUP
 * @date	Nov,2022
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *          All rights reserved.
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/

#ifndef APP_SIMPLE_FSM_H_
#define APP_SIMPLE_FSM_H_


#define APP_FSM_EN_PUSH_TO_FIFO

#ifdef APP_FSM_EN_PUSH_TO_FIFO
#include "app_fifo.h"
#endif
/**********************************************************************
 * GLOBAL TYPES
 */

typedef struct {
    unsigned short eventID;
    unsigned short bufLen;
    unsigned char *pBuf;
    unsigned char curState;
    unsigned char nextState;
}app_fsm_evnet_t;

typedef int (*app_fsm_eventActFunc_t)(app_fsm_evnet_t event);

typedef struct {
    unsigned char curState;                 // current state
    unsigned char nextState;                // next state
    unsigned short eventID;                 // event ID
    app_fsm_eventActFunc_t eventFunc;       // function pointer
}app_fsm_table_t;

typedef struct {
    app_fsm_table_t *pFsmTable;
    unsigned char fsmTabCnt;
    unsigned char curState;
    unsigned char maxState;
}app_fsm_t;



/**********************************************************************
 * GLOBAL FUNCTIONS
 */

/**
 * @brief This function is used to initialize FSM variables
 *
 * @param pFsm       - FSM variable pointer.This function is to initialize this variable
 * @param pFsmTable  - FSM state migration table
 * @param fmsTabCnt  - Number of FSM state migration tables
 * @param initState  - Initial value of FSM variable
 * @param maxState   - Maximum value of FSM variable. Used to prevent status from going out of scope
 * @return none
 */
void app_fsm_init(app_fsm_t *pFsm, app_fsm_table_t *pFsmTable, unsigned char fmsTabCnt, unsigned char initState, unsigned char maxState);

/**
 * @brief This function is used to trigger the EVENT of FMS
 * 
 * @param pFsm       - FSM variable pointer.
 * @param eventID    - Triggered EVNET ID
 * @param pBuf       - EVNET data pointer. If no data is required to be passed, set it to NULL
 * @param bufLen     - EVNET data length. If no data is required to be passed, set it to 0
 * @return none
 */
void app_fsm_triggerEvent(app_fsm_t *pFsm, unsigned short eventID, unsigned char *pBuf, unsigned short bufLen);

#ifdef APP_FSM_EN_PUSH_TO_FIFO
/**
 * @brief This function is used to trigger the EVENT of FMS. This function must be used in the callback function of FSM
 * 
 * @param pFsm       - FSM variable pointer.
 * @param eventID    - Triggered EVNET ID
 * @param pBuf       - EVNET data pointer. If no data is required to be passed, set it to NULL
 * @param bufLen     - EVNET data length. If no data is required to be passed, set it to 0
 */
void app_fsm_pushEvent(app_fsm_t *pFsm, unsigned short eventID, unsigned char *pBuf, unsigned short bufLen);

#endif /* APP_FSM_EN_PUSH_TO_FIFO */

#endif /* APP_SIMPLE_FSM_H_ */
