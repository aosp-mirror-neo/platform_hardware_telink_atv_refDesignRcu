/********************************************************************************************************
 * @file	app_fifo.h
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

#ifndef APP_FIFO_H_
#define APP_FIFO_H_

/**********************************************************************
 * GLOBAL DEFINE
 */

/* The number of fifos in fast mode must be the factorial of 2 */
//#define FIFO_QUICK_MODE

/**********************************************************************
 * GLOBAL TYPES
 */

typedef struct {
    unsigned int size;
    unsigned short num;
    unsigned char wptr;
    unsigned char rptr;
    unsigned char *p;
}app_fifo_t;


/**********************************************************************
 * GLOBAL FUNCTIONS
 */

/**
 * @brief This function is used to initialize the structure
 * 
 * @param f - FIFO structure pointer
 * @param s - buffer size
 * @param n - Number of buffers
 * @param p - buffer pointer
 */
void app_fifo_init(app_fifo_t *f, int s, unsigned char n, unsigned char *p);

/**
 * @brief This function is used to obtain the pointer of the buffer that can be used
 * 
 * @param f - FIFO structure pointer
 * @return unsigned char* - NULL: There is no empty buffer other: Obtained buffer pointer
 */
unsigned char* app_fifo_wptr(app_fifo_t *f);

/**
 * @brief This function is used to increment the write pointer
 * 
 * @param f - FIFO structure pointer
 */
void app_fifo_next(app_fifo_t *f);

/**
 * @brief This function is used to put data into fifo
 * 
 * @param f - FIFO structure pointer
 * @param p - Pointer to the passed data
 * @param n - length of data 
 * @return int - 0:success other:Failure
 */
int app_fifo_push(app_fifo_t *f, unsigned char *p, int n);

/**
 * @brief This function is used to increment the read pointer
 * 
 * @param f - FIFO structure pointer
 */
void app_fifo_pop(app_fifo_t *f);

/**
 * @brief This function is used to get the data pointer in fifo
 * 
 * @param f - FIFO structure pointer
 * @return unsigned char* - NULL: There is no data other: data pointer
 */
unsigned char* app_fifo_get(app_fifo_t *f);

#endif /* APP_FIFO_H_ */
