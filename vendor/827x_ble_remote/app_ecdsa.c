/********************************************************************************************************
 * @file     app_ecdsa.c
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
 *********************************************************************************************************/
#include "tl_common.h"
#include "drivers.h"
#include "app_ecdsa.h"
#include "./pke_common/app_pke.h"


/*
unsigned int secp256r1_p[8]    = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x00000000,0x00000000,0x00000001,0xFFFFFFFF};
unsigned int secp256r1_p_h[8]  = {0x00000003,0x00000000,0xFFFFFFFF,0xFFFFFFFB,0xFFFFFFFE,0xFFFFFFFF,0xFFFFFFFD,0x00000004};
unsigned int secp256r1_p_n1[1] = {1};
unsigned int secp256r1_a[8]    = {0xFFFFFFFC,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x00000000,0x00000000,0x00000001,0xFFFFFFFF};
unsigned int secp256r1_b[8]    = {0x27D2604B,0x3BCE3C3E,0xCC53B0F6,0x651D06B0,0x769886BC,0xB3EBBD55,0xAA3A93E7,0x5AC635D8};
unsigned int secp256r1_Gx[8]   = {0xD898C296,0xF4A13945,0x2DEB33A0,0x77037D81,0x63A440F2,0xF8BCE6E5,0xE12C4247,0x6B17D1F2};
unsigned int secp256r1_Gy[8]   = {0x37BF51F5,0xCBB64068,0x6B315ECE,0x2BCE3357,0x7C0F9E16,0x8EE7EB4A,0xFE1A7F9B,0x4FE342E2};
unsigned int secp256r1_n[8]    = {0xFC632551,0xF3B9CAC2,0xA7179E84,0xBCE6FAAD,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0xFFFFFFFF};
unsigned int secp256r1_n_h[8]  = {0xBE79EEA2,0x83244C95,0x49BD6FA6,0x4699799C,0x2B6BEC59,0x2845B239,0xF3D95620,0x66E12D94};
unsigned int secp256r1_n_n1[1] = {0xEE00BC4F};

eccp_curve_t secp256r1[1]={
{   256,
    256,
    secp256r1_p,
    secp256r1_p_h,
    secp256r1_p_n1,
    secp256r1_a,
    secp256r1_b,
    secp256r1_Gx,
    secp256r1_Gy,
    secp256r1_n,
    secp256r1_n_h,
    secp256r1_n_n1,}
};
*/

/**
 * @brief        Generate ECDSA Signature in U32 little-endian big integer style
 * @param[in]    curve    - ecc curve struct pointer, please make sure it is valid
 * @param[in]    e        - derived from hash value
 * @param[in]    k        - internal random integer k
 * @param[in]    dA        - private key
 * @param[out]    r        - signature r
 * @param[out]    s        - signature s
 * @return        ECDSA_SUCCESS(success); other(error)
 */
unsigned char ecdsa_sign4(eccp_curve_t *curve, unsigned int *e, unsigned int *k, unsigned int *dA, unsigned int *r, unsigned int *s)
{
    unsigned char ret;
    unsigned int nWordLen;
    unsigned int pWordLen;
    unsigned int tmp1[ECC_MAX_WORD_LEN];

    if(0 == curve || 0 == e || 0 == k || 0 == dA || 0 == r || 0 == s)
    {
        return ECDSA_POINTOR_NULL;
    }

    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
    pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);

    if(curve->eccp_p_bitLen > ECC_MAX_BIT_LEN)
    {
        return ECDSA_INVALID_INPUT;
    }

    //make sure k in [1, n-1]
    if(ismemzero4(k, nWordLen<<2))
    {
        return ECDSA_ZERO;
    }
    if(big_integer_compare(k, nWordLen, curve->eccp_n, nWordLen) >= 0)
    {
        return ECDSA_LARGE_INTEGER;
    }

    //get x1
    ret = pke_eccp_point_mul(curve, k, curve->eccp_Gx, curve->eccp_Gy, tmp1, 0);  //y coordinate is not needed
    if(PKE_SUCCESS != ret)
    {
        return ret;
    }

    //r = x1 mod n
    ret = pke_mod(tmp1, pWordLen, curve->eccp_n, curve->eccp_n_h, curve->eccp_n_n1, nWordLen, r);
    if(PKE_SUCCESS != ret)
    {
        return ret;
    }

    //make sure r is not zero
    if(ismemzero4(r, nWordLen<<2))
    {
        return ECDSA_ZERO;
    }

    //tmp1 =  r*dA mod n
    if(curve->eccp_n_h && curve->eccp_n_n1)
    {
        pke_load_pre_calc_mont(curve->eccp_n_h, curve->eccp_n_n1, nWordLen);
        ret = pke_mod_mul(curve->eccp_n, r, dA, tmp1, nWordLen);
    }
    if(PKE_SUCCESS != ret)
    {
        return ret;
    }

    //tmp1 = e + r*dA mod n
    ret = pke_mod_add(curve->eccp_n, e, tmp1, tmp1, nWordLen);
    if(PKE_SUCCESS != ret)
    {
        return ret;
    }

    //s = k^(-1) mod n
    ret = pke_mod_inv(curve->eccp_n, k, s, nWordLen, nWordLen);
    if(PKE_SUCCESS != ret)
    {
        return ret;
    }

    //s = (k^(-1))*(e + r*dA) mod n
    ret = pke_mod_mul(curve->eccp_n, s, tmp1, s, nWordLen);
    if(PKE_SUCCESS != ret)
    {
        return ret;
    }

    //make sure s is not zero
    if(ismemzero4(s, nWordLen<<2))
    {
        return ECDSA_ZERO;
    }

    return ECDSA_SUCCESS;
}
/**
 * @brief        Generate ECDSA Signature in octet string style
 * @param[in]    curve        - ecc curve struct pointer, please make sure it is valid
 * @param[in]    E            - hash value, big-endian
 * @param[in]    EByteLen    - byte length of E
 * @param[in]    priKey        - private key, big-endian
 * @param[out]    signature    - signature r and s, big-endian
 * @return        ECDSA_SUCCESS(success); other(error)
 */
unsigned char ecdsa_sign(eccp_curve_t *curve, unsigned char *E, unsigned int EByteLen, unsigned char *rand_k, unsigned char *priKey,
                   unsigned char *signature)
{
    unsigned int tmpLen;
    unsigned int nByteLen;
    unsigned int nWordLen;
    unsigned int e[ECC_MAX_WORD_LEN], k[ECC_MAX_WORD_LEN], dA[ECC_MAX_WORD_LEN];
    unsigned int r[ECC_MAX_WORD_LEN], s[ECC_MAX_WORD_LEN];
    unsigned char ret;

    if(0 == curve || 0 == E || 0 == priKey || 0 == signature)
    {
        return ECDSA_POINTOR_NULL;
    }

    if(0 == EByteLen)
    {
        return ECDSA_INVALID_INPUT;
    }

    if(curve->eccp_p_bitLen > ECC_MAX_BIT_LEN)
    {
        return ECDSA_INVALID_INPUT;
    }

    nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);

    //get integer e from hash value E(according to SEC1-V2 2009)
    memset(e, 0, nWordLen<<2);
    if(curve->eccp_n_bitLen >= (EByteLen<<3)) //in this case, make E as e directly
    {
        memcpy((unsigned char *)e, (unsigned char *)E, EByteLen);
    }
    else                                     //in this case, make left eccp_n_bitLen bits of E as e
    {
        memcpy((unsigned char *)e, (unsigned char *)E, nByteLen);
        tmpLen = (curve->eccp_n_bitLen)&7;
        if(tmpLen)
        {
            div2n_u32(e, nWordLen, 8-tmpLen);
        }
    }

    //get e = e mod n, i.e., make sure e in [0, n-1]
    if(big_integer_compare(e, nWordLen, curve->eccp_n, nWordLen) >= 0)
    {
        sub_u32(e, curve->eccp_n, k, nWordLen);
        memcpy(e, k, nWordLen<<2);
    }

    //make sure priKey in [1, n-1]
    memset(dA, 0, (nWordLen<<2)-nByteLen);
    memcpy((unsigned char *)dA, (unsigned char *)priKey, nByteLen);
    if(ismemzero4(dA, nWordLen<<2))
    {
        return ECDSA_ZERO;
    }
    if(big_integer_compare(dA, nWordLen, curve->eccp_n, nWordLen) >= 0)
    {
        return ECDSA_LARGE_INTEGER;
    }

    //get k
    memset(k, 0, (nWordLen<<2)-nByteLen);
    if(rand_k)
    {
        memcpy((unsigned char *)k, rand_k, nByteLen);
    }
    else
    {
ECDSA_SIGN_LOOP:

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
    }

    //sign
    ret = ecdsa_sign4(curve, e, k, dA, r, s);
    if(ECDSA_ZERO == ret || ECDSA_LARGE_INTEGER == ret)
    {
        if(rand_k)
        {
            return ret;
        }
        else
        {
            goto ECDSA_SIGN_LOOP;
        }
    }
    else if(ECDSA_SUCCESS != ret)
    {
        return ret;
    }

    memcpy(signature, (unsigned char *)r, nByteLen);
    memcpy(signature+nByteLen, (unsigned char *)s, nByteLen);

    return ECDSA_SUCCESS;
}
/**
 * @brief        Verify ECDSA Signature in octet string style
 * @param[in]    curve        - ecc curve struct pointer, please make sure it is valid
 * @param[in]    E            - hash value, big-endian
 * @param[in]    EByteLen    - byte length of E
 * @param[in]    pubKey        - public key, big-endian
 * @param[in]    signature    - signature r and s, big-endian
 * @return        ECDSA_SUCCESS(success); other(error)
 */

u8 B_proc = 0;
unsigned char ecdsa_verify(eccp_curve_t *curve, unsigned char *E, unsigned int EByteLen, unsigned char *pubKey, unsigned char *signature)
{
    unsigned int tmpLen;
    unsigned int nByteLen;
    unsigned int nWordLen;
    unsigned int pByteLen;
    unsigned int pWordLen;
    unsigned int maxWordLen;
    unsigned int e[ECC_MAX_WORD_LEN], r[ECC_MAX_WORD_LEN], s[ECC_MAX_WORD_LEN];
    unsigned int tmp[ECC_MAX_WORD_LEN], x[ECC_MAX_WORD_LEN];
    unsigned char ret;

    if(0 == curve || 0 == E || 0 == pubKey || 0 == signature)
    {
        return ECDSA_POINTOR_NULL;
    }

    if(0 == EByteLen)
    {
        return ECDSA_INVALID_INPUT;
    }

    if(curve->eccp_p_bitLen > ECC_MAX_BIT_LEN)
    {
        return ECDSA_INVALID_INPUT;
    }

    nByteLen = GET_BYTE_LEN(curve->eccp_n_bitLen);
    nWordLen = GET_WORD_LEN(curve->eccp_n_bitLen);
    pByteLen = GET_BYTE_LEN(curve->eccp_p_bitLen);
    pWordLen = GET_WORD_LEN(curve->eccp_p_bitLen);

    maxWordLen = GET_MAX_LEN(nWordLen,pWordLen);

    //make sure r in [1, n-1]
    memset(r, 0, (nWordLen<<2)-nByteLen);
    swapX(signature, (unsigned char *)r, nByteLen);
    //memcpy((unsigned char *)r, signature, nByteLen);
    if(ismemzero4(r, nWordLen<<2))
    {
        return ECDSA_ZERO;
    }
    if(big_integer_compare(r, nWordLen, curve->eccp_n, nWordLen) >= 0)
    {
        return ECDSA_LARGE_INTEGER;
    }

    //make sure s in [1, n-1]
    memset(s, 0, (nWordLen<<2)-nByteLen);
    swapX(signature+nByteLen, (unsigned char *)s, nByteLen);
    //memcpy((unsigned char *)s, signature+nByteLen, nByteLen);
    if(ismemzero4(s, nWordLen<<2))
    {
        return ECDSA_ZERO;
    }
    if(big_integer_compare(s, nWordLen, curve->eccp_n, nWordLen) >= 0)
    {
        return ECDSA_LARGE_INTEGER;
    }

    //tmp = s^(-1) mod n
    ret = pke_mod_inv(curve->eccp_n, s, tmp, nWordLen, nWordLen);
    if(PKE_SUCCESS != ret)
    {
        return ret;
    }

    //get integer e from hash value E(according to SEC1-V2 2009)
    memset(e, 0, nWordLen<<2);
    if(curve->eccp_n_bitLen >= (EByteLen<<3)) //in this case, make E as e directly
    {
        swapX((unsigned char *)E, (unsigned char *)e, nByteLen);
        //memcpy((unsigned char *)e, (unsigned char *)E, EByteLen);
    }
    else                                     //in this case, make left eccp_n_bitLen bits of E as e
    {
        memcpy(e, E, nByteLen);
        swapX((unsigned char *)E, (unsigned char *)e, nByteLen);
        //memcpy((unsigned char *)e, (unsigned char *)E, nByteLen);
        tmpLen = (curve->eccp_n_bitLen)&7;
        if(tmpLen)
        {
            div2n_u32(e, nWordLen, 8-tmpLen);
        }
    }

    //get e = e mod n, i.e., make sure e in [0, n-1]
    if(big_integer_compare(e, nWordLen, curve->eccp_n, nWordLen) >= 0)
    {
        sub_u32(e, curve->eccp_n, x, nWordLen);
        memcpy(e, x, nWordLen<<2);
    }

    //x =  e*(s^(-1)) mod n
    if(curve->eccp_n_h && curve->eccp_n_n1)
    {
        pke_load_pre_calc_mont(curve->eccp_n_h, curve->eccp_n_n1, nWordLen);
        ret = pke_mod_mul(curve->eccp_n, e, tmp, x, nWordLen);
    }
    if(PKE_SUCCESS != ret)
    {
        return ret;
    }

    //tmp =  r*(s^(-1)) mod n
    ret = pke_mod_mul(curve->eccp_n, r, tmp, tmp, nWordLen);
    if(PKE_SUCCESS != ret)
    {
        return ret;
    }

    //check public key
    memset(e, 0, (maxWordLen<<2)-pByteLen);
    memset(s, 0, (maxWordLen<<2)-pByteLen);
    swapX(pubKey, (unsigned char *)e, pByteLen);
    swapX(pubKey+pByteLen, (unsigned char *)s, pByteLen);
//    memcpy((unsigned char *)e, pubKey, pByteLen);
//    memcpy((unsigned char *)s, pubKey+pByteLen, pByteLen);
    static int B_len = 0;
    B_len = pByteLen;
    ret = pke_eccp_point_verify(curve, e, s);
    B_proc = 4;
    if(PKE_SUCCESS != ret)
    {
        return ret;
    }

    ret = pke_eccp_point_mul(curve, tmp, e, s, e, s);
    B_proc = 5;
    if(PKE_SUCCESS != ret)
    {
        return ret;
    }
    B_proc = 6;
    if(!ismemzero4(x, nWordLen<<2))
    {
        ret = pke_eccp_point_mul(curve, x, curve->eccp_Gx, curve->eccp_Gy, x, tmp);
        if(PKE_SUCCESS != ret)
        {
            return ret;
        }
        B_proc = 7;
        ret = pke_eccp_point_add(curve, e, s, x, tmp, e, s);
        if(PKE_SUCCESS != ret)
        {
            return ret;
        }
    }

    //x = x1 mod n
    ret = pke_mod(e, pWordLen, curve->eccp_n, curve->eccp_n_h, curve->eccp_n_n1, nWordLen, tmp);
    if(PKE_SUCCESS != ret)
    {
        return ret;
    }
    B_proc = 8;
    if(big_integer_compare(tmp, nWordLen, r, nWordLen))
    {
        return ECDSA_VERIFY_FAIL;
    }

    return ECDSA_SUCCESS;
}




