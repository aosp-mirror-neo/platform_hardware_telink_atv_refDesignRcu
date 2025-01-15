/*
 * app_backlight.c
 *
 *  Created on: 2023-12-1
 *      Author: name
 */

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "app_backlight.h"
#include "app_backlight_gatt.h"
#include "../app_ui.h"

typedef struct {
    u16 start_value;
    u16 end_value;
    s16 step;
    /* duration is a count of BACKLIGHT_REFRESH_INTERVAL */
    u16 duration;
}light_action_t;

typedef struct {
    u8 table_pos;
    u8 rsv0;
    u16 duration_cnt;
    u16 brightness;
    u16 rsv1;

}app_backlight_ctr_t;

#define EN_BACKLIGHT_DEBUG

#ifdef EN_BACKLIGHT_DEBUG
    #define BACKLIGHT_DEBUG                             printf
    #define BACKLIGHT_DEBUG_ARRAY                      	array_printf
#else
    #define BACKLIGHT_DEBUG(...)
    #define BACKLIGHT_DEBUG_ARRAY(...)
#endif


#define get_clock_time()                            (clock_time()|1)


#define BACKLIGHT_REFRESH_INTERVAL                  (20)            // ms

#define BACKLIGHT_PWM_GPIO                          (GPIO_PC5)
#define BACKLIGHT_PWM_ID                            (PWM3_ID)
#define BACKLIGHT_PWM_CLOCK                         CLOCK_SYS_CLOCK_HZ
#define BACKLIGHT_PWM_FREQUENCY                     (600)           // hz
#define BACKLIGHT_PWM_CYCLE_TICK                    (CLOCK_SYS_CLOCK_HZ/BACKLIGHT_PWM_FREQUENCY)     // hz
#define BACKLIGHT_PWM_MIN_CMP_TICK_PERCENTAGE       (0)             // %
#define BACKLIGHT_PWM_MAX_CMP_TICK_PERCENTAGE       (40)            // %


/**
 * Step 1. Getting brighter 0.5sec
 * Step 2. Maximum brightness 2.0sec with duty 40%
 * Step 3. Getting darker 2.5sec
 * */
#define BACKLIGHT_TIME_RISE                         (500)           // 500ms
#define BACKLIGHT_TIME_MAINTAIN                     (2000)          // 1000ms
#define BACKLIGHT_TIME_DECLINE                      (2500)          // 1500ms

#define BACKLIGHT_START_BRIGHTNESS                  (BACKLIGHT_PWM_CYCLE_TICK * BACKLIGHT_PWM_MIN_CMP_TICK_PERCENTAGE / 100)
#define BACKLIGHT_END_BRIGHTNESS                    (BACKLIGHT_PWM_CYCLE_TICK * BACKLIGHT_PWM_MAX_CMP_TICK_PERCENTAGE / 100)
#define BACKLIGHT_RISE_STEP                         ((BACKLIGHT_END_BRIGHTNESS - BACKLIGHT_START_BRIGHTNESS) / (BACKLIGHT_TIME_RISE / BACKLIGHT_REFRESH_INTERVAL))
#define PWM_DECLINE_STEP                            ((BACKLIGHT_START_BRIGHTNESS - BACKLIGHT_END_BRIGHTNESS) / (BACKLIGHT_TIME_DECLINE / BACKLIGHT_REFRESH_INTERVAL))


const u16 sin_table[] = {

#if(1)
    /*  range: 0~PI/2 */
    0,     1040,  2079, 3118,   4156,  5194,  6229,  7264,  8296,  9327,
    10355, 11380, 12403, 13422, 14438, 15450, 16459, 17463, 18463, 19459,
    20449, 21434, 22414, 23389, 24357, 25319, 26275, 27224, 28167, 29102,
    30030, 30950, 31863, 32768, 33664, 34552, 35431, 36301, 37162, 38014,
    38856, 39689, 40511, 41323, 42125, 42916, 43697, 44466, 45224, 45971,
    46706, 47430, 48141, 48841, 49528, 50203, 50865, 51514, 52150, 52773,
    53383, 53980, 54562, 55132, 55687, 56228, 56755, 57268, 57766, 58250,
    58719, 59173, 59613, 60037, 60446, 60841, 61219, 61583, 61931, 62263,
    62580, 62880, 63165, 63435, 63688, 63925, 64146, 64351, 64539, 64712,
    64868, 65008, 65131, 65238, 65329, 65403, 65461, 65502, 65527, 65535,
#else
    /*  range: -PI/ ~ PI/2 */
    0,     16,    66,    148,   264,   412,   592,   805,   1050,  1327,
    1636,  1976,  2347,  2749,  3181,  3643,  4134,  4654,  5202,  5778,
    6381,  7011,  7666,  8347,  9053,  9782,  10534, 11309, 12106, 12923,
    13760, 14617, 15492, 16384, 17292, 18217, 19155, 20108, 21073, 22050,
    23038, 24036, 25042, 26056, 27077, 28104, 29136, 30171, 31208, 32248,
    33287, 34327, 35364, 36399, 37431, 38458, 39479, 40493, 41499, 42497,
    43485, 44462, 45427, 46380, 47318, 48243, 49151, 50043, 50918, 51775,
    52612, 53429, 54226, 55001, 55753, 56482, 57188, 57869, 58524, 59154,
    59757, 60333, 60881, 61401, 61892, 62354, 62786, 63188, 63559, 63899,
    64208, 64485, 64730, 64943, 65123, 65271, 65387, 65469, 65519, 65535,

#endif
};


const light_action_t s_backlight_behavior[] = {
    {BACKLIGHT_START_BRIGHTNESS,    BACKLIGHT_END_BRIGHTNESS,   BACKLIGHT_RISE_STEP,    BACKLIGHT_TIME_RISE/BACKLIGHT_REFRESH_INTERVAL},
    {BACKLIGHT_END_BRIGHTNESS,      BACKLIGHT_END_BRIGHTNESS,   0,                      BACKLIGHT_TIME_MAINTAIN/BACKLIGHT_REFRESH_INTERVAL},
    {BACKLIGHT_END_BRIGHTNESS,      BACKLIGHT_START_BRIGHTNESS, PWM_DECLINE_STEP,       BACKLIGHT_TIME_DECLINE/BACKLIGHT_REFRESH_INTERVAL}
};


_attribute_data_retention_ u8 s_en_backlight = 0;
_attribute_data_retention_ u16 s_boost_gpio = BOOST_IC_EN_GPIO;
_attribute_data_retention_ app_backlight_ctr_t app_backlight_ctr = {0};
static u32 s_backlight_task_tick = 0;

void app_backlight_set_ability(u8 en, u16 boost_pin){

    s_en_backlight = (0xff==en)?0:en;

    if(0 == s_en_backlight) return;

    switch (boost_pin) {
        case GPIO_PA0:
        case GPIO_PA1:
        case GPIO_PA2:
        case GPIO_PA3:
        case GPIO_PA4:
        case GPIO_PA5:
        case GPIO_PA6:
        case GPIO_PA7:
        case GPIO_PB0:
        case GPIO_PB1:
        case GPIO_PB2:
        case GPIO_PB3:
        case GPIO_PB4:
        case GPIO_PB5:
        case GPIO_PB6:
        /* GPIO_PB7 is used for battery detection */
        case GPIO_PC0:
        /* GPIO_PC1 is used for AMIC input */
        case GPIO_PC2:
        case GPIO_PC3:
        case GPIO_PC4:
        /* GPIO_PC5 is used for on/off backlight */
        case GPIO_PC6:
        case GPIO_PC7:
        case GPIO_PD0:
        case GPIO_PD1:
        case GPIO_PD2:
        case GPIO_PD3:
        case GPIO_PD4:
        case GPIO_PD5:
        case GPIO_PD6:
        case GPIO_PD7:
            s_boost_gpio = boost_pin;
            break;
        case 0xFFFF:
            s_boost_gpio = BOOST_IC_EN_GPIO;
            break;
        default:
            s_en_backlight = 0;
            break;
    }

    BACKLIGHT_DEBUG("BACKLIGHT SETTINGS. EN[0x%02X] GPIO:[0x%04X]\n", s_en_backlight, s_boost_gpio);
}

u16 app_backlight_get_boost_pin(void){

    u16 my_boost_pin = s_boost_gpio;
    switch (my_boost_pin) {
        case GPIO_PA0:
        case GPIO_PA1:
        case GPIO_PA2:
        case GPIO_PA3:
        case GPIO_PA4:
        case GPIO_PA5:
        case GPIO_PA6:
        case GPIO_PA7:
        case GPIO_PB0:
        case GPIO_PB1:
        case GPIO_PB2:
        case GPIO_PB3:
        case GPIO_PB4:
        case GPIO_PB5:
        case GPIO_PB6:
        /* GPIO_PB7 is used for battery detection */
        case GPIO_PC0:
        /* GPIO_PC1 is used for AMIC input */
        case GPIO_PC2:
        case GPIO_PC3:
        case GPIO_PC4:
        /* GPIO_PC5 is used for on/off backlight */
        case GPIO_PC6:
        case GPIO_PC7:
        case GPIO_PD0:
        case GPIO_PD1:
        case GPIO_PD2:
        case GPIO_PD3:
        case GPIO_PD4:
        case GPIO_PD5:
        case GPIO_PD6:
        case GPIO_PD7:
            break;
        default:
            s_en_backlight = 0;
            my_boost_pin = 0;
            break;
    }

    return my_boost_pin;
}


u8 app_backlight_is_busy(void){

    return s_backlight_task_tick?1:0;
}

void app_backlight_init(void){
    BACKLIGHT_DEBUG("%s\n", __FUNCTION__);

    /* enable boost */
    u16 my_boot_pin = app_backlight_get_boost_pin();
    if(0 == my_boot_pin) return;

    gpio_set_func(my_boot_pin, AS_GPIO);
    gpio_set_input_en(my_boot_pin, 0);
    gpio_set_output_en(my_boot_pin, 1);

    gpio_setup_up_down_resistor(my_boot_pin, PM_PIN_PULLUP_10K);
    gpio_write(s_boost_gpio, 1);

    //    pwm_set_clk(CLOCK_SYS_CLOCK_HZ, BACKLIGHT_PWM_CLOCK);
    app_ui_init_pwm_clock();
    gpio_set_func(BACKLIGHT_PWM_GPIO, AS_PWM3_N);
    pwm_set_mode(BACKLIGHT_PWM_ID, PWM_NORMAL_MODE);
    pwm_n_revert(PWM3_ID);
}

void app_backlight_stop(void){

    if(0 == s_en_backlight) return;

    pwm_stop(BACKLIGHT_PWM_ID);

    u16 my_boot_pin = app_backlight_get_boost_pin();
    if(0 == my_boot_pin) return;

    gpio_setup_up_down_resistor(my_boot_pin, PM_PIN_UP_DOWN_FLOAT);
    gpio_write(s_boost_gpio, 0);
}


void app_backlight_start(void){

    if(0 == s_en_backlight) return;
    if(0 == app_backlight_gatt_is_allow_backlight()) return;


    BACKLIGHT_DEBUG("%s\n", __FUNCTION__);

    if(s_backlight_task_tick){
        // backlight already on
        app_backlight_ctr.table_pos = 1;
    }else{
        // backlight fist on
        app_backlight_ctr.table_pos = 0;
        app_backlight_init();
        /* When initialization fails, backlight disable may be set in app_backlight_init */
        if(0 == s_en_backlight) return;
    }

    app_backlight_ctr.duration_cnt = 0;
//    app_backlight_ctr.s_backlight_behavior_table = (light_action_t *)s_backlight_behavior;

    app_backlight_ctr.brightness = s_backlight_behavior[app_backlight_ctr.table_pos].start_value;

    s_backlight_task_tick = get_clock_time();

    // TODO start PWM
    pwm_set_cycle_and_duty(BACKLIGHT_PWM_ID, BACKLIGHT_PWM_CYCLE_TICK, app_backlight_ctr.brightness);

//    BACKLIGHT_DEBUG("song_note_index: %d cycle: %d counter: %d duty: %d\n",s_app_buzzer_ctrl.song_note_index, note ,counter, s_app_buzzer_ctrl.sound_level);
    pwm_start(BACKLIGHT_PWM_ID);

    bls_pm_setSuspendMask (SUSPEND_DISABLE);
}


void app_backlight_task(void){

    if(0 == s_en_backlight) return;

    if(s_backlight_task_tick && clock_time_exceed(s_backlight_task_tick, BACKLIGHT_REFRESH_INTERVAL*1000)){
        s_backlight_task_tick = get_clock_time();
    }else{
        return;
    }

    app_backlight_ctr.duration_cnt++;

    //    s32 my_brightness = app_backlight_ctr.brightness;


    if(app_backlight_ctr.duration_cnt >= s_backlight_behavior[app_backlight_ctr.table_pos].duration){
        app_backlight_ctr.duration_cnt = 0;
        app_backlight_ctr.table_pos++;
        app_backlight_ctr.brightness = s_backlight_behavior[app_backlight_ctr.table_pos].start_value;
    }

    if(app_backlight_ctr.table_pos >= ARRAY_SIZE(s_backlight_behavior)){
        // TODO disable
        // TODO Boost disable
        app_backlight_stop();
        s_backlight_task_tick = 0;
        return;
    }

    u16 my_start_value = s_backlight_behavior[app_backlight_ctr.table_pos].start_value;
    u16 my_end_value = s_backlight_behavior[app_backlight_ctr.table_pos].end_value;
    u16 my_duration = s_backlight_behavior[app_backlight_ctr.table_pos].duration;

//    s32 my_brightness = app_backlight_ctr.brightness;
//    my_brightness += s_backlight_behavior[app_backlight_ctr.table_pos].step;

    s32 my_brightness = s_backlight_behavior[app_backlight_ctr.table_pos].start_value + (my_end_value- my_start_value)*sin_table[app_backlight_ctr.duration_cnt*100/my_duration]/U16_MAX;

    BACKLIGHT_DEBUG("duration_cnt:[%d]/[%d] table_pos:[%d] my_brightness:[%d] brightness:[%d]\n",
            app_backlight_ctr.duration_cnt,
            s_backlight_behavior[app_backlight_ctr.table_pos].duration,
            app_backlight_ctr.table_pos,
            my_brightness,
            app_backlight_ctr.brightness);


    if( (s_backlight_behavior[app_backlight_ctr.table_pos].step >= 0) && (my_brightness > s_backlight_behavior[app_backlight_ctr.table_pos].end_value)){
        /* Brighten or maintain brightness stage */
        my_brightness = s_backlight_behavior[app_backlight_ctr.table_pos].end_value;
    }else if( (s_backlight_behavior[app_backlight_ctr.table_pos].step < 0) && (my_brightness < s_backlight_behavior[app_backlight_ctr.table_pos].end_value)){
        /* darkening stage */
        my_brightness = s_backlight_behavior[app_backlight_ctr.table_pos].end_value;
    }


//    BACKLIGHT_DEBUG("duration_cnt:[%d]/[%d] table_pos:[%d] my_brightness:[%d] brightness:[%d]\n", app_backlight_ctr.duration_cnt, app_backlight_ctr.table_pos, my_brightness, app_backlight_ctr.brightness);

    /* Refresh only if they are not equal */
    if(app_backlight_ctr.brightness != my_brightness){

        if( (my_brightness>=0) && (my_brightness<=BACKLIGHT_END_BRIGHTNESS)){
            app_backlight_ctr.brightness = my_brightness&0xFFFF;
        }else if(my_brightness>BACKLIGHT_END_BRIGHTNESS){
            app_backlight_ctr.brightness = BACKLIGHT_END_BRIGHTNESS;
        }else{
            app_backlight_ctr.brightness = 0;
        }

        pwm_set_cycle_and_duty(BACKLIGHT_PWM_ID, BACKLIGHT_PWM_CYCLE_TICK, app_backlight_ctr.brightness);
    }
}




u32 s_priv_trigger_tick = 0;

u32 s_y = 0;


#define CONSTANT_A                              (0.955)
#define CONSTANT_B                              (1-CONSTANT_A)

#define CONSTANT_THRESHOLD                      (0.900)


#define FIXED_A                                 int /* data type for fixed point accumulator */
#define FIXED_T                                 short /* data type for fixed point constants */
#define CHAR_BIT                                (8)

#define F_PROTO8(x)                             (FIXED_A) ((x * 2) * ( (FIXED_A)1 << (sizeof(FIXED_T) * CHAR_BIT - 1)) + 0.5)


#define CONSTANT_JUDGE_TIME                     (30000)     //ms
#define CONSTANT_JUDGE_COUNT                    (50)


inline u16 formula_filter(u8 x){
    u32 my_y = s_y*F_PROTO8(CONSTANT_A)/U16_MAX + x* F_PROTO8(CONSTANT_B);
    printf("A:[%d] B:[%d] Y:[%05d] RES:[%s] \n", F_PROTO8(CONSTANT_A), F_PROTO8(CONSTANT_B), my_y, (my_y>=F_PROTO8(CONSTANT_THRESHOLD))?"Y":"N");
    return my_y;
}

void app_backlight_record_trigger(void){

    u32 my_spend_time_cnt = 0;
    u32 my_spend_time_remainder = 1;

    u32 my_spend_time_ms = 0;
    if(s_priv_trigger_tick){
        my_spend_time_ms = (clock_time() - s_priv_trigger_tick)/CLOCK_16M_SYS_TIMER_CLK_1MS;
        my_spend_time_cnt = my_spend_time_ms/(CONSTANT_JUDGE_TIME/CONSTANT_JUDGE_COUNT);
        my_spend_time_remainder = my_spend_time_ms%(CONSTANT_JUDGE_TIME/CONSTANT_JUDGE_COUNT);

    }

    s_priv_trigger_tick = clock_time()|1;
//    printf("MS:[%d] CNT:[%d] REM:[%d] ", my_spend_time_ms, my_spend_time_cnt, my_spend_time_remainder);
    if(my_spend_time_cnt){
        for(int i=0;i<my_spend_time_cnt;i++){
            s_y = formula_filter(0);
        }
    }

    if(my_spend_time_remainder){
        s_y = formula_filter(1);
    }
}

#ifdef DEBUG_BACKLIGHT
void app_backlight_record_trigger_test(void){

    s_priv_trigger_tick = 0;
    s_y = 0;
    for(int i=0;i<50;i++){
        app_backlight_record_trigger();
        WaitMs(100);
    }

    printf("########################################\n");
    s_priv_trigger_tick = 0;
    s_y = 0;

    for(int i=0;i<100;i++){
        app_backlight_record_trigger();
        WaitMs(650);
    }

}
#endif
