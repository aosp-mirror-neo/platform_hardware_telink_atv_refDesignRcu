/******************************************************************************
 * @file     app_ir.c
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
#include "../common/blt_soft_timer.h"
#include "app_ir.h"
#include "rc_ir.h"
#include "app_ir.h"
#include "app_flash_write.h"
#include "app_ui.h"
#include "app_custom.h"
#include "../../stack/ble/host/attr/att.h"

#if APP_IR_OVER_BLE

extern void google_reset_rsp_delay(void);
extern u8 is_mic_enable(void);

//programming timeout 30s
#define PROGRAMMING_TIMEOUT 30000000

//Regularly query whether the current time can be erased
#define FLASH_ERASE_TIMEOUT 100000

#define INVALID_KEY_ID      0xFF00
#define MAX_CODE_LENGTH     300            /*  The maximum number of ir codes for each key. */
#define MAX_KEY_COUNT       5              /*  The maximum number of key for ir programming.*/

#define KEY_IDX_NULL        0xff

typedef struct{
    u16 key_id;                    //Key ID, comes from ATV (host keyid)
    u16 code_size;
    u8  button_idx;                //button index, based on enum in firmware
    u8  code[MAX_CODE_LENGTH];
}key_code_t;

typedef struct{
    u32 release_timer;
    u32 repeat_delay;
}ir_repeat_delay_t;

typedef struct {
    u8 programming_start;
    u8 programmed_key_count;
    u8 key_notification_flag;
    u16 current_programming_key_id;
}ir_programming_key_t;

typedef struct {
    u16 ir_code_pos;
    u16 ir_keycode_total_len;
    u8 ir_current_button;
}ir_keycode_merge_t;

typedef struct {
    u8  ir_save_en;
    u8  ir_save_button;
    u16 ir_save_pos;
}ir_save_t;


typedef struct {
    u8  duty_cycle;
    u16 carrier_frequency;
    u8  current_programming_key_send;
    u8  odd_or_even_press;
    u8  is_key_programming;
    u8  programming_timer;
    u8  ir_suppress[MAX_KEY_COUNT];
    ir_repeat_delay_t ir_repeat_delay;
    ir_programming_key_t ir_programming;
    ir_keycode_merge_t   ir_merge;
    ir_save_t ir_save;
}ir_app_parm_t;

typedef struct {
    u16 key_id;
    u8  button_idx;
    u8  button;
}key_button_pair_t;

typedef enum
{
    APP_NEC_IR_TABLE_PREPARE=1,
    APP_NEC_IR_TABLE_PREPARE_END,
    APP_NEC_IR_TABLE_START,
    APP_NEC_IR_TABLE_WRITEFAIL,
    APP_NEC_IR_TABLE_END
}app_nec_ir_table_sts;

_attribute_data_retention_ app_nec_ir_table_sts nec_ir_table_start = APP_NEC_IR_TABLE_END;

//keyid: receive from the host. button_idx: Corresponding to the key index in the memory ir_table[].button:The actual keyid corresponding to the board
const key_button_pair_t key_button_map_g10[MAX_KEY_COUNT] = {
    {0x0018, KEY_IDX_VOLUP, 0x10},
    {0x0019, KEY_IDX_VOLDN, 0x02},
    {0x00A4, KEY_IDX_MUTE, 0x16},
    {0x001A, KEY_IDX_POWER, 0 },
    {0x00B2, KEY_IDX_INPUT, 0x03}
};

//keyid: receive from the host. button_idx: Corresponding to the key index in the memory ir_table[].button:The actual keyid corresponding to the board
const key_button_pair_t key_button_map_g20[MAX_KEY_COUNT] = {
    {0x0018, KEY_IDX_VOLUP, 0x1f},
    {0x0019, KEY_IDX_VOLDN, 0x02},
    {0x00A4, KEY_IDX_MUTE,  0x2B},
    {0x001A, KEY_IDX_POWER, 0 },
    {0x00B2, KEY_IDX_INPUT, 0x06}
};

//The following address corresponds to the flash memory address written each time. Upto 5 keys can be written at a time, and each key occupies a maximum of 256 bytes.
const u32 if_flash_sect_addr[IR_DATA_SECT_MAX_NUM] ={IR_DATA_SECT_0_ADDR+256,IR_DATA_SECT_0_ADDR+1536,IR_DATA_SECT_0_ADDR+2816,IR_DATA_SECT_1_ADDR+256,IR_DATA_SECT_1_ADDR+1536,IR_DATA_SECT_1_ADDR+2816, \
                                                 IR_DATA_SECT_2_ADDR+256,IR_DATA_SECT_2_ADDR+1536,IR_DATA_SECT_2_ADDR+2816,IR_DATA_SECT_3_ADDR+256,IR_DATA_SECT_3_ADDR+1536,IR_DATA_SECT_3_ADDR+2816};


static _attribute_data_retention_ ir_app_parm_t  ir_app_parm={0};

static _attribute_data_retention_ key_code_t ir_table[MAX_KEY_COUNT]={{0},{0},{0},{0},{0}};
extern u32    ir_flash_erase_tick;


/*
    "ir_flash_index" is used to record the current ir code programming times.(The maxinum value of
    "ir_flash_index" is 12,when the value of "ir_flash_index" is 6,the erase operation must be performed.)
*/
_attribute_data_retention_ u8 ir_flash_index=0;

/*
    ir_flash_need_erase_sector:0 -> erase the IR_DATA_SECT_0_ADDR,IR_DATA_SECT_1_ADDR sector
    ir_flash_need_erase_sector:2 -> erase the IR_DATA_SECT_2_ADDR,IR_DATA_SECT_3_ADDR sector

*/
_attribute_data_retention_ u8 ir_flash_need_erase_sector=0;
_attribute_data_retention_ u8 ir_flash_erase_sector_step=0;

_attribute_data_retention_ u8 ir_flash_erase_flag=0;
_attribute_data_retention_ u8 ir_save_error_flag=0;


extern _attribute_data_retention_    u8 device_in_connection_state;
extern int     blt_soft_timer_delete(blt_timer_callback_t func);
extern int bls_ll_requestConnBrxEventDisable(void);
extern void  bls_ll_disableConnBrxEvent(void);
extern void  bls_ll_restoreConnBrxEvent(void);

#if(BLT_TEST_SOFT_TIMER_ENABLE == 0)
int blt_soft_timer_add(blt_timer_callback_t func, u32 interval_us)
{
}

int     blt_soft_timer_delete(blt_timer_callback_t func)
{
}

#endif

_attribute_data_retention_ app_ir_programming_end_Cb_t    app_ir_programming_end_Cb = NULL;

/**
 * @brief       This function servers to register the callback at the end of ir programming
 * @param[in]   Callback function that needs to be registered
 * @return      none
 */
void app_ir_programming_end_register(app_ir_programming_end_Cb_t cb)
{
    app_ir_programming_end_Cb = cb;
}

/**
 * @brief       This function set the programming flag
 * @param[in]   flag the progtamming key. 0x55:current key is a programming key 0:current key not is a propramming key
 * @return      none
 */
void programming_key_set(u8 data)
{
    ir_app_parm.is_key_programming = data;
}

/**
 * @brief       This function is used to determine whether the current key is a programming key
 * @param[in]   none
 * @return      1: programming key
 *              0: not a programming key
 */
u8 is_programming_key_send(void)
{
    if(ir_app_parm.is_key_programming == 0x55)
        return 1;
    else
        return 0;
}

/**
 * @brief       This function is used to get the corresponding button index by board keyid
 * @param[in]   button: keyid on the board
 * @return      index 0-4
 */
u8 ir_get_button_idx_from_button(u8 button)
{
    u8 i;
    for(i = 0; i < MAX_KEY_COUNT; i++)
    {
        if(app_custom_get_device_type() == REMOTE_G10)
        {
            if (key_button_map_g10[i].button == button)
            {
                return key_button_map_g10[i].button_idx;
            }
        }
        else
        {
            if (key_button_map_g20[i].button == button)
            {
                return key_button_map_g20[i].button_idx;
            }
        }
    }
    return KEY_IDX_NULL;
}

/**
 * @brief       This function is used to get the corresponding button index by host keyid
 * @param[in]   button: keyid issued by host
 * @return      index 0-4
 */
u8 ir_get_button_idx_from_key_id(u16 key_id)
{
    u8 i;
    for(i = 0; i < MAX_KEY_COUNT; i++)
    {
        if(app_custom_get_device_type() == REMOTE_G10)
        {
            if (key_button_map_g10[i].key_id == key_id)
            {
                return key_button_map_g10[i].button_idx;
            }
        }
        else
        {
            if (key_button_map_g20[i].key_id == key_id)
            {
                return key_button_map_g20[i].button_idx;
            }
        }
    }
    return KEY_IDX_NULL;
}

/**
 * @brief       This function is used to get the corresponding ir data by board keyid
 * @param[in]   key_idx: keyid on the board
 * @return      ir data
 */
key_code_t* ir_get_key_code_from_keyid_idx(u8 key_idx)
{
    u8 i;
    if(key_idx == 0) //powerkey
    {
        if(ir_table[KEY_IDX_POWER].button_idx == KEY_IDX_POWER)
        {
            ir_app_parm.current_programming_key_send = KEY_IDX_POWER;
            return &ir_table[KEY_IDX_POWER];
        }
    }
    else
    {
        for(i = 0; i < MAX_KEY_COUNT; i++)
        {
            if(ir_table[i].key_id == key_idx)
            {
                if(ir_table[i].button_idx == i)
                {
                    ir_app_parm.current_programming_key_send = i;
                    return &ir_table[i];
                }
            }
        }
    }
    return NULL;
}

/**
 * @brief       This function is used to get the corresponding button index by board keyid
 * @param[in]   key_idx: keyid on the board
 * @return      button index
 */
u8 ir_get_suppress_index_from_keyid_idx(u8 key_idx)
{
    u8 i;
    //printf("ir_app_parm.ir_programming.programmed_key_count=%x\r\n",ir_app_parm.ir_programming.programmed_key_count);
    if(key_idx == 0) //powerkey
    {
        if(ir_table[KEY_IDX_POWER].button_idx == KEY_IDX_POWER)
        {
            //printf("ir power key\r\n");
            return KEY_IDX_POWER;
        }
    }
    else
    {
        for(i = 0; i < MAX_KEY_COUNT; i++)
        {
            if(ir_table[i].key_id == key_idx)
            {
                if(ir_table[i].button_idx == i)
                {
                    //printf("button index i=%x\r\n",i);
                    return i;
                }
            }
        }
    }
    return KEY_IDX_NULL;
}

/**
 * @brief       This function is init the ir buffer
 * @param[in]   none
 * @return      none
 */
void ir_table_init(void)
{
    u8 i;
    memset(ir_table, 0, sizeof(key_code_t) * MAX_KEY_COUNT);
    for (i = 0; i < MAX_KEY_COUNT; i++)
    {
        ir_table[i].key_id = INVALID_KEY_ID;
    }
    ir_app_parm.ir_save.ir_save_en = 0;
    ir_app_parm.ir_save.ir_save_pos = 0;
    ir_app_parm.ir_save.ir_save_button = 0xff;
    ir_app_parm.programming_timer = NULL;
    ir_app_parm.ir_programming.programmed_key_count = 0;
    ir_app_parm.ir_programming.current_programming_key_id = INVALID_KEY_ID;
    ir_app_parm.ir_merge.ir_current_button = MAX_KEY_COUNT;
}


/*
1)A total of 6 flash sectors are used to store ir code. Each sector can store ir code
  programming three times. Each ir code programming occupies 5*256 bytes. 5 means that
  each programming supports up to 5 keys, 256 Indicates the maximum number of ir codes
  for each key.
2)The first 6 bytes of each sector are used to check whether the ir code programming
  data is valid for 3 times. If the value is 0xa5,0xxx, the ir code programming data for
  this time is valid. If the value is 0x00,0xxx this time The data is invalid
  (0xxx is a reserved byte, to be expanded).
3)The api "ir_flash_set_flag" is used for set the ir code data to invalid. The variable
  "ir_flash_index" is used to record the current ir code programming times.(The maxinum value of
  "ir_flash_index" is 12,when the value of "ir_flash_index" is 6,the erase operation must be performed.)
4)Use 16 bytes as a unit for flash write operation.
*/

#define APP_IR_DATA_HEAD          0xa5
#define APP_IR_DATA_HEAD_INVALID  0x0

/**
 * @brief       Write a flag to indicate that the ir data is invalid
 * @param[in]   0:indicate the ir data is invalid 1:indicate the ir data is valid
 * @return      none
 */
void ir_flash_set_flag(u8 flag)
{
    u8 sect= ir_flash_index/3;
    u8 sect_index = ir_flash_index%3;
    u8 data;

    //printf("ir_flash_set_flag =%x,ir_flash_index =%x\r\n",flag,ir_flash_index);
    u32 addr = IR_DATA_SECT_0_ADDR+sect*0x1000+sect_index*2;
    if(flag == 0)
    {
        data = APP_IR_DATA_HEAD_INVALID;
        flash_write_page(addr,1,&data);
    }
    else
    {
        data = APP_IR_DATA_HEAD;
        flash_write_page(addr,1,&data);
        //printf("ir_flash_set_flag addr =%x\r\n",addr);
    }
}

/**
 * @brief       Read the ir data from the flash
 * @param[in]   flash addr
 * @return      none
 */
void ir_flash_check(u32 addr)
{
    u8 button_index=0,i;
    u16 code_size=0;
    u8 buf[300];
    u16 key_id=0;

    for(i=0;i<MAX_KEY_COUNT;i++)
    {
        flash_read_page(addr+i*256, 256, buf);
        button_index = ir_get_button_idx_from_button(buf[0]);
        if (button_index == KEY_IDX_NULL)
        {
            continue;
        }
        code_size = *(u16*)&buf[2];
        key_id  = buf[0]|(buf[1]<<8);
        ir_table[button_index].key_id = key_id;
        ir_table[button_index].code_size =  code_size;
        ir_table[button_index].button_idx = button_index;
        if(code_size>256)
        {
            printf("addr=%x\r\n",addr-0x7000+i*256);
            flash_read_page(addr-0x7000+i*256, code_size-251, &buf[256]);
        }
        memcpy(ir_table[button_index].code, &buf[5], code_size);


        printf("button =%x \r\n",button_index);
        /*
        for(u16 k=0;k<code_size;k++ )
        {
            printf(" %x",ir_table[button_index].code[k]);
        }
        printf("\r\n");
        */
    }
}

/**
 * @brief       Read the flag from the flash,and find the address which backup the valid ir data
 * @param[in]   none
 * @return      none
 */
void ir_param_init(void)
{
    u32 addr;
    u8 data[6],i,j;
    u8 nodata_flag=0;
    u8 find_flag=0;

    ir_flash_index = IR_DATA_SECT_MAX_NUM;
    printf("ir flash head=\r\n");
    for(i=0;i<IR_DATA_SECT_NUM;i++)
    {
        flash_read_page(IR_DATA_SECT_0_ADDR+i*0x1000, 6, data);
        for(u8 k=0;k<6;k++)
            printf(" %x",data[k]);
        for(j=0;j<3;j++)
        {
            if(data[j*2] == APP_IR_DATA_HEAD)
            {
                ir_flash_index= i*3+j;
                nodata_flag = 0;
                find_flag = 1;
                break;
            }
            if(data[j*2] == APP_IR_DATA_HEAD_INVALID)
            {
                ir_flash_index = i*3+j;
                nodata_flag = 1;
            }
        }
        if(find_flag == 1) break;
    }
    if(ir_flash_index == IR_DATA_SECT_MAX_NUM)
    {
        ir_flash_index = 0;
        printf("\r\nno ir data\r\n");
        return;
    }

    if(ir_flash_index >= IR_DATA_NEED_ERASE_NUM)
    {
        flash_read_page(IR_DATA_SECT_0_ADDR,1,&data[0]);
        flash_read_page(IR_DATA_SECT_0_EXT_ADDR,1,&data[1]);
        flash_read_page(IR_DATA_SECT_1_EXT_ADDR,1,&data[2]);
        if(data[0] != 0xff)
        {
            printf("erase 1 part flash\r\n");
            flash_erase_sector(IR_DATA_SECT_0_ADDR);
            flash_erase_sector(IR_DATA_SECT_1_ADDR);
        }
        if(data[1]!= 0xff)
        {
            flash_erase_sector(IR_DATA_SECT_0_EXT_ADDR);
        }
        if(data[2]!= 0xff)
        {
            flash_erase_sector(IR_DATA_SECT_1_EXT_ADDR);
        }
    }
    if(ir_flash_index < IR_DATA_NEED_ERASE_NUM)
    {
        flash_read_page(IR_DATA_SECT_2_ADDR,1,&data[0]);
        flash_read_page(IR_DATA_SECT_2_EXT_ADDR,1,&data[1]);
        flash_read_page(IR_DATA_SECT_3_EXT_ADDR,1,&data[2]);
        if((data[0] != 0xff))
        {
            printf("erase 2 part flash\r\n");
            flash_erase_sector(IR_DATA_SECT_2_ADDR);
            flash_erase_sector(IR_DATA_SECT_3_ADDR);
        }
        if(data[1]!= 0xff)
        {
            flash_erase_sector(IR_DATA_SECT_2_EXT_ADDR);
        }
        if(data[2]!= 0xff)
        {
            flash_erase_sector(IR_DATA_SECT_3_EXT_ADDR);
        }
    }

    if(nodata_flag == 1)
    {
        printf("\r\nno ir data, but ir_flash_index =%x\r\n",ir_flash_index);
    }
    else
    {
        addr = if_flash_sect_addr[ir_flash_index];
        printf("current sector= %x app_ir_param_init addr=%x\r\n",ir_flash_index,addr);
        ir_flash_check(addr);
    }

}

void ir_flash_save_en(u8 button_index)
{
    ir_app_parm.ir_save.ir_save_en |= 1<<button_index;
    printf("ir_flash_save_en ir_save_en=%x\r\n",ir_app_parm.ir_save.ir_save_en);

}

void ir_flash_save_flag_for_ext_sector(void)
{
    u8 index;
    u32 addr;
    u8 data;

    index = ir_flash_index/3;
    addr = IR_DATA_SECT_0_EXT_ADDR + index*0x1000;
    flash_read_page(addr,1,&data);
    if(data != APP_IR_DATA_HEAD)
    {
       data = APP_IR_DATA_HEAD;
       flash_write_page(addr,1,&data);
       printf("ir_flash_save_flag_for_ext_sector =%x",index);
    }
}

void ir_flash_save(u8 button)
{
    u32 flash_addr;
    u8 write_len_per;

    flash_addr = if_flash_sect_addr[ir_flash_index] + button*256;
    if(((ir_table[button].code_size+5) - ir_app_parm.ir_save.ir_save_pos) >= 16)
    {
        write_len_per = 16;
    }
    else
    {
        write_len_per = ir_table[button].code_size + 5 - ir_app_parm.ir_save.ir_save_pos;
    }
    if(ir_app_parm.ir_save.ir_save_pos<256)
    {
        if(bls_ll_requestConnBrxEventDisable() > 2)
        {
            bls_ll_disableConnBrxEvent();
            //device_led_on(1);
            flash_write_page(flash_addr+ir_app_parm.ir_save.ir_save_pos,write_len_per,(((u8*)(&ir_table[button]))+ir_app_parm.ir_save.ir_save_pos));
            //device_led_off(1);
            bls_ll_restoreConnBrxEvent();
        }
        else
            return;
    }
    else
    {
        if(bls_ll_requestConnBrxEventDisable() > 2)
        {

            bls_ll_disableConnBrxEvent();
            //device_led_on(1);
            flash_write_page(flash_addr-0x7000+ir_app_parm.ir_save.ir_save_pos-256,write_len_per,(((u8*)(&ir_table[button]))+ir_app_parm.ir_save.ir_save_pos));
            //device_led_off(1);
            bls_ll_restoreConnBrxEvent();
            ir_flash_save_flag_for_ext_sector();
        }
        else
            return;
    }
    ir_app_parm.ir_save.ir_save_pos += write_len_per;
    ir_app_parm.ir_save.ir_save_button = button;
    //printf("pos=%x,button=%x\r\n",ir_app_parm.ir_save.ir_save_pos,ir_app_parm.ir_save.ir_save_button);
    if(ir_app_parm.ir_save.ir_save_pos == (ir_table[button].code_size+5))
    {
        printf("\r\n w butt=%x\r\n",button);
        ir_app_parm.ir_save.ir_save_en &= ~(1<<button);
        ir_app_parm.ir_save.ir_save_button = 0xff;
    }

}


/**
 * @brief       Backup the ir data in flash
 * @param[in]   none
 * @return      none
 */
int ir_flash_save_timeoutcb(void)
{
    u32 flash_addr;
    u8 i,buf;
    u8 ir_save_flag;

    if((ir_app_parm.ir_save.ir_save_button == 0xff) && (ir_app_parm.ir_save.ir_save_pos == 0))
    {
        if(ir_save_error_flag == 0xaa)
        {
            if((ir_flash_erase_tick == 0) && (ir_flash_erase_sector_step == 0))
            {
                ir_flash_index = 0;
                ir_save_error_flag = 0;
            }
            return 0;
        }
        else
        {
            flash_addr = if_flash_sect_addr[ir_flash_index];
            flash_read_page(flash_addr, 1, &buf);
            if(buf != 0xff)
            {
                printf("ir_flash_save error\r\n");
                printf("ir_flash_save error\r\n");
                ir_save_error_flag = 0xaa;
                if(ir_flash_index >= IR_DATA_NEED_ERASE_NUM)
                {
                    flash_addr = if_flash_sect_addr[0];
                    flash_read_page(flash_addr, 1, &buf);
                    if(buf == 0xff)
                    {
                       ir_flash_need_erase_sector = 2;
                       ir_flash_erase_tick = clock_time() | 1;
                       ir_flash_index = 0;
                    }
                    else
                    {
                       //erase all
                       ir_flash_need_erase_sector = 4;
                    }
                }
                else
                {
                    flash_addr = if_flash_sect_addr[IR_DATA_NEED_ERASE_NUM];
                    flash_read_page(flash_addr, 1, &buf);
                    if(buf == 0xff)
                    {
                       ir_flash_need_erase_sector = 0;
                       ir_flash_erase_tick = clock_time() | 1;
                       ir_flash_index = IR_DATA_NEED_ERASE_NUM;
                    }
                    else
                    {
                       //erase all
                       ir_flash_need_erase_sector = 4;
                    }
                }
                return 0;
            }
        }
    }
    ir_save_flag = ir_app_parm.ir_save.ir_save_en;
    if((ir_app_parm.ir_save.ir_save_button != 0xff) && (ir_app_parm.ir_save.ir_save_button <= 4))
    {
        ir_flash_save(ir_app_parm.ir_save.ir_save_button);
        return 0;
    }
    else
    {
        for(i=0;i<MAX_KEY_COUNT;i++)
        {
            if(ir_save_flag & 0x01)
            {
                if(ir_app_parm.ir_save.ir_save_button == 0xff)
                    ir_app_parm.ir_save.ir_save_pos = 0;
                ir_app_parm.ir_save.ir_save_button = i;
                ir_flash_save(ir_app_parm.ir_save.ir_save_button);
                return 0;
            }
            else
            {
               ir_save_flag = ir_save_flag>>1;
            }
        }
        return -1;
    }
}


void ir_init_key_event_notify(u8 value)
{
    ir_app_parm.ir_programming.key_notification_flag = value;
}

/**
 * @brief       Erase the flash sector
 * @return      0: not erase success
               -1: erase success
 */
int ir_flash_erase_timeoutcb(void)
{
    //printf("ir_flash_erase_timeoutcb\r\n");
    //u32 time;
    u8 data;
    u32 pos_addr_ext=0,pos_addr=0;

    if(is_mic_enable() == 1)
        return 0;

    //u32 erase_time=clock_time();
    //printf("flash erase sector=%x \r\n",ir_flash_need_erase_sector);
   // time = bls_ll_requestConnBrxEventDisable();
    if(ir_flash_need_erase_sector == 0)
    {
        pos_addr = 0;
        pos_addr_ext = 0;
    }
    else if(ir_flash_need_erase_sector == 2)
    {
        pos_addr = 0x2000;
        pos_addr_ext = 0x7000;
    }
    else
    {
        //erase all
        if(bls_ll_requestConnBrxEventDisable() > 150)
        {
            bls_ll_disableConnBrxEvent();
            if(ir_flash_erase_sector_step == 0)
            {
                flash_erase_sector(IR_DATA_SECT_0_ADDR);
                flash_erase_sector(IR_DATA_SECT_1_ADDR);
                ir_flash_erase_sector_step = 1;
            }
            else if(ir_flash_erase_sector_step == 1)
            {
                flash_erase_sector(IR_DATA_SECT_0_EXT_ADDR);
                flash_erase_sector(IR_DATA_SECT_1_EXT_ADDR);
                ir_flash_erase_sector_step = 2;
            }
            else if(ir_flash_erase_sector_step == 2)
            {
                flash_erase_sector(IR_DATA_SECT_2_ADDR);
                flash_erase_sector(IR_DATA_SECT_3_ADDR);
                ir_flash_erase_sector_step = 3;
            }
            else
            {
                flash_erase_sector(IR_DATA_SECT_2_EXT_ADDR);
                flash_erase_sector(IR_DATA_SECT_3_EXT_ADDR);
                ir_flash_erase_sector_step = 0;
            }
            bls_ll_restoreConnBrxEvent();

           if(ir_flash_erase_sector_step)
               return 0;
           else
               return -1;
        }
    }
    if(bls_ll_requestConnBrxEventDisable() > 70)
    {
        //device_led_on(1);
        bls_ll_disableConnBrxEvent();
        //printf("flash erase sector=%x \r\n",ir_flash_need_erase_sector);

       if(ir_flash_erase_sector_step == 0)
       {
           flash_erase_sector(IR_DATA_SECT_0_ADDR + pos_addr);
           ir_flash_erase_sector_step = 1;
       }
       else if(ir_flash_erase_sector_step == 1)
       {
           flash_erase_sector(IR_DATA_SECT_1_ADDR + pos_addr);
           ir_flash_erase_sector_step = 2;
       }
       else
       {
           if(ir_flash_erase_sector_step == 2)
           {
               flash_read_page(IR_DATA_SECT_0_EXT_ADDR + pos_addr,1,&data);
               if(data == APP_IR_DATA_HEAD)
               {
                   printf("step_2\r\n");
                   flash_erase_sector(IR_DATA_SECT_0_EXT_ADDR + pos_addr);
                   ir_flash_erase_sector_step = 3;
               }
               else
               {
                   printf("step_2_2\r\n");
                   flash_read_page(IR_DATA_SECT_1_EXT_ADDR + pos_addr,1,&data);
                   if(data == APP_IR_DATA_HEAD)
                   {
                       flash_erase_sector(IR_DATA_SECT_1_EXT_ADDR + pos_addr);
                       ir_flash_erase_sector_step = 0;
                       printf("erase_succ\r\n");
                   }
                   else
                   {
                        printf("step_2_2_2\r\n");
                        ir_flash_erase_sector_step = 0;
                   }
               }
           }
           else
           {
               printf("step_3\r\n");
               flash_read_page(IR_DATA_SECT_1_EXT_ADDR - pos_addr_ext,1,&data);
               if(data == APP_IR_DATA_HEAD)
               {
                   printf("step_3_3\r\n");
                   flash_erase_sector(IR_DATA_SECT_1_EXT_ADDR - pos_addr_ext);
                   ir_flash_erase_sector_step = 0;
                   printf("erase_succ\r\n");
               }
               else
               {
                    printf("step_3_3_3\r\n");
                    ir_flash_erase_sector_step = 0;
               }
           }
        }
        bls_ll_restoreConnBrxEvent();
        if(ir_flash_erase_sector_step)
            return 0;
        else
            return -1;
    }

    return 0;
}

#if 0
/**
 * @brief       Add a timer for erase flash
 * @return      none
 */
void ir_flash_erase(void)
{
    printf("ir_flash_erase\r\n");
    //blt_soft_timer_add(ir_flash_erase_timeoutcb, FLASH_ERASE_TIMEOUT);
    ir_flash_erase_timeoutcb();
}
#endif

/**
 * @brief       Ir flash factory
 * @return      none
 */
void ir_flash_factory(void)
{
    printf("ir_flash_factory\r\n");
    ir_flash_set_flag(0);
}


/**
 * @brief       Erase the flash sector
 * @return      0: not erase success
               -1: erase success
 */
int ir_key_event_notify_erase_timeoutcb(void)
{
    printf("ir_key_event_notify_erase_timeoutcb\r\n");

    if(bls_ll_requestConnBrxEventDisable() > 120)
    {
        bls_ll_disableConnBrxEvent();
        flash_erase_sector(IR_KEY_EVENT_NOTIFY_SECT_ADDR);
        printf("erase_succ\r\n");
        bls_ll_restoreConnBrxEvent();
        return -1;
    }

    return 0;
}


/**
 * @brief       ir_programming_timeoutcb
 * @return      -1: exit the timer
                 0: exit the timer this time
 */
int ir_programming_timeoutcb(void)
{
    printf("ir_programming_timeoutcb\r\n");
    ir_app_parm.ir_programming.programming_start = 0;

    if (ir_app_parm.programming_timer)
    {
        ir_app_parm.programming_timer = NULL;
    }
    return -1;
}

/**
 * @brief       Recognize the programming key is IR mode or BLE HID modes
 * @param[in]   button_index: which button
 * @return      1: BLE HID mode
                0: IR mode
 */
u8 ir_key_is_suppress(u8 key_idx)
{
    u8 index;
    index = ir_get_suppress_index_from_keyid_idx(key_idx);
    if (index == KEY_IDX_NULL) return KEY_IDX_NULL;
    if(ir_app_parm.ir_suppress[index])
        return true;
    else
        return false;
}
void ir_suppress_reinit(void)
{
    u8 i;
    for (i = 0; i < MAX_KEY_COUNT; i++)
    {
        ir_app_parm.ir_suppress[i] = 0;
    }
}
/**
 * @brief       ir_type_read
 * @param[in]   none
 * @return      1 or 0
 */
int ir_type_read(void)
{
    u8 type;
    u8* code = ir_table[ir_app_parm.current_programming_key_send].code;

    type = code[0];

    if(type == 1) //one time ir seqnence
    {
        return 0;
    }
    else if(type == 3)  //one time + repeated ir sequence
    {
       u16 sequence_length = ((u16)code[4] << 8 | code[5])<<2;
       sequence_length += 8;
       u8* p_sequence = &code[sequence_length];
       sequence_length = ((u16)code[6] << 8 | code[7]);
       T_dmaData_buf.data_num = 0;
       for(u8 i = 0; i < sequence_length; i++)
       {
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = \
                pwm_config_dma_fifo_waveform(1, PWM0_PULSE_NORMAL,(u16)(p_sequence[0]<<8) | p_sequence[1]);
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = \
                pwm_config_dma_fifo_waveform(0, PWM0_PULSE_NORMAL,(u16)(p_sequence[2]<<8) | p_sequence[3]);
            p_sequence += 4;
       }
       T_dmaData_buf.dma_len = T_dmaData_buf.data_num * 2;
    }
    else if(type == 4)  // two repeated ir sequence
    {

    }

    return 1;

}

/**
 * @brief       Obtain the duty,frequency,repear delay
 * @param[in]   none
 * @return      none
 */
void ir_fallback_get_protocol_configure(unsigned char* p_duty_cycle, unsigned short* p_carrier_frequency,
                                        unsigned short* p_repeat_delay)
{
    *p_duty_cycle = ir_app_parm.duty_cycle;
    *p_carrier_frequency = ir_app_parm.carrier_frequency;
    *p_repeat_delay = ir_app_parm.ir_repeat_delay.repeat_delay;
}

/**
 * @brief       Load ir data to ir send buffer
 * @param[in]   ir data
 * @return      none
 */
void ir_send_key_code(key_code_t* p_key_code)
{
    u8* code = p_key_code->code;
    u16 sequence_length,sequence_length2,length;    //4 bytes per sequence
    u8* p_sequence;

    T_dmaData_buf.data_num = 0;

    u8  type = code[0];
    if(type)
    {
        ir_app_parm.duty_cycle = code[1]; //as percentage
        ir_app_parm.carrier_frequency = ((u16)code[2] << 8 | code[3]) * 100; //in unit of 100Hz
        sequence_length = (u16)code[4] << 8 | code[5];
    }
    //disable button
    if (type == 0)
    {
       return;
    }
    //one time ir sequence
    else if (type == 1)
    {
        p_sequence = &code[6];
        for(u8 i = 0; i < sequence_length; i++)
        {
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = \
                pwm_config_dma_fifo_waveform(1, PWM0_PULSE_NORMAL,(u16)(p_sequence[0]<<8) | p_sequence[1]);
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = \
                pwm_config_dma_fifo_waveform(0, PWM0_PULSE_NORMAL,(u16)(p_sequence[2]<<8) | p_sequence[3]);
            p_sequence += 4;
        }
    }
    //repeat ir sequence
    else if (type == 2)
    {
        ir_app_parm.ir_repeat_delay.repeat_delay = (u16)code[6] << 8 | code[7];
        //printf("ir_app_parm.ir_repeat_delay_cycle=%x\r\n",ir_app_parm.ir_repeat_delay.repeat_delay);
        ir_app_parm.ir_repeat_delay.repeat_delay = \
            ir_app_parm.ir_repeat_delay.repeat_delay*(1000000/ir_app_parm.carrier_frequency);
        //printf("ir_repeat_delay_us=%x\r\n",ir_app_parm.ir_repeat_delay.repeat_delay);
        p_sequence = &code[8];

        for(u8 i = 0; i < sequence_length; i++)
        {
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = \
                pwm_config_dma_fifo_waveform(1, PWM0_PULSE_NORMAL,(u16)(p_sequence[0]<<8) | p_sequence[1]);
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = \
                pwm_config_dma_fifo_waveform(0, PWM0_PULSE_NORMAL,(u16)(p_sequence[2]<<8) | p_sequence[3]);
            p_sequence += 4;
        }
        //printf("duty_cycle=%x\r\n",ir_app_parm.duty_cycle);
        //printf("carrier_frequency=%x\r\n",ir_app_parm.carrier_frequency);
        //printf("sequence_length=%x\r\n",sequence_length);
    }
    //one time + repeat ir sequence
    else if (type == 3)
    {
        sequence_length2 = (u16)code[6] << 8 | code[7];
        //printf("sequence_length=%x\r\n",sequence_length);
        p_sequence = &code[8];

        for(u8 i = 0; i < sequence_length; i++)
        {
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = \
                pwm_config_dma_fifo_waveform(1, PWM0_PULSE_NORMAL,(u16)(p_sequence[0]<<8) | p_sequence[1]);
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = \
                pwm_config_dma_fifo_waveform(0, PWM0_PULSE_NORMAL,(u16)(p_sequence[2]<<8) | p_sequence[3]);
            p_sequence += 4;
        }
    }
    else
    {
        sequence_length2 = (u16)code[6] << 8 | code[7];
        if(ir_app_parm.odd_or_even_press % 2 == 1)
        {
            p_sequence = &code[8];
            length = sequence_length;
        }
        else
        {
            length = sequence_length*4 + 8;
            p_sequence = &code[length];
            length = sequence_length2;

        }
        for(u8 i = 0; i < length; i++)
        {
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = \
                pwm_config_dma_fifo_waveform(1, PWM0_PULSE_NORMAL,(u16)(p_sequence[0]<<8) | p_sequence[1]);
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = \
                pwm_config_dma_fifo_waveform(0, PWM0_PULSE_NORMAL,(u16)(p_sequence[2]<<8) | p_sequence[3]);
            p_sequence += 4;
        }
        ir_app_parm.odd_or_even_press++;
    }

    ir_send_specil(ir_app_parm.carrier_frequency,ir_app_parm.duty_cycle);
}

extern void app_ota_status(u8 status);
_attribute_data_retention_ u8  ir_cache_key[3]={0};

/**
 * @brief       Ir_repeat_delay_release_time
 * @param[in]   none
 * @return      0
 */
int ir_cache_key_timer(void)
{
    printf("ir_cache_key_timer\r\n");
    if(device_in_connection_state)
    {
        printf("send cachekey_notify\r\n");
        ir_cache_key[0] = 0;   //down
        bls_att_pushNotifyData(ATV_IR_KEY_EVENT_IDX, ir_cache_key, sizeof(ir_cache_key));
        sleep_us(100);
        ir_cache_key[0] = 1;   //up
        bls_att_pushNotifyData(ATV_IR_KEY_EVENT_IDX, ir_cache_key, sizeof(ir_cache_key));
        return -1;
    }
    printf("send cachekey_notify timeout\r\n");
    return -1;
}

/**
 * @brief       send ir data and send nofity
 * @param[in]   1: down 0:up
 * @return      0
 */
int ir_fallback_send(u8 key_down)
{
    key_code_t* p_key_code = &ir_table[ir_app_parm.current_programming_key_send];
    u16 key_id;

    //printf("ir_fallback_send\r\n");
    if(key_down)
        app_ota_status(0);
    else
        app_ota_status(1);
    if(key_down)
    {
        printf("key down\r\n");
        ir_send_key_code(p_key_code);
    }

    if (ir_app_parm.ir_programming.key_notification_flag)
    {
        //Besides sending IR code, also notify ATV that the key event
        printf("send notify\r\n");
        if(app_custom_get_device_type() == REMOTE_G10)
            key_id = key_button_map_g10[ir_app_parm.current_programming_key_send].key_id;
        else
            key_id = key_button_map_g20[ir_app_parm.current_programming_key_send].key_id;
        //printf("key_id=%x\r\n",key_id);
        ir_cache_key[0] = key_down ? 0:1;
        ir_cache_key[1] = (key_id >> 8);
        ir_cache_key[2] = key_id;
        if(device_in_connection_state)
            bls_att_pushNotifyData(ATV_IR_KEY_EVENT_IDX, ir_cache_key, sizeof(ir_cache_key));
        else
        {
            if(key_down == 1)
            {
                printf("add cache key\r\n");
                blt_soft_timer_delete(ir_cache_key_timer);
                blt_soft_timer_add(ir_cache_key_timer, 5000000);
            }
        }
    }
    return 0;
}

/**
 * @brief       Ir_repeat_delay_release_time
 * @param[in]   none
 * @return      0
 */
int ir_repeat_delay_release_time(void)
{
    ir_app_parm.ir_repeat_delay.release_timer = clock_time();
    //printf("ir_app_parm.ir_repeat_delay_release_time=%x\r\n",ir_app_parm.ir_repeat_delay.release_timer);
    return 0;
}

/**
 * @brief       ir_repeat_delay_timer
 * @param[in]   none
 * @return      -1
 */
int ir_repeat_delay_timer(void)
{
    printf("ir_repeat_delay_timer\r\n");
    if(app_is_key_released())
    {
        programming_key_set(0x55);
        ir_fallback_send(1);
    }
    return -1;
}

/**
 * @brief       ir_fallback_send_key_code
 * @param[in]   button_idx
 * @param[in]   key_down  1:down 0:up
 * @return      false or true
 */
u8 ir_fallback_send_key_code(u8 button_idx, u8 key_down)
{
    u8 last_key_send = ir_app_parm.current_programming_key_send;
    u32 current_time;
    u32 time;
    key_code_t* p_key_code = ir_get_key_code_from_keyid_idx(button_idx);
    //printf("current_programming_key=%x\r\n",ir_app_parm.current_programming_key_send);
    u8* code = ir_table[ir_app_parm.current_programming_key_send].code;

    u8 type = code[0];
    if (p_key_code == NULL)
    {
        return false;
    }

    if(type == 2)
    {
        if((last_key_send == ir_app_parm.current_programming_key_send))
        {
             current_time = clock_time();
             /*
             printf("current_time=%x\r\n",current_time);
             printf("current_time - ir_app_parm.ir_repeat_delay.release_timer=%x\r\n",\
                 current_time - ir_app_parm.ir_repeat_delay.release_timer);
             */
             time = (current_time - ir_app_parm.ir_repeat_delay.release_timer)>>4;
             if(time < ir_app_parm.ir_repeat_delay.repeat_delay)
             {
                 time = ir_app_parm.ir_repeat_delay.repeat_delay - time;
                 //printf("need delay=%x\r\n",time);
                 blt_soft_timer_add(ir_repeat_delay_timer, time);
             }
             else
             {
                 //printf("exceed delay time\r\n");
                 programming_key_set(0x55);
                 ir_fallback_send(key_down);
             }
        }
        else
        {
             programming_key_set(0x55);
             ir_fallback_send(key_down);
        }
    }
    else
    {
        programming_key_set(0x55);
        ir_fallback_send(key_down);
    }
    return true;
}

/**
 * @brief       ir_code_merge
 * @param[in]   key_id
 * @param[in]   button_index
 * @param[in]   code size: ir data length
 * @param[in]   code: ir data
 * @return      none
 */
void ir_code_merge(u16 key_id,u8 button_index,u16 code_size,u8* code)
{
    //u32 flash_addr_pos=0;
    u32 len;


    //printf("ir_code_merge,ir_app_parm.ir_merge.ir_current_button=%x,button_index=%x\r\n",ir_app_parm.ir_merge.ir_current_button,button_index);
    if(ir_app_parm.ir_merge.ir_current_button != button_index)
    {
        ir_app_parm.ir_merge.ir_code_pos = 0;
        ir_app_parm.ir_merge.ir_keycode_total_len = 0;
        if(code[0] == 1)
        {
            ir_app_parm.ir_merge.ir_keycode_total_len =  ((u16)code[4] << 8 | code[5])*4 + 6;
        }
        else if(code[0] == 2)
        {
            ir_app_parm.ir_merge.ir_keycode_total_len =  ((u16)code[4] << 8 | code[5])*4 + 8;
        }
        else if((code[0] == 3) || (code[0] == 4))
        {
            ir_app_parm.ir_merge.ir_keycode_total_len =  ((u16)code[4] << 8 | code[5])*4 + ((u16)code[6] << 8 | code[7])*4 + 8;
        }
        //printf(" ir_app_parm.ir_merge.ir_keycode_total_len=%x,codesize=%x\r\n", ir_app_parm.ir_merge.ir_keycode_total_len,code_size);
        memcpy(ir_table[button_index].code + ir_app_parm.ir_merge.ir_code_pos, code, code_size);
        ir_app_parm.ir_merge.ir_code_pos += code_size;

    }
    else
    {
         memcpy(ir_table[button_index].code + ir_app_parm.ir_merge.ir_code_pos, code, code_size);
         ir_app_parm.ir_merge.ir_code_pos += code_size;
    }
   // printf("ir_code_merge: ir_app_parm.ir_merge.ir_code_pos=%x,ir_app_parm.ir_merge.ir_keycode_total_len=%x,code_size=%x",ir_app_parm.ir_merge.ir_code_pos,ir_app_parm.ir_merge.ir_keycode_total_len,code_size);
    if(ir_app_parm.ir_merge.ir_code_pos == ir_app_parm.ir_merge.ir_keycode_total_len)
    {
         len = ir_app_parm.ir_merge.ir_keycode_total_len;
         ir_table[button_index].key_id = key_id;
         ir_table[button_index].code_size = len;
         ir_table[button_index].button_idx = button_index;
         //flash_addr_pos += button_index*256;
         //ir_flash_save(button_index,flash_addr_pos,len+5);
         ir_flash_save_en(button_index);
        /*
         for(u8 i=0;i<len;i++ )
         {
             printf(" %x\r\n",*(ir_table[button_index].code+i));
         }
         */
         ir_app_parm.ir_programming.programmed_key_count++;
         //printf("add key\r\n");
    }
}

/**
 * @brief       ir_table_add_key
 * @param[in]   key_id,code_size,code
 * @return      0,11
 */
u8 ir_table_add_key(u16 key_id, u16 code_size, u8* code)
{
    u8 button_index=0;
    u16 button;

    button_index = ir_get_button_idx_from_key_id(key_id);
    if (button_index == KEY_IDX_NULL)
    {
        return false;
    }
    if ((ir_app_parm.ir_programming.programmed_key_count < MAX_KEY_COUNT)
        && (code_size <= MAX_CODE_LENGTH))
    {
        if(app_custom_get_device_type() == REMOTE_G10)
        {
            button = key_button_map_g10[button_index].button;
            //printf("ir_table[button_index].key_id=%x \r\n",button);
        }
        else
        {
            button = key_button_map_g20[button_index].button;
           // printf("ir_table[button_index].key_id=%x \r\n",button);
        }
        //printf("ir_table[button_index].button_idx=%x \r\n",button_index);

        ir_code_merge(button,button_index,code_size,code);
    }
    return true;
}

/**
 * @brief       ir_key_event_notify_erase
 * @param[in]   none
 * @return      none
 */
void ir_key_event_notify_erase(void)
{
    blt_soft_timer_add(ir_key_event_notify_erase_timeoutcb, 100000);
}


/**
 * @brief       Receive ATV commands and processed
 * @param[in]   handle :ATV command
 * @param[in]   buf:command
 * @param[in]   len: command data length
 * @return      false or true
 */
void ir_fallback_process(u16 handle, u8* buf, u16 len)
{
    u8 index,data;
  //  u8 testdata[6],i;
  //  u32 addr;
    u8 k;

    switch (handle)
    {
        case ATV_IR_PROG_CONTROL_IDX:
        {
            if (len == 1)
            {
                ir_app_parm.ir_programming.programming_start = *buf;
                if (ir_app_parm.ir_programming.programming_start == 1)
                {
                    printf("ir table programming start\r\n");
                    #if BLE_AUDIO_ENABLE
                    google_reset_rsp_delay();
                    #endif
                    ir_table_init();
                    ir_flash_set_flag(0);
                    if(ir_flash_index == 0)
                    {
                        flash_read_page(IR_DATA_SECT_0_ADDR,1,&data);
                        if(data == APP_IR_DATA_HEAD_INVALID)
                         ir_flash_index++;
                    }
                    else
                        ir_flash_index++;

                    if(ir_flash_index >= IR_DATA_SECT_MAX_NUM)
                        ir_flash_index = 0;
                    printf("ir_flash_index=%x\r\n",ir_flash_index);
                    if (ir_app_parm.programming_timer)
                    {
                        blt_soft_timer_delete(ir_programming_timeoutcb);
                        ir_app_parm.programming_timer = NULL;
                    }

                    ir_app_parm.programming_timer = 1;
                    blt_soft_timer_add(ir_programming_timeoutcb, PROGRAMMING_TIMEOUT);
                }
                else
                {
                    printf("ir table programming end\r\n");
                    extern u32    ir_flash_erase_tick;
                    if(ir_app_parm.ir_programming.programmed_key_count == 0)
                    {
                        ir_flash_set_flag(0);
                        printf(" empty ir programming\r\n");
                    }
                    if(ir_app_parm.ir_programming.programmed_key_count >= 1)
                    {
                        ir_flash_set_flag(1);
                    }
                    if(ir_flash_index == IR_DATA_NEED_ERASE_NUM)
                    {
                        ir_flash_need_erase_sector = 0;
                        ir_flash_erase_tick = clock_time() | 1;
                    }
                    if(ir_flash_index == 0)
                    {
                        ir_flash_need_erase_sector = 2;
                        ir_flash_erase_tick = clock_time() | 1;
                    }
                    if (ir_app_parm.programming_timer)
                    {
                        blt_soft_timer_delete(ir_programming_timeoutcb);
                        ir_app_parm.programming_timer = NULL;
                    }
                    printf("ir_flash_erase_tick=%x\r\n",ir_flash_erase_tick);
                    /*
                    for(i=0;i<IR_DATA_SECT_NUM;i++)
                    {
                        addr = IR_DATA_SECT_0_ADDR+i*0x1000;
                        flash_read_page(addr, 6, testdata);
                        printf("sect i=%x addr =%x\r\n",i,addr);
                        for(k=0;k<6;k++)
                            //printf(" %x",testdata[k]);
                    }
                    */
                    if(app_ir_programming_end_Cb != NULL)
                    {
                        app_ir_programming_end_Cb();
                    }
                }
            }
            break;
        }
        case ATV_IR_KEY_ID_IDX:
        {
            if ((ir_app_parm.ir_programming.programming_start == 1)
                //&& (ir_app_parm.ir_programming.current_programming_key_id == INVALID_KEY_ID)
                && (len == 2))
            {
                ir_app_parm.ir_programming.current_programming_key_id = buf[0];
                ir_app_parm.ir_programming.current_programming_key_id <<= 8;
                ir_app_parm.ir_programming.current_programming_key_id |= buf[1];
                if (ir_app_parm.programming_timer)
                {
                     blt_soft_timer_delete(ir_programming_timeoutcb);
                     ir_app_parm.programming_timer = NULL;
                }
                blt_soft_timer_add(ir_programming_timeoutcb, PROGRAMMING_TIMEOUT);
                ir_app_parm.programming_timer = 1;
            }
            break;
        }
        case ATV_IR_CODE_IDX:
        {
            if ((ir_app_parm.ir_programming.programming_start == 1) && (len > 0))
            {
                if (ir_app_parm.ir_programming.current_programming_key_id != INVALID_KEY_ID)
                {
                    u8 return_value;
                    return_value = ir_table_add_key(ir_app_parm.ir_programming.current_programming_key_id, len, buf);
                    ir_app_parm.ir_programming.current_programming_key_id = INVALID_KEY_ID;
                }
                else
                {
                   //response error
                }
                if (ir_app_parm.programming_timer)
                {
                     blt_soft_timer_delete(ir_programming_timeoutcb);
                     ir_app_parm.programming_timer = NULL;
                }
                blt_soft_timer_add(ir_programming_timeoutcb, PROGRAMMING_TIMEOUT);
                ir_app_parm.programming_timer = 1;
            }
            break;
        }
        case ATV_IR_SUPPRESS_IDX:
        {
            if(len)
            {
                printf("switch to ble\r\n");
                if(len%2 == 0)
            {
                    for(k=0;k<len/2;k++)
                    {
                        u16 id = (buf[2*k]<<8) | buf[2*k+1];
                        index = ir_get_button_idx_from_key_id(id);
                        printf("index=%x\r\n",index);
                        if(ir_app_parm.ir_suppress[index] == 0)
                {
                            ir_app_parm.ir_suppress[index] = 1;
                }
                    }
            }
        }
                else
            {
                printf("switch to ir\r\n");
                for(k=0;k<MAX_KEY_COUNT;k++)
                {
                    if(ir_app_parm.ir_suppress[k])
                    {
                        ir_app_parm.ir_suppress[k] = 0;
                    }
                }
            }
            break;
        }
        case ATV_IR_KEY_EVENT_CCCD_IDX:
        {
            if (len == 2)
            {
                //printf("ccc buf[0]=%x\r\n",buf[0]);
                if(buf[0] != ir_app_parm.ir_programming.key_notification_flag)
                {
                    ir_app_parm.ir_programming.key_notification_flag = buf[0];

                    if(is_ir_key_event_notify_flash_info_full() == 1)
                    {
                        ir_key_event_notify_erase();
                        //flash_erase_sector(IR_KEY_EVENT_NOTIFY_SECT_ADDR);
                        //write_ir_key_event_notify(buf[0]);
                    }
                    else
                        write_ir_key_event_notify(buf[0]);
                }
            }
            break;
        }
        default:
        {
            //printf("!!unhandled IDX\r\n");
        }
        break;
    }
}

void ir_flash_save_loop(void)
{
    if(ir_app_parm.ir_save.ir_save_en)
    {
        ir_flash_save_timeoutcb();
    }
}

void ir_flash_erase_loop(void)
{
    if((ir_flash_erase_tick) && (ir_send_ctrl.is_sending== 0))
    {
        if(ir_flash_erase_timeoutcb() == -1)
        {
            ir_flash_erase_tick = 0;
        }
    }
}

u8 ir_flash_busy(void)
{
     if(ir_app_parm.ir_save.ir_save_en || ir_flash_erase_tick || ir_app_parm.ir_programming.programming_start)
        return 1;
     else
        return 0;
}


#define APP_IR_CMD_OTA_NEC_IR_TABLE_PREPARE      0xEF00
#define APP_IR_CMD_OTA_NEC_IR_TABLE_START        0xEF01
#define APP_IR_CMD_OTA_NEC_IR_TABLE_END          0xEF02

void ir_nec_ir_table_process(u8 *data,u16 len)
{
    u8 datasend[3];
    u16 crc=0;
    u32 flash_addr=0,pos=0;
    u8 readdata[16];
    u8 result=0;
    u16 cmd = data[0] | (data[1]<<8);

    if(cmd == APP_IR_CMD_OTA_NEC_IR_TABLE_PREPARE)
    {
        nec_ir_table_start = APP_NEC_IR_TABLE_PREPARE;
    }
    else if(cmd == APP_IR_CMD_OTA_NEC_IR_TABLE_START)
    {
        nec_ir_table_start = APP_NEC_IR_TABLE_START;
    }
    else if(((data[0] == 2) && (data[1] == 0xEF)))  //IR end
    {
        datasend[0] = 0xEE;
        datasend[1] = 2;
        datasend[2] = 0;
        bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, datasend, sizeof(datasend));
        if(nec_ir_table_start == APP_NEC_IR_TABLE_START)
        {
            app_custom_set_new_ir_table();
        }
        nec_ir_table_start = APP_NEC_IR_TABLE_END;
    }

    if(nec_ir_table_start == APP_NEC_IR_TABLE_START)
    {
        if(len == 20)
        {
            crc = (data[19]<<8) | data[18];
            pos = (data[0]) | (data[1]<<8);
            extern unsigned short crc16 (unsigned char *pD, int len);
            if(crc == crc16(data,18))
            {
                flash_addr = APP_NEC_IR_CODE_TABLE +  pos*16;
                flash_write_page(flash_addr,16,&data[2]);
                flash_read_page(flash_addr,16, readdata);
                if(memcmp(&data[2],readdata,16))
                {
                    result = 1;
                }
            }
            else
            {
                result = 1;
            }
            if(result == 1)
            {
                datasend[0] = 0xEE;
                datasend[1] = 2;
                datasend[2] = 1;
                bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, datasend, sizeof(datasend));
                nec_ir_table_start = APP_NEC_IR_TABLE_WRITEFAIL;   //data invalid, must erase
            }
        }
    }
}

int ir_nec_ir_table_erase_timeoutcb(void)
{
    if(is_mic_enable() == 1)
        return 0;
    if(bls_ll_requestConnBrxEventDisable() > 120)
    {
        printf("ir_nec_ir_table_erase_timeoutcb\r\n");
        bls_ll_disableConnBrxEvent();
        flash_erase_sector(APP_NEC_IR_CODE_TABLE);
        bls_ll_restoreConnBrxEvent();
        return -1;
    }
    return 0;
}

void ir_nec_ir_table_erase_loop(void)
{
    u8 datasend[3];
    if((nec_ir_table_start == APP_NEC_IR_TABLE_PREPARE) || (nec_ir_table_start == APP_NEC_IR_TABLE_WRITEFAIL))
    {
        if(ir_send_ctrl.is_sending == 0)
        {
            if(ir_nec_ir_table_erase_timeoutcb() == -1)
            {
                if(device_in_connection_state && (nec_ir_table_start == APP_NEC_IR_TABLE_PREPARE))
                {
                    //printf("ir_nec_ir_table_erase_end\r\n");
                    datasend[0] = 0XEE;
                    datasend[1] = 0;
                    datasend[2] = 0;
                    bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, datasend, sizeof(datasend));
                }
                nec_ir_table_start = APP_NEC_IR_TABLE_PREPARE_END;
            }
        }
    }
}

#endif

