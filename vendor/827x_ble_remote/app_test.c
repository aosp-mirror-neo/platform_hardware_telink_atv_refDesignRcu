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
#include "app_test.h"

_attribute_data_retention_ _tCustom_Data _custom_data = { 0x00, };

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
#define _RF_CMD_CONN_COMPLETED       0x2F
#define _RF_CMD_DEV_TYPE_REQ         0x84
#define _RF_CMD_DEV_TYPE_RES         0x85
#define _RF_CMD_PNP_ID_REQ           0x86
#define _RF_CMD_PNP_ID_RES           0x87
#define _RF_CMD_DEV_NAME_REQ         0x88
#define _RF_CMD_DEV_NAME_RES         0x89
#define _RF_CMD_DEV_UI_LAYOUT_REQ    0x8A
#define _RF_CMD_DEV_UI_LAYOUT_RES    0x8B
#define _RF_CMD_WAKEUP_KEYS_REQ      0x8C
#define _RF_CMD_WAKEUP_KEYS_RES      0x8D
#define _RF_CMD_WAKEUP_PACKET_REQ    0x8E
#define _RF_CMD_WAKEUP_PACKET_RES    0x8F
#define _RF_CMD_FACTORY_REQ          0x16
#define _RF_CMD_FACTORY_RES          0x17

#define _FLASH_MODE_AREA             0x73000
#define _FLASH_MODE_LEN              1

#define _FLASH_CUSTOM_DATA_AREA      0x78000

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
    _KEY_IDX_POWER,
    _KEY_IDX_INPUT,
    _KEY_IDX_BOOKMARK,
    _KEY_IDX_ASSISTANT,
    _KEY_IDX_SETTINGS,
    _KEY_IDX_DPAD_UP,
    _KEY_IDX_DPAD_LEFT,
    _KEY_IDX_DPAD_CENTER,
    _KEY_IDX_DPAD_RIGHT,
    _KEY_IDX_DPAD_DOWN,
    _KEY_IDX_BACK,
    _KEY_IDX_HOME,
    _KEY_IDX_GUIDE,
    _KEY_IDX_VOLUME_PLUS,
    _KEY_IDX_PROGRAM_UP,
    _KEY_IDX_MUTE,
    _KEY_IDX_VOLUME_MINUS,
    _KEY_IDX_PROGRAM_DOWN,
    _KEY_IDX_YOUTUBE,
    _KEY_IDX_NETFLIX,
    _KEY_IDX_APP03,
    _KEY_IDX_APP04,
    _KEY_IDX_NUM_1,
    _KEY_IDX_NUM_2,
    _KEY_IDX_NUM_3,
    _KEY_IDX_NUM_4,
    _KEY_IDX_NUM_5,
    _KEY_IDX_NUM_6,
    _KEY_IDX_NUM_7,
    _KEY_IDX_NUM_8,
    _KEY_IDX_NUM_9,
    _KEY_IDX_SUBTITLES,
    _KEY_IDX_NUM_0,
    _KEY_IDX_INFORMATION,
    _KEY_IDX_RED,
    _KEY_IDX_GREEN,
    _KEY_IDX_YELLOW,
    _KEY_IDX_BLUE,
    _KEY_IDX_NONE = 0xFF
} _tKey_Index;

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
static _tKey_Index _key = _KEY_IDX_NONE;

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

static _tKey_Index _conv_key_code_to_idx(u8 key_code)
{
    u8 key_codes_G10[] = {
        0x00, 0x03, 0x06, 0xC0, 0x0C, 0x0F, 0x12, 0x15, 0x01, 0x04, 0x07,
        0x0A, 0x0D, 0x10, 0x13, 0x16, 0x02, 0x05, 0x08, 0x0B, 0x0E, 0x11
    };

    u8 key_codes_G20[] = {
        0x00, 0x06, 0x0C, 0xC0, 0x18, 0x1E, 0x24, 0x2A, 0x01, 0x07, 0x0D,
        0x13, 0x19, 0x1F, 0x25, 0x2B, 0x02, 0x08, 0x0E, 0x14, 0x1A, 0x20,
        0x16, 0x10, 0x0A, 0x04, 0x17, 0x11, 0x21, 0x1B, 0x15, 0x0F, 0x09,
        0x03, 0x0B, 0x05, 0x22, 0x1C
    };

    u8 *key_codes = (_custom_data.device_type == REMOTE_G20) ? key_codes_G20 : key_codes_G10;
    u8 key_codes_cnt = (_custom_data.device_type == REMOTE_G20) ? sizeof(key_codes_G20) : sizeof(key_codes_G10);
    u8 key_idx = _KEY_IDX_POWER;

    for (; key_idx < key_codes_cnt; key_idx++)
        if (key_codes[key_idx] == key_code) return (_tKey_Index)key_idx;

    return _KEY_IDX_NONE;
}
#if BLE_AUDIO_ENABLE
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
#endif

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

static void _parse_custom_data(void)
{
    u8 buf[sizeof(_tCustom_Data)] = { 0x00, };
    u8 idx = 0;

    flash_read_page(_FLASH_CUSTOM_DATA_AREA, sizeof(_tCustom_Data), buf);
    if(buf[0] == 0xff)
    {
        _custom_data.device_type = REMOTE_G10;
    }
    else
        _custom_data.device_type = buf[idx++];

    memcpy(_custom_data.pnp_id, &buf[idx], sizeof(_custom_data.pnp_id));
    idx += sizeof(_custom_data.pnp_id);

    _custom_data.device_name_length =
        (buf[idx] > sizeof(_custom_data.device_name)) ? sizeof(_custom_data.device_name) : buf[idx];
    memcpy(_custom_data.device_name, &buf[idx + 1], _custom_data.device_name_length);
    idx += sizeof(_custom_data.device_name_length) + buf[idx];

    _custom_data.device_ui_layout = buf[idx++];

    memcpy(_custom_data.wakeup_key_1, &buf[idx], sizeof(_custom_data.wakeup_key_1));
    idx += sizeof(_custom_data.wakeup_key_1);

    memcpy(_custom_data.wakeup_key_2, &buf[idx], sizeof(_custom_data.wakeup_key_2));
    idx += sizeof(_custom_data.wakeup_key_2);

    if (_custom_data.wakeup_key_1[0] == 0xFF && _custom_data.wakeup_key_1[1] == 0xFF
        && _custom_data.wakeup_key_2[0] == 0xFF && _custom_data.wakeup_key_2[1] == 0xFF)
    {
        memset(_custom_data.wakeup_packet, 0xFF, sizeof(_custom_data.wakeup_packet));
        return;
    }

    _custom_data.wakeup_packet[0] = (buf[idx] > 31) ? 31 : buf[idx];
    if (_custom_data.wakeup_packet[0] == 0) /// use google format only
    {
        memset(&_custom_data.wakeup_packet[1], 0xFF, sizeof(_custom_data.wakeup_packet) - 1);
        return;
    }
    idx++;
    memcpy(&_custom_data.wakeup_packet[1], &buf[idx], 3 +  _custom_data.wakeup_packet[0]);
}

void test_entry_mode(void)
{
    if (!ir_hw_initialed)
    {
        ir_hw_initialed = 1;
        rc_ir_init();
    }

    app_factory_reset();


    _parse_custom_data();

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

    flash_read_page(_FLASH_MODE_AREA, _FLASH_MODE_LEN, &cur_mode);

    return cur_mode;
}

void test_set_mode(u8 mode)
{
    u8 cur_mode = MODE_TEST;

    flash_read_page(_FLASH_MODE_AREA, _FLASH_MODE_LEN, &cur_mode);

    if (cur_mode == mode) return;

    if (cur_mode == MODE_USER) flash_erase_sector(_FLASH_MODE_AREA);
    flash_write_page(_FLASH_MODE_AREA, _FLASH_MODE_LEN, &mode);
}

void test_proc_key(void)
{
    if (kb_event.cnt == 1)
    {
        _tKey_Index key_idx = _conv_key_code_to_idx(kb_event.keycode[0]);

        _ctrl_led(_OFF, _OFF);

        if (_stat == TEST_STAT_ROOT)
        {
            switch (key_idx)
            {
            case _KEY_IDX_POWER:
                _output_ir_nec(IR_POWER);
                break;

            case _KEY_IDX_BOOKMARK:
                {
                    u8 mac[6] = { 0x00, };

                    memcpy(mac, (u32 *)CFG_ADR_MAC_512K_FLASH, sizeof(mac));
                    _output_ir_nec_custom(mac[5], mac[4], mac[3], 0x00);
                }
                break;

            case _KEY_IDX_ASSISTANT:
                {
                    u8 mac[6] = { 0x00, };

                    memcpy(mac, (u32 *)CFG_ADR_MAC_512K_FLASH, sizeof(mac));
                    _output_ir_nec_custom(mac[2], mac[1], mac[0], 0x00);
                }
                break;

            case _KEY_IDX_DPAD_CENTER:
                {
                    u8 version_major = 0x00, version_minor = 0x00;

                    version_major = (FW_VERSION >> 8) & 0xFF;
                    version_minor = FW_VERSION & 0xFF;
                    _output_ir_nec_custom((u8)_IR_CODE_CUSTOM, (u8)(~_IR_CODE_CUSTOM),
                                          version_major, version_minor);

                    _stat = TEST_STAT_IR_KEY;
                }
                break;

            case _KEY_IDX_GUIDE:
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

            case _KEY_IDX_MUTE:
                bls_ll_setAdvParam(ADV_INTERVAL_20MS, ADV_INTERVAL_25MS,
                                   ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
                                   0, NULL, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
                bls_ll_setAdvEnable(1);
                break;

            case _KEY_IDX_YOUTUBE:
            case _KEY_IDX_NETFLIX:
                if (key_idx == _KEY_IDX_YOUTUBE) _ctrl_led(_OFF, _ON);
                else if (key_idx == _KEY_IDX_NETFLIX) _ctrl_led(_ON, _OFF);
                _stat = TEST_STAT_LED;
                break;

            case _KEY_IDX_PROGRAM_UP:
                {
                    u8 vender_id[2] = { 0x00, };
                    u8 product_id[2] = { 0x00, };
                    flash_read_page(_FLASH_CUSTOM_DATA_AREA + 2, 2, vender_id);
                    flash_read_page(_FLASH_CUSTOM_DATA_AREA + 4, 2, product_id);
                    _output_ir_nec_custom(vender_id[1], vender_id[0], product_id[1], product_id[0]);
                }
                break;
            default:
                _ctrl_led(_ON, _ON);
                break;
            }
        }
        else if (_stat == TEST_STAT_IR_KEY)
        {
            _output_ir_nec((u8)key_idx);

            if (key_idx == _KEY_IDX_BACK) _stat = TEST_STAT_ROOT;
        }
        else if (_stat == TEST_STAT_CURRENT)
        {
            if (key_idx == _KEY_IDX_BACK) _stat = TEST_STAT_ROOT;
        }
        else if (_stat == TEST_STAT_LED)
        {
            if (key_idx == _KEY_IDX_YOUTUBE) _ctrl_led(_OFF, _ON);
            else if (key_idx == _KEY_IDX_NETFLIX) _ctrl_led(_ON, _OFF);
            else if (key_idx == _KEY_IDX_BACK) _stat = TEST_STAT_ROOT;
        }

        _key = key_idx;
    }
    else
    {
        if (_stat == TEST_STAT_ROOT && _key == _KEY_IDX_HOME) _stat = TEST_STAT_CURRENT;
        else
        {
            if (ir_send_ctrl.is_sending) ir_dispatch(_TYPE_IR_RELEASE, 0, 0);

            if (_stat == TEST_STAT_ROOT && _key == _KEY_IDX_POWER) _switch_to_user_mode();
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
#if BLE_AUDIO_ENABLE
    case _RF_CMD_VOICE_START:
        blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, 0x009e);
        _ctrl_google_voice(TRUE);
        break;

    case _RF_CMD_VOICE_STOP:
        _ctrl_google_voice(FALSE);
        break;
#endif
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
        buf[idx++] = _RF_CMD_DEV_TYPE_RES;
        buf[idx++] = _custom_data.device_type;
        bls_att_pushNotifyData(TEST_MODE_DP_H, buf, idx);
        break;

    case _RF_CMD_PNP_ID_REQ:
        buf[idx++] = _RF_CMD_PNP_ID_RES;
        memcpy(buf+idx, _custom_data.pnp_id, sizeof(_custom_data.pnp_id));
        idx += sizeof(_custom_data.pnp_id);
        bls_att_pushNotifyData(TEST_MODE_DP_H, buf, idx);
        break;

    case _RF_CMD_DEV_NAME_REQ:
        buf[idx++] = _RF_CMD_DEV_NAME_RES;
        memcpy(buf+idx, _custom_data.device_name, _custom_data.device_name_length);
        idx += _custom_data.device_name_length;
        bls_att_pushNotifyData(TEST_MODE_DP_H, buf, idx);
        break;

    case _RF_CMD_DEV_UI_LAYOUT_REQ:
        buf[idx++] = _RF_CMD_DEV_UI_LAYOUT_RES;
        buf[idx++] = _custom_data.device_ui_layout;
        bls_att_pushNotifyData(TEST_MODE_DP_H, buf, idx);
        break;

    case _RF_CMD_WAKEUP_KEYS_REQ:
        buf[idx++] = _RF_CMD_WAKEUP_KEYS_RES;
        memcpy(buf+idx, _custom_data.wakeup_key_1, sizeof(_custom_data.wakeup_key_1));
        idx += sizeof(_custom_data.wakeup_key_1);
        memcpy(buf+idx, _custom_data.wakeup_key_2, sizeof(_custom_data.wakeup_key_2));
        idx += sizeof(_custom_data.wakeup_key_2);
        bls_att_pushNotifyData(TEST_MODE_DP_H, buf, idx);
        break;

    case _RF_CMD_WAKEUP_PACKET_REQ:
        buf[idx++] = _RF_CMD_WAKEUP_PACKET_RES;
        buf[idx++] = _custom_data.wakeup_packet[0];
        if (_custom_data.wakeup_packet[0] != 0 && _custom_data.wakeup_packet[0] != 0xFF)
        {
            memcpy(buf+idx, &_custom_data.wakeup_packet[1], 3 + _custom_data.wakeup_packet[0]);
            idx += 3 + _custom_data.wakeup_packet[0];
        }
        bls_att_pushNotifyData(TEST_MODE_DP_H, buf, idx);
        break;

    case _RF_CMD_FACTORY_REQ:
        buf[0] = _RF_CMD_FACTORY_RES;
        bls_att_pushNotifyData(TEST_MODE_DP_H, buf, 1);
        _switch_to_user_mode();
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

