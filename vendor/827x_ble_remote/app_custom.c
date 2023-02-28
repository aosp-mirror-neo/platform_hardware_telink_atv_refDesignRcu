/******************************************************************************
 * @file     app_custom.c
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
#include "tl_common.h"
#include "drivers.h"
#include "app_config.h"
#include "app_custom.h"
#include "stack/ble/ble.h"
#include "app_test.h"


const u8 *pREMOTE_G10 = (u8 *)("RemoteG10");
const u8 *pREMOTE_G20 = (u8 *)("RemoteG20");

extern u8 *p_kb_map_ble;
extern u8 *p_kb_map_ir;
extern u8 *p_kb_map_normal;
extern  _attribute_data_retention_ u8 my_FWRevtrs [];
extern void kb_p_map_init(void);
extern u8 device_in_connection_state;


typedef enum
{
    APP_CUSTOM_DATA_UPDATE_PREPARE=1,
    APP_CUSTOM_DATA_UPDATE_PREPARE_END,
    APP_CUSTOM_DATA_UPDATE_ERASE,
    APP_CUSTOM_DATA_UPDATE_START,
    APP_CUSTOM_DATA_UPDATE_WRITEFAIL,
    APP_CUSTOM_DATA_UPDATE_ERASE_SUCC,
    APP_CUSTOM_DATA_UPDATE_END
}app_custom_data_update_sts;


typedef struct{
    u8 vid_src;
    u16 vid;
    u16 pid;
    u16 prod_ver;
}app_pnpid_format_t;

#define APP_CUSTOM_CMD_DATA_UPDATE_PREPARE      0xED00
#define APP_CUSTOM_CMD_DATA_UPDATE_START        0xED01
#define APP_CUSTOM_CMD_DATA_UPDATE_END          0xED02

#define LAYOUT_MASK_BIT0               0X01
#define LAYOUT_MASK_BIT1               0X02
#define LAYOUT_MASK_BIT2               0X04
#define LAYOUT_MASK_BIT5               0X20
#define LAYOUT_MASK_BIT6               0X40

_attribute_data_retention_  u8 en_powerkey_cache=1;
_attribute_data_retention_  u8 en_google_wakeuppack=1;
_attribute_data_retention_  u8 en_ble_adv=1;
_attribute_data_retention_  u8 en_slave_rpa=1;
_attribute_data_retention_  u8 wakeup_key1[APP_CUSTOM_WAKEUPKEY_NUM];
_attribute_data_retention_  u8 wakeup_key2[APP_CUSTOM_WAKEUPKEY_NUM];
_attribute_data_retention_  u8 comb_key_keyid[9]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};  //back,home,center,mute,setting,guide,subtitle,down
_attribute_data_retention_  u8 nec_ir_addr1_table[48]={0};
_attribute_data_retention_  u8 nec_ir_addr2_table[48]={0};
_attribute_data_retention_  u8 nec_ir_cmd1_table[48]={0};
_attribute_data_retention_  u8 nec_ir_cmd2_table[48]={0};
_attribute_data_retention_  u8 key_map_ble[48]={0};

extern _tCustom_Data _custom_data;


_attribute_data_retention_ u8 app_custom_data_update_over_ota_flash_addr_pos=0;
_attribute_data_retention_ u8 app_custom_data_update_over_ota_flash_page_pos=0;


/***************************************************************************************************************

 *** If Kb drive pin number and scan pin number changed, it want to changed below in keyboard.c :            ***

 *** 1) macro
       #define KB_DRIVE_PIN_MAX_NUM   KB_DRIVE_PIN_G20
       #define KB_DRIVE_PIN_G10       3
       #define KB_DRIVE_PIN_G20       6
       #define KB_SCAN_PIN            8

 *** 2) The arrays a[][],b[][] in the kb_remap_key_row()
     u8 a[8][3]={{0},{0},{0},{0},{0},{0},{0},{0}};
     u8 b[8][6]={{0},{0},{0},{0},{0},{0},{0},{0}};

 *** 3) The arrays a[][],b[][] in the kb_scan_row()
     u8 a[8][3]={{0},{0},{0},{0},{0},{0},{0},{0}};
     u8 b[8][6]={{0},{0},{0},{0},{0},{0},{0},{0}};

 ****************************************************************************************************************/

 /***************************************************************************************************************

 *** If Kb drive pins and scan pins changed, and want to modify keymap    :           ***
 *** 1) Change the pin setting(drive pin and scan pin) contained in app_config.h      ***
 *** 2) The following three points also need to be changed synchronously              ***

 ****************************************************************************************************************/

/* 1. Pin definition */
const unsigned int Kb_Drive_Pins_G10[3] = {GPIO_PD2, GPIO_PD3, GPIO_PD4};
const unsigned int Kb_Drive_Pins_G20[6] = {GPIO_PD2, GPIO_PD3, GPIO_PD4, GPIO_PD7, GPIO_PA0, GPIO_PD1};

/* 2. The following indexes are all extracted from Kb_Map_Normal_G10/Kb_Map_Normal_G20 */
const unsigned char Kb_CombKey_KeyId_G10[9] = {7,  0xa, 0x15,0x16,0x0c,0x0d,0xff,4,6};                 //back,home,center,mute,setting,guide,subtitle,down,bookmark keyid in g10
const unsigned char Kb_CombKey_KeyId_G20[9] = {0xd,0x13,0x2a,0x2b,0x18,0x19,0x0f,7,12};                //back,home,center,mute,setting,guide,subtitle,down,bookmark keyid in g20

const unsigned char Kb_Wakeup_Key_Map_Correspend_Index_G10[] =  {VOICE,10,0,3,8,11,14,17};             //voice,home,power,input,youtobe,netflix,disney,hbomax
const unsigned char Kb_Wakeup_Key_Map_Correspend_Index_G20[] =  {VOICE,19,0,6,14,20,26,32,28,5,11,34}; //voice,home,power,input,youtobe,netflix,disney,hbomax,blue,green,red,yellow
const unsigned char Kb_Map_devicelayout_Correspend_Index_G10[] =  {12,13,6};                           //notification,livetv,,userprofile/allapps
const unsigned char Kb_Map_devicelayout_Correspend_Index_G20[] =  {24,25,15,12,11,5,34,28,3};          //notification,livetv,teletext,userprofile/allapps,fast rewind,record,play/pause,fast forward,info/period
const unsigned char navi_key_index_g10[4] = {1,4,15,18}; //left,right,up,down keyid in g10
const unsigned char navi_key_index_g20[4] = {1,7,30,36}; //left,right,up,down keyid in g20

const unsigned char Kb_Map_Testmode_Seq_Correspend_Index_G10[] =  {16,2,22,19,5};                      //vol up,vol dn,mute,chn up,chn dn
const unsigned char Kb_Map_Testmode_Seq_Correspend_Index_G20[] =  {31,2,43,37,8};                      //vol up,vol dn,mute,chn up,chn dn


/* 3. Change key index and ir/ble value*/
const unsigned char Kb_Map_Normal_G10[8][3] = {\
                        {0,          1,             2}, \
                        {3,          4,             5}, \
                        {6,          7,             8}, \
                        {VOICE,      10,            11},\
                        {12,         13,            14},\
                        {15,         16,            17},\
                        {18,         19,            20},\
                        {21,         22,            23}};

const unsigned char Kb_Map_Ble_G10[] = {\
                        CR_POWER,      CR_RIGHT,       CR_VOL_DN,    \
                        CR_INPUT,      CR_DN,          CR_CHN_DN,    \
                        CR_BOOKMARK,   CR_BACK,        CR_YOUTUBE,   \
                        CR_ASSIST,     CR_HOME,        CR_NETFLIX,   \
                        CR_DASHBOARD,  CR_GUIDE,       CR_DISNEY,    \
                        CR_UP,         CR_VOL_UP,      CR_HBOMAX,    \
                        CR_LEFT,       CR_CHN_UP,      VK_NONE,      \
                        CR_CENTER,     CR_VOL_MUTE,    VK_NONE   };

const unsigned char Kb_Map_Ir_G10[] =  {\
                        IR_POWER,      IR_RIGHT,       IR_VOL_DN,    \
                        IR_INPUT,      IR_DN,          IR_CHN_DN,    \
                        IR_BOOKMARK,   IR_BACK,        IR_YOUTUBE,   \
                        IR_ASSIST,     IR_HOME,        IR_NETFLIX,   \
                        IR_DASHBOARD,  IR_GUIDE,       IR_DISNEY,    \
                        IR_UP,         IR_VOL_UP,      IR_HBOMAX,    \
                        IR_LEFT,       IR_CHN_UP,      VK_NONE,      \
                        IR_CENTER,     IR_MUTE,        VK_NONE    };

const unsigned char Kb_Map_Normal_G20[8][6] = \
                        {\
                        {0,            1,               2,             3,          4,         5},  \
                        {6,            7,               8,             9,          10,        11},  \
                        {12,           13,              14,            15,         16,        17},  \
                        {VOICE,        19,              20,            21,         22,        23},  \
                        {24,           25,              26,            27,         28,        29},  \
                        {30,           31,              32,            33,         34,        35},  \
                        {36,           37,              38,            39,         40,        41},  \
                        {42,           43,              44,            45,         46,        47}};

const unsigned char Kb_Map_Ble_G20[] = {\
                        CR_POWER,      CR_RIGHT,        CR_VOL_DN,     CR_INFO,     VK_4,      CR_GREEN,\
                        CR_INPUT,      CR_DN,           CR_CHN_DN,     VK_0,        VK_3,      CR_RED,\
                        CR_BOOKMARK,   CR_BACK,         CR_YOUTUBE,    CR_SUBTITLE, VK_2,      VK_6,\
                        CR_ASSIST,     CR_HOME,         CR_NETFLIX,    VK_9,        VK_1,      VK_5,\
                        CR_DASHBOARD,  CR_GUIDE,        CR_DISNEY,     VK_8,        CR_BLUE,   VK_NONE,\
                        CR_UP,         CR_VOL_UP,       CR_HBOMAX,     VK_7,        CR_YELLOW, VK_NONE,\
                        CR_LEFT,       CR_CHN_UP,       VK_NONE,       VK_NONE,     VK_NONE,   VK_NONE,\
                        CR_CENTER,     CR_VOL_MUTE,     VK_NONE,       VK_NONE,     VK_NONE,   VK_NONE};

const unsigned char Kb_Map_Ir_G20[] =  {\
                        IR_POWER,      IR_RIGHT,        IR_VOL_DN,     IR_INFO,     IR_VK_4,   IR_GREEN,\
                        IR_INPUT,      IR_DN,           IR_CHN_DN,     IR_VK_0,     IR_VK_3,   IR_RED, \
                        IR_BOOKMARK,   IR_BACK,         IR_YOUTUBE,    IR_SUBTITLE, IR_VK_2,   IR_VK_6,\
                        IR_ASSIST,     IR_HOME,         IR_NETFLIX,    IR_VK_9,     IR_VK_1,   IR_VK_5,\
                        IR_DASHBOARD,  IR_GUIDE,        IR_DISNEY,     IR_VK_8,     IR_BLUE,   VK_NONE,\
                        IR_UP,         IR_VOL_UP,       IR_HBOMAX,     IR_VK_7,     IR_YELLOW, VK_NONE,\
                        IR_LEFT,       IR_CHN_UP,       VK_NONE,       VK_NONE,     VK_NONE,   VK_NONE,\
                        IR_CENTER,     IR_MUTE,         VK_NONE,       VK_NONE,     VK_NONE,   VK_NONE};

/**
 * @brief       Device layout configuration
 * @param[in]   ui_layout: from flash 0x78000
 * @return      none
 * See document "Reference RCU Flash Storage Format" (Table 3.1) for details.
 */
void app_custom_ui_layout(u8 ui_layout)
{
    if((ui_layout == 0xff) || (ui_layout == 0))
    {
        return;
    }
    if(_custom_data.device_type == REMOTE_G10)
    {
        if((ui_layout&LAYOUT_MASK_BIT0))
        {
            key_map_ble[Kb_Map_devicelayout_Correspend_Index_G10[0]] = CR_NOTIFICATION;
            nec_ir_cmd1_table[Kb_Map_devicelayout_Correspend_Index_G10[0]] = IR_NOTIFICATION;
        }
        if((ui_layout&LAYOUT_MASK_BIT1))
        {

            key_map_ble[Kb_Map_devicelayout_Correspend_Index_G10[1]] = CR_LIVETV;
            nec_ir_cmd1_table[Kb_Map_devicelayout_Correspend_Index_G10[1]] = IR_LIVETV;
        }
        if((ui_layout&0x18) == 0x08)
        {
            key_map_ble[Kb_Map_devicelayout_Correspend_Index_G10[2]] = CR_USERPROFILE;
            nec_ir_cmd1_table[Kb_Map_devicelayout_Correspend_Index_G10[2]] = IR_USERPROFILE;
        }
        else if((ui_layout&0x18) == 0x10)
        {
            key_map_ble[Kb_Map_devicelayout_Correspend_Index_G10[2]] = CR_ALLAPPS;
            nec_ir_cmd1_table[Kb_Map_devicelayout_Correspend_Index_G10[2]] = IR_ALLAPPS;
        }
    }
    else
    {
        if((ui_layout&LAYOUT_MASK_BIT0))
        {
            key_map_ble[Kb_Map_devicelayout_Correspend_Index_G20[0]] = CR_NOTIFICATION;
            nec_ir_cmd1_table[Kb_Map_devicelayout_Correspend_Index_G20[0]] = IR_NOTIFICATION;
        }
        if((ui_layout&LAYOUT_MASK_BIT1))
        {
            key_map_ble[Kb_Map_devicelayout_Correspend_Index_G20[1]] = CR_LIVETV;
            nec_ir_cmd1_table[Kb_Map_devicelayout_Correspend_Index_G20[1]] = IR_LIVETV;
        }
        if((ui_layout&LAYOUT_MASK_BIT2))
        {
           key_map_ble[Kb_Map_devicelayout_Correspend_Index_G20[2]] = CR_TELETEXT;
           nec_ir_cmd1_table[Kb_Map_devicelayout_Correspend_Index_G20[2]] = IR_TELETEXT;
        }
        if((ui_layout&0x18) == 0x08)
        {
            key_map_ble[Kb_Map_devicelayout_Correspend_Index_G20[3]] = CR_USERPROFILE;
            nec_ir_cmd1_table[Kb_Map_devicelayout_Correspend_Index_G20[3]] = IR_USERPROFILE;
        }
        else if((ui_layout&0x18) == 0x10)
        {
            key_map_ble[Kb_Map_devicelayout_Correspend_Index_G20[3]] = CR_ALLAPPS;
            nec_ir_cmd1_table[Kb_Map_devicelayout_Correspend_Index_G20[3]] = IR_ALLAPPS;
        }
        if((ui_layout&LAYOUT_MASK_BIT5))
        {
            key_map_ble[Kb_Map_devicelayout_Correspend_Index_G20[4]] = CR_FASTREWIND;
            key_map_ble[Kb_Map_devicelayout_Correspend_Index_G20[5]] = CR_RECORD;
            key_map_ble[Kb_Map_devicelayout_Correspend_Index_G20[6]] = CR_PLAYPAUSE;
            key_map_ble[Kb_Map_devicelayout_Correspend_Index_G20[7]] = CR_FASTFORWARD;
            nec_ir_cmd1_table[Kb_Map_devicelayout_Correspend_Index_G20[4]] = IR_FASTREWIND;
            nec_ir_cmd1_table[Kb_Map_devicelayout_Correspend_Index_G20[5]] = IR_RECORD;
            nec_ir_cmd1_table[Kb_Map_devicelayout_Correspend_Index_G20[6]] = IR_PLAYPAUSE;
            nec_ir_cmd1_table[Kb_Map_devicelayout_Correspend_Index_G20[7]] = IR_FASTFORWARD;
        }
        if((ui_layout&LAYOUT_MASK_BIT6))
        {
            key_map_ble[Kb_Map_devicelayout_Correspend_Index_G20[8]] = CR_PERIOD;
            nec_ir_cmd1_table[Kb_Map_devicelayout_Correspend_Index_G20[8]] = IR_PERIOD;
        }
    }
}

/**
 * @brief       load nec ir table
 * @param[in]   none
 * @return      none
 */
void app_custom_set_new_ir_table(void)
{
    u8 dat[192];
    u8 g10_table[96];
    u8 i,j;

    if(_custom_data.device_type == REMOTE_G10)
    {
        flash_read_page(APP_NEC_IR_CODE_TABLE, 48*4, dat);
        //line up g10,g20
        for(i=0;i<8;i++)
        {
            for(j=0;j<3;j++)
            {
                g10_table[i*12+4*j]   = dat[i*24+4*j];
                g10_table[i*12+4*j+1] = dat[i*24+4*j+1];
                g10_table[i*12+4*j+2] = dat[i*24+4*j+2];
                g10_table[i*12+4*j+3] = dat[i*24+4*j+3];
            }
        }
        memcpy(dat,g10_table,24*4);
        for(i=0;i<24;i++)
        {
            nec_ir_addr1_table[i] = dat[i*4];
            nec_ir_addr2_table[i] = dat[i*4+1];
            if(dat[i*4+2] != 0xff)
            {
                nec_ir_cmd1_table[i]  = dat[i*4+2];
            }
            nec_ir_cmd2_table[i]  = dat[i*4+3];
        }
    }
    else if(_custom_data.device_type == REMOTE_G20)
    {
        flash_read_page(APP_NEC_IR_CODE_TABLE, 48*4, dat);
        for(i=0;i<48;i++)
        {
            nec_ir_addr1_table[i] = dat[i*4];
            nec_ir_addr2_table[i] = dat[i*4+1];
            if(dat[i*4+2] != 0xff)
            {
                nec_ir_cmd1_table[i]  = dat[i*4+2];
            }
            nec_ir_cmd2_table[i]  = dat[i*4+3];
        }
    }
}

/**
 * @brief       app_custom_param_init
 * @param[in]   device_layout: from flash 0x78000
 * @return      none
 */
void app_custom_param_init(u8 device_layout)
{
    if(_custom_data.device_type == REMOTE_G10)
    {
        p_kb_map_ble =  key_map_ble;
        memcpy(key_map_ble,(u8 *)Kb_Map_Ble_G10,sizeof(Kb_Map_Ble_G10));
        p_kb_map_ir = (u8 *)nec_ir_cmd1_table;
        memcpy(p_kb_map_ir,(u8 *)Kb_Map_Ir_G10,sizeof(Kb_Map_Ir_G10));
        p_kb_map_normal = (u8 *)Kb_Map_Normal_G10;
    }
    else if(_custom_data.device_type == REMOTE_G20)
    {
        p_kb_map_ble =  key_map_ble;
        memcpy(key_map_ble,(u8 *)Kb_Map_Ble_G20,sizeof(Kb_Map_Ble_G20));
        p_kb_map_normal = (u8 *)Kb_Map_Normal_G20;
        p_kb_map_ir = (u8 *)nec_ir_cmd1_table;
        memcpy(p_kb_map_ir,(u8 *)Kb_Map_Ir_G20,sizeof(Kb_Map_Ir_G20));
    }
    app_custom_ui_layout(device_layout);
    app_custom_set_new_ir_table();
    kb_p_map_init();
}

/**
 * @brief       load wakeup key by mask
 * @param[in]   wakeup_mask1,wakeup_mask2
 * @return      none
 * See document "Reference RCU Flash Storage Format" (Table 3.2) for details.
 */
void app_custom_wakeup_key_load(u8* wakeup_mask1, u8* wakeup_mask2)
{
    u8 j=0,i;
    u8 wakeupkey_num=0;
    u8 *p_kb_map;

    u16 mask1 = (u16)(((u16)wakeup_mask1[0]<<8) | ((u16)wakeup_mask1[1]));
    u16 mask2 = (u16)(((u16)wakeup_mask2[0]<<8) | ((u16)wakeup_mask2[1]));

    if((mask1 != 0xffff) && (mask2 != 0xffff) && ((mask1 & mask2) == 0))
    {
        if(_custom_data.device_type == REMOTE_G10)
        {
            wakeupkey_num = APP_CUSTOM_WAKEUPKEY_NUM-4;
            p_kb_map = (u8*)Kb_Wakeup_Key_Map_Correspend_Index_G10;
        }
        else
        {
            wakeupkey_num = APP_CUSTOM_WAKEUPKEY_NUM;
            p_kb_map = (u8*)Kb_Wakeup_Key_Map_Correspend_Index_G20;
        }
        for(i=0;i<wakeupkey_num;i++)
        {
            if(mask1&0x01)
            {
                wakeup_key1[j] = p_kb_map[i];
                j++;
            }
            mask1 = mask1>>1;
        }
        j=0;
        for(i=0;i<wakeupkey_num;i++)
        {
            if(mask2&0x01)
            {
                wakeup_key2[j] = p_kb_map[i];
                j++;
            }
            mask2 = mask2>>1;
        }

        printf("\r\nwakeup key1 =\r\n");
        for(i=0;i<wakeupkey_num;i++)
            printf(" %x",wakeup_key1[i]);
        printf("\r\nwakeup key2 =\r\n");
        for(i=0;i<wakeupkey_num;i++)
            printf(" %x",wakeup_key2[i]);

    }
}

/**
 * @brief       update manufacturer data by id
 * @param[in]   id (See document "Reference RCU Flash Storage Format" (Table 3.1) for details)
 * @return      none
 */
void app_custom_data_update_by_id(u8 id, u8* data, u8 length)
{
    switch(id)
    {
        case 1:  //device type
            if((data[0] == REMOTE_G10) || (data[0] == REMOTE_G20))
            {
                _custom_data.device_type = data[0];
            }
            break;

        case 2:  //vendor source
            _custom_data.pnp_id[0] = data[0];
            break;

        case 3:  //vid
            _custom_data.pnp_id[1] = data[0];
            _custom_data.pnp_id[2] = data[1];
            break;

        case 4:  //pid
            _custom_data.pnp_id[3] = data[0];
            _custom_data.pnp_id[4] = data[1];
            break;

        case 5:  //product version
            _custom_data.pnp_id[5] = data[0];
            _custom_data.pnp_id[6] = data[1];
            break;

        case 6:  //device name length
            _custom_data.device_name_length = data[0];
            if(_custom_data.device_name_length>16)
                _custom_data.device_name_length = 16;
            break;

        case 7:  //device name
            memcpy(_custom_data.device_name,data,_custom_data.device_name_length);

            break;

        case 8:  //device ui layout
            _custom_data.device_ui_layout = data[0];
            break;

        case 9:  //wakeup key 1
            _custom_data.wakeup_key_1[0] = data[0];
            _custom_data.wakeup_key_1[1] = data[1];
            break;

        case 10: //wakeup key 2
            _custom_data.wakeup_key_2[0] = data[0];
            _custom_data.wakeup_key_2[1] = data[1];
            break;

        case 11: //wakeup packet custom format
            memcpy(_custom_data.wakeup_packet,data,length);
            break;

        case 31: //enable or disable slave rpa function
            en_slave_rpa = data[0];
            break;

        case 32: //enable/disable ble adv
            en_ble_adv = data[0];
            break;

        case 33: //wakeup packet sending mode
            en_google_wakeuppack = data[0];
            break;

        case 34: //cache power switch
            en_powerkey_cache = data[0];
            break;
    }
}

/**
 * @brief       update manufacturer data
 * @param[in]   none
 * @return      none
 * See document "Reference RCU Flash Storage Format" (Chapter 6) for details.
 */
void app_custom_data_update(void)
{
    u8 buff[256];
    u16 i;
    u8 length;
    u8 id=0;
    u16 boundlen;
    u8 boundbuff[40];
    u8 flag=0,start;

    flash_read_page(APP_DATA_UPDATE_OVER_OTA_ADDR, 256, buff);
    if(buff[0] != 0xa5)
        return ;
    for(u8 k=0;k<16;k++)
    {
        if(k)
            flash_read_page(APP_DATA_UPDATE_OVER_OTA_ADDR+k*256, 256, buff);
        if(flag)
        {
            flag = 0;
            app_custom_data_update_over_ota_flash_page_pos = k;
            start = app_custom_data_update_over_ota_flash_addr_pos;
        }
        else
        {
            if(buff[0] != 0xa5)
                break;
            app_custom_data_update_over_ota_flash_addr_pos = 0;
            start = 0;
        }
        if(buff[start] != 0xa5)
            break;
        for(i=start;i<=255;i++)
        {
            if(buff[i] == 0xa5)
            {
                if(i == 255)
                {
                    flash_read_page(APP_DATA_UPDATE_OVER_OTA_ADDR+(k+1)*256, 1, &length);
                    flash_read_page(APP_DATA_UPDATE_OVER_OTA_ADDR+(k+1)*256+1, 1, &id);
                    flash_read_page(APP_DATA_UPDATE_OVER_OTA_ADDR+(k+1)*256+2, length, boundbuff);
                    app_custom_data_update_by_id(id,boundbuff,length);
                    app_custom_data_update_over_ota_flash_addr_pos = length+2;
                    flag = 1;
                }
                else if(i == 254)
                {
                    length = buff[i+1];
                    flash_read_page(APP_DATA_UPDATE_OVER_OTA_ADDR+(k+1)*256, 1, &id);
                    boundlen = i+length+2;
                    flash_read_page(APP_DATA_UPDATE_OVER_OTA_ADDR+(k+1)*256+1, length, boundbuff);
                    app_custom_data_update_by_id(id,boundbuff,length);
                    app_custom_data_update_over_ota_flash_addr_pos = length+1;
                    flag = 1;
                }
                else
                {
                    length = buff[i+1];
                    id = buff[i+2];
                    boundlen = i+length+2;
                    if(boundlen>255)
                    {
                        boundlen -= 255;
                        memcpy(boundbuff,&buff[i+3],length-boundlen);
                        flash_read_page(APP_DATA_UPDATE_OVER_OTA_ADDR+(k+1)*256, boundlen, &boundbuff[length-boundlen]);
                        app_custom_data_update_by_id(id,boundbuff,length);
                        app_custom_data_update_over_ota_flash_addr_pos = boundlen;
                        flag = 1;
                    }
                    else if(boundlen == 255)
                    {
                        app_custom_data_update_by_id(id,&buff[i+3],length);
                        app_custom_data_update_over_ota_flash_addr_pos = 0;
                        flag = 1;
                    }
                    else
                    {
                        app_custom_data_update_by_id(id,&buff[i+3],length);
                        app_custom_data_update_over_ota_flash_addr_pos += (3+length);
                    }
                }

                i += length+2;
            }
            else
            {
                break;
            }
        }
        app_custom_data_update_over_ota_flash_page_pos = k;
        //printf("duo_flash_addr_pos =%x,duo_flash_page_pos =%x",app_custom_data_update_over_ota_flash_addr_pos,app_custom_data_update_over_ota_flash_page_pos);
    }
    printf("duo_flash_addr_pos =%x,duo_flash_page_pos =%x",app_custom_data_update_over_ota_flash_addr_pos,app_custom_data_update_over_ota_flash_page_pos);
}

/**
 * @brief       app_custom_init
 * @param[in]   none
 * @return      none
 * See document "Reference RCU Flash Storage Format" and "Reference RCU Wake up Service"for details.
 */
void app_custom_init(void)
{
    u8 buffer[80];
    u8 addr_pos=0,i;
    u8 *pREMOTE_B046 = (u8 *)("RemoteB046");
    u8 my_PnPtrs []       = {0x02, 0x8a, 0x24, 0x66, 0x82, 0x01, 0x00};

    flash_read_page(APP_CUSTOM_ADDR, 80, buffer);
    for(i=0;i<APP_CUSTOM_WAKEUPKEY_NUM;i++)
    {
        wakeup_key1[i] = 0xff;
        wakeup_key2[i] = 0xff;
    }
    _custom_data.device_type = buffer[0];
    _custom_data.wakeup_packet[0] = 0xff;
    if((_custom_data.device_type == 0xff) || ((_custom_data.device_type != REMOTE_G10) && (_custom_data.device_type != REMOTE_G20)))
    {
        printf("default custom info\r\n");
        _custom_data.device_type = REMOTE_G10;
        _custom_data.device_name_length = 10;
        memcpy(_custom_data.device_name,pREMOTE_B046,10);
        memcpy(_custom_data.pnp_id,my_PnPtrs,7);
        _custom_data.device_ui_layout= 0;
        app_custom_data_update();
        app_custom_wakeup_key_load(_custom_data.wakeup_key_1,_custom_data.wakeup_key_2);
        my_FWRevtrs[3] = (_custom_data.device_type) + 0x30;
        my_FWRevtrs[4] = (_custom_data.device_ui_layout) + 0x30;
        app_custom_param_init(_custom_data.device_ui_layout);
        return;
    }

    memcpy(_custom_data.pnp_id,&buffer[1],7);

    _custom_data.device_name_length = buffer[8];

    if(_custom_data.device_name_length>16)
        memcpy(_custom_data.device_name,&buffer[9],16);
    else
        memcpy(_custom_data.device_name,&buffer[9],_custom_data.device_name_length);

    addr_pos = 9 + _custom_data.device_name_length;
    _custom_data.device_ui_layout = buffer[addr_pos];
    addr_pos = addr_pos+1;

    //wakeup key
    _custom_data.wakeup_key_1[0] = buffer[addr_pos];
    _custom_data.wakeup_key_1[1] = buffer[addr_pos+1];
    _custom_data.wakeup_key_2[0] = buffer[addr_pos+2];
    _custom_data.wakeup_key_2[1] = buffer[addr_pos+3];

    u8 wakeup_key_pos = addr_pos + 4;
    if(buffer[wakeup_key_pos] != 0xff)
        memcpy(_custom_data.wakeup_packet,&buffer[wakeup_key_pos],buffer[wakeup_key_pos]);

    if(_custom_data.device_name_length>16)
        _custom_data.device_name_length = 16;

    flash_read_page(APP_EN_CACHEKEY, 1, &en_powerkey_cache);
    flash_read_page(APP_EN_GOOGLE_WAKEUPPACK, 1, &en_google_wakeuppack);
    flash_read_page(APP_EN_BLE_ADV, 1, &en_ble_adv);
    flash_read_page(APP_EN_SLAVE_RPA, 1, &en_slave_rpa);

    app_custom_data_update();
    my_FWRevtrs[3] = (_custom_data.device_type) + 0x30;
    my_FWRevtrs[4] = (_custom_data.device_ui_layout) + 0x30;
    app_custom_wakeup_key_load(_custom_data.wakeup_key_1,_custom_data.wakeup_key_2);
    app_custom_param_init(_custom_data.device_ui_layout);


    /* 230214 by ken. read TX power settings */
    extern void app_rcu_set_TxPower(u8 Index);

    app_pnpid_format_t *p_my_pnpid = (app_pnpid_format_t *)_custom_data.pnp_id;

    if((0x01 == p_my_pnpid->vid_src) && (0x0957 == p_my_pnpid->vid) && (0x0005 == p_my_pnpid->pid)){
        /* Customized settings for special customers */
        app_rcu_set_TxPower(RF_POWER_P7p37dBm);
    }else{
        u8 my_TxPoewr = 0x00;
        flash_read_page(APP_TX_POWER_ADDR, 1 , &my_TxPoewr);
        if(0xff != my_TxPoewr){
            app_rcu_set_TxPower(my_TxPoewr);
        }
    }



}

/**
 * @brief       Get device type
 * @param[in]   none
 * @return      1:G10 2:g20
 */
u8 app_custom_get_device_type(void)
{
    return _custom_data.device_type;
}

/**
 * @brief       Determine whether the button is a wakeup key
 * @param[in]   keyid
 * @return      1:yes 0:no
 */
u8 app_custom_is_wakeup_key(u8 keyid)
{
    u8 i;

    for(i=0;i<APP_CUSTOM_WAKEUPKEY_NUM;i++)
    {
        if((keyid == wakeup_key1[i]) || (keyid == wakeup_key2[i]))
        {
            printf("wakeup_key =%x\r\n",keyid);
            return 1;
        }
    }
    return 0;
}

/**
 * @brief       Determine whether enable wakeup key function
 * @param[in]   keyid
 * @return      1:yes 0:no
 */
u8 app_custom_is_enable_wakeup_key(void)
{
    u16 wakeupkey1_mask = (u16)(((u16) _custom_data.wakeup_key_1[0]<<8) | ((u16) _custom_data.wakeup_key_1[1]));
    u16 wakeupkey2_mask = (u16)(((u16) _custom_data.wakeup_key_2[0]<<8) | ((u16) _custom_data.wakeup_key_2[1]));
    if((wakeupkey1_mask != 0xffff) && (wakeupkey2_mask != 0xffff) && ((wakeupkey1_mask & wakeupkey2_mask) == 0))
        return 1;
    else
        return 0;
}

/**
 * @brief       Get KEYID
 * @param[in]   keyid
 * @return      1 or 2
 */
u8  app_custom_wakeupkey_packet_index(u8 keyid)
{
    u8 i;

    for(i=0;i<APP_CUSTOM_WAKEUPKEY_NUM;i++)
    {
        if(keyid == wakeup_key1[i])
            return 1;
        else if(keyid == wakeup_key2[i])
            return 2;
    }
    return 0;
}
/**
 * @brief       Get drive len
 * @param[in]   none
 * @return      len
 */
u8 app_custom_get_drive_len(void)
{
    u8 len;

    if(_custom_data.device_type == REMOTE_G10)
        len = sizeof(Kb_Drive_Pins_G10)/sizeof(Kb_Drive_Pins_G10[0]);
    else
        len = sizeof(Kb_Drive_Pins_G20)/sizeof(Kb_Drive_Pins_G20[0]);
    return len;
}


#if APP_DATA_UPDATE_OVER_BLE
_attribute_data_retention_ app_custom_data_update_sts app_custom_data_update_state = APP_CUSTOM_DATA_UPDATE_END;

/**
 * @brief       parse data update commands and packets
 * @param[in]   data and len
 * @return      none
 */
void app_custom_data_update_process(u8 *data,u16 len)
{
    u8 datasend[3];
    u16 crc=0;
    u32 flash_addr=0,pos=0;
    u8 readdata[16];
    u8 result=0;
    u16 cmd = data[0] | (data[1]<<8);

    if(cmd == APP_CUSTOM_CMD_DATA_UPDATE_PREPARE)
    {
        if(app_custom_data_update_state == APP_CUSTOM_DATA_UPDATE_WRITEFAIL)
        {
            app_custom_data_update_state = APP_CUSTOM_DATA_UPDATE_ERASE;
        }
        else
        {
            printf("APP_CUSTOM_CMD_DATA_UPDATE_PREPARE \r\n");
            app_custom_data_update_state = APP_CUSTOM_DATA_UPDATE_PREPARE;
        }
    }
    else if(cmd == APP_CUSTOM_CMD_DATA_UPDATE_START)
    {
        app_custom_data_update_state = APP_CUSTOM_DATA_UPDATE_START;
    }
    else if(cmd == APP_CUSTOM_CMD_DATA_UPDATE_END)
    {
        if(app_custom_data_update_state == APP_CUSTOM_DATA_UPDATE_START)
        {
            datasend[0] = 0xEC;
            datasend[1] = 2;
            datasend[2] = 0;
            bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, datasend, sizeof(datasend));
            app_custom_init();
            app_custom_data_update_state = APP_CUSTOM_DATA_UPDATE_END;
        }
    }

    if(app_custom_data_update_state == APP_CUSTOM_DATA_UPDATE_START)
    {
        if(len == 20)
        {
            crc = (data[19]<<8) | data[18];
            pos = (data[0]) | (data[1]<<8);
            extern unsigned short crc16 (unsigned char *pD, int len);
            if(crc == crc16(data,18))
            {
                {
                    flash_addr = APP_DATA_UPDATE_OVER_OTA_ADDR +  pos*16 + app_custom_data_update_over_ota_flash_addr_pos + \
                                 app_custom_data_update_over_ota_flash_page_pos*256;
                    //printf("flash_addr=%x\r\n",flash_addr);
                    flash_write_page(flash_addr,16,&data[2]);
                    flash_read_page(flash_addr,16, readdata);
                    if(memcmp(&data[2],readdata,16))
                    {
                        printf("crc error\r\n");
                        result = 1;
                    }
                }
            }
            else
            {
                result = 1;
            }
            if(result == 1)
            {
                datasend[0] = 0xEC;
                datasend[1] = 2;
                datasend[2] = 1;
                bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, datasend, sizeof(datasend));
                app_custom_data_update_state = APP_CUSTOM_DATA_UPDATE_WRITEFAIL;   //data invalid, must erase
            }
        }
    }
}

/**
 * @brief       Erase data update area
 * @param[in]   none
 * @return      -1:already erase  0:not erase
 */
int app_custom_data_update_sector_erase_timeoutcb(void)
{
    extern u8 is_mic_enable(void);
    if(is_mic_enable() == 1)
        return 0;
    if(bls_ll_requestConnBrxEventDisable() > 120)
    {
        printf("app_custom_data_update_sector_erase_timeoutcb\r\n");
        bls_ll_disableConnBrxEvent();
        flash_erase_sector(APP_DATA_UPDATE_OVER_OTA_ADDR);
        bls_ll_restoreConnBrxEvent();
        return -1;
    }
    return 0;
}

/**
 * @brief       loop interface for erase data update area
 * @param[in]   none
 * @return      none
 */
void app_custom_data_update_sector_erase_loop(void)
{
    u8 datasend[3];
    if(app_custom_data_update_state == APP_CUSTOM_DATA_UPDATE_PREPARE)
    {
        if(device_in_connection_state)
        {
            datasend[0] = 0XEC;
            datasend[1] = 0;
            datasend[2] = 0;
            bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, datasend, sizeof(datasend));
        }
        app_custom_data_update_state = APP_CUSTOM_DATA_UPDATE_PREPARE_END;
    }
    else if(app_custom_data_update_state == APP_CUSTOM_DATA_UPDATE_WRITEFAIL)
    {
        if(app_custom_data_update_sector_erase_timeoutcb() == -1)
        {
            app_custom_data_update_state = APP_CUSTOM_DATA_UPDATE_ERASE_SUCC;
            app_custom_data_update_over_ota_flash_addr_pos = 0;
            app_custom_data_update_over_ota_flash_page_pos = 0;
        }
    }
    else if(app_custom_data_update_state == APP_CUSTOM_DATA_UPDATE_ERASE)
    {
        if(app_custom_data_update_sector_erase_timeoutcb() == -1)
        {
            app_custom_data_update_state = APP_CUSTOM_DATA_UPDATE_PREPARE;
            app_custom_data_update_over_ota_flash_addr_pos = 0;
            app_custom_data_update_over_ota_flash_page_pos = 0;
        }
    }
}
#endif
