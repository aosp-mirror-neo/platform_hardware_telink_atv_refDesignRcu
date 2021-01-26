/******************************************************************************
 * @file     app_pke.h
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


#pragma once

#include "../app_config.h"
#include "tl_common.h"
#include "drivers.h"

#define GET_MAX_LEN(a,b)         (((a)>(b))?(a):(b))

enum TRNG_RET_CODE
{
    TRNG_SUCCESS = 0,
    TRNG_ERROR,
};

//KDF define
typedef void *(*KDF_FUNC)(const void *input, unsigned int byteLen, unsigned char *key, unsigned int keyByteLen);
/**
 * @brief        get ECCP key pair(the key pair could be used in ECDSA/ECDH)
 * @param[in]    eccp_curve    - eccp_curve_t struct pointer
 * @param[in]    priKey        - private key, big-endian
 * @param[in]    pubKey        - public key, big-endian
 * @return        PKE_SUCCESS(success), other(error)
 */
unsigned char eccp_getkey(eccp_curve_t *curve, unsigned char *priKey, unsigned char *pubKey);
/**
 * @brief        get rand
 * @param[in]     rand    - the buffer rand
 * @param[in]     byteLen    - the length of rand
 * @return        0:TRNG_SUCCESS,   1:TRNG_ERROR
 */
//unsigned char get_rand(unsigned char *rand, unsigned int byteLen);
unsigned char rand_get(unsigned char *rand, unsigned int byteLen);


