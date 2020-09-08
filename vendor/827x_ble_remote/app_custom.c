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

#define APP_CUSTOM_ADDR 0x78000

const u8 *pREMOTE_G10 = (u8 *)("RemoteG10");
const u8 *pREMOTE_G20 = (u8 *)("RemoteG20");

extern u8  my_PnPtrs [];
extern u8 *p_kb_map_ble;
extern u8 *p_kb_map_ir;
extern u8 *p_kb_map_normal;
extern u8 my_devName[20];
extern u8 my_devNamelen;

_attribute_data_retention_  u8 device_type=1;

_attribute_data_retention_  u8 key_setting_or_noti[2]={IR_DASHBOARD,G00GLE_MKEY_DASHBOARD};

const unsigned char Kb_Map_Normal[8][3] = \
                        {\
                        {0,          1,             2}, \
                        {3,          4,             5}, \
                        {6,          7,             8}, \
                        {VOICE,      10,            11},\
                        {12,         13,            14},\
                        {15,         16,            17},\
                        {18,         19,            20},\
                        {21,         22,            23}};


const unsigned char Kb_Map_Ble[] = {\
                        CR_POWER,      CR_RIGHT,       CR_VOL_DN,    \
                        CR_INPUT,      CR_DN,          CR_CHN_DN,    \
                        CR_BOOKMARK,   CR_BACK,        CR_YOUTUBE,   \
                        CR_ASSIST,     CR_HOME,        CR_NETFLIX,   \
                        CR_DASHBOARD,  CR_GUIDE,       CR_DISNEY,    \
                        CR_UP,         CR_VOL_UP,      CR_HBOMAX,    \
                        CR_LEFT,       CR_CHN_UP,      VK_NONE,      \
                        CR_CENTER,     CR_VOL_MUTE,    VK_NONE   };


const unsigned char Kb_Map_Ir[] =  {\
                        IR_POWER,      IR_RIGHT,       IR_VOL_DN,    \
                        IR_INPUT,      IR_DN,          IR_CHN_DN,    \
                        IR_BOOKMARK,   IR_BACK,        IR_YOUTUBE,   \
                        IR_ASSIST,     IR_HOME,        IR_NETFLIX,   \
                        IR_DASHBOARD,  IR_GUIDE,       IR_DISNEY,    \
                        IR_UP,         IR_VOL_UP,      IR_HBOMAX,    \
                        IR_LEFT,       IR_CHN_UP,      VK_NONE,      \
                        IR_CENTER,     IR_MUTE,        VK_NONE    };

const unsigned int Kb_Drive_Pins_G10[3] = {GPIO_PD2, GPIO_PD3, GPIO_PD4};
const unsigned int Kb_Drive_Pins_G20[6] = {GPIO_PD2, GPIO_PD3, GPIO_PD4, GPIO_PD7, GPIO_PA0, GPIO_PD1};


#define IR_SETTING                     0X0F
#define CR_SETTING                     0X0096

#define IR_NOTIFICATION                0X0F
#define CR_NOTIFICATION                0X0097

void app_custom_ui_layout(u8 ui_layout)
{
    if((ui_layout != 0) && (ui_layout !=1) )
		return;
	
    if(ui_layout == 1)
    {
        printf("notification key\r\n");
        key_setting_or_noti[0] = IR_NOTIFICATION;
		key_setting_or_noti[1] = CR_NOTIFICATION;
    }
	else
	{
	    printf("setting key\r\n");
	    key_setting_or_noti[0] = IR_SETTING;
		key_setting_or_noti[1] = CR_SETTING;
	}
}

void app_custom_param_init(u8 device)
{
    if(device == REMOTE_G10)
    {
        p_kb_map_ble    = (u8 *)Kb_Map_Ble;
        p_kb_map_ir     = (u8 *)Kb_Map_Ir;
		p_kb_map_normal = (u8 *)Kb_Map_Normal;
		extern void kb_p_map_init(void);
		kb_p_map_init();
    }
	else if(device == REMOTE_G20)
	{
	}
}

void app_custom_init(void)
{
	u8 buffer[64];
	u8 addr_pos=0;
	u8 device_ui_layout_type=0;
	u8 *pREMOTE_B046 = (u8 *)("RemoteB046");

	flash_read_page(APP_CUSTOM_ADDR, 64, buffer);

	if(buffer[0] == 0xff)
	{
	    printf("default custom info\r\n");
	    device_type = REMOTE_G10;
		/*
		my_devNamelen = 9;
		memcpy(my_devName,pREMOTE_G10,9);
		printf("device_ui_layout_type=%x\r\n",buffer[0x12]);
		app_custom_ui_layout(buffer[0x12]);
		*/
		my_devNamelen = 10;
		memcpy(my_devName,pREMOTE_B046,10);
		app_custom_ui_layout(0);
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
		    app_custom_ui_layout(0);
			app_custom_param_init(device_type);
			return;
		}

		memcpy(my_devName,&buffer[9],my_devNamelen);
	   
		addr_pos = 9 + my_devNamelen;
	    device_ui_layout_type = buffer[addr_pos];
		printf("device_type = %x ,device_ui_type=%x\r\n",device_type,device_ui_layout_type);
		app_custom_ui_layout(device_ui_layout_type);
		for(u8 i=0;i<7;i++)
			printf(" %x",my_PnPtrs[i]);

	}
	app_custom_param_init(device_type);
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

void app_custom_test(void)
{
    u8 buffer[]={0x01,0x02, 0x8a, 0x24, 0x66, 0x82, 0x01, 0x00,0x03,0x52,0x65,0x6d,0x6f,0x74,0x65,0x47,0x31,0x30,0x01};
	flash_erase_sector(APP_CUSTOM_ADDR);
    flash_write_page(APP_CUSTOM_ADDR,sizeof(buffer),buffer);
}


