/*
 * app_backlight_gatt.c
 *
 *  Created on: 2024-5-8
 *      Author: name
 */


#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "../app_common/app_simple_fsm.h"
#include "../../common/blt_soft_timer.h"
#include "../app_att.h"


/**********************************************************************
 * LOCAL DEFINE
 */

//#define BACKLIGHT_FSM_DEBUG

#define BACKLIGHT_FIX_HANDLE


//#define BACKLIGHT_DEBUG_LOG(...)

//#define BACKLIGHT_DEBUG_LOG                 printf

#define BACKLIGHT_HOUR_MAX                  (24)
#define BACKLIGHT_MINUTE_MAX                (60)


#define BACKLIGHT_NIGHT_START_HOUR          (18)
#define BACKLIGHT_NIGHT_START_MINUTE        (00)

#define BACKLIGHT_NIGHT_END_HOUR            (6)
#define BACKLIGHT_NIGHT_END_MINUTE          (00)

#define BACKLIGHT_RTC_MINUTE                (60)    // unit:sec   should be 60sec.  10sec just for test


#define GOOGLE_BACKLIGHT_SERVICE_UUID        0x64,0xB6,0x17,0xF6,0x01,0xAF,0x7D,0xBC,0x05,0x4F,0x21,0x5A,0x69,0x8B,0xB3,0xA2
#define GOOGL_BACKLIGHT_CHAR_UUID            0x64,0xB6,0x17,0xF6,0x01,0xAF,0x7D,0xBC,0x05,0x4F,0x21,0x5A,0x6A,0x8B,0xB3,0xA2



#define MACROS_BACKLIGHT_STATE_TABLE                                                \
            TLK_BACKLIGHT_FSM_MACRO(BACKLIGHT_STATE_IDLE)                           \
            TLK_BACKLIGHT_FSM_MACRO(BACKLIGHT_STATE_WAIT_TIME)


#define MACROS_BACKLIGHT_EVENT_TABLE                                                \
            TLK_BACKLIGHT_FSM_MACRO(BACKLIGHT_EVENT_MODE_NAVER)                     \
            TLK_BACKLIGHT_FSM_MACRO(BACKLIGHT_EVENT_MODE_ALWAYS)                    \
            TLK_BACKLIGHT_FSM_MACRO(BACKLIGHT_EVENT_MODE_DAYTIME_NIGHTTIME)         \
            TLK_BACKLIGHT_FSM_MACRO(BACKLIGHT_EVENT_SET_TIME)                       \
            TLK_BACKLIGHT_FSM_MACRO(BACKLIGHT_EVENT_WAIT_TIME_TIMEOUT)              \
            TLK_BACKLIGHT_FSM_MACRO(BACKLIGHT_EVENT_INVALID_DATA)


/**********************************************************************
 * LOCAL TYPES
 */

typedef enum{
    BACKLIGHT_CMD_SET_BACKLIGHT_MODE        = 0x00,
    BACKLIGHT_CMD_SET_TIME                  = 0x01,

    BACKLIGHT_CMD_INVALID_REQUEST           = 0xFF,
}BACKLIGHT_CMD_T;

typedef enum{
    BACKLIGHT_MODE_NAVER                    = 0x0000,
    BACKLIGHT_MODE_ALWAYS                   = 0x0001,
    BACKLIGHT_MODE_DAYTIME_NIGHTTIME        = 0x0002,
}BACKLIGHT_MODE_T;

#define TLK_BACKLIGHT_FSM_MACRO(a)    a,
typedef enum{
    MACROS_BACKLIGHT_STATE_TABLE
}APP_BACKLIGHT_STATE_T;

typedef enum{
    MACROS_BACKLIGHT_EVENT_TABLE
}APP_BACKLIGHT_EVNET_T;
#undef TLK_BACKLIGHT_FSM_MACRO

typedef struct {
    unsigned char event;
    unsigned char data_hi;
    unsigned char data_lo;
}backlight_event_format_t;

typedef struct {
    unsigned char event;
    unsigned char hour;
    unsigned char minute;
}backlight_time_format_t;



/**********************************************************************
 * ATT SERVICE
 */

#define WRAPPING_BRACES(__DATAS__)          { __DATAS__ }

const u8 app_backlight_gatt_service_uuid[16]   = WRAPPING_BRACES(GOOGLE_BACKLIGHT_SERVICE_UUID);
const u8 app_backlight_gatt_char_uuid[16]      = WRAPPING_BRACES(GOOGL_BACKLIGHT_CHAR_UUID);


const u8 app_backlight_gatt_char_val[19] = {
    CHAR_PROP_READ | CHAR_PROP_NOTIFY | CHAR_PROP_WRITE_WITHOUT_RSP,
    U16_LO(APP_BACKLIGHT_GATT_CMD_DP_H), U16_HI(APP_BACKLIGHT_GATT_CMD_DP_H),
    GOOGL_BACKLIGHT_CHAR_UUID,
};

//_attribute_data_retention_ volatile u8 app_backlight_gatt_data[3] = {0x00};
_attribute_data_retention_ volatile u8 app_backlight_gatt_data[3] = {BACKLIGHT_CMD_SET_TIME, 0x00, 0x00};
_attribute_data_retention_ u16 app_backlight_gatt_ccc = 0;

/**********************************************************************
 * LOCAL FUNCTIONS DEFINE
 */

static int backlight_event_cb_trygger_naver(app_fsm_evnet_t event);
static int backlight_event_cb_trygger_always(app_fsm_evnet_t event);
static int backlight_event_cb_wait_time(app_fsm_evnet_t event);
static int backlight_event_cb_set_time(app_fsm_evnet_t event);
static int backlight_event_cb_wait_time_timeout(app_fsm_evnet_t event);
static int backlight_event_cb_invalid_data(app_fsm_evnet_t event);


/**********************************************************************
 * LOCAL VARIABLES
 */

const app_fsm_table_t backlight_fsm_table[] = {
    //{current state,               next state,                 EVENT,                                  call back   }
    { BACKLIGHT_STATE_IDLE,         BACKLIGHT_STATE_IDLE,       BACKLIGHT_EVENT_MODE_NAVER,             backlight_event_cb_trygger_naver},
    { BACKLIGHT_STATE_IDLE,         BACKLIGHT_STATE_IDLE,       BACKLIGHT_EVENT_MODE_ALWAYS,            backlight_event_cb_trygger_always},

    { BACKLIGHT_STATE_IDLE,         BACKLIGHT_STATE_WAIT_TIME,  BACKLIGHT_EVENT_MODE_DAYTIME_NIGHTTIME, backlight_event_cb_wait_time},
    { BACKLIGHT_STATE_WAIT_TIME,    BACKLIGHT_STATE_IDLE,       BACKLIGHT_EVENT_SET_TIME,               backlight_event_cb_set_time},

    /* The following are exceptions */
    { BACKLIGHT_STATE_WAIT_TIME,    BACKLIGHT_STATE_IDLE,       BACKLIGHT_EVENT_WAIT_TIME_TIMEOUT,      backlight_event_cb_wait_time_timeout},
    { BACKLIGHT_STATE_WAIT_TIME,    BACKLIGHT_STATE_IDLE,       BACKLIGHT_EVENT_INVALID_DATA,           backlight_event_cb_invalid_data},

    { BACKLIGHT_STATE_WAIT_TIME,    BACKLIGHT_STATE_IDLE,       BACKLIGHT_EVENT_MODE_NAVER,             backlight_event_cb_trygger_naver},
    { BACKLIGHT_STATE_WAIT_TIME,    BACKLIGHT_STATE_IDLE,       BACKLIGHT_EVENT_MODE_ALWAYS,            backlight_event_cb_trygger_always},

    { BACKLIGHT_STATE_IDLE,         BACKLIGHT_STATE_IDLE,       BACKLIGHT_EVENT_INVALID_DATA,           backlight_event_cb_invalid_data},

    { BACKLIGHT_STATE_IDLE,         BACKLIGHT_STATE_IDLE,       BACKLIGHT_EVENT_SET_TIME,               backlight_event_cb_invalid_data},
};


#ifdef BACKLIGHT_FSM_DEBUG
    #define TLK_BACKLIGHT_FSM_MACRO(a) #a,
    static const char* tlk_backlight_fms_state_str[] = {
            MACROS_BACKLIGHT_STATE_TABLE
    };

    static const char* tlk_backlight_fms_event_str[] = {
            MACROS_BACKLIGHT_EVENT_TABLE
    };
    #undef TLK_BACKLIGHT_FSM_MACRO
#endif


_attribute_data_retention_ app_fsm_t s_backlight_fms = {
    .pFsmTable  = (app_fsm_table_t *)backlight_fsm_table,
    .fsmTabCnt  = ARRAY_SIZE(backlight_fsm_table),
    .curState   = BACKLIGHT_STATE_IDLE,
    .maxState   = BACKLIGHT_STATE_WAIT_TIME,
};

#ifndef BACKLIGHT_FIX_HANDLE
_attribute_data_retention_ u16 s_backlight_gatt_handle = 0;
#endif
/**********************************************************************
 * LOCAL FUNCTIONS
 */

// TODO



#ifndef BACKLIGHT_FSM_DEBUG

#define app_backlight_gatt_trigger_event(eventID, p_buf, buf_len)  app_fsm_triggerEvent(&s_backlight_fms, eventID, p_buf, buf_len);

#else

void app_backlight_gatt_trigger_event(u16 eventID, u8 *p_buf, u16 buf_len){

    printf("Trigger EVENT[%d]:[%s] currState[%d]:[%s]",
            eventID,
            tlk_backlight_fms_event_str[eventID]+16,
            s_backlight_fms.curState,
            tlk_backlight_fms_state_str[s_backlight_fms.curState]+16);

    app_fsm_triggerEvent(&s_backlight_fms, eventID, p_buf, buf_len);
}

#endif

#define app_backlight_gatt_push_event(eventID, p_buf, buf_len)  app_fsm_pushEvent(&s_backlight_fms, eventID, p_buf, buf_len);


static short backlight_minute_sub(short minute0, short minute1){

    if( (minute0>=BACKLIGHT_HOUR_MAX*BACKLIGHT_MINUTE_MAX) || (minute1>=BACKLIGHT_HOUR_MAX*BACKLIGHT_MINUTE_MAX)){
        printf("[FATAL] This shouldn't happen !!!!\n");
        return -1;
    }

    short my_sub = minute0 - minute1;

    return my_sub>=0?my_sub:(my_sub+BACKLIGHT_HOUR_MAX*BACKLIGHT_MINUTE_MAX);
}


u8 backlight_day_night_mode_is_night(u8 hour, u8 minute){
    short my_night_start = BACKLIGHT_NIGHT_START_HOUR*BACKLIGHT_MINUTE_MAX + BACKLIGHT_NIGHT_START_MINUTE;
    short my_night_end = BACKLIGHT_NIGHT_END_HOUR*BACKLIGHT_MINUTE_MAX + BACKLIGHT_NIGHT_END_MINUTE;

    short my_current_minute = hour*BACKLIGHT_MINUTE_MAX + minute;

    short my_distance = backlight_minute_sub(my_current_minute, my_night_start) + backlight_minute_sub(my_night_end, my_current_minute);

    u8 my_is_night = 0;

    printf("DISTANCE:[%d] = [%d] + [%d]\n", my_distance, backlight_minute_sub(my_current_minute, my_night_start), backlight_minute_sub(my_night_end, my_current_minute));

    if(my_distance<BACKLIGHT_HOUR_MAX*BACKLIGHT_MINUTE_MAX){
        my_is_night = 1;
    }

    return my_is_night;
}


int backlight_rtc_timer(void){

    backlight_time_format_t *p_my_time = (backlight_time_format_t *)app_backlight_gatt_data;

    if(BACKLIGHT_CMD_SET_TIME == p_my_time->event){

    }else{
        printf("[ERROR] backlight_rtc_timer works in exception mode\n");
        return -1;
    }

    p_my_time->minute++;
    if(p_my_time->minute >= BACKLIGHT_MINUTE_MAX){
        p_my_time->hour++;
        p_my_time->minute = 0;
    }

    if(p_my_time->hour >= BACKLIGHT_HOUR_MAX){
        p_my_time->hour = 0;
    }

    if( backlight_day_night_mode_is_night(p_my_time->hour, p_my_time->minute)){
        printf("UPDATE BACKLIGHT NIGHT TIME:[%02d:%02d]\n", p_my_time->hour, p_my_time->minute);
    }else{
        printf("UPDATE BACKLIGHT TIME:[%02d:%02d]\n", p_my_time->hour, p_my_time->minute);
    }

    return 0;
}

int backlight_set_rtc_timeout_timer(void){
    printf("%s\n", __FUNCTION__);

    app_backlight_gatt_trigger_event(BACKLIGHT_EVENT_WAIT_TIME_TIMEOUT, NULL, 0);

    return -1;
}

void bakclight_data_update(u8 *p_buf){
    backlight_event_format_t *p_my_evnt = (backlight_event_format_t *)p_buf;
    if(BACKLIGHT_CMD_SET_BACKLIGHT_MODE == p_my_evnt->event){
        u16 my_event_mode = MAKE_U16(p_my_evnt->data_hi, p_my_evnt->data_lo);
        if( (BACKLIGHT_MODE_NAVER == my_event_mode) || (BACKLIGHT_MODE_ALWAYS == my_event_mode) ){
            blt_soft_timer_delete(backlight_rtc_timer);
        }else{
            printf("[BACLIGHT MODE] Incoming error message in mode setting. MODE:[0x%04X]\n", my_event_mode);
            return;
        }
    }else if(BACKLIGHT_CMD_SET_TIME == p_my_evnt->event){
        backlight_time_format_t *p_my_time = (backlight_time_format_t *)p_buf;

        if( (p_my_time->hour < BACKLIGHT_HOUR_MAX) && (p_my_time->minute < BACKLIGHT_MINUTE_MAX) ){
            /* Delete wait time timeout timer and start RTC timer */
            blt_soft_timer_delete(backlight_set_rtc_timeout_timer);
            blt_soft_timer_delete(backlight_rtc_timer);
            blt_soft_timer_add(backlight_rtc_timer, BACKLIGHT_RTC_MINUTE*1000000);
        }else{
            printf("[BACLIGHT TIME] Incoming error message in time setting. HH:[0x%02X] MM:[0x%02X]\n", p_my_time->hour, p_my_time->minute);
            return;
        }
    }else{
        printf("[BACLIGHT EVENT] Passing in wrong event value. EVENT:[0x%02X]\n", p_my_evnt->event);
        return;
    }

    app_backlight_gatt_data[0] = p_buf[0];
    app_backlight_gatt_data[1] = p_buf[1];
    app_backlight_gatt_data[2] = p_buf[2];
#ifdef BACKLIGHT_FIX_HANDLE
    ble_sts_t my_ret = bls_att_pushNotifyData(APP_BACKLIGHT_GATT_CMD_DP_H, (u8 *)app_backlight_gatt_data, sizeof(app_backlight_gatt_data));
    printf("RET:[%d] NOTIFY BACKLIGHT UPDATE DATA:", my_ret);
    array_printf((u8 *)app_backlight_gatt_data, sizeof(app_backlight_gatt_data));
#else
    if(s_backlight_gatt_handle){
        ble_sts_t my_ret = bls_att_pushNotifyData(s_backlight_gatt_handle, (u8 *)app_backlight_gatt_data, sizeof(app_backlight_gatt_data));
        printf("RET:[%d] NOTIFY BACKLIGHT UPDATE DATA:", my_ret);
        array_printf((u8 *)app_backlight_gatt_data, sizeof(app_backlight_gatt_data));
    }
#endif
}

static int backlight_event_cb_trygger_naver(app_fsm_evnet_t event){
    printf("%s\n", __FUNCTION__);

    u8 my_update_data[3] = {BACKLIGHT_CMD_SET_BACKLIGHT_MODE, U16_HI(BACKLIGHT_MODE_NAVER), U16_LO(BACKLIGHT_MODE_NAVER)};
    bakclight_data_update(my_update_data);

    return 0;
}

static int backlight_event_cb_trygger_always(app_fsm_evnet_t event){
    printf("%s\n", __FUNCTION__);

    u8 my_update_data[3] = {BACKLIGHT_CMD_SET_BACKLIGHT_MODE, U16_HI(BACKLIGHT_MODE_ALWAYS), U16_LO(BACKLIGHT_MODE_ALWAYS)};
    bakclight_data_update(my_update_data);

    return 0;
}

static int backlight_event_cb_wait_time(app_fsm_evnet_t event){
    printf("%s\n", __FUNCTION__);


#ifdef BACKLIGHT_FIX_HANDLE
    u8 my_wait_time_data[3] = {0x00, 0x00, 0x02};
    bls_att_pushNotifyData(APP_BACKLIGHT_GATT_CMD_DP_H, my_wait_time_data, sizeof(my_wait_time_data));
    printf("NOTIFY BACKLIGHT WAIT TIME DATA:");
    array_printf(my_wait_time_data, sizeof(my_wait_time_data));
#else
    if(s_backlight_gatt_handle){
        u8 my_wait_time_data[3] = {0x00, 0x00, 0x02};
        bls_att_pushNotifyData(s_backlight_gatt_handle, my_wait_time_data, sizeof(my_wait_time_data));
        printf("NOTIFY BACKLIGHT WAIT TIME DATA:");
        array_printf(my_wait_time_data, sizeof(my_wait_time_data));
    }
#endif
    blt_soft_timer_delete(backlight_set_rtc_timeout_timer);
    blt_soft_timer_add(backlight_set_rtc_timeout_timer, 3*1000000);

    return 0;
}

static int backlight_event_cb_set_time(app_fsm_evnet_t event){
    printf("%s\n", __FUNCTION__);

    array_printf(event.pBuf, event.bufLen);

    backlight_time_format_t *p_my_time = (backlight_time_format_t *)event.pBuf;

    if(p_my_time->hour < BACKLIGHT_HOUR_MAX){

    }else{
        blt_soft_timer_delete(backlight_set_rtc_timeout_timer);
        app_backlight_gatt_push_event(BACKLIGHT_EVENT_INVALID_DATA, event.pBuf, event.bufLen);
        return -1;
    }

    if(p_my_time->minute < BACKLIGHT_MINUTE_MAX){

    }else{
        blt_soft_timer_delete(backlight_set_rtc_timeout_timer);
        app_backlight_gatt_push_event(BACKLIGHT_EVENT_INVALID_DATA, event.pBuf, event.bufLen);
        return -1;

    }

    bakclight_data_update(event.pBuf);
    return 0;
}


static int backlight_event_cb_wait_time_timeout(app_fsm_evnet_t event){
    printf("%s\n", __FUNCTION__);

#ifdef BACKLIGHT_FIX_HANDLE
    u8 my_wait_time_data[4] = {0xff, 0x00, 0x00, 0x02};
    bls_att_pushNotifyData(APP_BACKLIGHT_GATT_CMD_DP_H, my_wait_time_data, sizeof(my_wait_time_data));
    printf("NOTIFY BACKLIGHT WAIT TIME TIMEOUT DATA:");
    array_printf(my_wait_time_data, sizeof(my_wait_time_data));
#else
    if(s_backlight_gatt_handle){
        u8 my_wait_time_data[4] = {0xff, 0x00, 0x00, 0x02};
        bls_att_pushNotifyData(s_backlight_gatt_handle, my_wait_time_data, sizeof(my_wait_time_data));
        printf("NOTIFY BACKLIGHT WAIT TIME TIMEOUT DATA:");
        array_printf(my_wait_time_data, sizeof(my_wait_time_data));
    }
#endif
    return 0;
}

//u8 my_invalid_notify[4] = {0};

static int backlight_event_cb_invalid_data(app_fsm_evnet_t event){
    printf("%s\n", __FUNCTION__);
//    array_printf(event.pBuf, event.bufLen);

    blt_soft_timer_delete(backlight_set_rtc_timeout_timer);

#ifdef BACKLIGHT_FIX_HANDLE
    u8 my_invalid_notify[4] = {BACKLIGHT_CMD_INVALID_REQUEST, event.pBuf[0], event.pBuf[1], event.pBuf[2]};

    ble_sts_t my_ret = bls_att_pushNotifyData(APP_BACKLIGHT_GATT_CMD_DP_H, my_invalid_notify, sizeof(my_invalid_notify));//
    printf("RET:[%d] NOTIFY BACKLIGHT INVALID DATA:", my_ret);
    array_printf(my_invalid_notify, sizeof(my_invalid_notify));
#else
    if(s_backlight_gatt_handle){
        u8 my_invalid_notify[4] = {BACKLIGHT_CMD_INVALID_REQUEST, event.pBuf[0], event.pBuf[1], event.pBuf[2]};

        ble_sts_t my_ret = bls_att_pushNotifyData(s_backlight_gatt_handle, my_invalid_notify, sizeof(my_invalid_notify));//
        printf("RET:[%d] NOTIFY BACKLIGHT INVALID DATA:", my_ret);
        array_printf(my_invalid_notify, sizeof(my_invalid_notify));

    }
#endif

    return 0;
}


/**********************************************************************
 * GLOBAL FUNCTIONS
 */

void app_backlight_gatt_init_state(void){

    if(s_backlight_fms.curState != BACKLIGHT_STATE_IDLE){
        printf("[warning]The initial value of s_atvv_fsm should be BACKLIGHT_STATE_IDLE");
    }

    app_fsm_init(&s_backlight_fms, (app_fsm_table_t *)backlight_fsm_table  ,ARRAY_SIZE(backlight_fsm_table), BACKLIGHT_STATE_IDLE, BACKLIGHT_STATE_WAIT_TIME);
}

u8 app_backlight_gatt_fsm_get_state(void){
    return s_backlight_fms.curState;
}




int app_backlight_gatt_write_cb(void *p){

    rf_packet_att_data_t *req = (rf_packet_att_data_t*)p;
    u16 my_data_len = req->l2cap - 3;

    array_printf(req->dat, my_data_len);

    if(my_data_len < 3){
        printf("BACKLIGHT CMD LEN error\n");
        return 0;
    }
#ifndef BACKLIGHT_FIX_HANDLE
    if(0 == s_backlight_gatt_handle){
        s_backlight_gatt_handle = req->handle;
    }
    printf("CMD HANDLE:[%d] NOTIFY HANDLE:[%d] SET:{%d}\n", req->handle, APP_BACKLIGHT_GATT_CMD_DP_H, s_backlight_gatt_handle);
#endif

    backlight_event_format_t *p_my_event = (backlight_event_format_t *)req->dat;

    if(BACKLIGHT_CMD_SET_BACKLIGHT_MODE == p_my_event->event){

        u16 my_backlight_mode = MAKE_U16(p_my_event->data_hi, p_my_event->data_lo);
        switch (my_backlight_mode) {
            case BACKLIGHT_MODE_NAVER:
                app_backlight_gatt_trigger_event(BACKLIGHT_EVENT_MODE_NAVER, req->dat, my_data_len);
                break;
            case BACKLIGHT_MODE_ALWAYS:
                app_backlight_gatt_trigger_event(BACKLIGHT_EVENT_MODE_ALWAYS, req->dat, my_data_len);
                break;
            case BACKLIGHT_MODE_DAYTIME_NIGHTTIME:
                app_backlight_gatt_trigger_event(BACKLIGHT_EVENT_MODE_DAYTIME_NIGHTTIME, req->dat, my_data_len);
                break;
            default:
                app_backlight_gatt_trigger_event(BACKLIGHT_EVENT_INVALID_DATA, req->dat, my_data_len);
                break;
        }
    }else if(BACKLIGHT_CMD_SET_TIME == p_my_event->event){
        app_backlight_gatt_trigger_event(BACKLIGHT_EVENT_SET_TIME, req->dat, my_data_len);
    }else{
        app_backlight_gatt_trigger_event(BACKLIGHT_EVENT_INVALID_DATA, req->dat, my_data_len);
    }

    return 0;
}

u8 app_backlight_gatt_is_allow_backlight(void){
    u8 my_allow = 0;

    backlight_event_format_t *p_my_event = (backlight_event_format_t *)app_backlight_gatt_data;

    if(BACKLIGHT_CMD_SET_BACKLIGHT_MODE == p_my_event->event){
        u16 my_backlight_mode = MAKE_U16(p_my_event->data_hi, p_my_event->data_lo);
        switch (my_backlight_mode) {
            case BACKLIGHT_MODE_NAVER:
                break;
            case BACKLIGHT_MODE_ALWAYS:
                my_allow = 1;
                break;
            default:
                printf("[ERROR] INVALID BACkLIGHT DATA");
                array_printf((u8 *)app_backlight_gatt_data, sizeof(app_backlight_gatt_data));
                break;
        }

    }else if(BACKLIGHT_CMD_SET_TIME == p_my_event->event){
        u16 my_night_start_time = MAKE_U16(BACKLIGHT_NIGHT_START_HOUR, BACKLIGHT_NIGHT_START_MINUTE);
        u16 my_night_end_time = MAKE_U16(BACKLIGHT_NIGHT_END_HOUR, BACKLIGHT_NIGHT_END_MINUTE);

        backlight_time_format_t *p_my_time = (backlight_time_format_t *)app_backlight_gatt_data;
        u16 my_current_time = MAKE_U16(p_my_time->hour, p_my_time->minute);

        if( (my_current_time >= my_night_start_time) || (my_current_time <= my_night_end_time)){
            my_allow = 1;
        }
    }else{
        printf("[ERROR] INVALID BACkLIGHT DATA");
        array_printf((u8 *)app_backlight_gatt_data, sizeof(app_backlight_gatt_data));
    }


    return my_allow;
}

//#define TEST_BACKLIGHT_GATT

#ifdef TEST_BACKLIGHT_GATT

void app_backlightr_gatt_test_write_to_cb(u8 *p_buf, u8 buf_size){

    rf_packet_att_data_t my_test_pkt;

    my_test_pkt.l2cap = 3+buf_size;
    memcpy(my_test_pkt.dat, p_buf, buf_size);

    app_backlight_gatt_write_cb(&my_test_pkt);
}



typedef struct {
    unsigned char len;
    unsigned char data[19];
}test_cmd_t;

void app_backlight_gatt_test_code(void){
//    int app_backlight_gatt_write_cb(void * p){

    test_cmd_t my_test_cmd_table[] = {
        {2, {0x01, 0x02}       },           // LEN error
        {3, {0x02, 0x02, 0x00} },           // EVENT ID error
        {3, {0x00, 0x00, 0x03} },           // mode data error
        {3, {0x00, 0x00, 0x00} },           // set mode to NAVER
        {3, {0x00, 0x00, 0x01} },           // set mode to ALWAYS

        {3, {0x01, 0x00, 0x03} },           // set time error0. Before setting the time, you need to set it to DAYTIME_NIGHTTIME mode

        {3, {0x00, 0x00, 0x02} },           // DAYTIME_NIGHTTIME
        {3, {0x01, 24,   0x03} },           // set time error1. hour out of range

        {3, {0x01, 0x00, 0x01} },           // set time error2. Before setting the time, you need to set it to DAYTIME_NIGHTTIME mode

        {3, {0x00, 0x00, 0x02} },           // DAYTIME_NIGHTTIME
        {3, {0x01, 0x00, 60}   },           // set time error3. minute out of range

        {3, {0x00, 0x00, 0x02} },           // DAYTIME_NIGHTTIME
        {3, {0x01, 0x01, 0x02} },           // set time

        {3, {0x00, 0x00, 0x02} },           // DAYTIME_NIGHTTIME
    };

    printf("########################### TEST ##########################\n");

    for(int i=0;i<ARRAY_SIZE(my_test_cmd_table);i++){
        printf("\nCASE %d:\n", i);
        app_backlightr_gatt_test_write_to_cb(my_test_cmd_table[i].data, my_test_cmd_table[i].len);
    }

    printf("########################### END ##########################\n");

//    extern void app_backlight_record_trigger_test(void);
//    app_backlight_record_trigger_test();


}

#endif
