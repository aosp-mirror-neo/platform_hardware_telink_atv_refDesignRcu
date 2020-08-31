/******************************************************************************
 * @file     keyboard.c
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

#include "tl_common.h"
#include "drivers.h"
#include "keyboard.h"
#include "../usbstd/usbkeycode.h"
#include "../../vendor/827x_ble_remote/app_custom.h"

#define KB_DRIVE_PIN_MAX_NUM   KB_DRIVE_PIN_G20
#define KB_DRIVE_PIN_G10       3   
#define KB_DRIVE_PIN_G20       6   
#define KB_SCAN_PIN            8


_attribute_data_retention_ u32 drive_pins[KB_DRIVE_PIN_MAX_NUM];
u32 scan_pins[] = KB_SCAN_PINS;




#if (STUCK_KEY_PROCESS_ENABLE)
u32 stuckkey_keypresstimer=0;
unsigned char stuckKeyPress[KB_DRIVE_PIN_MAX_NUM];
#endif

kb_data_t    kb_event;
kb_data_t    kb_event_cache;
unsigned char  deepback_key_state;
u32 deepback_key_tick;

#ifndef        SCAN_PIN_50K_PULLUP_ENABLE
#define        SCAN_PIN_50K_PULLUP_ENABLE        0
#endif

#ifndef        KB_MAP_DEFAULT
#define        KB_MAP_DEFAULT        1
#endif

#ifndef        KB_LINE_MODE
#define        KB_LINE_MODE        0
#endif

#ifndef        KB_LINE_HIGH_VALID
#define        KB_LINE_HIGH_VALID        1
#endif

#ifndef        KB_KEY_FLASH_PIN_MULTI_USE
#define        KB_KEY_FLASH_PIN_MULTI_USE        0
#endif

#ifndef        KB_HAS_CTRL_KEYS
#define        KB_HAS_CTRL_KEYS        1
#endif

#ifndef        KB_RM_GHOST_KEY_EN
#define        KB_RM_GHOST_KEY_EN        0
#endif

#ifndef        KB_HAS_FN_KEY
#define        KB_HAS_FN_KEY        1
#endif

#ifndef        KB_DRV_DELAY_TIME
#define        KB_DRV_DELAY_TIME        10
#endif


#if  KB_REPEAT_KEY_ENABLE

#ifndef        KB_REPEAT_KEY_INTERVAL_MS
#define        KB_REPEAT_KEY_INTERVAL_MS        200
#endif
#ifndef        KB_REPEAT_KEY_NUM
#define        KB_REPEAT_KEY_NUM                4
#endif
static const unsigned char kb_map_repeat[KB_REPEAT_KEY_NUM] = KB_MAP_REPEAT;

repeatKey_t repeat_key = {
    0,
    0,
    0,
    0,
    U32_MAX,
};

#endif



_attribute_data_retention_ unsigned char *kb_p_map[1];
_attribute_data_retention_ unsigned char *kb_k_mp;



u32    scan_pin_need;

static unsigned char     kb_is_fn_pressed = 0;


extern u8 *p_kb_map_normal;
extern const unsigned int Kb_Drive_Pins_G10[];
void kb_p_map_init(void)
{
    u8 len = app_custom_get_drive_len();
    kb_p_map[0] = p_kb_map_normal;//p_kb_map_normal;
	memcpy(drive_pins,Kb_Drive_Pins_G10,len*4);
	//kb_k_mp = kb_p_map[0];
}

void kb_rmv_ghost_key(u32 * pressed_matrix){
    u32 mix_final = 0;
	u8 len = app_custom_get_drive_len();
    for(u8 i=0;i<len;i++){
        for(int j = (i+1); j < len; ++j){
            u32 mix = (pressed_matrix[i] & pressed_matrix[j]);
            //four or three key at "#" is pressed at the same time, should remove ghost key
            if( mix && (!BIT_IS_POW2(mix) || (pressed_matrix[i] ^ pressed_matrix[j])) ){
                // remove ghost keys
                //pressed_matrix[i] &= ~mix;
                //pressed_matrix[j] &= ~mix;
                mix_final |= mix;
            }
        }
        pressed_matrix[i] &= ~mix_final;
    }
}

#if (LONG_PRESS_KEY_POWER_OPTIMIZE)
int key_matrix_same_as_last_cnt = 0;  //record key matrix no change cnt
#endif

unsigned int key_debounce_filter( u32 mtrx_cur[], u32 filt_en ){
    u32 kc = 0;
#if (LONG_PRESS_KEY_POWER_OPTIMIZE)
    unsigned char matrix_differ = 0;
#endif
    static u32 mtrx_pre[KB_DRIVE_PIN_MAX_NUM];
    static u32 mtrx_last[KB_DRIVE_PIN_MAX_NUM];
    u8 len = app_custom_get_drive_len();
    for(u8 i=0;i<len;i++){
        u32 mtrx_tmp = mtrx_cur[i];
#if (STUCK_KEY_PROCESS_ENABLE)
        stuckKeyPress[i] = mtrx_tmp ? 1 : 0;
#endif
        if( filt_en ){
            //mtrx_cur[i] = (mtrx_last[i] ^ mtrx_tmp) ^ (mtrx_last[i] | mtrx_tmp);  //key_matrix_pressed is valid when current and last value is the same
            mtrx_cur[i] = ( ~mtrx_last[i] & (mtrx_pre[i] & mtrx_tmp) ) | ( mtrx_last[i] & (mtrx_pre[i] | mtrx_tmp) );
        }
        if ( mtrx_cur[i] != mtrx_last[i] ) {
            kc = 1;
        }
#if (LONG_PRESS_KEY_POWER_OPTIMIZE)
        if(mtrx_cur[i]^mtrx_pre[i]){  //when same, XOR value is 0
            matrix_differ = 1;
        }
#endif
        mtrx_pre[i] = mtrx_tmp;
        mtrx_last[i] = mtrx_cur[i];
    }

#if (LONG_PRESS_KEY_POWER_OPTIMIZE)
    if(matrix_differ){
        key_matrix_same_as_last_cnt = 0;
    }
    else{
        key_matrix_same_as_last_cnt++;
    }
#endif

    return kc;
}


// input:          pressed_matrix,
// key_code:   output keys array
// key_max:    max keys should be returned
static inline void kb_remap_key_row(int drv_ind, u32 m, int key_max, kb_data_t *kb_data){

    u8 a[8][3]={{0},{0},{0},{0},{0},{0},{0},{0}};
	u8 b[8][6]={{0},{0},{0},{0},{0},{0},{0},{0}};
	unsigned char kc;
    u8 len = app_custom_get_drive_len();
	if(len == KB_DRIVE_PIN_G10)
        memcpy(a,kb_k_mp,8*3);
	else
		memcpy(b,kb_k_mp,8*6);
	
    foreach_arr(i, scan_pins){
        if(m & 0x01){
			if(len == KB_DRIVE_PIN_G10)
                kc = (a[i][drv_ind]);//(kb_k_mp[i][drv_ind]);
            else
				kc = (b[i][drv_ind]);//(kb_k_mp[i][drv_ind]);
#if(KB_HAS_CTRL_KEYS)

            if(kc >= VK_CTRL && kc <= VK_RWIN)
                kb_data->ctrl_key |= BIT(kc - VK_CTRL);
            //else if(kc == VK_MEDIA_END)
                //lock_button_pressed = 1;
            else if(VK_ZOOM_IN == kc || VK_ZOOM_OUT == kc){
                kb_data->ctrl_key |= VK_MSK_LCTRL;
                kb_data->keycode[kb_data->cnt++] = (VK_ZOOM_IN == kc)? VK_EQUAL : VK_MINUS;
            }
            else if(kc != VK_FN)//fix fn ghost bug
                kb_data->keycode[kb_data->cnt++] = kc;

#else
            kb_data->keycode[kb_data->cnt++] = kc;
#endif
            if(kb_data->cnt >= key_max){
                break;
            }
        }
        m = m >> 1;
        if(!m){
            break;
        }
    }
}

static inline void kb_remap_key_code(u32 * pressed_matrix, int key_max, kb_data_t *kb_data, int numlock_status){

    kb_k_mp = kb_p_map[(numlock_status&1) | (kb_is_fn_pressed << 1)];
	u8 len = app_custom_get_drive_len();
    for(u8 i=0;i<len;i++){
        u32 m = pressed_matrix[i];
        if(!m) continue;
        kb_remap_key_row(i, m, key_max, kb_data);
        if(kb_data->cnt >= key_max){
            break;
        }
    }
}


u32 kb_scan_row(int drv_ind, unsigned char * gpio){
    u8 a[8][3]={{0},{0},{0},{0},{0},{0},{0},{0}};
	u8 b[8][6]={{0},{0},{0},{0},{0},{0},{0},{0}};

    u8 len = app_custom_get_drive_len();
	if(len == KB_DRIVE_PIN_G10)
        memcpy(a,kb_k_mp,8*3);
	else
		memcpy(b,kb_k_mp,8*6);
	
    unsigned char sr = irq_disable();
#if    (KB_KEY_FLASH_PIN_MULTI_USE)
    MSPI_AS_GPIO;
#endif

#if(!KB_LINE_MODE)
    u32 drv_pin = drive_pins[drv_ind];
    gpio_write(drv_pin, KB_LINE_HIGH_VALID);
    gpio_set_output_en(drv_pin, 1);
#endif

    u32 matrix = 0;
    foreach_arr(j, scan_pins){
        if(scan_pin_need & BIT(j)){
            int key = !gpio_read_cache (scan_pins[j], gpio);
            if(KB_LINE_HIGH_VALID != key) {
				if(len == KB_DRIVE_PIN_G10)
				{
				    if ((KB_HAS_FN_KEY && a[j][drv_ind]) == VK_FN) {
                    kb_is_fn_pressed = 1;
                    }
				}
				else
				{
				    if ((KB_HAS_FN_KEY && b[j][drv_ind]) == VK_FN) {
                    kb_is_fn_pressed = 1;
                    }
				}
                
                matrix |= (1 << j);
            }
        }
    }
    //sleep_us(KB_DRV_DELAY_TIME);
    gpio_read_all (gpio);
    /*
     * set as spi mode  if using spi flash pin
     */
#if    (KB_KEY_FLASH_PIN_MULTI_USE)
    MSPI_AS_SPI;
#endif

#if(!KB_LINE_MODE)
    ////////        float drive pin    ////////////////////////////
    //sleep_us(KB_SCAN_DELAY_TIME);
    gpio_write(drv_pin, 0);
    gpio_set_output_en(drv_pin, 0);
#endif

    irq_restore(sr);
    return matrix;
}

u32     matrix_buff[4][KB_DRIVE_PIN_MAX_NUM];
int     matrix_wptr, matrix_rptr;


u32 kb_key_pressed(unsigned char * gpio)
{
    u8 len = app_custom_get_drive_len();
    for(u8 i=0;i<len;i++){
        gpio_write(drive_pins[i], KB_LINE_HIGH_VALID);
        gpio_set_output_en(drive_pins[i], 1);
    }
    sleep_us (20);
    gpio_read_all (gpio);

    u32 ret = 0;
    static unsigned char release_cnt = 0;
    static u32 ret_last = 0;

    foreach_arr(i,scan_pins){
        if(KB_LINE_HIGH_VALID != !gpio_read_cache (scan_pins[i], gpio)){
            ret |= (1 << i);
            release_cnt = 6;
            ret_last = ret;
        }
        //ret = ret && gpio_read(scan_pins[i]);
    }
    if(release_cnt){
        ret = ret_last;
        release_cnt--;
    }
    for(u8 i=0;i<len;i++)
    {
        gpio_write(drive_pins[i], 0);
        gpio_set_output_en(drive_pins[i], 0);
    }
    return ret;
}

u32 kb_scan_key_value (int numlock_status, int read_key,unsigned char * gpio)
{
        kb_event.cnt = 0;
        kb_event.ctrl_key = 0;
        kb_is_fn_pressed = 0;

        u32 pressed_matrix[KB_DRIVE_PIN_MAX_NUM] = {0};
        kb_k_mp = kb_p_map[0];

        kb_scan_row (0, gpio);
		u8 len = app_custom_get_drive_len();
        for (int i=0; i<=len; i++) {
            u32 r = kb_scan_row (i < len ? i : 0, gpio);
            if (i) {
                pressed_matrix[i - 1] = r;
            }
        }

#if(KB_RM_GHOST_KEY_EN)
        kb_rmv_ghost_key(&pressed_matrix[0]);
#endif

        u32 key_changed = key_debounce_filter( pressed_matrix, \
                        (numlock_status & KB_NUMLOCK_STATUS_POWERON) ? 0 : 1);
       if(key_changed)
         printf("key_changed === 1\r\n");

#if (KB_REPEAT_KEY_ENABLE)
        if(key_changed){
            repeat_key.key_change_flg = KEY_CHANGE;
            repeat_key.key_change_tick = clock_time();
        }
        else{
            if(repeat_key.key_change_flg == KEY_CHANGE){
                repeat_key.key_change_flg = KEY_SAME;
            }

            if( repeat_key.key_change_flg == KEY_SAME &&  repeat_key.key_repeat_flg && \
                clock_time_exceed(repeat_key.key_change_tick,(KB_REPEAT_KEY_INTERVAL_MS-5)*1000)){
                repeat_key.key_change_tick = clock_time();
                key_changed = 1;
            }
        }
#endif

        ///////////////////////////////////////////////////////////////////
        //    insert buffer here
        //       key mapping requires NUMLOCK status
        ///////////////////////////////////////////////////////////////////
        u32 *pd;
        if (key_changed) {

            /////////// push to matrix buffer /////////////////////////
            pd = matrix_buff[matrix_wptr&3];
			u8 len = app_custom_get_drive_len();
            for (int k=0; k<len; k++) {
                *pd++ = pressed_matrix[k];
            }
            matrix_wptr = (matrix_wptr + 1) & 7;
            if ( ((matrix_wptr - matrix_rptr) & 7) > 4 ) {    //overwrite older data
                matrix_rptr = (matrix_wptr - 4) & 7;
            }
        }

        if (numlock_status & KB_NUMLOCK_STATUS_INVALID) {
            return 1;        //return empty key
        }

        ////////// read out //////////
        if (matrix_wptr == matrix_rptr || !read_key) {
            return 0;            //buffer empty, no data
        }
        pd = matrix_buff[matrix_rptr&3];
        matrix_rptr = (matrix_rptr + 1) & 7;

        ///////////////////////////////////////////////////////////////////
        kb_remap_key_code(pd, KB_RETURN_KEY_MAX, &kb_event, numlock_status);
        extern u8 TwoKeyPressFlag;
        if((TwoKeyPressFlag == 0x55) && (kb_event.cnt == 1))
        {
             printf("filter\r\n");
             return 0;
        }

#if (KB_REPEAT_KEY_ENABLE)
        if(repeat_key.key_change_flg == KEY_CHANGE){
            repeat_key.key_repeat_flg = 0;

            if(kb_event.cnt == 1){ //handle one key repeat only
                for(int i=0;i<KB_REPEAT_KEY_NUM;i++){
                    if(kb_event.keycode[0] == kb_map_repeat[i]){
                        repeat_key.key_repeat_flg = 1;
                        break;
                    }
                }
            }
        }
#endif

        return 1;
}

void kb_wake_config(void)
{
    u8 len=0;
	
    len = app_custom_get_drive_len();
	printf("len =%x\r\n",len);
    for(u8 i=0;i<len;i++)
    {
	    cpu_set_gpio_wakeup (drive_pins[i], !KB_LINE_HIGH_VALID,1);  //drive pin pad high wakeup deepsleep
    }
		
}

u8 kb_powerdet(void)
{
    unsigned char gpio[8],i;
    u32 ret = 0;

    for(i=0;i<2;i++)
    {
        gpio_write(drive_pins[1], KB_LINE_HIGH_VALID);
        gpio_set_output_en(drive_pins[1], 1);
        sleep_us (600);
        gpio_read_all (gpio);

        foreach_arr(j,scan_pins){
            if(KB_LINE_HIGH_VALID != !gpio_read_cache (scan_pins[j], gpio)){
                ret = (1 << j);
            }
        }
        gpio_write(drive_pins[1], 0);
        gpio_set_output_en(drive_pins[1], 0);
        if(ret != 4)
            return 0;
    }

    return 1;
}



///////////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////////
