/********************************************************************************************************
 * @file     app_audio.h
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

#ifndef APP_AUDIO_H_
#define APP_AUDIO_H_


#if (BLE_AUDIO_ENABLE)


#define AUDIO_GOOGLE_CMD_CAP     	0x0A
#define AUDIO_GOOGLE_CMD_OPEN    	0x0C
#define AUDIO_GOOGLE_CMD_CLOSE   	0x0D
#define AUDIO_GOOGLE_CMD_EXTEND  	0x0E


#define ATV_MIC_CHAR_RSP_CAP     0x0B
#define ATV_MIC_CHAR_RSP_OPEN    0x04
#define ATV_MIC_CHAR_RSP_CLOSE   0x00
#define ATV_MIC_CHAR_RSP_SYNC    0x0A
#define ATV_MIC_CHAR_RSP_MIC_OPEN_ERROR   0x0C

#define APP_AUDIO_FLAG_NONE      0x0000
#define APP_AUDIO_FLAG_T4H       BIT(15) //bit[15]:0--google mode, 1--t4h mode
#define APP_AUDIO_FLAG_8K        BIT(14) //bit[14]:0--16k sample, 1--8k sample
#define APP_AUDIO_FLAG_READY     BIT(13) //bit[13]:0-google voice is not ready, 1-ready
#define APP_AUDIO_FLAG_WAIT      BIT(12) //bit[12]:0-audio has no event to deal, 1-audio has event to deal
#define APP_AUDIO_FLAG_BLE       BIT(11) //bit[11]:0-ble disconnect, 1-connect
#define APP_AUDIO_FLAG_PAIR      BIT(10) //bit[10]:0-has no pair info, 1-has pair info
#define APP_AUDIO_FLAG_MULPKT    BIT(1) //bit[1]:0-multi packet request disable, 1-enable
#define APP_AUDIO_FLAG_ENABLE    BIT(0) //bit[0]:0-audio status(close), 1-open
#define APP_AUDIO_FLAG_SYNC      BIT(4) //bit[4]:0-no need sync packet, 1-need
#define APP_AUDIO_FLAG_UPDATE0   BIT(5) //bit[5]:0-no need update param, 1-need update param (7.5ms)
#define APP_AUDIO_FLAG_UPDATE1   BIT(6) //bit[6]:0-no need update param, 1-need update param (10ms)


#define APP_AUDIO_KEY_FLAG_NONE           0x00
#define APP_AUDIO_KEY_FLAG_PRESS          0x80 //bit[8]:1-press, 0-release
#define APP_AUDIO_KEY_FLAG_REPORT_PRESS   0x01
#define APP_AUDIO_KEY_FLAG_REPORT_RELEASE 0x02
#define APP_AUDIO_KEY_FLAG_REPORT_START   0x04
#define APP_AUDIO_KEY_FLAG_REPORT_STOP    0x08
#define APP_AUDIO_KEY_FLAG_EVENT_MASK     0x0F



extern u8 app_audio_key_flags;
extern u32 app_audio_timer;
extern u32 app_audio_waitack_timer;


void app_audio_init(void);


void app_audio_handler(void);

void app_audio_key_start(u8 isPress);

void app_audio_set_ble_conn(u8 isConn);


void app_audio_enable(void);
void app_audio_disable(void);

int app_auido_google_callback(void* p);

// ref the google audio spec

typedef enum {
	 VERSION_10_HIGH = 0x01,
	 VERSION_10_LOW = 0x00,
	 VERSION_04_HIGH = 0x00,
	 VERSION_04_LOW = 0x04,
}VoiceVersion_TypeDef;

typedef enum {
	 CS_8K16B = 0x01,
	 CS_16K16B = 0x02,
	 CS_BOTH = 0x03,
}CodecsSupported_TypeDef;

typedef enum {
	 v4CS_BOTH_HIGH = 0x00,
	 v4CS_BOTH_LOW = 0x03,
}Version04CodecsSupported_TypeDef;

typedef enum {
	 v4BF_HIGH = 0x00,
	 v4BF_LOW = 0x86,
}Version04ByteFrame_TypeDef;

typedef enum {
	 ON_REQUEST = 0x00,
	 PTT = 0x01,
	 HTT = 0x03,
}AssistantInteractionModel_TypeDef;

typedef enum {
	 BYTE_20_HIGH = 0x00,
	 BYTE_20_LOW = 0x14,
	 BYTE_160_HIGH = 0x00,
	 BYTE_160_LOW = 0xA0,
}AudioFrameSize_TypeDef;

typedef enum {
	 EcSet = 0x01,
	 EcReserved = 0x00,
}ExtraConfiguration_TypeDef;


typedef enum {
	 AS_MICCLOSE = 0x00,
	 AS_RELEASE_BUTTON = 0x02,
	 AS_AUDIO_START_COME = 0x04,
	 AS_TIMEOUT = 0x08,
	 AS_DISABLE_CCC = 0x10,
	 AS_OTHERS = 0x80,

}AudioStop_TypeDef;

typedef enum {
	 REASON_MICOPEN = 0x00,
	 REASON_PTT = 0x01,
	 REASON_HTT = 0x03,
}AudioStartReason_TypeDef;



typedef enum {
	 MIC_OPEN_ERROR_HIGH = 0x0F,
	 MIC_ALREADY_OPEN_LOW = 0x01,
	 MIC_REMOTE_ERROR_LOW = 0x02,
	 MIC_DISABLE_CCC_LOW = 0x03,
	 MIC_ONGOING_LOW = 0x80,
	 MIC_INTERNAL_ERROR_LOW = 0xff,
}MicOpenError_TypeDef;

#endif //BLE_AUDIO_ENABLE

#endif /* APP_AUDIO_H_ */
