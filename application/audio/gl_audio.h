/******************************************************************************
 * @file     gl_audio.h
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

#ifndef GL_AUDIO_H_
#define GL_AUDIO_H_

#include "audio_config.h"

#if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)

#define ENABLE_GOOGLE_VOICE_1P0             1

#define FLAG_NOTIFY_AUDIO_DATA              BIT(15) //notify audio data(0: disable , 1: enable)
#define FLAG_AUDIO_CLOSE                    BIT(14) //
#define FLAG_DELAY_NOTIFY                   BIT(13) //

//google voice v0.4e
#define FLAG_SEARCH_KEY                     BIT(0) //SEARCH_KEY(0x0c,0x221)
#define FLAG_GOOGLE_SEARCH                  BIT(1) //Search(CHAR_CTL:0x08)
#define FLAG_GOOGLE_OPEN_ERROR              BIT(2) //mic_open_error.(CHAR_CTL:0x0c,error_code(2))
#define FLAG_GOOGLE_AUDIO_START             BIT(3) //auido_start.(CHAR_CTL:0x04)
#define FLAG_GOOGLE_DPAD_SELECT             BIT(4) //DPAD Select key(OK button).(CHAR_CTL:0x07)

#define FLAG_GOOGLE_AUDIO_SYNC              BIT(5) //auido_sync.(CHAR_CTL:0x0A,frame number(2))

#define FLAG_GOOGLE_CAPS_RESP               BIT(6)

//google voice v1.0
#define FLAG_GOOGLE_START_SEARCH            FLAG_GOOGLE_SEARCH    //cmd same as v0.4e
#define FLAG_GOOGLE_AUDIO_FIRST_SYNC        BIT(11)             //used for send sync cmd before start to send audio data


/**************************************************************
 ******************   google voice v0.4   *********************
 **************************************************************/

#define APP_AUDIO_GOOGLE_TIMEOUT1           60000000  //60s  //1000000 // 1s
#define APP_AUDIO_GOOGLE_TIMEOUT2           15000000 // 15s

#define PLAYBACK_MODE_THRESHOLD1            4    //16k audio change to 8k audio
#define PLAYBACK_MODE_THRESHOLD2            2    //8k audio change to 16k audio
#define PLAYBACK_MODE_HOLD_TIME             600    //60*10 =600ms

#define CAPTURE_MODE_THRESHOLD1             6
#define CAPTURE_MODE_THRESHOLD2             2
#define CAPTURE_MODE_HOLD_TIME              24    //24*10 =240ms

#define AUDIO_GOOGLE_CMD_CAP                0x0A
#define AUDIO_GOOGLE_CMD_OPEN               0x0C
#define AUDIO_GOOGLE_CMD_CLOSE              0x0D
#define AUDIO_GOOGLE_CMD_EXTEND             0x0E


#define ATV_MIC_CHAR_CTL_CAPS_RESP          0x0B
#define ATV_MIC_CHAR_CTL_AUDIO_START        0x04
#define ATV_MIC_CHAR_CTL_AUDIO_STOP         0x00
#define ATV_MIC_CHAR_CTL_SYNC               0x0A
#define ATV_MIC_CHAR_CTL_MIC_OPEN_ERROR     0x0C
#define ATV_MIC_CHAR_CTL_SEARCH             0x08
#define ATV_MIC_CHAR_CTL_DPAD_SELECT        0x07

#define GOOGLE_VOICE_VERSION_1P0            0x0100
#define GOOGLE_VOICE_VERSION_0P4            0x0004

#define CODEC_USED_8K                       0x0001
#define CODEC_USED_16K                      0x0002
#define CODEC_USED_OPUS                     0x0004      //not support

#define CODEC_SUPPORTED_8K                  0x01        //ADPCM,8khz/16bit
#define CODEC_SUPPORTED_16K                 0x02        //ADPCM,16khz/16bit(default)
#define CODEC_SUPPORTED_8K16K               0x03        //ADPCM (8khz/16bit & 16khz/16bit)
#define CODEC_SUPPORTED_8KOPUS              0x05        //Opus and ADPCM 8khz/16bit
#define CODEC_SUPPORTED_8K16KOPUS           0x07        //Opus and ADPCM (8khz/16bit & 16khz/16bit)

#define ASSISTANT_SUPPORTED_ON_REQUEST      0x00        //only On-request model is supported;
#define ASSISTANT_SUPPORTED_PTT             0x01        //Press-to-Talk and On-request models are supported;
#define ASSISTANT_SUPPORTED_HTT             0x03        //Hold-to-Talk, Press-to-Talk and On-request models are supported.

/**************************************************************
 ******************   google voice v1.0   *********************
 **************************************************************/
#define FIRST_SYNC                          BIT(0)
#define SAMPLING_CHANGE                     BIT(1)
#define PACKET_LOSS                         BIT(2)
#define SYNC_PACKET                         BIT(3)
typedef enum {
    REASON_MICCLOSE                         = 0x00,
    REASON_RELEASE_HTT                      = 0x02,
    REASON_UPCOMING_AUDIO_START             = 0x04,
    REASON_TIMEOUT                          = 0x08,
    REASON_DISABLE_CCC                      = 0x10,
    REASON_OTHERS                           = 0x80,
}AudioStop_TypeDef;

typedef enum {
    REASON_MICOPEN                          = 0x00,
    REASON_PTT                              = 0x01,
    REASON_HTT                              = 0x03,
}AudioStartReason_TypeDef;

typedef enum {
    PLAYBACK_MODE                           = 0x00,
    CAPTURE_MODE                            = 0x01,
}MicOpenMode_TypeDef;


typedef enum {
     //google voice v0.4e
    ERROR_INVALIED_CODEC                    = 0x0F01,
    //google voice v1.0
    ERROR_MIC_ALREADY_OPEN                  = 0x0F01,
    ERROR_RCU_NOT_ACTIVE                    = 0x0F02,
    ERROR_CCC_NOT_ENABLED                   = 0x0F03,
    ERROR_ONGOING_PTT_HTT                   = 0x0F04,
    ERROR_INTERNAL_ERROR                    = 0x0FFF,
}MicOpenError_TypeDef;

//parameter
extern u16 google_voice_ctl;

extern u8  google_voice_ver;
extern u16 google_voice_codec_used;
extern u16 google_voice_packet_length;
extern u16 google_voice_pcm_sample_packet;

extern u8  audio_start_reason;
extern u8  audio_stop_reason;
extern u8  stream_id;

extern u8  mic_open_mode;

extern u16 atv_char_ctl_ccc;
extern u16 atv_char_rx_ccc;

extern u8  enable_rsp_gaps_rsp;

extern u8 notify_get_rsp_en;
extern u32 notify_get_rsp_tick;
extern u32 g_delay_send_audio_stop;

void google_voice_on_request();
void google_voice_dpad_select();
u8 app_audio_key_start(u8 start_reason);
void google_voice_start();
void active_mic_open();

int app_auido_google_callback(void* p);

u8 read_audio_frame_size();
void set_audio_frame_size(u8 frame_size);

void app_audio_task(void);
extern void google_get_rsp_delay(void);

#endif

#endif /* GL_AUDIO_H_ */
