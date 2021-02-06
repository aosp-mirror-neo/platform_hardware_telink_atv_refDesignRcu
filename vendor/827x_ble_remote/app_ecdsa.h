/******************************************************************************
 * @file     app_ecdsa.h
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

#ifndef APP_ECDSA_H_
#define APP_ECDSA_H_

#pragma once


//ECDSA return code
enum ECDSA_RET_CODE
{
    ECDSA_SUCCESS = PKE_SUCCESS,
    ECDSA_POINTOR_NULL = PKE_SUCCESS+0x50,
    ECDSA_INVALID_INPUT,
    ECDSA_ZERO,
    ECDSA_LARGE_INTEGER,
    ECDSA_VERIFY_FAIL,
};

extern eccp_curve_t secp256r1[1];

//unsigned char ecdsa_rand_test(eccp_curve_t *curve);
unsigned char ecdsa_verify(eccp_curve_t *curve, unsigned char *E, unsigned int EByteLen, unsigned char *pubKey, unsigned char *signature);
#endif /* APP_ECDSA_H_ */
