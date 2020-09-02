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
#include "../../stack/ble/attr/att.h"


#if(BLT_TEST_SOFT_TIMER_ENABLE == 0)
int blt_soft_timer_add(blt_timer_callback_t func, u32 interval_us)
{
}

int     blt_soft_timer_delete(blt_timer_callback_t func)
{
}

#endif

#define PROGRAMMING_TIMEOUT 30000000
#define FLASH_ERASE_TIMEOUT 600000

#define INVALID_KEY_ID      0xFF00
#define MAX_CODE_LENGTH     160
#define MAX_KEY_COUNT       5

#define KEY_IDX_NULL        0xff

typedef struct{
    u16 key_id;                 //Key ID, comes from ATV
    u16 code_size;
    u8  button_idx;                //button index, based on enum in firmware
    u8  code[MAX_CODE_LENGTH];
}key_code_t;


typedef struct {
    u16 key_id;
    u8  button_idx;
    u8  button;
}key_button_pair_t;

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
	u8 ir_code_pos;
	u8 ir_keycode_total_len;
	u8 ir_current_button;
}ir_keycode_merge_t;

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
}ir_app_parm_t;


const key_button_pair_t key_button_map[MAX_KEY_COUNT] = {
    {0x0018, KEY_IDX_VOLUP, 0x10},
    {0x0019, KEY_IDX_VOLDN, 0x02},
    {0x001A, KEY_IDX_POWER, 0 },
    {0x00A4, KEY_IDX_MUTE, 0x16},
    {0x00B2, KEY_IDX_INPUT, 0x03}
};


static _attribute_data_retention_ ir_app_parm_t  ir_app_parm={0};

static _attribute_data_retention_ key_code_t ir_table[MAX_KEY_COUNT]={{0},{0},{0},{0},{0}};

_attribute_data_retention_ u8 ir_flash_sector=0;
_attribute_data_retention_ u8 ir_flash_erase_flag=0;


void ir_test_init(void)
{

}

extern _attribute_data_retention_    int device_in_connection_state;
extern int     blt_soft_timer_delete(blt_timer_callback_t func);
void programming_key_set(u8 data)
{
    ir_app_parm.is_key_programming = data;
}

u8 is_programming_key_send(void)
{
    if(ir_app_parm.is_key_programming == 0x55)
        return 1;
    else
        return 0;
}

u8 ir_get_button_idx_from_button(u8 button)
{
    u8 i;
    for(i = 0; i < MAX_KEY_COUNT; i++)
    {
        if (key_button_map[i].button == button)
        {

            return key_button_map[i].button_idx;
        }
    }
    return KEY_IDX_NULL;
}

u8 ir_get_button_idx_from_key_id(u16 key_id)
{
    u8 i;
    for(i = 0; i < MAX_KEY_COUNT; i++)
    {
        if (key_button_map[i].key_id == key_id)
        {

            return key_button_map[i].button_idx;
        }
    }
    return KEY_IDX_NULL;
}

key_code_t* ir_get_key_code_from_keyid_idx(u8 key_idx)
{
    u8 i;
    for(i = 0; i < MAX_KEY_COUNT; i++)
    {
        if (ir_table[i].key_id == key_idx)
        {
            ir_app_parm.current_programming_key_send = i;
            return &ir_table[i];
        }
    }
    return NULL;
}

u8 ir_get_suppress_index_from_keyid_idx(u8 key_idx)
{
    u8 i;
    printf("ir_app_parm.ir_programming.programmed_key_count=%x\r\n",ir_app_parm.ir_programming.programmed_key_count);
    if(ir_table[0].key_id == 0) return KEY_IDX_NULL;
    for(i = 0; i < MAX_KEY_COUNT; i++)
    {
        if (ir_table[i].key_id == key_idx)
        {
            printf("tset i=%x\r\n",i);
            return i;
        }
    }
    return KEY_IDX_NULL;
}

void ir_table_init(void)
{
    u8 i;
    memset(ir_table, 0, sizeof(key_code_t) * MAX_KEY_COUNT);
    for (i = 0; i < MAX_KEY_COUNT; i++)
    {
        ir_table[i].key_id = INVALID_KEY_ID;
    }
    ir_app_parm.programming_timer = NULL;
    ir_app_parm.ir_programming.programmed_key_count = 0;
    ir_app_parm.ir_programming.current_programming_key_id = INVALID_KEY_ID;
	ir_app_parm.ir_merge.ir_current_button = MAX_KEY_COUNT;
}

int ir_programming_timeoutcb(void)
{
    printf("ir_programming_timeoutcb\r\n");
    ir_app_parm.ir_programming.programming_start = 0;

    if (ir_app_parm.programming_timer)
    {
        blt_soft_timer_delete(ir_programming_timeoutcb);
        ir_app_parm.programming_timer = NULL;
    }
    return 0;
}

u8 ir_is_button_fallback_override(u8 button_idx)
{
    u8 i;
    for(i = 0; i < ir_app_parm.ir_programming.programmed_key_count; i++)
    {
        if (ir_table[i].button_idx == button_idx)
        {
            return true;
        }
    }
    return false;
}

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

void ir_fallback_get_protocol_configure(unsigned char* p_duty_cycle, unsigned short* p_carrier_frequency,
                                        unsigned short* p_repeat_delay)
{
    *p_duty_cycle = ir_app_parm.duty_cycle;
    *p_carrier_frequency = ir_app_parm.carrier_frequency;
    *p_repeat_delay = ir_app_parm.ir_repeat_delay.repeat_delay;
}

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
        printf("ir_app_parm.ir_repeat_delay_cycle=%x\r\n",ir_app_parm.ir_repeat_delay.repeat_delay);
        ir_app_parm.ir_repeat_delay.repeat_delay = \
            ir_app_parm.ir_repeat_delay.repeat_delay*(1000000/ir_app_parm.carrier_frequency);
        printf("ir_repeat_delay_us=%x\r\n",ir_app_parm.ir_repeat_delay.repeat_delay);
        p_sequence = &code[8];

        for(u8 i = 0; i < sequence_length; i++)
        {
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = \
                pwm_config_dma_fifo_waveform(1, PWM0_PULSE_NORMAL,(u16)(p_sequence[0]<<8) | p_sequence[1]);
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = \
                pwm_config_dma_fifo_waveform(0, PWM0_PULSE_NORMAL,(u16)(p_sequence[2]<<8) | p_sequence[3]);
            p_sequence += 4;
        }
        printf("duty_cycle=%x\r\n",ir_app_parm.duty_cycle);
        printf("carrier_frequency=%x\r\n",ir_app_parm.carrier_frequency);
        printf("sequence_length=%x\r\n",sequence_length);
    }
    //one time + repeat ir sequence
    else if (type == 3)
    {
        sequence_length2 = (u16)code[6] << 8 | code[7];
        printf("sequence_length=%x\r\n",sequence_length);
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
int ir_fallback_send(u8 key_down)
{
    key_code_t* p_key_code = &ir_table[ir_app_parm.current_programming_key_send];
    u16 key_id;

    if(key_down)
        app_ota_status(0);
	else
		app_ota_status(1);
    if(key_down)
    {
        printf("key down\r\n");
        ir_send_key_code(p_key_code);
    }

    if (ir_app_parm.ir_programming.key_notification_flag && device_in_connection_state)
    {
        //Besides sending IR code, also notify ATV that the key event
        printf("notify down\r\n");
        key_id = key_button_map[ir_app_parm.current_programming_key_send].key_id;
        printf("key_id=%x\r\n",key_id);
        u8 TempBuf[3];
        TempBuf[0] = key_down ? 0:1;
        TempBuf[1] = (key_id >> 8);
        TempBuf[2] = key_id;
        bls_att_pushNotifyData(ATV_IR_KEY_EVENT_IDX, TempBuf, sizeof(TempBuf));
    }
    return 0;
}

int ir_repeat_delay_release_time(void)
{
    ir_app_parm.ir_repeat_delay.release_timer = clock_time();
    printf("ir_app_parm.ir_repeat_delay_release_time=%x\r\n",ir_app_parm.ir_repeat_delay.release_timer);
    return 0;
}

int ir_repeat_delay_timer(void)
{
    printf("ir_repeat_delay_timer\r\n");
    blt_soft_timer_delete(&ir_repeat_delay_timer);
    ir_fallback_send(1);
    return 0;
}

u8 ir_fallback_send_key_code(u8 button_idx, u8 key_down)
{
    u8 last_key_send = ir_app_parm.current_programming_key_send;
    u32 current_time;
    u32 time;
    key_code_t* p_key_code = ir_get_key_code_from_keyid_idx(button_idx);
    u8* code = ir_table[ir_app_parm.current_programming_key_send].code;

    u8 type = code[0];
    if (p_key_code == NULL)
    {
        return false;
    }
    programming_key_set(0x55);
    if(type == 2)
    {
        if((last_key_send == ir_app_parm.current_programming_key_send))
        {
             current_time = clock_time();
             printf("current_time=%x\r\n",current_time);
             printf("current_time - ir_app_parm.ir_repeat_delay.release_timer=%x\r\n",\
                 current_time - ir_app_parm.ir_repeat_delay.release_timer);
             time = (current_time - ir_app_parm.ir_repeat_delay.release_timer)>>4;
             if(time < ir_app_parm.ir_repeat_delay.repeat_delay)
             {
                 time = ir_app_parm.ir_repeat_delay.repeat_delay - time;
                 printf("need delay=%x\r\n",time);
                 blt_soft_timer_add(&ir_repeat_delay_timer, time);
             }
             else
             {
                 printf("exceed delay time\r\n");
                  ir_fallback_send(key_down);
             }
        }
        else
             ir_fallback_send(key_down);
    }
    else
    {
        ir_fallback_send(key_down);
    }
    return true;
}

void ir_flash_save(u32 addr,u32 len)
{
    u32 flash_addr;
	u8 i,buf;
	
	flash_addr = IR_DATA_SECT_1_ADDR + ir_flash_sector*0x1000 + addr;
	
	flash_read_page(flash_addr, 1, &buf);
	if(buf != 0xff) 
	{
		return;
	}
    printf("flash_addr=%x\r\n",flash_addr);
    for(i=0;i<len/16;i++)
    {
        flash_write_page(flash_addr+i*16,16,(((u8*)(&ir_table[ir_app_parm.ir_programming.programmed_key_count]))+i*16));
    }
    if(len%16)
        flash_write_page(flash_addr+i*16,len-i*16,(((u8*)(&ir_table[ir_app_parm.ir_programming.programmed_key_count]))+i*16));
	printf("ir_flash_save addr=%x\r\n",addr);
}

extern int bls_ll_requestConnBrxEventDisable(void);
extern void  bls_ll_disableConnBrxEvent(void);
extern void  bls_ll_restoreConnBrxEvent(void);
int ir_flash_erase_timeoutcb(void)
{
    printf("ir_flash_erase_timeoutcb\r\n");

	if(bls_ll_requestConnBrxEventDisable() > 120)
	{
	    printf("flash erase \r\n");
		bls_ll_disableConnBrxEvent();
	    if(ir_flash_sector)
	        flash_erase_sector(IR_DATA_SECT_1_ADDR);
		else
			flash_erase_sector(IR_DATA_SECT_2_ADDR);
		bls_ll_restoreConnBrxEvent();
		if(ir_app_parm.ir_programming.programmed_key_count>=2)
        {
   	        if(ir_flash_sector)
   		        ir_flash_sector = 0;
   		    else
   			    ir_flash_sector = 1;
      	}
		printf("flash erase succ\r\n");
		blt_soft_timer_delete(ir_flash_erase_timeoutcb);
	}
    
    return 0;
}

int ir_flash_eraseall_timeoutcb(void)
{
    u8 data[2];
    printf("ir_flash_eraseall_timeoutcb\r\n");

	if(bls_ll_requestConnBrxEventDisable() > 200)
	{
	    printf("flash erase \r\n");
		bls_ll_disableConnBrxEvent();
		flash_read_page(IR_DATA_SECT_1_ADDR, 1, &data[0]);
	    flash_read_page(IR_DATA_SECT_2_ADDR, 1, &data[1]);
	    if(data[0] != 0xff)
	        flash_erase_sector(IR_DATA_SECT_1_ADDR);
		if(data[1] != 0xff)
	        flash_erase_sector(IR_DATA_SECT_2_ADDR);
		bls_ll_restoreConnBrxEvent();
		printf("flash erase succ\r\n");
		blt_soft_timer_delete(ir_flash_eraseall_timeoutcb);
		ir_flash_sector = 0;
		ir_flash_erase_flag = 0;
	}
    
    return 0;
}

void ir_flash_erase(void)
{
    u8 data[2];
	
    flash_read_page(IR_DATA_SECT_1_ADDR, 1, &data[0]);
	flash_read_page(IR_DATA_SECT_2_ADDR, 1, &data[1]);
	if((data[0] == 0xff) && (data[1] = 0xff))
		return;
	blt_soft_timer_add(&ir_flash_erase_timeoutcb, FLASH_ERASE_TIMEOUT);	
}


void ir_flash_eraseall(void)
{
    printf("ir_flash_eraseall\r\n");
	blt_soft_timer_add(&ir_flash_eraseall_timeoutcb, FLASH_ERASE_TIMEOUT);	
}

void ir_flash_factory(void)
{
    printf("ir_flash_factory\r\n");
	flash_erase_sector(IR_DATA_SECT_1_ADDR);
    flash_erase_sector(IR_DATA_SECT_2_ADDR);
	ir_flash_sector = 0;
}

void ir_code_merge(u16 key_id,u8 button_index,u16 code_size,u8* code)
{
    u32 flash_addr_pos=0,len;

	printf("ir_code_merge,ir_app_parm.ir_merge.ir_current_button=%x,button_index=%x\r\n",ir_app_parm.ir_merge.ir_current_button,button_index);
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
	    else if(code[0] == 3)
	    {
	        ir_app_parm.ir_merge.ir_keycode_total_len =  ((u16)code[4] << 8 | code[5])*4 + ((u16)code[6] << 8 | code[7])*4 + 8;
    	}
		printf(" ir_app_parm.ir_merge.ir_keycode_total_len=%x,codesize=%x\r\n", ir_app_parm.ir_merge.ir_keycode_total_len,code_size);
		memcpy(ir_table[ir_app_parm.ir_programming.programmed_key_count].code + ir_app_parm.ir_merge.ir_code_pos, code, code_size);
		ir_app_parm.ir_merge.ir_code_pos += code_size;
		
    }
	else
	{
	     memcpy(ir_table[ir_app_parm.ir_programming.programmed_key_count].code + ir_app_parm.ir_merge.ir_code_pos, code, code_size);
		 ir_app_parm.ir_merge.ir_code_pos += code_size;
	}
	if(ir_app_parm.ir_merge.ir_code_pos == ir_app_parm.ir_merge.ir_keycode_total_len)
	{
	     len = ir_app_parm.ir_merge.ir_keycode_total_len;
		 ir_table[ir_app_parm.ir_programming.programmed_key_count].key_id = key_id;
         ir_table[ir_app_parm.ir_programming.programmed_key_count].code_size = len;
         ir_table[ir_app_parm.ir_programming.programmed_key_count].button_idx = button_index;
		 flash_addr_pos += button_index*256;
		 ir_flash_save(flash_addr_pos,len+5);
		
		 for(u8 i=0;i<len;i++ )
         {
             printf(" %x\r\n",*(ir_table[ir_app_parm.ir_programming.programmed_key_count].code+i));
         }
		 ir_app_parm.ir_programming.programmed_key_count++;
	}
}

u8 ir_table_add_key(u16 key_id, u16 code_size, u8* code)
{
    u8 button_index=0;

    button_index = ir_get_button_idx_from_key_id(key_id);
    if (button_index == KEY_IDX_NULL)
    {
        return false;
    }
    if ((ir_app_parm.ir_programming.programmed_key_count < MAX_KEY_COUNT)
        && (code_size <= MAX_CODE_LENGTH))
    {
        printf("ir_table[programmed_key_count].key_id=%x \r\n",key_button_map[button_index].button);
        printf("ir_table[programmed_key_count].code_size=%x \r\n",code_size);
        printf("ir_table[programmed_key_count].button_idx=%x \r\n",button_index);
        ir_code_merge(key_button_map[button_index].button,button_index,code_size,code);
    }

    return true;
}

void ir_fallback_process(u16 handle, u8* buf, u8 len)
{
    u8 index;

	#if 0
    if (handle < ATV_IR_SERVICE_IDX || handle > ATV_IR_KEY_EVENT_CCCD_IDX)
    {
        //A programming session shell always be performed on one ATT Bearer.
        if (ir_app_parm.ir_programming.programming_start == 1)
        {
            ir_app_parm.ir_programming.programming_start = 0;
            if (ir_app_parm.programming_timer)
            {
                blt_soft_timer_delete(ir_programming_timeoutcb);
                ir_app_parm.programming_timer = NULL;
            }
            ir_table_init();
            printf("ir_table programming is interrupted.\r\n");
        }
        return;
    }
	#endif

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
                    ir_table_init();
                    if(ir_app_parm.programming_timer == NULL)
                    {
                        ir_app_parm.programming_timer = 1;
                        blt_soft_timer_add(&ir_programming_timeoutcb, PROGRAMMING_TIMEOUT);
                    }
                }
                else
                {
                    printf("ir table programming end\r\n");
					extern u32    ir_flash_erase_tick;
					ir_flash_erase_tick |= clock_time() | 1;
                    if (ir_app_parm.programming_timer)
                    {
                        blt_soft_timer_delete(ir_programming_timeoutcb);
                        ir_app_parm.programming_timer = NULL;
                    }
                }
            }
            break;
        }
        case ATV_IR_KEY_ID_IDX:
        {
            if ((ir_app_parm.ir_programming.programming_start == 1)
                && (ir_app_parm.ir_programming.current_programming_key_id == INVALID_KEY_ID)
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
                blt_soft_timer_add(&ir_programming_timeoutcb, PROGRAMMING_TIMEOUT);
                ir_app_parm.programming_timer = 1;
            }
            break;
        }
        case ATV_IR_CODE_IDX:
        {
            if(ir_flash_erase_flag)
            {
                ir_app_parm.ir_programming.programming_start = 0;
				printf("ir flash eraseall not succ\r\n");
            }
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
                blt_soft_timer_add(&ir_programming_timeoutcb, PROGRAMMING_TIMEOUT);
                ir_app_parm.programming_timer = 1;
            }
            break;
        }
        case ATV_IR_SUPPRESS_IDX:
        {
            if(len == 2)
            {
                ir_app_parm.ir_programming.current_programming_key_id = buf[0];
                ir_app_parm.ir_programming.current_programming_key_id <<= 8;
                ir_app_parm.ir_programming.current_programming_key_id |= buf[1];
                index = ir_get_suppress_index_from_keyid_idx(ir_app_parm.ir_programming.current_programming_key_id);
                if(ir_app_parm.ir_suppress[index])
                {
                    ir_app_parm.ir_suppress[index] = 0;
                }
                else
                    ir_app_parm.ir_suppress[index] = 1;
                ir_app_parm.ir_programming.current_programming_key_id = INVALID_KEY_ID;

            }
            break;
        }
        case ATV_IR_KEY_EVENT_CCCD_IDX:
        {
            if (len == 2)
            {
                printf("ccc buf[0]=%x\r\n",buf[0]);
                ir_app_parm.ir_programming.key_notification_flag = buf[0];
            }
            break;
        }
        default:
        {
            printf("!!unhandled IDX\r\n");
        }
        break;
    }
}

void ir_flash_check(u32 addr)
{
    u8 button_index=0,i;
	u16 code_size=0;
	u8 buf[256];
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
        memcpy(ir_table[button_index].code, &buf[5], code_size);
		printf("button =%x \r\n",button_index);
		for(u8 k=0;k<code_size;k++ )
        {
            printf(" %x",ir_table[button_index].code[k]);
        }
		printf("\r\n");
	}
}

void ir_param_init(void)
{
    u8 sect;
	u32 addr;

    flash_read_page(IR_DATA_SECT_2_ADDR, 1, &sect);
	if(sect == 0xff) 
	{
	    ir_flash_sector = 0;
	}
	else
	{
	    ir_flash_sector = 1;
	}
	addr = IR_DATA_SECT_1_ADDR + ir_flash_sector*0x1000;
	printf("app_ir_param_init addr=%x\r\n",addr);
	ir_flash_check(addr);
	if(ir_flash_sector)
		ir_flash_sector = 0;
	else
		ir_flash_sector = 1;
}




