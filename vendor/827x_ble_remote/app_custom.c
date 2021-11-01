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


const u8 *pREMOTE_G10 = (u8 *)("RemoteG10");
const u8 *pREMOTE_G20 = (u8 *)("RemoteG20");

extern u8  my_PnPtrs [];
extern u8 *p_kb_map_ble;
extern u8 *p_kb_map_ir;
extern u8 *p_kb_map_normal;
extern u8 my_devName[20];
extern u8 my_devNamelen;
extern  _attribute_data_retention_ u8 my_FWRevtrs [];
extern void kb_p_map_init(void);
extern u8 device_in_connection_state;


#define LAYOUT_MASK_BIT0               0X01
#define LAYOUT_MASK_BIT1               0X02
#define LAYOUT_MASK_BIT2               0X04
#define LAYOUT_MASK_BIT5               0X20

_attribute_data_retention_  u8 device_type=1;
_attribute_data_retention_  u8 en_powerkey_cache=1;
_attribute_data_retention_  u8 en_ble_adv=1;
_attribute_data_retention_  u8 wakeup_key1[APP_CUSTOM_WAKEUPKEY_NUM];
_attribute_data_retention_  u8 wakeup_key2[APP_CUSTOM_WAKEUPKEY_NUM];
_attribute_data_retention_  u8 wakeup_key_pos=0;
_attribute_data_retention_  u8 comb_key_keyid[9]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};  //back,home,center,mute,setting,guide,subtitle,down
_attribute_data_retention_  u8 nec_ir_addr1_table[48]={0};
_attribute_data_retention_  u8 nec_ir_addr2_table[48]={0};
_attribute_data_retention_  u8 nec_ir_cmd1_table[48]={0};
_attribute_data_retention_  u8 nec_ir_cmd2_table[48]={0};
_attribute_data_retention_  u8 key_map_ble[48]={0};


/*****************************************************************************************
 *** If you get a new hardware, and want to modify keymap    :                        ***
 *** 1) Change the pin setting(drive pin and scan pin) contained in app_config.h      ***
 *** 2) The following three points also need to be changed synchronously              ***
 ****************************************************************************************/

/* 1. Pin definition */
const unsigned int Kb_Drive_Pins_G10[3] = {GPIO_PD2, GPIO_PD3, GPIO_PD4};
const unsigned int Kb_Drive_Pins_G20[6] = {GPIO_PD2, GPIO_PD3, GPIO_PD4, GPIO_PD7, GPIO_PA0, GPIO_PD1};

/* 2. The following indexes are all extracted from Kb_Map_Normal_G10/Kb_Map_Normal_G20 */
const unsigned char Kb_CombKey_KeyId_G10[9] = {7,  0xa, 0x15,0x16,0x0c,0x0d,0xff,4,6};                 //back,home,center,mute,setting,guide,subtitle,down,bookmark keyid in g10
const unsigned char Kb_CombKey_KeyId_G20[9] = {0xd,0x13,0x2a,0x2b,0x18,0x19,0x0f,7,12};                //back,home,center,mute,setting,guide,subtitle,down,bookmark keyid in g20

const unsigned char Kb_Wakeup_Key_Map_Correspend_Index_G10[] =  {VOICE,10,0,3,8,11,14,17};             //voice,home,power,input,youtobe,netflix,disney,hbomax
const unsigned char Kb_Wakeup_Key_Map_Correspend_Index_G20[] =  {VOICE,19,0,6,14,20,26,32,28,5,11,34}; //voice,home,power,input,youtobe,netflix,disney,hbomax,blue,green,red,yellow

const unsigned char Kb_Map_devicelayout_Correspend_Index_G10[] =  {12,13,6};                           //notification,livetv,,userprofile/allapps
const unsigned char Kb_Map_devicelayout_Correspend_Index_G20[] =  {24,25,15,12,11,5,34,28};            //notification,livetv,teletext,userprofile/allapps,fast rewind,record,play/pause,fast forward


/* 3. Change key index  and ir/ble value*/
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

void app_custom_ui_layout(u8 ui_layout)
{
    printf("ui_layout=%x\r\n",ui_layout);
    if((ui_layout == 0xff) || (ui_layout == 0))
    {
        return;
    }
    if(device_type == REMOTE_G10)
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
    }
}

void app_custom_set_new_ir_table(void)
{
    u8 dat[192];
    u8 g10_table[96];
    u8 i,j;

    if(device_type == REMOTE_G10)
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
    else if(device_type == REMOTE_G20)
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

void app_custom_param_init(u8 device_layout)
{
    if(device_type == REMOTE_G10)
    {
        p_kb_map_ble =  key_map_ble;
        memcpy(key_map_ble,(u8 *)Kb_Map_Ble_G10,sizeof(Kb_Map_Ble_G10));
        p_kb_map_ir = (u8 *)nec_ir_cmd1_table;
        memcpy(p_kb_map_ir,(u8 *)Kb_Map_Ir_G10,sizeof(Kb_Map_Ir_G10));

        p_kb_map_normal = (u8 *)Kb_Map_Normal_G10;
    }
    else if(device_type == REMOTE_G20)
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

void app_custom_wakeup_key_load(u16 mask1, u16 mask2)
{
    u8 j=0,i;
    u8 wakeupkey_num=0;
    u8 *p_kb_map;

    if((mask1 != 0xffff) && (mask2 != 0xffff) && ((mask1 & mask2) == 0))
    {
        if(device_type == REMOTE_G10)
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

void app_custom_init(void)
{
    u8 buffer[64];
    u8 addr_pos=0,i;
    u8 device_ui_layout_type=0;
    u8 *pREMOTE_B046 = (u8 *)("RemoteB046");
    u16 wakeupkey1_mask,wakeupkey2_mask;

    flash_read_page(APP_CUSTOM_ADDR, 64, buffer);
    for(i=0;i<APP_CUSTOM_WAKEUPKEY_NUM;i++)
    {
        wakeup_key1[i] = 0xff;
        wakeup_key2[i] = 0xff;
    }
    if(buffer[0] == 0xff)
    {
        printf("default custom info\r\n");
        device_type = REMOTE_G10;
        my_devNamelen = 10;
        memcpy(my_devName,pREMOTE_B046,10);
        app_custom_param_init(0);
        my_FWRevtrs[3] = (device_type) + 0x30;
        my_FWRevtrs[4] = 0x30;
        wakeup_key_pos = 24;
        return;
    }
    else
    {
        memcpy(my_PnPtrs,&buffer[1],7);
        my_devNamelen = buffer[8];
        device_type = buffer[0];
        if((device_type != REMOTE_G10) && (device_type != REMOTE_G20))
        {
            device_type = REMOTE_G10;
            my_devNamelen = 10;
            memcpy(my_devName,pREMOTE_B046,10);
            app_custom_param_init(0);
            my_FWRevtrs[3] = (device_type) + 0x30;
            my_FWRevtrs[4] = 0x30;
            wakeup_key_pos = 24;
            return;
        }
        if(my_devNamelen>16)
            memcpy(my_devName,&buffer[9],16);
        else
            memcpy(my_devName,&buffer[9],my_devNamelen);

        addr_pos = 9 + my_devNamelen;
        device_ui_layout_type = buffer[addr_pos];
        addr_pos = addr_pos+1;

        //wakeup key
        wakeupkey1_mask = (u16)(((u16)buffer[addr_pos]<<8) | ((u16)buffer[addr_pos+1]));
        wakeupkey2_mask = (u16)(((u16)buffer[addr_pos+2]<<8) | ((u16)buffer[addr_pos+3]));
        printf("wakeupkey1_mask=%x,wakeupkey2_mask=%x\r\n",wakeupkey1_mask,wakeupkey2_mask);
        app_custom_wakeup_key_load(wakeupkey1_mask,wakeupkey2_mask);
        wakeup_key_pos = addr_pos + 4;
        printf("wakeup_key_pos=%x\r\n",wakeup_key_pos);
        if(my_devNamelen>16)
            my_devNamelen = 16;
    }
    app_custom_param_init(device_ui_layout_type);
    my_FWRevtrs[3] = (device_type) + 0x30;
    my_FWRevtrs[4] = (device_ui_layout_type) + 0x30;

    flash_read_page(APP_EN_CACHEKEY, 1, buffer);
    if(buffer[0] == 0)
    {
        en_powerkey_cache = 0;
    }
    flash_read_page(APP_EN_BLE_ADV, 1, &en_ble_adv);
}

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

u8 app_custom_is_enable_wakeup_key(void)
{
    u8 buffer[4];
    u16 wakeupkey1_mask,wakeupkey2_mask;
    u32 addr=0;

    addr = 0x78000+wakeup_key_pos-4;
    flash_read_page(addr, 4, buffer);
    wakeupkey1_mask = (u16)(((u16)buffer[0]<<8) | ((u16)buffer[1]));
    wakeupkey2_mask = (u16)(((u16)buffer[2]<<8) | ((u16)buffer[3]));
    if((wakeupkey1_mask != 0xffff) && (wakeupkey2_mask != 0xffff) && ((wakeupkey1_mask & wakeupkey2_mask) == 0))
        return 1;
    else
        return 0;
}

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
u8 app_custom_get_drive_len(void)
{
    u8 len;

    if(device_type == REMOTE_G10)
        len = sizeof(Kb_Drive_Pins_G10)/sizeof(Kb_Drive_Pins_G10[0]);
    else
        len = sizeof(Kb_Drive_Pins_G20)/sizeof(Kb_Drive_Pins_G20[0]);
    return len;
}



