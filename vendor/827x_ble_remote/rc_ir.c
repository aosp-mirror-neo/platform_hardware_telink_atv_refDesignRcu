/******************************************************************************
 * @file     rc_ir.c
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

#include "rc_ir.h"
#include "app_ir.h"

extern void         bls_pm_setSuspendMask (u8 mask);
#if (REMOTE_IR_ENABLE)


#define ADD_REPEAT_ONE_BY_ONE            1

/////////////  NEC  protocol  /////////////////////////////////////////////
//start
#define IR_INTRO_CARR_TIME_NEC          9000
#define IR_INTRO_NO_CARR_TIME_NEC       4500
//stop
#define IR_END_TRANS_TIME_NEC           563  // user define
//repeat
#define IR_REPEAT_CARR_TIME_NEC         9000
#define IR_REPEAT_NO_CARR_TIME_NEC      2250
#define IR_REPEAT_LOW_CARR_TIME_NEC     560
//data "1"
#define IR_HIGH_CARR_TIME_NEC           560
#define IR_HIGH_NO_CARR_TIME_NEC        1690
//data "0"
#define IR_LOW_CARR_TIME_NEC            560
#define IR_LOW_NO_CARR_TIME_NEC         560



pwm_dma_data_t T_dmaData_buf;

u16 waveform_start_bit_1st;
u16 waveform_start_bit_2nd;
u16 waveform_stop_bit_1st;
u16 waveform_stop_bit_2nd;

u16 waveform_logic_0_1st;
u16 waveform_logic_0_2nd;
u16 waveform_logic_1_1st;
u16 waveform_logic_1_2nd;

u16 waveform_repeat_1st;
u16 waveform_repeat_2nd;
u16 waveform_repeat_3rd;
u16 waveform_repeat_4th;

u16 waveform_wait_to_repeat;

int ir_sending_check(void);




void ir_nec_send(u8 addr1, u8 addr2, u8 cmd)
{

    if(ir_send_ctrl.last_cmd != cmd)
    {
        if(ir_sending_check())
        {
            return;
        }


        ir_send_ctrl.last_cmd = cmd;

    //// set waveform input in sequence //////
        T_dmaData_buf.data_num = 0;

        //waveform for start bit
        T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_start_bit_1st;
        T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_start_bit_2nd;


        //add data
        u32 data = (~cmd)<<24 | cmd<<16 | addr2<<8 | addr1;
        for(int i=0;i<32;i++){
            if(data & BIT(i)){
                //waveform for logic_1
                T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_logic_1_1st;
                T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_logic_1_2nd;
            }
            else{
                //waveform for logic_0
                T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_logic_0_1st;
                T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_logic_0_2nd;
            }
        }

        //waveform for stop bit
        T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_stop_bit_1st;
        T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_stop_bit_2nd;


        T_dmaData_buf.dma_len = T_dmaData_buf.data_num * 2;

        if(1){  //need repeat
            ir_send_ctrl.repeat_enable = 1;  //need repeat signal
        }
        else{  //no need repeat
            ir_send_ctrl.repeat_enable = 0;  //no need repeat signal
        }


        reg_pwm_irq_sta = FLD_IRQ_PWM0_IR_DMA_FIFO_DONE;   //clear  dma fifo mode done irq status
        reg_pwm_irq_mask |= FLD_IRQ_PWM0_IR_DMA_FIFO_DONE; //enable dma fifo mode done irq mask


		bls_pm_setSuspendMask(0);
        ir_send_ctrl.is_sending = IR_SENDING_DATA;

        ir_send_ctrl.sending_start_time = clock_time();

//        gpio_set_func(GPIO_PB3, AS_PWM0_N);
        pwm_start_dma_ir_sending();

    }
}






int ir_is_sending()
{
    if(ir_send_ctrl.is_sending && clock_time_exceed(ir_send_ctrl.sending_start_time, 300*1000))
    {
        ir_send_ctrl.is_sending = IR_SENDING_NONE;

        pwm_stop_dma_ir_sending();
    }

    return ir_send_ctrl.is_sending;
}

int ir_sending_check(void)
{
    u8 r = irq_disable();
    if(ir_is_sending()){
        irq_restore(r);
        return 1;
    }
    irq_restore(r);
    return 0;
}


#if 1
void ir_send_release(void)
{
    u8 r = irq_disable();

    if(ir_send_ctrl.is_full_sequence == 0)
    {
        printf("ir_send_release (not full sequence)\r\n");
        ir_send_ctrl.last_cmd = 0xff;
        ir_send_ctrl.is_sending = IR_SENDING_STOP;
    }
    else
    {
        printf("ir_send_release (is full sequence)\r\n");
        ir_send_ctrl.last_cmd = 0xff;

#if(!ADD_REPEAT_ONE_BY_ONE)
        if(ir_send_ctrl.is_sending != IR_SENDING_NONE){
            pwm_stop_dma_ir_sending();
        }
#endif

        ir_send_ctrl.is_sending = IR_SENDING_NONE;
        ir_send_ctrl.is_full_sequence = 0;
        pwm_stop_dma_ir_sending();
        reg_pwm_irq_sta = FLD_IRQ_PWM0_IR_DMA_FIFO_DONE;   //clear irq status
        reg_pwm_irq_mask &= ~FLD_IRQ_PWM0_IR_DMA_FIFO_DONE; //disable irq mask
    }
    irq_restore(r);
}
#else
void ir_send_release(void)
{
    printf("ir_send_release\r\n");
    ir_send_ctrl.last_cmd = 0xff;
    ir_send_ctrl.is_sending = IR_SENDING_STOP;
}

#endif




//int AA_pwm_irq_cnt = 0;

#if (ADD_REPEAT_ONE_BY_ONE)



#if (REMOTE_IR_ENABLE)
_attribute_ram_code_
#endif
void rc_ir_irq_prc(void)
{
   // printf("rc_ir_irq_prc\r\n");
    if(reg_pwm_irq_sta & FLD_IRQ_PWM0_IR_DMA_FIFO_DONE){
        reg_pwm_irq_sta = FLD_IRQ_PWM0_IR_DMA_FIFO_DONE; //clear irq status
        ir_send_ctrl.is_full_sequence = 0x55;
        if(is_programming_key_send() == 0)
        {
            if(ir_send_ctrl.repeat_enable){

                if(ir_send_ctrl.is_sending == IR_SENDING_DATA){
                    ir_send_ctrl.is_sending = IR_SENDING_REPEAT;

                    T_dmaData_buf.data_num = 0;

                    u32 tick_2_repeat_sysClockTimer16M = 107*CLOCK_16M_SYS_TIMER_CLK_1MS - (clock_time() - ir_send_ctrl.sending_start_time);
                    u32 tick_2_repeat_sysTimer = (tick_2_repeat_sysClockTimer16M*CLOCK_SYS_CLOCK_1US>>4);


                    waveform_wait_to_repeat = pwm_config_dma_fifo_waveform(0, PWM0_PULSE_NORMAL, tick_2_repeat_sysTimer/PWM_CARRIER_CYCLE_TICK);

                    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_wait_to_repeat;


                    T_dmaData_buf.dma_len = T_dmaData_buf.data_num * 2;
                    //printf("IR_SENDING_DATA\r\n");
                    pwm_start_dma_ir_sending();
                }
                else if(ir_send_ctrl.is_sending == IR_SENDING_REPEAT){
                    //printf("IR_SENDING_REPEAT\r\n");
                    T_dmaData_buf.data_num = 0;

                    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_repeat_1st;
                    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_repeat_2nd;
                    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_repeat_3rd;
                    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_repeat_4th;

                    T_dmaData_buf.dma_len = T_dmaData_buf.data_num * 2;
                    pwm_start_dma_ir_sending();

                }
                else
                {
                    printf("nec ir irq release\r\n");

                    reg_pwm_irq_mask &= ~FLD_IRQ_PWM0_IR_DMA_FIFO_DONE; //disable irq mask
                    ir_send_ctrl.is_sending = IR_SENDING_NONE;
 
                }
            }
            else{
                 printf("ir irq release\r\n");
                 reg_pwm_irq_mask &= ~FLD_IRQ_PWM0_IR_DMA_FIFO_DONE; //disable irq mask
                 ir_send_ctrl.is_sending = IR_SENDING_NONE;
            }
        }
        else
        {
            if(ir_type_read())
            {
                pwm_start_dma_ir_sending();
            }
        }

    }
}





#else

#if (REMOTE_IR_ENABLE)
_attribute_ram_code_
#endif
void rc_ir_irq_prc(void)
{

    if(reg_pwm_irq_sta & FLD_IRQ_PWM0_IR_DMA_FIFO_DONE){
        reg_pwm_irq_sta = FLD_IRQ_PWM0_IR_DMA_FIFO_DONE; //clear irq status

       if(is_programming_key_send() == 0)
       {

        if(ir_send_ctrl.repeat_enable){

            if(ir_send_ctrl.is_sending == IR_SENDING_DATA){
                ir_send_ctrl.is_sending = IR_SENDING_REPEAT;

                T_dmaData_buf.data_num = 0;

                u32 tick_2_repeat_sysClockTimer16M = 110*CLOCK_16M_SYS_TIMER_CLK_1MS - (clock_time() - ir_send_ctrl.sending_start_time);
                u32 tick_2_repeat_sysTimer = (tick_2_repeat_sysClockTimer16M*CLOCK_SYS_CLOCK_1US>>4);


                waveform_wait_to_repeat = pwm_config_dma_fifo_waveform(0, PWM0_PULSE_NORMAL, tick_2_repeat_sysTimer/PWM_CARRIER_CYCLE_TICK);

                T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_wait_to_repeat;

                for(int i=0;i<5;i++){
                    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_repeat_1st;
                    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_repeat_2nd;
                    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_repeat_3rd;
                    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_repeat_4th;
                }

                T_dmaData_buf.dma_len = T_dmaData_buf.data_num * 2;

                pwm_start_dma_ir_sending();
            }
            else if(ir_send_ctrl.is_sending == IR_SENDING_REPEAT){

                T_dmaData_buf.data_num = 0;
                for(int i=0;i<5;i++){
                    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_repeat_1st;
                    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_repeat_2nd;
                    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_repeat_3rd;
                    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_repeat_4th;
                }

                T_dmaData_buf.dma_len = T_dmaData_buf.data_num * 2;
                pwm_start_dma_ir_sending();

            }
            else
            {
                printf("ir irq release\r\n");
                reg_pwm_irq_mask &= ~FLD_IRQ_PWM0_IR_DMA_FIFO_DONE; //disable irq mask
                ir_send_ctrl.is_sending = IR_SENDING_NONE;

            }
        }
        else{

            //ir_send_release();
             printf("ir irq release\r\n");
             reg_pwm_irq_mask &= ~FLD_IRQ_PWM0_IR_DMA_FIFO_DONE; //disable irq mask
             ir_send_ctrl.is_sending = IR_SENDING_NONE;
        }
      }
      else
      {
          if(ir_type_read())
          {
              pwm_start_dma_ir_sending();
          }
      }

    }

}


#endif







void rc_ir_init(void)
{

//only pwm0 support fifo mode
    pwm_n_revert(PWM0_ID);    //if use PWMx_N, user must set "pwm_n_revert" before gpio_set_func(pwmx_N).
    gpio_set_func(GPIO_PB3, AS_PWM0_N);
    pwm_set_mode(PWM0_ID, PWM_IR_DMA_FIFO_MODE);
    pwm_set_phase(PWM0_ID, 0);   //no phase at pwm beginning
    pwm_set_cycle_and_duty(PWM0_ID, PWM_CARRIER_CYCLE_TICK,  PWM_CARRIER_HIGH_TICK );     //config carrier: 38k, 1/3 duty


    pwm_set_dma_address(&T_dmaData_buf);


//logic_0, 560 us carrier,  560 us low
    waveform_logic_0_1st = pwm_config_dma_fifo_waveform(1, PWM0_PULSE_NORMAL, 560 * CLOCK_SYS_CLOCK_1US/PWM_CARRIER_CYCLE_TICK);
    waveform_logic_0_2nd = pwm_config_dma_fifo_waveform(0, PWM0_PULSE_NORMAL, 560 * CLOCK_SYS_CLOCK_1US/PWM_CARRIER_CYCLE_TICK);

//logic_1, 560 us carrier,  1690 us low
    waveform_logic_1_1st = pwm_config_dma_fifo_waveform(1, PWM0_PULSE_NORMAL, 560 * CLOCK_SYS_CLOCK_1US/PWM_CARRIER_CYCLE_TICK);
    waveform_logic_1_2nd = pwm_config_dma_fifo_waveform(0, PWM0_PULSE_NORMAL, 1690 * CLOCK_SYS_CLOCK_1US/PWM_CARRIER_CYCLE_TICK);


//start bit, 9000 us carrier,  4500 us low
    waveform_start_bit_1st = pwm_config_dma_fifo_waveform(1, PWM0_PULSE_NORMAL, 9000 * CLOCK_SYS_CLOCK_1US/PWM_CARRIER_CYCLE_TICK);
    waveform_start_bit_2nd = pwm_config_dma_fifo_waveform(0, PWM0_PULSE_NORMAL, 4500 * CLOCK_SYS_CLOCK_1US/PWM_CARRIER_CYCLE_TICK);


//stop bit,  560 us carrier, 500 us low
    waveform_stop_bit_1st = pwm_config_dma_fifo_waveform(1, PWM0_PULSE_NORMAL, 560 * CLOCK_SYS_CLOCK_1US/PWM_CARRIER_CYCLE_TICK);
    waveform_stop_bit_2nd = pwm_config_dma_fifo_waveform(0, PWM0_PULSE_NORMAL, 500 * CLOCK_SYS_CLOCK_1US/PWM_CARRIER_CYCLE_TICK);



//repeat signal  first part,  9000 us carrier, 2250 us low
    waveform_repeat_1st = pwm_config_dma_fifo_waveform(1, PWM0_PULSE_NORMAL, 9000 * CLOCK_SYS_CLOCK_1US/PWM_CARRIER_CYCLE_TICK);
    waveform_repeat_2nd = pwm_config_dma_fifo_waveform(0, PWM0_PULSE_NORMAL, 2250 * CLOCK_SYS_CLOCK_1US/PWM_CARRIER_CYCLE_TICK);

//repeat signal  second part,  560 us carrier, 99190 us low(110 ms - 9000us - 2250us - 560us = 99190 us)
    waveform_repeat_3rd = pwm_config_dma_fifo_waveform(1, PWM0_PULSE_NORMAL, 560 * CLOCK_SYS_CLOCK_1US/PWM_CARRIER_CYCLE_TICK);
    waveform_repeat_4th = pwm_config_dma_fifo_waveform(0, PWM0_PULSE_NORMAL, 96190 * CLOCK_SYS_CLOCK_1US/PWM_CARRIER_CYCLE_TICK);




//add fifo stop irq, when all waveform send over, this irq will triggers
    //enable system irq PWM
    reg_irq_mask |= FLD_IRQ_SW_PWM_EN;

    //enable pwm0 fifo stop irq
    reg_pwm_irq_sta = FLD_IRQ_PWM0_IR_DMA_FIFO_DONE; //clear irq status
//    reg_pwm_irq_mask |= FLD_IRQ_PWM0_IR_DMA_FIFO_DONE;

    ir_send_ctrl.last_cmd = 0xff; //must
}


void  ir_send_specil (u16 freq, u8 cycle)
{
    u32 max_tick,cmp_tick;
    u8 length;

    printf("ir_send_specil\r\n");
    printf("cycle=%x\r\n",cycle);
    printf("freq=%x\r\n",freq);
    max_tick = CLOCK_SYS_CLOCK_HZ/freq;
    cmp_tick = cycle*max_tick/100;
    pwm_set_clk(CLOCK_SYS_CLOCK_HZ, CLOCK_SYS_CLOCK_HZ);
    pwm_n_revert(PWM0_ID);    //if use PWMx_N, user must set "pwm_n_revert" before gpio_set_func(pwmx_N).
    gpio_set_func(GPIO_PB3,AS_PWM0_N );
    pwm_set_mode(PWM0_ID, PWM_IR_DMA_FIFO_MODE);

    pwm_set_cycle_and_duty(PWM0_ID, max_tick, cmp_tick);

    length = T_dmaData_buf.data_num*2;
    printf("length=%x\r\n",length);
    unsigned char* buff = (unsigned char *)&T_dmaData_buf;
    buff[0]= length&0xff;
    buff[1]= (length>>8)&0xff;
    buff[2]= (length>>16)&0xff;
    buff[3]= (length>>24)&0xff;
    printf("T_dmaData_buf=\r\n");
    for(u8 i=0;i<T_dmaData_buf.data_num;i++)
        printf(" %x",T_dmaData_buf.data[i]);
    pwm_set_dma_address(&T_dmaData_buf);

    //enable pwm0 ir dma fifo done irq
    reg_pwm_irq_mask |= FLD_IRQ_PWM0_IR_DMA_FIFO_DONE;
    reg_irq_mask |= FLD_IRQ_SW_PWM_EN;

    //irq_enable();

	bls_pm_setSuspendMask(0);
    ir_send_ctrl.is_sending = IR_SENDING_DATA;
    ir_send_ctrl.repeat_enable = 0;

    pwm_start_dma_ir_sending();

    ir_send_ctrl.last_cmd = 0xff; //must
    ir_send_ctrl.is_full_sequence = 0;
}



#endif
