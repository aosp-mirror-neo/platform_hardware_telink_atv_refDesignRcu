/********************************************************************************************************
 * @file     app_test.c
 *
 * @brief    Test mode for TLSR chips
 *
 * @author   webadmin@ohsunghq.co.kr
 * @date     Aug. 20, 2020
 *
 * @attention
 *
 *  Copyright (C) 2020 Ohsung Electronics Co., Ltd.
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

#if (MP_TEST_MODE)


#include "version.h"
#include "drivers.h"

#include "application/keyboard/keyboard.h"
#include "application/audio/gl_audio.h"
#include "stack/ble/ble.h"

#include "../common/blt_common.h"
#include "../common/blt_led.h"

#include "rc_ir.h"
#include "battery_check.h"
#include "app_flash_write.h"
#include "app_test.h"


#define _ON                          1
#define _OFF                         0

#define _TYPE_IR_SEND                1
#define _TYPE_IR_RELEASE             2

#define _IR_CODE_CUSTOM              0xFF

#define _RF_CMD_VOICE_START          0x03
#define _RF_CMD_VOICE_STOP           0x04
#define _RF_CMD_RSSI_REQ             0x0B
#define _RF_CMD_RSSI_RES             0x0C
#define _RF_CMD_VER_REQ              0x11
#define _RF_CMD_VER_RES              0x12
#define _RF_CMD_DEV_TYPE_REQ         0x84
#define _RF_CMD_DEV_TYPE_RES         0x85
#define _RF_CMD_PNP_ID_REQ           0x86
#define _RF_CMD_PNP_ID_RES           0x87
#define _RF_CMD_DEV_NAME_REQ         0x88
#define _RF_CMD_DEV_NAME_RES         0x89
#define _RF_CMD_DEV_UI_LAYOUT_REQ    0x8A
#define _RF_CMD_DEV_UI_LAYOUT_RES    0x8B
#define _RF_CMD_FACTORY_REQ          0x16
#define _RF_CMD_FACTORY_RES          0x17

#define _RF_CMD_CONN_COMPLETED       0x2F

#define _FLASH_DATA_AREA             0x73000
#define _FLASH_DATA_LEN              1

#define _FLASH_DEV_TYPE_AREA         0x78000
#define _FLASH_DEV_TYPE_LEN          1

#define _FLASH_PNP_ID_AREA           (_FLASH_DEV_TYPE_AREA + _FLASH_DEV_TYPE_LEN)
#define _FLASH_PNP_ID_LEN            7

#define _FLASH_DEV_NAME_LEN_AREA     (_FLASH_PNP_ID_AREA + _FLASH_PNP_ID_LEN)
#define _FLASH_DEV_NAME_LEN_LEN      1

#define _FLASH_DEV_NAME_AREA         (_FLASH_DEV_NAME_LEN_AREA + _FLASH_DEV_NAME_LEN_LEN)

#define _FLASH_DEV_UI_LAYOUT_AREA(n) (_FLASH_DEV_NAME_AREA + n)
#define _FLASH_DEV_UI_LAYOUT_LEN     1


typedef enum
{
    _KEY_CODE_POWER         = 0x00,
    _KEY_CODE_INPUT         = 0x03,
    _KEY_CODE_BOOKMARK      = 0x06,
    _KEY_CODE_ASSISTANT     = 0xC0,
    _KEY_CODE_SETTINGS      = 0x0C,
    _KEY_CODE_DPAD_UP       = 0x0F,
    _KEY_CODE_DPAD_LEFT     = 0x12,
    _KEY_CODE_DPAD_CENTER   = 0x15,
    _KEY_CODE_DPAD_RIGHT    = 0x01,
    _KEY_CODE_DPAD_DOWN     = 0x04,
    _KEY_CODE_BACK          = 0x07,
    _KEY_CODE_HOME          = 0x0A,
    _KEY_CODE_GUIDE         = 0x0D,
    _KEY_CODE_VOLUME_UP     = 0x10,
    _KEY_CODE_PROGRAM_UP    = 0x13,
    _KEY_CODE_MUTE          = 0x16,
    _KEY_CODE_VOLUME_DOWN   = 0x02,
    _KEY_CODE_PROGRAM_DOWN  = 0x05,
    _KEY_CODE_YOUTUBE       = 0x08,
    _KEY_CODE_NETFLIX       = 0x0B,
    _KEY_CODE_APP003        = 0x0E,
    _KEY_CODE_APP004        = 0x11,
    _KEY_CODE_1             = 0xFF, /// unknown key code
    _KEY_CODE_2             = 0xFF, /// unknown key code
    _KEY_CODE_3             = 0xFF, /// unknown key code
    _KEY_CODE_4             = 0xFF, /// unknown key code
    _KEY_CODE_5             = 0xFF, /// unknown key code
    _KEY_CODE_6             = 0xFF, /// unknown key code
    _KEY_CODE_7             = 0xFF, /// unknown key code
    _KEY_CODE_8             = 0xFF, /// unknown key code
    _KEY_CODE_9             = 0xFF, /// unknown key code
    _KEY_CODE_EXTRA_1       = 0xFF, /// unknown key code
    _KEY_CODE_0             = 0xFF, /// unknown key code
    _KEY_CODE_EXTRA_2       = 0xFF, /// unknown key code
    _KEY_CODE_RED           = 0xFF, /// unknown key code
    _KEY_CODE_GREEN         = 0xFF, /// unknown key code
    _KEY_CODE_YELLOW        = 0xFF, /// unknown key code
    _KEY_CODE_BLUE          = 0xFF, /// unknown key code
    _KEY_CODE_NONE          = 0xFF
} _tKey_Code;


extern void my_att_init(void);
extern void ir_dispatch(u8 type, u8 syscode, u8 ircode);
extern int ir_sending_check(void);
extern void app_factory_reset(void);
extern void google_mic_enable(void);
extern void ui_enable_mic(int en);
extern _attribute_ram_code_ void user_set_rf_power(u8 e, u8 *p, int n);

extern u8 ir_hw_initialed;
extern u16 waveform_start_bit_1st;
extern u16 waveform_start_bit_2nd;
extern u16 waveform_stop_bit_1st;
extern u16 waveform_stop_bit_2nd;
extern u16 waveform_logic_0_1st;
extern u16 waveform_logic_0_2nd;
extern u16 waveform_logic_1_1st;
extern u16 waveform_logic_1_2nd;
extern u8 app_audio_send_index;
extern _attribute_data_retention_ u32 app_audio_transfer_timer;
extern _attribute_data_retention_ u8 bak_batpercent;

static tTest_State _stat = TEST_STAT_USERMODE;
static _tKey_Code _key = _KEY_CODE_NONE;


static void _output_ir_nec(u8 ir_code)
{
    ir_dispatch(_TYPE_IR_SEND, _IR_CODE_CUSTOM, ir_code);
}

static void _output_ir_nec_custom(u8 addr1, u8 addr2, u8 cmd1, u8 cmd2)
{
    if (ir_sending_check()) return;

    /// set waveform input in sequence
    T_dmaData_buf.data_num = 0;

    /// waveform for start bit
    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_start_bit_1st;
    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_start_bit_2nd;

    /// add data
    u32 data = cmd2<<24 | cmd1<<16 | addr2<<8 | addr1;
    for (int i = 0; i < 32; i++)
    {
        if (data & BIT(i))
        {
            ///waveform for logic_1
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_logic_1_1st;
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_logic_1_2nd;
        }
        else
        {
            ///waveform for logic_0
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_logic_0_1st;
            T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_logic_0_2nd;
        }
    }

    ///waveform for stop bit
    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_stop_bit_1st;
    T_dmaData_buf.data[T_dmaData_buf.data_num ++] = waveform_stop_bit_2nd;

    T_dmaData_buf.dma_len = T_dmaData_buf.data_num * 2;

    ir_send_ctrl.repeat_enable = 0; /// no need repeat signal

    reg_pwm_irq_sta = FLD_IRQ_PWM0_IR_DMA_FIFO_DONE; /// clear dma fifo mode done irq status
    reg_pwm_irq_mask |= FLD_IRQ_PWM0_IR_DMA_FIFO_DONE; /// enable dma fifo mode done irq mask

    ir_send_ctrl.is_sending = IR_SENDING_DATA;
    ir_send_ctrl.sending_start_time = clock_time();

    pwm_start_dma_ir_sending();
}

static void _ctrl_led(u8 green_stat, u8 red_stat)
{
    (green_stat == _ON) ? device_led_on(APP_LED_GREEN) : device_led_off(APP_LED_GREEN);
    (red_stat == _ON) ? device_led_on(APP_LED_RED) : device_led_off(APP_LED_RED);
}

static u8 _conv_key_to_ir(_tKey_Code key_code)
{
    _tKey_Code key_codes[] = {
        _KEY_CODE_POWER,       _KEY_CODE_INPUT,        _KEY_CODE_BOOKMARK,   _KEY_CODE_ASSISTANT,
        _KEY_CODE_SETTINGS,    _KEY_CODE_DPAD_UP,      _KEY_CODE_DPAD_LEFT,  _KEY_CODE_DPAD_CENTER,
        _KEY_CODE_DPAD_RIGHT,  _KEY_CODE_DPAD_DOWN,    _KEY_CODE_BACK,       _KEY_CODE_HOME,
        _KEY_CODE_GUIDE,       _KEY_CODE_VOLUME_UP,    _KEY_CODE_PROGRAM_UP, _KEY_CODE_MUTE,
        _KEY_CODE_VOLUME_DOWN, _KEY_CODE_PROGRAM_DOWN, _KEY_CODE_YOUTUBE,    _KEY_CODE_NETFLIX,
        _KEY_CODE_APP003,      _KEY_CODE_APP004,       _KEY_CODE_1,          _KEY_CODE_2,
        _KEY_CODE_3,           _KEY_CODE_4,            _KEY_CODE_5,          _KEY_CODE_6,
        _KEY_CODE_7,           _KEY_CODE_8,            _KEY_CODE_9,          _KEY_CODE_EXTRA_1,
        _KEY_CODE_0,           _KEY_CODE_EXTRA_2,      _KEY_CODE_RED,        _KEY_CODE_GREEN,
        _KEY_CODE_YELLOW,      _KEY_CODE_BLUE,
    };

    u8 ir_code = 0x00;

    for (; ir_code < sizeof(key_codes); ir_code++)
        if (key_codes[ir_code] == key_code) return ir_code;

    return 0xFF;
}

static void _ctrl_google_voice(bool is_start)
{
    if (is_start)
    {
        google_voice_codec_used = CODEC_USED_16K;
        google_voice_packet_length = VOICE_V0P4_ADPCM_PACKET_LEN;
        google_voice_pcm_sample_packet = VOICE_V0P4_ADPCM_UNIT_SIZE;

        app_audio_send_index = 0;

        google_mic_enable();

        app_audio_transfer_timer = 0;
    }
    else
    {
        ui_enable_mic(0);
        _ctrl_led(_ON, _ON);
    }
}

static void _switch_to_user_mode(void)
{
    led_cfg_t led_cfg = { APP_LED_RED, _OFF, 250, 250, 3, 0 };

    app_factory_reset();
    if (test_get_mode() == MODE_TEST) test_set_mode(MODE_USER);

    user_set_rf_power(0, NULL, 0);
    my_att_init();

    _stat = TEST_STAT_USERMODE;
    device_led_setup(led_cfg);
}

void test_entry_mode(void)
{
    if (!ir_hw_initialed)
    {
        ir_hw_initialed = 1;
        rc_ir_init();
    }

    app_factory_reset();

    _stat = TEST_STAT_ROOT;
    _ctrl_led(_ON, _ON);
}

tTest_State test_get_state(void)
{
    if (_stat > TEST_STAT_USERMODE) _stat = TEST_STAT_USERMODE;
    return _stat;
}

u8 test_get_mode(void)
{
    u8 cur_mode = MODE_TEST;

    flash_read_page(_FLASH_DATA_AREA, _FLASH_DATA_LEN, &cur_mode);

    return cur_mode;
}

void test_set_mode(u8 mode)
{
    u8 cur_mode = MODE_TEST;

    flash_read_page(_FLASH_DATA_AREA, _FLASH_DATA_LEN, &cur_mode);

    if (cur_mode == mode) return;

    if (cur_mode == MODE_USER) flash_erase_sector(_FLASH_DATA_AREA);
    flash_write_page(_FLASH_DATA_AREA, _FLASH_DATA_LEN, &mode);
}

void test_proc_key(void)
{
    if (kb_event.cnt == 1)
    {
        _tKey_Code key_code = _KEY_CODE_POWER;

        key_code = kb_event.keycode[0];

        _ctrl_led(_OFF, _OFF);

        if (_stat == TEST_STAT_ROOT)
        {
            switch (key_code)
            {
            case _KEY_CODE_DPAD_CENTER:
                {
                    u8 version_major = 0x00, version_minor = 0x00;

                    version_major = (FW_VERSION >> 8) & 0xFF;
                    version_minor = FW_VERSION & 0xFF;
                    _output_ir_nec_custom((u8)_IR_CODE_CUSTOM, (u8)(~_IR_CODE_CUSTOM),
                                          version_major, version_minor);

                    _stat = TEST_STAT_IR_KEY;
                }
                break;

            case _KEY_CODE_BOOKMARK:
                {
                    u8 mac[6] = { 0x00, };

                    memcpy(mac, (u32 *)CFG_ADR_MAC_512K_FLASH, sizeof(mac));
                    _output_ir_nec_custom(mac[5], mac[4], mac[3], 0x00);
                }
                break;

            case _KEY_CODE_ASSISTANT:
                {
                    u8 mac[6] = { 0x00, };

                    memcpy(mac, (u32 *)CFG_ADR_MAC_512K_FLASH, sizeof(mac));
                    _output_ir_nec_custom(mac[2], mac[1], mac[0], 0x00);
                }
                break;

            case _KEY_CODE_HOME:
                break;

            case _KEY_CODE_GUIDE:
                {
                    u8 battVal = bak_batpercent, battLvl = 0;

                    if (battVal >= 80) battLvl = 1;
                    else if (battVal >= 60) battLvl = 2;
                    else if (battVal >= 40) battLvl = 3;
                    else if (battVal >= 20) battLvl = 4;
                    else battLvl = 0;

                    _output_ir_nec(battLvl);
                }
                break;

            case _KEY_CODE_YOUTUBE:
            case _KEY_CODE_NETFLIX:
                if (key_code == _KEY_CODE_YOUTUBE) _ctrl_led(_OFF, _ON);
                else if (key_code == _KEY_CODE_NETFLIX) _ctrl_led(_ON, _OFF);
                _stat = TEST_STAT_LED;
                break;

            case _KEY_CODE_MUTE:
                bls_ll_setAdvParam(ADV_INTERVAL_20MS, ADV_INTERVAL_25MS,
                                   ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
                                   0, NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
                bls_ll_setAdvEnable(1);
                break;

            case _KEY_CODE_POWER:
                _output_ir_nec(IR_POWER);
                break;

            default:
                _ctrl_led(_ON, _ON);
                break;
            }
        }
        else if (_stat == TEST_STAT_IR_KEY)
        {
            u8 ir_code = 0xFF;

            ir_code = _conv_key_to_ir(key_code);
            _output_ir_nec(ir_code);

            if (key_code == _KEY_CODE_BACK) _stat = TEST_STAT_ROOT;
        }
        else if (_stat == TEST_STAT_CURRENT)
        {
            if (key_code == _KEY_CODE_BACK) _stat = TEST_STAT_ROOT;
        }
        else if (_stat == TEST_STAT_LED)
        {
            if (key_code == _KEY_CODE_YOUTUBE) _ctrl_led(_OFF, _ON);
            else if (key_code == _KEY_CODE_NETFLIX) _ctrl_led(_ON, _OFF);
            else if (key_code == _KEY_CODE_BACK) _stat = TEST_STAT_ROOT;
        }

        _key = key_code;
    }
    else
    {
        if (_stat == TEST_STAT_ROOT && _key == _KEY_CODE_HOME) _stat = TEST_STAT_CURRENT;
        else
        {
            if (ir_send_ctrl.is_sending) ir_dispatch(_TYPE_IR_RELEASE, 0, 0);

            if (_stat == TEST_STAT_ROOT && _key == _KEY_CODE_POWER) _switch_to_user_mode();
            else _ctrl_led(_ON, _ON);
        }
    }
}

int test_write_cb(void *p)
{
    rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;
    u16 len = pw->l2cap - 3;
    u8 cmd = pw->dat[0];
    u8 buf[256] = { 0x00, };
    u8 idx = 0;

    if (len != 1) return 1;

    switch (cmd)
    {
    case _RF_CMD_VOICE_START:
        blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, 0x009e);
        _ctrl_google_voice(TRUE);
        break;

    case _RF_CMD_VOICE_STOP:
        _ctrl_google_voice(FALSE);
        break;

    case _RF_CMD_RSSI_REQ:
        buf[idx++] = _RF_CMD_RSSI_RES;
        buf[idx++] = blc_ll_getLatestAvgRSSI();
        bls_att_pushNotifyData(TEST_MODE_DP_H, buf, idx);
        break;

    case _RF_CMD_VER_REQ:
        buf[idx++] = _RF_CMD_VER_RES;
        buf[idx++] = (FW_VERSION >> 8) & 0xFF;
        buf[idx++] = FW_VERSION & 0xFF;
        bls_att_pushNotifyData(TEST_MODE_DP_H, buf, idx);
        break;

    case _RF_CMD_DEV_TYPE_REQ:
        {
            u8 device_type = 0x00;

            buf[idx++] = _RF_CMD_DEV_TYPE_RES;
            flash_read_page(_FLASH_DEV_TYPE_AREA, _FLASH_DEV_TYPE_LEN, &device_type);
            buf[idx++] = device_type;
            bls_att_pushNotifyData(TEST_MODE_DP_H, buf, idx);
        }
        break;

    case _RF_CMD_PNP_ID_REQ:
        {
            u8 pnp_id[7] = { 0x00, };

            buf[idx++] = _RF_CMD_PNP_ID_RES;
            flash_read_page(_FLASH_PNP_ID_AREA, _FLASH_PNP_ID_LEN, pnp_id);
            memcpy(buf+idx, pnp_id, _FLASH_PNP_ID_LEN);
            idx += _FLASH_PNP_ID_LEN;
            bls_att_pushNotifyData(TEST_MODE_DP_H, buf, idx);
        }
        break;

    case _RF_CMD_DEV_NAME_REQ:
        {
            u8 device_name_len = 0;
            u8 device_name[32] = { 0x00, };

            buf[idx++] = _RF_CMD_DEV_NAME_RES;
            flash_read_page(_FLASH_DEV_NAME_LEN_AREA, _FLASH_DEV_NAME_LEN_LEN, &device_name_len);
            flash_read_page(_FLASH_DEV_NAME_AREA, device_name_len, device_name);
            memcpy(buf+idx, device_name, device_name_len);
            idx += device_name_len;
            bls_att_pushNotifyData(TEST_MODE_DP_H, buf, idx);
        }
        break;

    case _RF_CMD_DEV_UI_LAYOUT_REQ:
        {
            u8 device_name_len = 0;
            u8 device_ui_layout = 0x00;

            buf[idx++] = _RF_CMD_DEV_UI_LAYOUT_RES;
            flash_read_page(_FLASH_DEV_NAME_LEN_AREA, _FLASH_DEV_NAME_LEN_LEN, &device_name_len);
            flash_read_page(_FLASH_DEV_UI_LAYOUT_AREA(device_name_len), _FLASH_DEV_UI_LAYOUT_LEN,
                            &device_ui_layout);
            buf[idx++] = device_ui_layout;
            bls_att_pushNotifyData(TEST_MODE_DP_H, buf, idx);
        }
        break;

    case _RF_CMD_FACTORY_REQ:
        {
            u8 buf = _RF_CMD_FACTORY_RES;
            bls_att_pushNotifyData(TEST_MODE_DP_H, &buf, sizeof(buf));
            
            _switch_to_user_mode();
        }
        break;

    case _RF_CMD_CONN_COMPLETED:
        bls_ll_setAdvEnable(0);
        _ctrl_led(_ON, _ON);
        break;

    default: break;
    }

    return 1;
}


#endif /// MP_TEST_MODE

