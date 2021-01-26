/******************************************************************************
 * @file     app_pke.c
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

#include "app_pke.h"


extern unsigned int rnd_m_w;
/**
 * @brief        get rand
 * @param[in]     rand    - the buffer rand
 * @param[in]     byteLen    - the length of rand
 * @return        0:TRNG_SUCCESS,   1:TRNG_ERROR
 */
unsigned char rand_get(unsigned char *rand, unsigned int byteLen)
{
    unsigned int word_len;
    unsigned char left_len;

    if(0 == rand)
    {
        return TRNG_ERROR;
    }

    word_len = byteLen >> 2;
    left_len = byteLen & 0x3;

    // obtain the data by word
    while (word_len--)
    {
        random_generator_init();
        *((unsigned int *)rand) = rnd_m_w;
        rand += 4;
    }

    // if the byteLen is not aligned by word
    if (left_len)
    {
        random_generator_init();
        memcpy(rand, &rnd_m_w, left_len);
    }

    return TRNG_SUCCESS;
}
/**
 * @brief        get ECCP key pair(the key pair could be used in ECDSA/ECDH)
 * @param[in]    eccp_curve    - eccp_curve_t struct pointer
 * @param[in]    priKey        - private key, big-endian
 * @param[in]    pubKey        - public key, big-endian
 * @return        PKE_SUCCESS(success), other(error)
 */
unsigned char eccp_getkey(eccp_curve_t *curve, unsigned char *priKey, unsigned char *pubKey)
{
    unsigned int tmpLen;
    unsigned int nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
    unsigned int nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
    unsigned int pByteLen = GET_BYTE_LEN(curve->eccp_p_bitLen);
    unsigned int k[PKE_OPERAND_MAX_WORD_LEN] = {0};
    unsigned int x[PKE_OPERAND_MAX_WORD_LEN];
    unsigned int y[PKE_OPERAND_MAX_WORD_LEN];
    unsigned char ret;


ECCP_GETKEY_LOOP:
    ret = rand_get((unsigned char *)k, nByteLen);
    if(TRNG_SUCCESS != ret)
    {
        return ret;
    }

     //make sure k has the same bit length as n
    tmpLen = (curve->eccp_n_bitLen)&0x1F;
    if(tmpLen)
    {
        k[nWordLen-1] &= (1<<(tmpLen))-1;
    }

     //make sure k in [1, n-1]
    if(ismemzero4(k, nWordLen<<2))
    {
        goto ECCP_GETKEY_LOOP;
    }
    if(big_integer_compare(k, nWordLen, curve->eccp_n, nWordLen) >= 0)
    {
        goto ECCP_GETKEY_LOOP;
    }
     //get pubKey
    ret = pke_eccp_point_mul(curve, k, curve->eccp_Gx, curve->eccp_Gy, x, y);
    if(PKE_SUCCESS != ret)
    {
        return ret;
    }
    memcpy(priKey, (unsigned char *)k, nByteLen);
    memcpy(pubKey, (unsigned char *)x, pByteLen);
    memcpy(pubKey+pByteLen, (unsigned char *)y, pByteLen);

    return PKE_SUCCESS;
}
