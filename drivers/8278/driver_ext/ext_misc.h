/******************************************************************************
 * @file     ext_misc.h
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
#ifndef DRIVERS_8278_EXT_MISC_H_
#define DRIVERS_8278_EXT_MISC_H_
#include "../register_8278.h"

/******************************* stimer_start ******************************************************************/
//#define reg_system_tick_irq         REG_ADDR32(0x744)//reg_system_tick_irq_level
enum {
    FLD_SYSTEM_TICK_IRQ_EN  =         BIT(2),
};


enum{
    FLD_DMA_RPTR_MASK =            0x0F, // max 15
};


/**
 * @brief    This function serves to set system timer capture tick.
 * @return  none
 */
void systimer_set_irq_capture(unsigned int sys_timer_tick);

/**
 * @brief    This function serves to get system timer capture tick.
 * @return  none
 */
static inline unsigned int systimer_get_irq_capture(void)
{
    return reg_system_tick_irq;
}

/**
 * @brief    This function serves to set system timer interrupt mask.
 * @return  none
 */
static inline void systimer_set_irq_mask(void)
{
    reg_system_irq_mask |= FLD_SYSTEM_TICK_IRQ_EN;
}

/**
 * @brief    This function serves to enable system timer interrupt.
 * @return  none
 */
static inline void systimer_irq_enable(void)
{
    reg_irq_mask |= FLD_IRQ_SYSTEM_TIMER;
}

/**
 * @brief    This function serves to disable system timer interrupt.
 * @return  none
 */
static inline void systimer_irq_disable(void)
{
    reg_irq_mask &= ~FLD_IRQ_SYSTEM_TIMER;
}

/**
 * @brief    This function serves to clear system timer interrupt status.
 * @return  none
 */
static inline void systimer_clr_irq_status(void)
{
    reg_irq_src = FLD_IRQ_SYSTEM_TIMER;
}

/**
 * @brief    This function serves to get system timer interrupt status.
 * @return  none
 */
static inline unsigned int systimer_get_irq_status(void)
{
    return reg_irq_src & FLD_IRQ_SYSTEM_TIMER;
}

static inline int tick1_exceed_tick2(unsigned int tick1, unsigned int tick2)
{
    return (unsigned int)(tick1 - tick2) < BIT(30);
}

/*
 * @brief     This function performs to get system timer tick.
 * @param[in] none.
 * @return    system timer tick value.
**/
static inline unsigned int sys_get_stimer_tick(void)
{
    return reg_system_tick; //same as API: clock_time()
}

/******************************* stimer_end ********************************************************************/


#include "../analog.h"
#define  analog_write_reg8  analog_write
#define  analog_read_reg8   analog_read

#endif




