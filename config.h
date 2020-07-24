/******************************************************************************
 * @file     config.h
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


#define    CHIP_TYPE_8255      1
#define    CHIP_TYPE_8258      2
#define    CHIP_TYPE_8278      3


#ifndef CHIP_TYPE
#define    CHIP_TYPE             1000
#endif






#define    MCU_CORE_8255         1
#define    MCU_CORE_8258         2
#define    MCU_CORE_8278         3

#if(CHIP_TYPE == CHIP_TYPE_8255)
    #define MCU_CORE_TYPE    MCU_CORE_8255
#elif(CHIP_TYPE == CHIP_TYPE_8258)
    #define MCU_CORE_TYPE    MCU_CORE_8258
#elif(CHIP_TYPE == CHIP_TYPE_8278)
    #define MCU_CORE_TYPE    MCU_CORE_8278
#else
    #define MCU_CORE_TYPE    1000
#endif


