/********************************************************************************************************
 * @file     adpcm_encode.c
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

#include "tl_common.h"
#include "drivers.h"
#include "adpcm_encode.h"

#if (BLE_AUDIO_ENABLE)

void adpcm_encode_voice_iir(signed short *ps, signed short *pd, int* coef, int nsample,u8 shift);
void j(u8 flag, s16 *pSrc, int srcSampleLen, s16 *pDst);


#define ADPCM_ANDROID_ID    0x00
#define	ADPCM_EN_BUFFER_PACKET_SIZE (AUDIO_ADPCM_PACKET_LENGTH*AUDIO_ADPCM_BUFFER_COUNT)


static const signed char sAdpcmEnIdxtbl[] = { -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8};
static const unsigned short sAdpcmEnSteptbl[] = {
	7,  8,  9,  10,  11,  12,  13,  14,  16,  17,
	19,  21,  23,  25,  28,  31,  34,  37,  41,  45,
	50,  55,  60,  66,  73,  80,  88,  97,  107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};
#if 0
static int sAdpcmLPFFilter1[10] = {739,87,739,2419,-1401};   //Don't change. they are good for all project.
static int sAdpcmLPFFilter2[10] = {4301,5262,4299,889,-3601};//Don't change. they are good for all project.
static u8 sAdpcmLPFFilterShift1 = 0;
static u8 sAdpcmLPFFilterShift2 = 0;
#endif
static int sAdpcmEnPredictIdx = 0;
static int sAdpcmEnPredict = 0;



static u8 sAdpcmEnBuffer[ADPCM_EN_BUFFER_PACKET_SIZE];

extern s16 audio_raw_buffer[];

static u8 sAdpcmEnBufferWptr = 0;
static u8 sAdpcmEnBufferRptr = 0;
u16 gAdpcmEnSerialNum = 0;


void adpcm_encode_init(void)
{
	sAdpcmEnPredictIdx = 0;
	sAdpcmEnPredict = 0;

	gAdpcmEnSerialNum = 0;
	sAdpcmEnBufferWptr = 0;
	sAdpcmEnBufferRptr = 0;
}

u8 *adpcm_encode_get_data(void)
{
	if (sAdpcmEnBufferRptr == sAdpcmEnBufferWptr){
		return 0;
	}
	return (sAdpcmEnBuffer+AUDIO_ADPCM_PACKET_LENGTH*(sAdpcmEnBufferRptr & (AUDIO_ADPCM_BUFFER_COUNT - 1)));
}
void adpcm_encode_inc_data(void)
{
	sAdpcmEnBufferRptr ++;
}

#if IIR_FILTER_ENABLE

	//inner band EQ default parameter. user need to set according to actual situation.
	//int filter_1[10] = {16630, -22724, 15507, 22724, -15753};
	//int filter_2[10] = {15380, 0, 14450, 0, -13446};
	//int filter_3[10] = {14961, 14869, 0, -13446, 0};

	int filter_1[10] = {995*4, 1990*4, 995*4, 849*4, 734*4};
	int filter_2[10] = {3691*4, -5564*4, 2915*4, 5564*4, -2510*4};
	int filter_3[10] = {2534*4, -1482*4, 955*4,  3956*4, -1866*4};

	u8  filter1_shift = 0;
	u8  filter2_shift = 0;
	u8  filter3_shift = 0;

//used for OOB processing. i.e LPF. user need to set according to actual situation
	int LPF_FILTER_1[10] = {739,87,739,2419,-1401};
	int LPF_FILTER_2[10] = {4301,5262,4299,889,-3601};

	u8  lpf_filter1_shift = 0;
	u8  lpf_filter2_shift = 0;


//voice data out of band need to be processed using 12 bits.
_attribute_ram_code_ void voice_iir_OOB(signed short * ps, signed short *pd, int* coef, int nsample,u8 shift)
{
	int i = 0;
	long int s = 0;
	for (i=0; i<nsample; i++)
	{
		 //s = (*ps * coef[0])>>shift;
		s = (*ps * coef[0])>>0;          		//input 16-bit
		s += coef[5] * coef[1];
		s += coef[6] * coef[2];       		//coef 0,1,2: 12-bit
		s += coef[7] * coef[3];
		s += coef[8] * coef[4];      		//coef 4 & 5: 10-bit; coef 7 & 8: 18-bit
		//s = s >> 10;                        //18-bit
		//s = s >> 12;                        //18-bit

		s = ((s + (1 << 11)) >> 12); /////this line code indicate that process sample data with 12 bits

#if 0
		if (s >= (1<<18))
			  s = (1<<18) - 1;
		else if (s < -(1<<18))
			  s = - (1<<18);
#endif
		coef[6] = coef[5];                  //16-bit
		coef[5] = *ps++;              		//16-bit
		coef[8] = coef[7];                  //18-bit
		coef[7] = s;
		//*pd++ = s >> 3;
		//*pd++ = s >> 1;

		//limit
		if(s > 32767){
			s = 32767;
		}
		else if(s < -32767){
			s = -32767;
		}

		*pd++ = s >> shift;
	}
}


_attribute_ram_code_ static inline void audio_getHalfsample_func(s16*ps, u16 len)
{
	for(u16 i = 0;i < (len >> 1);i++)
	{
		ps[i] = ps[2 * i];
	}
}

//voice data inner band need to be processed using 14 bits.
_attribute_ram_code_ void voice_iir(signed short * ps, signed short *pd, int* coef, int nsample,u8 shift)
{
	int i = 0;
	long int s = 0;
	for (i=0; i<nsample; i++)
	{
		 //s = (*ps * coef[0])>>shift;
		s = (*ps * coef[0])>>0;          		//input 16-bit
		s += coef[5] * coef[1];
		s += coef[6] * coef[2];       		//coef 0,1,2: 12-bit
		s += coef[7] * coef[3];
		s += coef[8] * coef[4];      		//coef 4 & 5: 10-bit; coef 7 & 8: 18-bit
		//s = s >> 10;                        //18-bit
		//s = s >> 12;                        //18-bit

		s = ((s + (1 << 13)) >> 14);   /////this line code indicate that process sample data with 14 bits
//		s = ((s + (1 << 11)) >> 12);
#if 0
		if (s >= (1<<18))
			  s = (1<<18) - 1;
		else if (s < -(1<<18))
			  s = - (1<<18);
#endif
		coef[6] = coef[5];                  //16-bit
		coef[5] = *ps++;              		//16-bit
		coef[8] = coef[7];                  //18-bit
		coef[7] = s;
		//*pd++ = s >> 3;
		//*pd++ = s >> 1;

		//limit
		if(s > 32767){
			s = 32767;
		}
		else if(s < -32767){
			s = -32767;
		}

		*pd++ = s >> shift;
	}
}

void Audio_VolumeSet(unsigned char input_output_select,unsigned char volume_set_value){

	reg_mic_ctrl =    MASK_VAL( FLD_AUD_MIC_VOL_CONTROL,      	(volume_set_value & 0x3f),\
			                    FLD_AUD_MIC_MONO_EN, 	        1, \
			                    FLD_AUD_AMIC_DMIC_SELECT,    	0 );
//	if(input_output_select)
//		write_reg8(0xb41,volume_set_value);//ALC_VOL_L
//	else
//		write_reg8(0x562,volume_set_value);//VOL_CTRL
}


	#define  IIR_FILTER_ADR  0x71000
	#define  CBUFFER_SIZE    20
	u8 filter_step_enable = 0;
	u8 vol_gain_tmp       = 0xff;
	u8 pga_post_gain_tmp  = 0xff;
	u8 filter_cmp[20];
/*
 FLASH 0x71000
 +----------------0x71000
  | c1(20B) first inner band EQ
 +----------------0x71013
  | c2(20B) second inner band EQ
 +----------------0x71027
  | c3(20B) three inner band EQ
 +----------------0x7103B
  | c4(20B) first out of band EQ
 +----------------0x7104F
  | c5(20B) second out of band EQ
 +----------------0x71063
  | f1_sft(1B) | f2_sft(1B) |f3_sft(1B) |f4_sft(1B) | f5_sft(1B)	five shift
 +----------------0x7109F

 +----------------0x710A0
  | vol_gain(1B)volume gain setting
 +----------------0x710A1
  | PGA_POST_GAIN(1B)(0~49): 0x710A1
 +----------------0x710A2
*/
#define MANUAL_VOLUME_SETTINGS			0x1C
void filter_setting()
{
	//get the configuration data in the flash
	u32 *pfilter     = (u32*)IIR_FILTER_ADR;
	u8  *p_start_iir = (u8 *)(pfilter);

	memset(filter_cmp, 0xff,sizeof(filter_cmp));

	if(memcmp(p_start_iir,filter_cmp,sizeof(filter_cmp)))//step 1 disaptch
	{
		memcpy((u8 *)filter_1,p_start_iir,CBUFFER_SIZE);
		filter_step_enable |= BIT(1);
		memset(filter_cmp, 0xff,sizeof(filter_cmp));
	}
	if(memcmp(p_start_iir+CBUFFER_SIZE,filter_cmp,sizeof(filter_cmp)))//step 2 disaptch
	{
		memcpy((u8 *)filter_2,p_start_iir+CBUFFER_SIZE,CBUFFER_SIZE);
		filter_step_enable |= BIT(2);
		memset(filter_cmp, 0xff,sizeof(filter_cmp));
	}
	if(memcmp(p_start_iir+(CBUFFER_SIZE*2),filter_cmp,sizeof(filter_cmp)))//step 3 dispatch
	{
		memcpy((u8 *)filter_3,p_start_iir+(CBUFFER_SIZE*2),CBUFFER_SIZE);
		filter_step_enable |= BIT(3);
		memset(filter_cmp, 0xff,sizeof(filter_cmp));
	}

	if(memcmp(p_start_iir+(CBUFFER_SIZE*3),filter_cmp,sizeof(filter_cmp)))//step 4 dispatch
	{
		memcpy((u8 *)LPF_FILTER_1,p_start_iir+(CBUFFER_SIZE*3),CBUFFER_SIZE);
		filter_step_enable |= BIT(4);
		memset(filter_cmp, 0xff,sizeof(filter_cmp));
	}
	if(memcmp(p_start_iir+(CBUFFER_SIZE*4),filter_cmp,sizeof(filter_cmp)))//step 5 dispatch
	{
		memcpy((u8 *)LPF_FILTER_2,p_start_iir+(CBUFFER_SIZE*4),CBUFFER_SIZE);
		filter_step_enable |= BIT(5);
		memset(filter_cmp, 0xff,sizeof(filter_cmp));
	}

	int i;
	i = CBUFFER_SIZE*5;

	filter1_shift = (p_start_iir[i] == 0xff)	?	0:p_start_iir[i];
    filter2_shift = (p_start_iir[i+1] == 0xff)	?	0:p_start_iir[i+1];
	filter3_shift = (p_start_iir[i+2] == 0xff)	?  	0:p_start_iir[i+2];

	lpf_filter1_shift = (p_start_iir[i+3] == 0xff)?	0:p_start_iir[i+3];
	lpf_filter2_shift = (p_start_iir[i+4] == 0xff)?	0:p_start_iir[i+4];

	//volume gain setting .position 0x710A0
	vol_gain_tmp = p_start_iir[0xA0];
	if(vol_gain_tmp!=0xff){
		if(vol_gain_tmp&0x80){
			if(MANUAL_VOLUME_SETTINGS<(vol_gain_tmp&0x7f)){
				return;
			}
			Audio_VolumeSet(1,MANUAL_VOLUME_SETTINGS-(vol_gain_tmp&0x7f));
		}else{
			if(MANUAL_VOLUME_SETTINGS+vol_gain_tmp>0x3f){
				return;
			}
			Audio_VolumeSet(1,MANUAL_VOLUME_SETTINGS+vol_gain_tmp);
		}
	}else{
		vol_gain_tmp = 0x87;
		Audio_VolumeSet(1,MANUAL_VOLUME_SETTINGS-(vol_gain_tmp&0x7f));//根据测试结果默认值减去10db
	}
	//PGA_POST_GAIN setting .position 0x710A1
	pga_post_gain_tmp = p_start_iir[0xA1];
	if(pga_post_gain_tmp != 0xff){
		if(pga_post_gain_tmp > 0x0f){//0x0f: PGA_GAIN_VOL_0_0DB
			return;
		}
//		analog_write(codec_ana_cfg2,analog_read(codec_ana_cfg2) | 0x0a);
//		analog_write(codec_ana_cfg3,analog_read(codec_ana_cfg3) & 0x00);
		analog_write(codec_ana_cfg4,(analog_read(codec_ana_cfg4) & 0x00) | pga_post_gain_tmp);
	}

	return ;
}
#endif

void adpcm_gol_encode_handler(u8 flag)
{
	s16 *ps;
	static u16 sAdpcmRawSampleRptr = 0;
	u16 micWptr = get_mic_wr_ptr(); //step is 4
	u16 l = (micWptr >= sAdpcmRawSampleRptr) ? (micWptr-sAdpcmRawSampleRptr) : 0xffff;

	if(l<AUDIO_GOL_ADPCM_PACKET_SAMPLE) return;

	if(((sAdpcmEnBufferWptr - sAdpcmEnBufferRptr) & (AUDIO_ADPCM_BUFFER_COUNT*2-1)) >= AUDIO_ADPCM_BUFFER_COUNT){
		return;
	}
	if(flag == 1){ //drop the data
		sAdpcmRawSampleRptr = sAdpcmRawSampleRptr ? 0 : AUDIO_GOL_ADPCM_PACKET_SAMPLE;
		sAdpcmEnBufferWptr++;
		sAdpcmEnBufferRptr = sAdpcmEnBufferWptr;
		return ;
	}

	ps = audio_raw_buffer + sAdpcmRawSampleRptr;
#if IIR_FILTER_ENABLE
	if((filter_step_enable & 0x0e) != 0)
	{
		if(filter_step_enable & BIT(1))
		{
			voice_iir(ps,ps,filter_1,(AUDIO_GOL_ADPCM_PACKET_SAMPLE),filter1_shift);//14 bits
		}
		if(filter_step_enable & BIT(2))
		{
			voice_iir(ps,ps,filter_2,(AUDIO_GOL_ADPCM_PACKET_SAMPLE),filter2_shift);//14 bits
		}
		if(filter_step_enable & BIT(3))
		{
			voice_iir(ps,ps,filter_3,(AUDIO_GOL_ADPCM_PACKET_SAMPLE),filter3_shift);//14 bits
		}
	}
#endif
	adpcm_encode_pcm2adpcm(ps, AUDIO_GOL_ADPCM_PACKET_SAMPLE, (s16 *)(sAdpcmEnBuffer
		+ AUDIO_ADPCM_PACKET_LENGTH*(sAdpcmEnBufferWptr & (AUDIO_ADPCM_BUFFER_COUNT - 1))));
	sAdpcmRawSampleRptr = sAdpcmRawSampleRptr ? 0 : AUDIO_GOL_ADPCM_PACKET_SAMPLE;
	sAdpcmEnBufferWptr++;
}



_attribute_ram_code_
void adpcm_encode_pcm2adpcm(s16 *pSrc, int srcSampleLen, s16 *pDst)
{
	int i, j;
	unsigned short code=0;
	unsigned short code16=0;
	static signed short *pd;

	code = 0;
	pd = pDst;

	 //google audio format
	*pd++ = ((gAdpcmEnSerialNum>>8)&0x00ff)|((gAdpcmEnSerialNum<<8)&0xff00);
	*pd++ = (ADPCM_ANDROID_ID)|((sAdpcmEnPredict&0xff00));
	*pd++ = ((sAdpcmEnPredict)&0x00ff)|((sAdpcmEnPredictIdx<<8)&0xff00);
	gAdpcmEnSerialNum ++;

	for(i=0 ; i<srcSampleLen; i++){//srcLen: unit sample
		s16 di = pSrc[i];
		int step = sAdpcmEnSteptbl[sAdpcmEnPredictIdx];
		int diff = di - sAdpcmEnPredict;

		if(diff >= 0){
			code = 0;
		}else{
			diff = -diff;
			code = 8;
		}

		int diffq = step >> 3;

		for(j=4; j>0; j=j>>1){
			if(diff >= step){
				diff = diff - step;
				diffq = diffq + step;
				code = code + j;
			}
			step = step >> 1;
		}

		code16 = (code16 >> 4) | (code << 12);
		if((i&3) == 3){
			code16 = ((code16&0x0f)<<4)|((code16&0xf0)>>4) | ((code16&0x0f00)<<4)|((code16&0xf000)>>4);
			*pd++ = code16;
		}

		if(code >= 8) {
			sAdpcmEnPredict = sAdpcmEnPredict - diffq;
		}else {
			sAdpcmEnPredict = sAdpcmEnPredict + diffq;
		}

		if(sAdpcmEnPredict > 32767){
			sAdpcmEnPredict = 32767;
		}else if (sAdpcmEnPredict < -32768) {
			sAdpcmEnPredict = -32768;
		}

		sAdpcmEnPredictIdx = sAdpcmEnPredictIdx + sAdpcmEnIdxtbl[code & 15];
		if(sAdpcmEnPredictIdx < 0) {
			sAdpcmEnPredictIdx = 0;
		}else if(sAdpcmEnPredictIdx > 88){
			sAdpcmEnPredictIdx = 88;
		}
	}
}

#endif
