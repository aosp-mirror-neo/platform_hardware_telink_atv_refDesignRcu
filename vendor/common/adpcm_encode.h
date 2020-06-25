/********************************************************************************************************
 * @file     adpcm_encode.h
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
 *******************************************************************************************************/

#ifndef __ADPCM_ENCODE_H_
#define __ADPCM_ENCODE_H_


extern s16 audio_raw_buffer[];
extern u16 gAdpcmEnSerialNum;


void adpcm_encode_init(void);

u8 *adpcm_encode_get_data(void);
void adpcm_encode_inc_data(void);


void adpcm_gol_encode_handler(u8 flag);


#endif /* __AUDIO_ENCODE_H_ */
