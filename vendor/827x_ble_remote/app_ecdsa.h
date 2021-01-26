/*
 * app_ecdsa.h
 *
 *  Created on: 2020-12-4
 *      Author: Telink
 */

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
