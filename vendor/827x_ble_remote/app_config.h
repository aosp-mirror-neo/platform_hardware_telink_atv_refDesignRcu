/********************************************************************************************************
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
 *******************************************************************************************************/

#pragma once

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif


/////////////////// FEATURE SELECT /////////////////////////////////
#define BLE_REMOTE_PM_ENABLE				1
#define PM_DEEPSLEEP_RETENTION_ENABLE		1
#define BLE_REMOTE_SECURITY_ENABLE      	1
#define BLE_REMOTE_OTA_ENABLE				1
#define REMOTE_IR_ENABLE					1
#define BATT_CHECK_ENABLE       			1   //must enable
#define BLE_AUDIO_ENABLE					1
#define BLT_APP_LED_ENABLE					1
#define BLT_TEST_SOFT_TIMER_ENABLE			0
//flash firmware_check
#define FIRMWARE_CHECK_ENABLE				0
//firmware check
#define FIRMWARES_SIGNATURE_ENABLE          0

#define AUDIO_TRANS_USE_2M_PHY_ENABLE		0

#define UNUSED_GPIO_PULLDOWN_ENABLE			1

/////////////////// DEEP SAVE FLG //////////////////////////////////
#define USED_DEEP_ANA_REG                   DEEP_ANA_REG0 //u8,can save 8 bit info when deep
#define	LOW_BATT_FLG					    BIT(0) //if 1: low battery
#define CONN_DEEP_FLG	                    BIT(1) //if 1: conn deep, 0: adv deep
#define IR_MODE_DEEP_FLG	 				BIT(2) //if 1: IR mode, 0: BLE mode
#define LOW_BATT_SUSPEND_FLG				BIT(3) //if 1 : low battery, < 1.8v

#if (REMOTE_IR_ENABLE)
	//ir key
	#define TYPE_IR_SEND			1
	#define TYPE_IR_RELEASE			2

	///////////////////// key mode //////////////////////
	//#define KEY_MODE_BLE	   		0    //ble key
	#define KEY_MODE_IR        		1    //ir  key

#endif
///////////////////// key mode //////////////////////
	#define KEY_MODE_BLE	   		0    //ble key
#if (BATT_CHECK_ENABLE)
#if (__PROJECT_8278_BLE_REMOTE__)
	//use VBAT(8278) , then adc measure this VBAT voltage
	#define ADC_INPUT_PCHN					VBAT    //corresponding  ADC_InputPchTypeDef in adc.h
#else
	//telink device: you must choose one gpio with adc function to output high level(voltage will equal to vbat), then use adc to measure high level voltage
	//use PB7(8258) output high level, then adc measure this high level voltage
	#define GPIO_VBAT_DETECT				GPIO_PB7
	#define PB7_FUNC						AS_GPIO
	#define PB7_INPUT_ENABLE				0
	#define ADC_INPUT_PCHN					B7P    //corresponding  ADC_InputPchTypeDef in adc.h
#endif
#endif


//////////////////// LED CONFIG (RCU board)///////////////////////////
#if (BLT_APP_LED_ENABLE)
	#define LED_ON_LEVAL 						1 			//gpio output high voltage to turn on led
	#define	GPIO_LED							GPIO_PC6
	#define PC6_FUNC							AS_GPIO
#endif

#if (BLT_TEST_SOFT_TIMER_ENABLE)
	#define BLT_SOFTWARE_TIMER_ENABLE		1
#endif


////////////////////////// GOOGLE AUDIO CONFIG (RCU board) /////////////////////////////

#if (BLE_AUDIO_ENABLE)
/////////////     GOOGLE VOICE      ////////
	#define AUDIO_BUFFER_SIZE               1024
	#define AUDIO_BUFFER_SAMPLE             (AUDIO_BUFFER_SIZE>>1)
	#define AUDIO_ADPCM_BUFFER_COUNT        4
	#define AUDIO_ADPCM_PACKET_LENGTH       (128+6+2) // 2 is just for 4*n
	#define AUDIO_GOL_ADPCM_PACKET_SAMPLE   256
	#define AUDIO_BUFFER_HALF_SIZE          (AUDIO_BUFFER_SIZE>>1)
#endif
////////////////////////// AUDIO CONFIG (RCU board) /////////////////////////////
#if (BLE_AUDIO_ENABLE)
	#define BLE_DMIC_ENABLE					0  //0: Amic   1: Dmic
	#define IIR_FILTER_ENABLE				1

	#define	ADPCM_PACKET_LEN				128
	#define TL_MIC_ADPCM_UNIT_SIZE			248

	#if IIR_FILTER_ENABLE
		#define	TL_MIC_BUFFER_SIZE				1984
	#else
		#define	TL_MIC_BUFFER_SIZE				992
	#endif

	#if BLE_DMIC_ENABLE
//		#define GPIO_DMIC_BIAS					GPIO_PC4// need check
		#define GPIO_DMIC_DI					GPIO_PA0
		#define GPIO_DMIC_CK					GPIO_PA1
	#else
		#define GPIO_AMIC_BIAS					GPIO_PC4// need check ,v1.0 PC4, V1.1 PC0
	#endif
#endif

//PB3 IRout 100K pulldown when  IR not working,  when IR begin, disable this 100K pulldown
#define	PULL_WAKEUP_SRC_PB3		PM_PIN_PULLDOWN_100K



//////////////////////////// KEYSCAN/MIC  GPIO //////////////////////////////////
#define	MATRIX_ROW_PULL					PM_PIN_PULLDOWN_100K
#define	MATRIX_COL_PULL					PM_PIN_PULLUP_10K

#define	KB_LINE_HIGH_VALID				0   //dirve pin output 0 when keyscan, scanpin read 0 is valid
#define DEEPBACK_FAST_KEYSCAN_ENABLE	0   //proc fast scan when deepsleep back trigged by key press, in case key loss
#define LONG_PRESS_KEY_POWER_OPTIMIZE	1   //lower power when pressing key without release

//stuck key
#define STUCK_KEY_PROCESS_ENABLE		1
#define STUCK_KEY_ENTERDEEP_TIME		60  //in s

//repeat key
#define KB_REPEAT_KEY_ENABLE			0
#define	KB_REPEAT_KEY_INTERVAL_MS		200
#define KB_REPEAT_KEY_NUM				1
#define KB_MAP_REPEAT					{VK_1, }

//////////////////// key type ///////////////////////
#define IDLE_KEY	   			0
#define CONSUMER_KEY   	   		1
#define KEYBOARD_KEY   	   		2
#define IR_KEY   	   			3

#define			CR_VOL_UP				0xf0  ////
#define			CR_VOL_DN				0xf1
#define			CR_VOL_MUTE				0xf2
#define			CR_CHN_UP				0xf3
#define			CR_CHN_DN				0xf4  ////
#define			CR_POWER				0xf5
#define			CR_SEARCH				0xf6
#define			CR_RECORD				0xf7
#define			CR_PLAY					0xf8  ////
#define			CR_PAUSE				0xf9
#define			CR_STOP					0xfa
#define			CR_FAST_BACKWARD		0xfb
#define			CR_FAST_FORWARD			0xfc  ////
#define			CR_HOME					0xfd
#define			CR_BACK					0xfe
#define			CR_MENU					0xff

//special key
#define		 	VOICE					0xc0
#define 		KEY_MODE_SWITCH			0xc1
#define		 	PHY_TEST				0xc2


#define 		IR_VK_0			0x00
#define 		IR_VK_1			0x01
#define 		IR_VK_2			0x02
#define			IR_VK_3			0x03
#define			IR_VK_4			0x04
#define 		IR_VK_5			0x05
#define 		IR_VK_6			0x06
#define 		IR_VK_7			0x07
#define 		IR_VK_8			0x08
#define 		IR_VK_9			0x09

#define 		IR_POWER		0x12
#define			IR_AUDIO_MUTE	0x0d
#define 		IR_NETFLIX		0x0f
#define			IR_BACK			0x0e
#define			IR_VOL_UP		0x0b
#define			IR_VOL_DN		0x0c
#define 		IR_NEXT			0x20
#define 		IR_PREV			0x21
#define			IR_MENU			0x23
#define 		IR_HOME			0x24
#define 		IR_OPER_KEY		0x2e
#define 		IR_INFO			0x2f
#define			IR_REWIND		0x32
#define 		IR_FAST_FORWARD	0x34
#define 		IR_PLAY_PAUSE	0x35
#define			IR_GUIDE		0x41
#define 		IR_UP			0x45
#define			IR_DN			0x44
#define 		IR_LEFT			0x42
#define 		IR_RIGHT		0x43
#define			IR_SEL			0x46
#define 		IR_RED_KEY		0x6b
#define 		IR_GREEN_KEY	0x6c
#define 		IR_YELLOW_KEY	0x6d
#define 		IR_BLUE_KEY		0x6e
#define 		IR_RECORD		0x72
#define 		IR_OPTION		0x73
#define 		IR_STOP			0x74
#define 		IR_SEARCH		0x75
#define 		IR_TEXT			0x76
#define 		IR_VOICE		0x77
#define 		IR_PAUSE		0x78

#define			T_VK_CH_UP		0xd0
#define			T_VK_CH_DN		0xd1


#if (REMOTE_IR_ENABLE)  //with IR keymap
//	#define 		GPIO_IR_CONTROL			GPIO_PD0

	#if (__PROJECT_8278_BLE_REMOTE__)

		#define		KB_MAP_NORMAL	{\
						{0, 	    1,				2,			3,		4,		}, \
						{VOICE,	 	6,				7,			8,		9		}, \
						{10,		11,				12,			13,		14,		}, \
						{15,		16,				17,			18,		19,		}, \
						{20,		21,				22,			23,		24,		}, \
						{25,		26,				27,			28,		29,		}, }

		#define		KB_MAP_BLE	{\
						VK_B,		CR_POWER,		VK_NONE,		VK_C,				CR_HOME,	 \
						VK_NONE,	VK_NONE,		VK_NONE,		CR_VOL_UP,			CR_VOL_DN,	 \
						VK_2,		VK_RIGHT,		CR_VOL_DN,		VK_3,				VK_1,	 \
						VK_5,		VK_ENTER,		CR_VOL_UP,		VK_6,				VK_4,	 \
						VK_8,		VK_DOWN,		VK_UP ,			VK_9,				VK_7,	 \
						VK_0,		CR_BACK,		VK_LEFT,		CR_VOL_MUTE,		CR_MENU,	 }


		#define		KB_MAP_IR	{\
						VK_NONE,	VK_NONE,		VK_NONE,	VK_NONE,			VK_NONE,	 \
						VK_NONE,	VK_NONE,		VK_NONE,	VK_NONE,			VK_NONE,	 \
						IR_VK_2,	VK_NONE,		VK_NONE,	IR_VK_3,			IR_VK_1, 	 \
						IR_VK_5,	VK_NONE,		VK_NONE,	IR_VK_6,			IR_VK_4,	 \
						IR_VK_8,	VK_NONE,		VK_NONE,	IR_VK_9,			IR_VK_7,	 \
						IR_VK_0,	VK_NONE,		VK_NONE,	VK_NONE,			VK_NONE,	 }
	#elif (__PROJECT_8258_BLE_REMOTE__)
		#define		KB_MAP_NORMAL	{\
						{0, 	    1,				2,			3,		4,		}, \
						{VOICE,	 KEY_MODE_SWITCH,	7,			8,		9		}, \
						{10,		11,				12,			13,		14,		}, \
						{15,		16,				17,			18,		19,		}, \
						{20,		21,				22,			23,		24,		}, \
						{25,		26,				27,			28,		29,		}, }

		#define		KB_MAP_BLE	{\
						VK_NONE,	VK_NONE,		VK_NONE,		VK_NONE,			VK_NONE,	 \
						VOICE,		VK_NONE,		VK_NONE,		CR_VOL_UP,			CR_VOL_DN,	 \
						VK_2,		VK_NONE,		VK_NONE,		VK_3,				VK_1,	 \
						VK_5,		VK_NONE,		VK_NONE,		VK_6,				VK_4,	 \
						VK_8,		VK_NONE,		VK_NONE,		VK_9,				VK_7,	 \
						VK_0,		VK_NONE,		VK_NONE,		VK_NONE,			VK_NONE,	 }


		#define		KB_MAP_IR	{\
						VK_NONE,	VK_NONE,		VK_NONE,	VK_NONE,			VK_NONE,	 \
						VK_NONE,	VK_NONE,		VK_NONE,	VK_NONE,			VK_NONE,	 \
						IR_VK_2,	VK_NONE,		VK_NONE,	IR_VK_3,			IR_VK_1, 	 \
						IR_VK_5,	VK_NONE,		VK_NONE,	IR_VK_6,			IR_VK_4,	 \
						IR_VK_8,	VK_NONE,		VK_NONE,	IR_VK_9,			IR_VK_7,	 \
						IR_VK_0,	VK_NONE,		VK_NONE,	VK_NONE,			VK_NONE,	 }
	#endif
#else   //key map
	#if (__PROJECT_8278_BLE_REMOTE__)
		#define		KB_MAP_NORMAL	{\
						VK_B,		CR_POWER,		VK_NONE,		VK_C,				CR_HOME,	 \
						VOICE,		VK_NONE,		VK_NONE,		CR_VOL_UP,			CR_VOL_DN,	 \
						VK_2,		VK_RIGHT,		CR_VOL_DN,		VK_3,				VK_1,	 \
						VK_5,		VK_ENTER,		CR_VOL_UP,		VK_6,				VK_4,	 \
						VK_8,		VK_DOWN,		VK_UP ,			VK_9,				VK_7,	 \
						VK_0,		CR_BACK,		VK_LEFT,		VK_NONE,				CR_MENU,	 }
	#elif (__PROJECT_8258_BLE_REMOTE__)
		#define		KB_MAP_NORMAL	{\
						VK_B,		CR_POWER,		VK_NONE,		VK_C,				CR_HOME,	 \
						VOICE,		VK_NONE,		VK_NONE,		CR_VOL_UP,			CR_VOL_DN,	 \
						VK_2,		VK_RIGHT,		CR_VOL_DN,		VK_3,				VK_1,	 \
						VK_5,		VK_ENTER,		CR_VOL_UP,		VK_6,				VK_4,	 \
						VK_8,		VK_DOWN,		VK_UP ,			VK_9,				VK_7,	 \
						VK_0,		CR_BACK,		VK_LEFT,		VOICE,		CR_MENU,	 }
	#endif
#endif  //end of REMOTE_IR_ENABLE

#if (__PROJECT_8258_BLE_REMOTE__)
//////////////////// KEY CONFIG (RCU board) ///////////////////////////
#define  KB_DRIVE_PINS  {GPIO_PD5, GPIO_PD2, GPIO_PD4, GPIO_PD6, GPIO_PD7}
#define  KB_SCAN_PINS   {GPIO_PC5, GPIO_PA0, GPIO_PB2, GPIO_PA4, GPIO_PA3, GPIO_PD3}

//drive pin as gpio
#define	PD5_FUNC				AS_GPIO
#define	PD2_FUNC				AS_GPIO
#define	PD4_FUNC				AS_GPIO
#define	PD6_FUNC				AS_GPIO
#define	PD7_FUNC				AS_GPIO
//drive pin need 100K pulldown
#define	PULL_WAKEUP_SRC_PD5		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PD2		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PD4		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PD6		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PD7		MATRIX_ROW_PULL
//drive pin open input to read gpio wakeup level
#define PD5_INPUT_ENABLE		1
#define PD2_INPUT_ENABLE		1
#define PD4_INPUT_ENABLE		1
#define PD6_INPUT_ENABLE		1
#define PD7_INPUT_ENABLE		1
//scan pin as gpio
#define	PC5_FUNC				AS_GPIO
#define	PA0_FUNC				AS_GPIO
#define	PB2_FUNC				AS_GPIO
#define	PA4_FUNC				AS_GPIO
#define	PA3_FUNC				AS_GPIO
#define	PD3_FUNC				AS_GPIO
//scan  pin need 10K pullup
#define	PULL_WAKEUP_SRC_PC5		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PA0		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PB2		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PA4		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PA3		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PD3		MATRIX_COL_PULL
//scan pin open input to read gpio level
#define PC5_INPUT_ENABLE		1
#define PA0_INPUT_ENABLE		1
#define PB2_INPUT_ENABLE		1
#define PA4_INPUT_ENABLE		1
#define PA3_INPUT_ENABLE		1
#define PD3_INPUT_ENABLE		1

#else
//////////////////// KEY CONFIG (RCU board) ///////////////////////////

#define  KB_DRIVE_PINS  {GPIO_PD5, GPIO_PD2, GPIO_PD4, GPIO_PD6, GPIO_PD7}  //GPIO_PD7
#define  KB_SCAN_PINS   {GPIO_PC5, GPIO_PA0, GPIO_PB2, GPIO_PA4, GPIO_PA3, GPIO_PD3} //GPIO_PA0

//drive pin as gpio
#define	PD5_FUNC				AS_GPIO
#define	PD2_FUNC				AS_GPIO
#define	PD4_FUNC				AS_GPIO
#define	PD6_FUNC				AS_GPIO
#define	PD7_FUNC				AS_GPIO

//drive pin need 100K pulldown
#define	PULL_WAKEUP_SRC_PD5		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PD2		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PD4		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PD6		MATRIX_ROW_PULL
#define	PULL_WAKEUP_SRC_PD7		MATRIX_ROW_PULL

//drive pin open input to read gpio wakeup level
#define PD5_INPUT_ENABLE		1
#define PD2_INPUT_ENABLE		1
#define PD4_INPUT_ENABLE		1
#define PD6_INPUT_ENABLE		1
#define PD7_INPUT_ENABLE		1

//scan pin as gpio
#define	PC5_FUNC				AS_GPIO
#define	PA0_FUNC				AS_GPIO
#define	PB2_FUNC				AS_GPIO
#define	PA4_FUNC				AS_GPIO
#define	PA3_FUNC				AS_GPIO
#define	PD3_FUNC				AS_GPIO

//scan  pin need 10K pullup
#define	PULL_WAKEUP_SRC_PC5		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PA0		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PB2		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PA4		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PA3		MATRIX_COL_PULL
#define	PULL_WAKEUP_SRC_PD3		MATRIX_COL_PULL

//scan pin open input to read gpio level
#define PC5_INPUT_ENABLE		1
#define PA0_INPUT_ENABLE		1
#define PB2_INPUT_ENABLE		1
#define PA4_INPUT_ENABLE		1
#define PA3_INPUT_ENABLE		1
#define PD3_INPUT_ENABLE		1

#endif

#if UNUSED_GPIO_PULLDOWN_ENABLE
	#define PULL_WAKEUP_SRC_PA5			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PA6			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB0			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB1			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB4			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB5			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB6			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB7			PM_PIN_PULLDOWN_100K
    

	#define PULL_WAKEUP_SRC_PD0			PM_PIN_PULLDOWN_100K    //note: A0 version, if enable pull down 100k, will make current leakage
	#define PULL_WAKEUP_SRC_PD1			PM_PIN_PULLDOWN_100K

#endif

#define		KB_MAP_NUM		KB_MAP_NORMAL
#define		KB_MAP_FN		KB_MAP_NORMAL

/////////////////// Clock  /////////////////////////////////
#define CLOCK_SYS_CLOCK_HZ  	16000000

enum{
	CLOCK_SYS_CLOCK_1S = CLOCK_SYS_CLOCK_HZ,
	CLOCK_SYS_CLOCK_1MS = (CLOCK_SYS_CLOCK_1S / 1000),
	CLOCK_SYS_CLOCK_1US = (CLOCK_SYS_CLOCK_1S / 1000000),
};



/////////////////// watchdog  //////////////////////////////
#define MODULE_WATCHDOG_ENABLE		0
#define WATCHDOG_INIT_TIMEOUT		500  //ms





///////////////////////////////////// ATT  HANDLER define ///////////////////////////////////////
typedef enum
{
	ATT_H_START = 0,


	//// Gap ////1-7
	/**********************************************************************************************/
	GenericAccess_PS_H, 					//UUID: 2800, 	VALUE: uuid 1800
	GenericAccess_DeviceName_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	GenericAccess_DeviceName_DP_H,			//UUID: 2A00,   VALUE: device name
	GenericAccess_Appearance_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read
	GenericAccess_Appearance_DP_H,			//UUID: 2A01,	VALUE: appearance
	CONN_PARAM_CD_H,						//UUID: 2803, 	VALUE:  			Prop: Read
	CONN_PARAM_DP_H,						//UUID: 2A04,   VALUE: connParameter


	//// gatt ////8-11
	/**********************************************************************************************/
	GenericAttribute_PS_H,					//UUID: 2800, 	VALUE: uuid 1801
	GenericAttribute_ServiceChanged_CD_H,	//UUID: 2803, 	VALUE:  			Prop: Indicate
	GenericAttribute_ServiceChanged_DP_H,   //UUID:	2A05,	VALUE: service change
	GenericAttribute_ServiceChanged_CCB_H,	//UUID: 2902,	VALUE: serviceChangeCCC


	//// device information ////12-14
	/**********************************************************************************************/
	DeviceInformation_PS_H,					//UUID: 2800, 	VALUE: uuid 180A
	DeviceInformation_pnpID_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read
	DeviceInformation_pnpID_DP_H,			//UUID: 2A50,	VALUE: PnPtrs


	//// HID ////15-41
	/**********************************************************************************************/
	HID_PS_H, 								//UUID: 2800, 	VALUE: uuid 1812

	//include
	HID_INCLUDE_H,							//UUID: 2802, 	VALUE: include

	//protocol
	HID_PROTOCOL_MODE_CD_H,					//UUID: 2803, 	VALUE:  			Prop: read | write_without_rsp
	HID_PROTOCOL_MODE_DP_H,					//UUID: 2A4E,	VALUE: protocolMode

	//boot keyboard input report
	HID_BOOT_KB_REPORT_INPUT_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	HID_BOOT_KB_REPORT_INPUT_DP_H,			//UUID: 2A22, 	VALUE: bootKeyInReport
	HID_BOOT_KB_REPORT_INPUT_CCB_H,			//UUID: 2902, 	VALUE: bootKeyInReportCCC

	//boot keyboard output report
	HID_BOOT_KB_REPORT_OUTPUT_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | write| write_without_rsp
	HID_BOOT_KB_REPORT_OUTPUT_DP_H,		    //UUID: 2A32, 	VALUE: bootKeyOutReport

	//consume report in
	HID_CONSUME_REPORT_INPUT_CD_H,			//24 UUID: 2803, 	VALUE:  			Prop: Read | Notify
	HID_CONSUME_REPORT_INPUT_DP_H,			//25 UUID: 2A4D, 	VALUE: reportConsumerIn
	HID_CONSUME_REPORT_INPUT_CCB_H,			//26 UUID: 2902, 	VALUE: reportConsumerInCCC
	HID_CONSUME_REPORT_INPUT_REF_H, 		//27 UUID: 2908    VALUE: REPORT_ID_CONSUMER, TYPE_INPUT

	//keyboard report in
	HID_NORMAL_KB_REPORT_INPUT_CD_H,		//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	HID_NORMAL_KB_REPORT_INPUT_DP_H,		//UUID: 2A4D, 	VALUE: reportKeyIn
	HID_NORMAL_KB_REPORT_INPUT_CCB_H,		//UUID: 2902, 	VALUE: reportKeyInInCCC
	HID_NORMAL_KB_REPORT_INPUT_REF_H, 		//UUID: 2908    VALUE: REPORT_ID_KEYBOARD, TYPE_INPUT

	//keyboard report out
	HID_NORMAL_KB_REPORT_OUTPUT_CD_H,		//UUID: 2803, 	VALUE:  			Prop: Read | write| write_without_rsp
	HID_NORMAL_KB_REPORT_OUTPUT_DP_H,  		//UUID: 2A4D, 	VALUE: reportKeyOut
	HID_NORMAL_KB_REPORT_OUTPUT_REF_H, 		//UUID: 2908    VALUE: REPORT_ID_KEYBOARD, TYPE_OUTPUT

	// report map
	HID_REPORT_MAP_CD_H,					//UUID: 2803, 	VALUE:  			Prop: Read
	HID_REPORT_MAP_DP_H,					//UUID: 2A4B, 	VALUE: reportKeyIn
	HID_REPORT_MAP_EXT_REF_H,				//UUID: 2907 	VALUE: extService

	//hid information
	HID_INFORMATION_CD_H,					//UUID: 2803, 	VALUE:  			Prop: read
	HID_INFORMATION_DP_H,					//UUID: 2A4A 	VALUE: hidInformation

	//control point
	HID_CONTROL_POINT_CD_H,					//UUID: 2803, 	VALUE:  			Prop: write_without_rsp
	HID_CONTROL_POINT_DP_H,					//UUID: 2A4C 	VALUE: controlPoint


	//// battery service ////42-45
	/**********************************************************************************************/
	BATT_PS_H, 								//UUID: 2800, 	VALUE: uuid 180f
	BATT_LEVEL_INPUT_CD_H,					//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	BATT_LEVEL_INPUT_DP_H,					//UUID: 2A19 	VALUE: batVal
	BATT_LEVEL_INPUT_CCB_H,					//UUID: 2902, 	VALUE: batValCCC


	//// Ota ////46-49
	/**********************************************************************************************/
	OTA_PS_H, 								//UUID: 2800, 	VALUE: telink ota service uuid
	OTA_CMD_OUT_CD_H,						//UUID: 2803, 	VALUE:  			Prop: read | write_without_rsp
	OTA_CMD_OUT_DP_H,						//UUID: telink ota uuid,  VALUE: otaData
	OTA_CMD_OUT_DESC_H,						//UUID: 2901, 	VALUE: otaName


#if(BLE_AUDIO_ENABLE)
	//// Audio ////
	/********************************************************************************************
	* GOOGLE AUDIO service 9:50-58
	********************************************************************************************/
	AUDIO_GOOGLE_PS_H ,    //50  UUID:
	AUDIO_GOOGLE_TX_CD_H,   //51  UUID:
	AUDIO_GOOGLE_TX_DP_H ,  //52  UUID:
	AUDIO_GOOGLE_RX_CD_H ,  //53  UUID:
	AUDIO_GOOGLE_RX_DP_H,   //54  UUID:
	AUDIO_GOOGLE_RX_CCC_H,  //55  UUID:
	AUDIO_GOOGLE_CTL_CD_H,  //56  UUID:
	AUDIO_GOOGLE_CTL_DP_H,  //57  UUID:
	AUDIO_GOOGLE_CTL_CCC_H, //58  UUID:
#endif

	ATT_END_H,

}ATT_HANDLE;




#define DEBUG_GPIO_ENABLE							0

#if(DEBUG_GPIO_ENABLE)
	//define debug GPIO here according to your hardware

	#define GPIO_CHN0							GPIO_PB4
	#define GPIO_CHN1							GPIO_PB5
	#define GPIO_CHN2							GPIO_PB6
	#define GPIO_CHN3							//GPIO_PC2  // PC2/PC3 may used for external crystal input
	#define GPIO_CHN4							//GPIO_PC3  // PC2/PC3 may used for external crystal input
	#define GPIO_CHN5							GPIO_PB0
	#define GPIO_CHN6							GPIO_PB1


	#define PB4_OUTPUT_ENABLE					1
	#define PB5_OUTPUT_ENABLE					1
	#define PB6_OUTPUT_ENABLE					1
	//#define PC2_OUTPUT_ENABLE					1
	//#define PC3_OUTPUT_ENABLE					1
	#define PB0_OUTPUT_ENABLE					1
	#define PB1_OUTPUT_ENABLE					1




	#define DBG_CHN0_LOW		gpio_write(GPIO_CHN0, 0)
	#define DBG_CHN0_HIGH		gpio_write(GPIO_CHN0, 1)
	#define DBG_CHN0_TOGGLE		gpio_toggle(GPIO_CHN0)
	#define DBG_CHN1_LOW		gpio_write(GPIO_CHN1, 0)
	#define DBG_CHN1_HIGH		gpio_write(GPIO_CHN1, 1)
	#define DBG_CHN1_TOGGLE		gpio_toggle(GPIO_CHN1)
	#define DBG_CHN2_LOW		gpio_write(GPIO_CHN2, 0)
	#define DBG_CHN2_HIGH		gpio_write(GPIO_CHN2, 1)
	#define DBG_CHN2_TOGGLE		gpio_toggle(GPIO_CHN2)
	#define DBG_CHN3_LOW		//gpio_write(GPIO_CHN3, 0)
	#define DBG_CHN3_HIGH		//gpio_write(GPIO_CHN3, 1)
	#define DBG_CHN3_TOGGLE		//gpio_toggle(GPIO_CHN3)
	#define DBG_CHN4_LOW		//gpio_write(GPIO_CHN4, 0)
	#define DBG_CHN4_HIGH		//gpio_write(GPIO_CHN4, 1)
	#define DBG_CHN4_TOGGLE		//gpio_toggle(GPIO_CHN4)
	#define DBG_CHN5_LOW		gpio_write(GPIO_CHN5, 0)
	#define DBG_CHN5_HIGH		gpio_write(GPIO_CHN5, 1)
	#define DBG_CHN5_TOGGLE		gpio_toggle(GPIO_CHN5)
	#define DBG_CHN6_LOW		gpio_write(GPIO_CHN6, 0)
	#define DBG_CHN6_HIGH		gpio_write(GPIO_CHN6, 1)
	#define DBG_CHN6_TOGGLE		gpio_toggle(GPIO_CHN6)
#else
	#define DBG_CHN0_LOW
	#define DBG_CHN0_HIGH
	#define DBG_CHN0_TOGGLE
	#define DBG_CHN1_LOW
	#define DBG_CHN1_HIGH
	#define DBG_CHN1_TOGGLE
	#define DBG_CHN2_LOW
	#define DBG_CHN2_HIGH
	#define DBG_CHN2_TOGGLE
	#define DBG_CHN3_LOW
	#define DBG_CHN3_HIGH
	#define DBG_CHN3_TOGGLE
	#define DBG_CHN4_LOW
	#define DBG_CHN4_HIGH
	#define DBG_CHN4_TOGGLE
	#define DBG_CHN5_LOW
	#define DBG_CHN5_HIGH
	#define DBG_CHN5_TOGGLE
	#define DBG_CHN6_LOW
	#define DBG_CHN6_HIGH
	#define DBG_CHN6_TOGGLE
	#define DBG_CHN7_LOW
	#define DBG_CHN7_HIGH
	#define DBG_CHN7_TOGGLE
#endif  //end of DEBUG_GPIO_ENABLE





#define BLE_PHYTEST_MODE						PHYTEST_MODE_DISABLE



///////////////////////////////////// Param  define ///////////////////////////////////////

#define     MY_APP_ADV_CHANNEL					BLT_ENABLE_ADV_ALL
#define 	MY_ADV_INTERVAL_MIN					ADV_INTERVAL_30MS
#define 	MY_ADV_INTERVAL_MAX					ADV_INTERVAL_35MS

#define     DEBUG           printf


#ifndef UART_PRINT_DEBUG_ENABLE
#define UART_PRINT_DEBUG_ENABLE  			0
#endif
/////////////////////////////////////// PRINT DEBUG INFO ///////////////////////////////////////
#if (UART_PRINT_DEBUG_ENABLE)
	//the baud rate should not bigger than 1M(system timer clock is constant 16M),1M_BAUD_MAX鈫�6M_SYS_CLOCK,2M_BAUD_MAX鈫�2M_SYS_CLOCK
	#define PRINT_BAUD_RATE             					1000000
	#define DEBUG_INFO_TX_PIN           					GPIO_PB7
	#define PULL_WAKEUP_SRC_PB7         					PM_PIN_PULLUP_10K
	#define PB7_OUTPUT_ENABLE         						1
	#define PB7_DATA_OUT                                    1 //must
#endif

#include "../common/default_config.h"

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
