/********************************************************************************************************
 * @file	app_simple_fsm.c
 *
 * @brief	This is the source file for b85m
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
#include "tl_common.h"
#include "app_simple_fsm.h"

/**********************************************************************
 * LOCAL DEFINE
 */

//#define APP_FSM_EN_LOG

#ifdef APP_FSM_EN_LOG
    #define APP_FSM_LOG_TRACE                     log_trace
    #define APP_FSM_LOG_DEBUG                     log_debug
    #define APP_FSM_LOG_WARN                      log_warn
    #define APP_FSM_LOG_ERROR                     log_error
    #define APP_FSM_ARRAY_LOG_TRACE               array_log_trace
    #define APP_FSM_ARRAY_LOG_DEBUG               array_log_debug
    #define APP_FSM_ARRAY_LOG_WARN                array_log_warn
#else
    #define APP_FSM_LOG_TRACE(...)
    #define APP_FSM_LOG_DEBUG(...)
    #define APP_FSM_LOG_WARN(...)
    #define APP_FSM_LOG_ERROR(...)
    #define APP_FSM_ARRAY_LOG_TRACE(...)
    #define APP_FSM_ARRAY_LOG_DEBUG(...)
    #define APP_FSM_ARRAY_LOG_WARN(...)
#endif


#ifdef APP_FSM_EN_PUSH_TO_FIFO

/* APP VOICE FSM FIFO settings */
#define APP_FSM_BUF_SIZE                   (8)
#define APP_FSM_BUF_NUM                    (2)

#endif

/**********************************************************************
 * LOCAL TYPES
 */

typedef struct {
    u16 eventID;
    u16 paraLen;
    u8 para[0];
}fsm_event_cache_t;


/**********************************************************************
 * LOCAL VARIABLES
 */

#ifdef APP_FSM_EN_PUSH_TO_FIFO

/* FMS FIFO */
_attribute_data_no_init_ u8 s_fsm_fifo_b[APP_FSM_BUF_SIZE * APP_FSM_BUF_NUM] = {0};
_attribute_data_no_init_ app_fifo_t s_fsm_fifo = {
                                                    .size = APP_FSM_BUF_SIZE,
                                                    .num  = APP_FSM_BUF_NUM,
                                                    .wptr = 0,
                                                    .rptr = 0,
                                                    .p    = s_fsm_fifo_b};
#endif

/**********************************************************************
 * LOCAL FUNCTIONS
 */

inline void app_fsm_stateTrans(app_fsm_t *pFsm,u8 state ){
    pFsm->curState = state;
}

static void trigger_event(app_fsm_t *pFsm, u16 eventID, u8 *pBuf, u16 bufLen){

    app_fsm_table_t *p_my_table = pFsm->pFsmTable;

    if(NULL == p_my_table) return;

    u8 my_nextState;
    u8 my_curState = pFsm->curState;
    u8 my_fsmTabCnt = pFsm->fsmTabCnt;;
    app_fsm_eventActFunc_t my_eventFunc = NULL;

    u8 flag_hit = 0;
    for(u16 i=0; i<my_fsmTabCnt; i++){
        if((my_curState == p_my_table[i].curState) && (eventID == p_my_table[i].eventID)){
            flag_hit = 1;
            my_eventFunc = p_my_table[i].eventFunc;
            my_nextState = p_my_table[i].nextState;
            break;
        }
    }

    if(flag_hit){

        int ret_eventFunc = 0;
        if(my_eventFunc){
            app_fsm_evnet_t my_event = {eventID, bufLen, pBuf, my_curState, my_nextState};
            ret_eventFunc = my_eventFunc(my_event);
        }

        if(0 == ret_eventFunc){
            /* When the return value of the event function is 0, it is converted according to the FSM table. */
            app_fsm_stateTrans(pFsm, my_nextState);
        }else if(ret_eventFunc < 0){
            /* When the return value of the event function is less than 0, the conversion state is refused and the original state is maintained */
        }else{
            /* When the return value of the event function is greater than 0, it is forced to be set to the state of the return value. */
            if(ret_eventFunc <= pFsm->maxState){
                app_fsm_stateTrans(pFsm, ret_eventFunc);
            }
        }
    }
}


/**********************************************************************
 * GLOBAL FUNCTIONS
 */

void app_fsm_init(app_fsm_t *pFsm, app_fsm_table_t *pFsmTable, u8 fmsTabCnt, u8 initState, u8 maxState){

    if((NULL == pFsm) || (NULL == pFsmTable)) return;
    pFsm->pFsmTable = pFsmTable;
    pFsm->fsmTabCnt = fmsTabCnt;
    pFsm->curState = initState;
    pFsm->maxState = maxState;
}

void app_fsm_triggerEvent(app_fsm_t *pFsm, u16 eventID, u8 *pBuf, u16 bufLen){

#ifndef APP_FSM_EN_PUSH_TO_FIFO

    trigger_event(pFsm, eventID, pBuf, bufLen);

#else
    app_fifo_init(&s_fsm_fifo, APP_FSM_BUF_SIZE, APP_FSM_BUF_NUM, s_fsm_fifo_b);

    trigger_event(pFsm, eventID, pBuf, bufLen);

    /* Check whether FMS events is generated in the callback function */
    fsm_event_cache_t *p_my_event_buf = (fsm_event_cache_t *)app_fifo_get(&s_fsm_fifo);

    if(NULL == p_my_event_buf){
        return;
    }

    /* Process the events generated in the FSM event callback function sequentially. */
    for(u8 i=0; i<pFsm->maxState; i++){

        trigger_event(pFsm, p_my_event_buf->eventID, p_my_event_buf->para, p_my_event_buf->paraLen);

        app_fifo_pop(&s_fsm_fifo);

        p_my_event_buf = (fsm_event_cache_t *)app_fifo_get(&s_fsm_fifo);

        if(NULL == p_my_event_buf){
            break;
        }
    }
#endif
}

#ifdef APP_FSM_EN_PUSH_TO_FIFO

void app_fsm_pushEvent(app_fsm_t *pFsm, unsigned short eventID, unsigned char *pBuf, unsigned short bufLen){

    if(bufLen > (APP_FSM_BUF_SIZE-sizeof(fsm_event_cache_t))){
        APP_FSM_LOG_WARN("bufLen of pBuf passed in is too long");
        return;
    }

    fsm_event_cache_t *p_my_fmsEvent = (fsm_event_cache_t *)app_fifo_wptr(&s_fsm_fifo);

    if(NULL == p_my_fmsEvent){
        APP_FSM_LOG_WARN("FSM FIFO not enough !!!");
        return;
    }

    p_my_fmsEvent->eventID = eventID;
    p_my_fmsEvent->paraLen = 0;

    if((NULL == pBuf) || (0 == bufLen)){

    }else{
        p_my_fmsEvent->paraLen = bufLen;
        memcpy(p_my_fmsEvent->para, pBuf, bufLen);
    }

    app_fifo_next(&s_fsm_fifo);
    APP_FSM_ARRAY_LOG_TRACE("FSM EVENT", (u8 *)p_my_fmsEvent, bufLen+sizeof(fsm_event_cache_t));
}

#endif
