/********************************************************************************************************
 * @file	app_fifio.c
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
#include "drivers.h"
#include "app_fifo.h"

/**********************************************************************
 * GLOBAL FUNCTIONS
 */

void app_fifo_init(app_fifo_t *f, int s, u8 n, u8 *p)
{
    f->size = s;
    f->num = n;
    f->wptr = 0;
    f->rptr = 0;
    f->p = p;
}

u8* app_fifo_wptr(app_fifo_t *f)
{
    if( ((f->wptr - f->rptr) & 255) < f->num)
    {
#ifdef FIFO_QUICK_MODE
        return f->p + (f->wptr & (f->num-1)) * f->size;
#else
        return f->p + (f->wptr % f->num) * f->size;
#endif
    }
    return 0;
}

void app_fifo_next(app_fifo_t *f)
{
    f->wptr++;
}

int app_fifo_push(app_fifo_t *f, u8 *p, int n)
{
    if( ((f->wptr - f->rptr) & 255) >= f->num)
    {
        return -1;
    }

    if(n > f->size)
    {
        return -2;
    }
#ifdef FIFO_QUICK_MODE
    u8 *pd = f->p + (f->wptr++ & (f->num-1)) * f->size;
#else
    u8 *pd = f->p + (f->wptr++ % f->num) * f->size;
#endif
    memcpy (pd, p, n);
    return 0;
}

void app_fifo_pop(app_fifo_t *f)
{
    f->rptr++;
}

u8* app_fifo_get(app_fifo_t *f)
{
    if(f->rptr != f->wptr)
    {
#ifdef FIFO_QUICK_MODE
        u8 *p = f->p + (f->rptr & (f->num-1)) * f->size;
#else
        u8 *p = f->p + (f->rptr % f->num) * f->size;
#endif
        return p;
    }
    return 0;
}


#ifdef FIFO_TEST_CODE_EN

void app_fifo_test(void){

#define FIFO_BUF_SIZE                   (8)
#define FIFO_BUF_NUM                    (5)

    u8 my_fifo_b[FIFO_BUF_SIZE * FIFO_BUF_NUM] = {0};
    app_fifo_t my_fifo = {
                            .size = FIFO_BUF_SIZE,
                            .num  = FIFO_BUF_NUM,
                            .wptr = 0,
                            .rptr = 0,
                            .p    = my_fifo_b};

    array_log_debug("my_fifo", (u8 *)&my_fifo, sizeof(app_fifo_t));

    u8 my_test_buf[] = {0,1,2,3,4,5,6};

    u8 i=0;
    for(;i<=FIFO_BUF_NUM;i++){
        my_test_buf[0] = i;
        app_fifo_push(&my_fifo, my_test_buf, sizeof(my_test_buf));
        array_log_debug("PUSH1", my_test_buf, sizeof(my_test_buf));

    }

    u8 *p_my_pop_buf = app_fifo_get(&my_fifo);

    while(p_my_pop_buf){
        array_log_debug("POP1", p_my_pop_buf, FIFO_BUF_SIZE);
        app_fifo_pop(&my_fifo);

        p_my_pop_buf = app_fifo_get(&my_fifo);
    }


    for(;i<=2*FIFO_BUF_NUM;i++){
        my_test_buf[0] = i;
        app_fifo_push(&my_fifo, my_test_buf, sizeof(my_test_buf));
        array_log_debug("PUSH2", my_test_buf, sizeof(my_test_buf));
    }

    p_my_pop_buf = app_fifo_get(&my_fifo);

    while(p_my_pop_buf){
        array_log_debug("POP2", p_my_pop_buf, FIFO_BUF_SIZE);
        app_fifo_pop(&my_fifo);

        p_my_pop_buf = app_fifo_get(&my_fifo);
    }
}




#endif

