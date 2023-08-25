/******************************************************************************
 * @file     app_config.h
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

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

#include  "application/audio/audio_common.h"
#include  "application/usbstd/usbkeycode.h"



#define APP_TEST_SECT_ADDR                    0x73000

#define APP_CUSTOM_ADDR                       0x78000

#define APP_COUCH_MODE_ADDR                   0x780EF
#define APP_TX_POWER_ADDR                     0x780F0

#define APP_2ND_PAIR_KEY0                     0x780F6
#define APP_2ND_PAIR_KEY1                     0x780F7

#define APP_WOBLE_ADV_TYPE_ADDR               0x780F8
#define APP_PERIODIC_WAKE_UP_TIMER            0x780F9
#define APP_EN_PERIODIC_WAKE_UP               0x780FB
#define APP_EN_SLAVE_RPA                      0x780FC
#define APP_EN_BLE_ADV                        0x780FD
#define APP_EN_GOOGLE_WAKEUPPACK              0x780FE
#define APP_EN_CACHEKEY                       0x780FF
#define APP_AUTHENTICATION_ADDR               0x78100
#define APP_ENCRYPTION_ADDR                   0x78200
#define APP_ECDSA_PUBKEY_ADDR                 0x78300

#define APP_NEC_IR_CODE_TABLE                 0x79000
#define APP_DATA_UPDATE_OVER_OTA_ADDR         0x7a000

#define APP_DATA_UPDATE_ADDR_OFFSET           APP_DATA_UPDATE_OVER_OTA_ADDR - APP_CUSTOM_ADDR


#define IR_DATA_SECT_0_EXT_ADDR               0x38000
#define IR_DATA_SECT_1_EXT_ADDR               0x39000
#define IR_DATA_SECT_2_EXT_ADDR               0x3a000
#define IR_DATA_SECT_3_EXT_ADDR               0x3b000

#define IR_DATA_SECT_0_ADDR                   0x3c000
#define IR_DATA_SECT_1_ADDR                   0x3d000
#define IR_DATA_SECT_2_ADDR                   0x3e000
#define IR_DATA_SECT_3_ADDR                   0x3f000

#define IR_SECTOR_ADD_OFFSET                  IR_DATA_SECT_0_ADDR-IR_DATA_SECT_0_EXT_ADDR

#define MAC_DATA_SECT_ADDR                    0x7b000
#define WAKEUP_KEYINDEX_DATA_SECT_ADDR        0x7c000
#define IR_KEY_EVENT_NOTIFY_SECT_ADDR         0x7d000
#define CCC_DATA_AREA                         0x7e000


#define IR_DATA_SECT_MAX_NUM                  12   //Each sector can be written 3 times ,4*3
#define IR_DATA_SECT_NUM                      4
#define IR_DATA_NEED_ERASE_NUM                6


/////////////////// FEATURE SELECT /////////////////////////////////
#define BLE_REMOTE_PM_ENABLE                  1
#define PM_DEEPSLEEP_RETENTION_ENABLE         1
#define BLE_REMOTE_SECURITY_ENABLE            1
#define BLE_REMOTE_OTA_ENABLE                 1
#define REMOTE_IR_ENABLE                      1
#define BATT_CHECK_ENABLE                     1   //must enable
#define BLE_AUDIO_ENABLE                      1
#define APP_IR_OVER_BLE                       1
#define APP_DATA_UPDATE_OVER_BLE              1
#define BLT_APP_LED_ENABLE                    1
#define BLT_TEST_SOFT_TIMER_ENABLE            1
#define FIND_ME_ENABLE                        1
#define DEVICE_INFO                           0

#define UNUSED_GPIO_PULLDOWN_ENABLE           0

//flash firmware_check
#define FIRMWARE_CHECK_ENABLE                 0

//firmware check
#define FIRMWARES_SIGNATURE_ENABLE            0

#define AUDIO_TRANS_USE_2M_PHY_ENABLE         0

#define REMOTE_G10                            1
#define REMOTE_G20                            2


#define MP_TEST_MODE                          1

/////////////////// TEST MODE & DUT ////////////////////////////////
#if (MP_TEST_MODE)
#define MODE_TEST                             0xFF
#define MODE_USER                             0x00

#define DUT_INPUT_PORT                        GPIO_PB2
#define PB2_INPUT_ENABLE                      1
#endif
/////////////////// DEEP SAVE FLG //////////////////////////////////
#define USED_DEEP_ANA_REG                     DEEP_ANA_REG0 //u8,can save 8 bit info when deep
#define LOW_BATT_FLG                          BIT(0) //if 1: low battery
#define CONN_DEEP_FLG                         BIT(1) //if 1: conn deep, 0: adv deep
#define IR_MODE_DEEP_FLG                      BIT(2) //if 1: IR mode, 0: BLE mode
#define LOW_BATT_SUSPEND_FLG                  BIT(3) //if 1 : low battery, < 1.8v
#define OTA_FLG                               BIT(4) //if 1: OTA SUCCESS
#define POWER_ON_FLG                          BIT(5)

#if (BATT_CHECK_ENABLE)
#if 0//(__PROJECT_8278_BLE_REMOTE__)
    //use VBAT(8278) , then adc measure this VBAT voltage
    #define ADC_INPUT_PCHN                    VBAT    //corresponding  ADC_InputPchTypeDef in adc.h
#else
    //telink device: you must choose one gpio with adc function to output high level(voltage will equal to vbat), then use adc to measure high level voltage
    //use PB7(8258) output high level, then adc measure this high level voltage
    #define GPIO_VBAT_DETECT                  GPIO_PB7
    #define PB7_FUNC                          AS_GPIO
    #define PB7_INPUT_ENABLE                  0
    #define ADC_INPUT_PCHN                    B7P    //corresponding  ADC_InputPchTypeDef in adc.h
#endif
#endif

//#define IR_KEY_SEQUENCE_ADDR                0x71000

//////////////////// LED CONFIG (RCU board)///////////////////////////
#if (BLT_APP_LED_ENABLE)
    #define LED_ON_LEVAL                      1             //gpio output high voltage to turn on led
    #define GPIO_LED                          GPIO_PD6
    #define PD6_FUNC                          AS_GPIO

    #define GPIO_LED2                         GPIO_PD5
    #define PD5_FUNC                          AS_GPIO
    #define PD5_OUTPUT_ENABLE                 1
    #define PD5_INPUT_ENABLE                  0
#endif

#if (BLT_TEST_SOFT_TIMER_ENABLE)
    #define BLT_SOFTWARE_TIMER_ENABLE         1
#endif

////////////////////////// AUDIO CONFIG (RCU board) /////////////////////////////
#if (BLE_AUDIO_ENABLE)
    #define BLE_DMIC_ENABLE                   0  //0: Amic   1: Dmic
    #define IIR_FILTER_ENABLE                 0

    #if BLE_DMIC_ENABLE
        #define GPIO_DMIC_BIAS                GPIO_PC4
        #define GPIO_DMIC_DI                  GPIO_PA0
        #define GPIO_DMIC_CK                  GPIO_PA1
    #else
        #define GPIO_AMIC_BIAS                GPIO_PC4// need check ,v1.0 PC4, V1.1 PC0
        #define GPIO_AMIC_C1                  GPIO_PC1
    #endif

    /* RCU Audio MODE:
     * TL_AUDIO_RCU_ADPCM_GATT_TLEINK
     * TL_AUDIO_RCU_ADPCM_GATT_GOOGLE
     * TL_AUDIO_RCU_ADPCM_HID
     * TL_AUDIO_RCU_SBC_HID                        //need config 32k retention
     * TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB
     * TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB          //need config 32k retention
     * TL_AUDIO_RCU_MSBC_HID                       //need config 32k retention
     */
    #define TL_AUDIO_MODE                     TL_AUDIO_RCU_ADPCM_GATT_GOOGLE
#endif

//PB3 IRout 100K pulldown when  IR not working,  when IR begin, disable this 100K pulldown
#define PULL_WAKEUP_SRC_PB3                   PM_PIN_PULLDOWN_100K



//////////////////////////// KEYSCAN/MIC  GPIO //////////////////////////////////
#define MATRIX_ROW_PULL                       PM_PIN_PULLUP_1M//PM_PIN_PULLDOWN_100K//PM_PIN_PULLUP_1M
#define MATRIX_COL_PULL                       PM_PIN_PULLDOWN_100K//PM_PIN_PULLUP_10K//PM_PIN_PULLDOWN_100K

#define KB_LINE_HIGH_VALID                    1   //dirve pin output 0 when keyscan, scanpin read 0 is valid
#define DEEPBACK_FAST_KEYSCAN_ENABLE          0   //proc fast scan when deepsleep back trigged by key press, in case key loss
#define LONG_PRESS_KEY_POWER_OPTIMIZE         1   //lower power when pressing key without release

//stuck key
#define STUCK_KEY_PROCESS_ENABLE              1
#define STUCK_KEY_ENTERDEEP_TIME              30  //in s

//repeat key
#define KB_REPEAT_KEY_ENABLE                  0
#define KB_REPEAT_KEY_INTERVAL_MS             200
#define KB_REPEAT_KEY_NUM                     1
#define KB_MAP_REPEAT                         {VK_1, }


#define CR_VOL_UP                             0x80
#define CR_VOL_DN                             0x81
#define CR_PERIOD                             0x37

#define CR_MEDIA_KEY_INDEX                    CR_MAGIC  // 230529 by ken. CR_NOTIFICATION -> CR_MAGIC

//device layout
/* 230529 by ken. add magic key code */
#define CR_MAGIC                              0xdc

#define CR_NOTIFICATION                       0xdd
#define CR_LIVETV                             0xde
#define CR_TELETEXT                           0xdf
#define CR_USERPROFILE                        0xe0
#define CR_ALLAPPS                            0xe1
#define CR_FASTREWIND                         0xe2
#define CR_RECORD                             0xe3
#define CR_PLAYPAUSE                          0xe4
#define CR_FASTFORWARD                        0xe5

//G20
#define CR_INFO                               0xe6
#define CR_SUBTITLE                           0xe7
#define CR_RED                                0xe8
#define CR_GREEN                              0xe9
#define CR_YELLOW                             0xea
#define CR_BLUE                               0xeb

//G10
#define CR_YOUTUBE                            0xec
#define CR_NETFLIX                            0xed
#define CR_DISNEY                             0xee
#define CR_HBOMAX                             0xef
#define CR_UP                                 0xf0
#define CR_DN                                 0xf1
#define CR_LEFT                               0xf2
#define CR_RIGHT                              0xf3
#define CR_CENTER                             0xf4
#define CR_HOME                               0xf5
#define CR_BACK                               0xf6
#define CR_POWER                              0xf7
#define CR_VOL_MUTE                           0xf8
#define CR_CHN_UP                             0xf9
#define CR_CHN_DN                             0xfa
#define CR_GUIDE                              0xfb
#define CR_BOOKMARK                           0xfc
#define CR_ASSIST                             0xfd
#define CR_INPUT                              0xfe
#define CR_DASHBOARD                          0xff

//special key
#define VOICE                                 0xc0
#define KEY_MODE_SWITCH                       0xc1
#define PHY_TEST                              0xc2
#define VOICE_PTT                             0xc3
#define VOICE_HTT                             0xc4
#define VOICE_ON_REQ                          0xc5
#define AES_TEST                              0xc6

#define IR_VK_0                               0x0A
#define IR_VK_1                               0x01
#define IR_VK_2                               0x02
#define IR_VK_3                               0x03
#define IR_VK_4                               0x04
#define IR_VK_5                               0x05
#define IR_VK_6                               0x06
#define IR_VK_7                               0x07
#define IR_VK_8                               0x08
#define IR_VK_9                               0x09

#define IR_UP                                 0x15
#define IR_DN                                 0x16
#define IR_LEFT                               0x17
#define IR_RIGHT                              0x18
#define IR_CENTER                             0x19

#define IR_POWER                              0x21
#define IR_HOME                               0x47
#define IR_BACK                               0x48
#define IR_VOL_UP                             0x23
#define IR_VOL_DN                             0x24
#define IR_CHN_UP                             0x33
#define IR_CHN_DN                             0x34
#define IR_ASSIST                             0x46
#define IR_YOUTUBE                            0x64
#define IR_NETFLIX                            0x63
#define IR_DISNEY                             0x67
#define IR_HBOMAX                             0x68
#define IR_GUIDE                              0x32
#define IR_BOOKMARK                           0x74
#define IR_INPUT                              0x60
#define IR_MUTE                               0x25
#define IR_DASHBOARD                          0x0F

#define T_VK_CH_UP                            0xd0
#define T_VK_CH_DN                            0xd1

//G20
#define IR_INFO                               0x29
#define IR_SUBTITLE                           0x58
#define IR_RED                                0x4b
#define IR_GREEN                              0x4a
#define IR_YELLOW                             0x49
#define IR_BLUE                               0x4c

//device lyaout
#define IR_PERIOD                             0X2a
#define IR_NOTIFICATION                       0x10
#define IR_LIVETV                             0x61
#define IR_TELETEXT                           0x75
#define IR_USERPROFILE                        0x59
#define IR_ALLAPPS                            0x57
#define IR_FASTREWIND                         0x51
#define IR_RECORD                             0x54
#define IR_PLAYPAUSE                          0x52
#define IR_FASTFORWARD                        0x53
#define IR_MAGIC                              0x68

#define KB_SCAN_PINS   {GPIO_PA1, GPIO_PA2, GPIO_PA3, GPIO_PA4, GPIO_PA5, GPIO_PA6,GPIO_PB5,GPIO_PB6}


//drive pin as gpio

#define PD2_FUNC                              AS_GPIO
#define PD3_FUNC                              AS_GPIO
#define PD4_FUNC                              AS_GPIO
#define PD7_FUNC                              AS_GPIO
#define PA0_FUNC                              AS_GPIO
#define PD1_FUNC                              AS_GPIO


//drive pin need 100K pulldown
#define PULL_WAKEUP_SRC_PD2                   MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PD3                   MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PD4                   MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PD7                   MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PA0                   MATRIX_ROW_PULL
#define PULL_WAKEUP_SRC_PD1                   MATRIX_ROW_PULL


//drive pin open input to read gpio wakeup level
#define PD2_INPUT_ENABLE                      1
#define PD3_INPUT_ENABLE                      1
#define PD4_INPUT_ENABLE                      1
#define PD7_INPUT_ENABLE                      1
#define PA0_INPUT_ENABLE                      1
#define PD1_INPUT_ENABLE                      1


//scan pin as gpio
#define PA1_FUNC                              AS_GPIO
#define PA2_FUNC                              AS_GPIO
#define PA3_FUNC                              AS_GPIO
#define PA4_FUNC                              AS_GPIO
#define PA5_FUNC                              AS_GPIO
#define PA6_FUNC                              AS_GPIO
#define PB5_FUNC                              AS_GPIO
#define PB6_FUNC                              AS_GPIO


//scan  pin need 10K pullup
#define PULL_WAKEUP_SRC_PA1                   MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PA2                   MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PA3                   MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PA4                   MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PA5                   MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PA6                   MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PB5                   MATRIX_COL_PULL
#define PULL_WAKEUP_SRC_PB6                   MATRIX_COL_PULL



//scan pin open input to read gpio level
#define PA1_INPUT_ENABLE                      1
#define PA2_INPUT_ENABLE                      1
#define PA3_INPUT_ENABLE                      1
#define PA4_INPUT_ENABLE                      1
#define PA5_INPUT_ENABLE                      1
#define PA6_INPUT_ENABLE                      1
#define PB5_INPUT_ENABLE                      1
#define PB6_INPUT_ENABLE                      1



//#define KB_MAP_NUM                            KB_MAP_NORMAL
//#define KB_MAP_FN                             KB_MAP_NORMAL

/////////////////// Clock  /////////////////////////////////
#if (TL_AUDIO_MODE & (TL_AUDIO_MASK_SBC_MODE | TL_AUDIO_MASK_MSBC_MODE))
#define CLOCK_SYS_CLOCK_HZ                    32000000
#else
#define CLOCK_SYS_CLOCK_HZ                    16000000
#endif
enum{
    CLOCK_SYS_CLOCK_1S = CLOCK_SYS_CLOCK_HZ,
    CLOCK_SYS_CLOCK_1MS = (CLOCK_SYS_CLOCK_1S / 1000),
    CLOCK_SYS_CLOCK_1US = (CLOCK_SYS_CLOCK_1S / 1000000),
};

#define CLOCK_32K_CLOCK_HZ                    (32000)
enum{
    CLOCK_32K_CLOCK_1MS   = (CLOCK_32K_CLOCK_HZ / 1000),
    CLOCK_32K_CLOCK_1S    = CLOCK_32K_CLOCK_HZ,
    CLOCK_32K_CLOCK_1MIN  = (CLOCK_32K_CLOCK_1S * 60),
    CLOCK_32K_CLOCK_1HOUR = (CLOCK_32K_CLOCK_1MIN * 60),
};


/////////////////// watchdog  //////////////////////////////
#define MODULE_WATCHDOG_ENABLE                0
#define WATCHDOG_INIT_TIMEOUT                 500  //ms




#if (MP_TEST_MODE)
#define BLE_PHYTEST_MODE                      PHYTEST_MODE_THROUGH_2_WIRE_UART
#else
#define BLE_PHYTEST_MODE                      PHYTEST_MODE_DISABLE
#endif


#define UART_PRINT_ENABLE                     0   //uart_tx as uart print func
#define UART_PRINT_DEBUG_ENABLE               1   //GPIO simulate uart print func

#if (UART_PRINT_DEBUG_ENABLE)

    //the baud rate should not bigger than 1M(system timer clock is constant 16M)
    #define PRINT_BAUD_RATE                   1000000
    #define DEBUG_INFO_TX_PIN                 GPIO_PB1
    #define PULL_WAKEUP_SRC_PB1               PM_PIN_PULLUP_10K
    #define PB1_OUTPUT_ENABLE                 1
    #define PB1_DATA_OUT                      1 //must
    #include "application/print/u_printf.h"
#endif



#include "app_att.h"
#include "../common/default_config.h"

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
