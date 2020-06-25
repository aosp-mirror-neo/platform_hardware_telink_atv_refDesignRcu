/********************************************************************************************************
 * @file     usbmouse_i.h
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

#include <application/app/usbmouse.h>

#include "../usbstd/usb.h"
#include "../usbstd/usbhw.h"
#include "../usbstd/usbhw_i.h"
#include "../../vendor/common/default_config.h"
/** HID class report descriptor. This is a special descriptor constructed with values from the
 *  USBIF HID class specification to describe the reports and capabilities of the HID device. This
 *  descriptor is parsed by the host and its contents used to determine what data (and in what encoding)
 *  the device will send, and what it may be sent back from the host. Refer to the HID specification for
 *  more details on HID report descriptors.
 */
/* debug note: */
/*
 0x85, 0x01, //Report ID (1) keyboard
 0x85, 0x02, //report ID 02 mouse
 ÊÇ·ñÐèÒªÅäÖÃÕâ¸ö , 1,2¶¨ÒåÊÇ·ñÕýÈ·
 */
static const USB_Descriptor_HIDReport_Datatype_t mouse_report_desc[] = {
#if 0
	//Ã¿ÐÐ¿ªÊ¼µÄµÚÒ»×Ö½ÚÎª¸ÃÌõÄ¿µÄÇ°×º£¬Ç°×ºµÄ¸ñÊ½Îª£º
	//D7~D4£ºbTag¡£D3~D2£ºbType£»D1~D0£ºbSize¡£ÒÔÏÂ·Ö±ð¶ÔÃ¿¸öÌõÄ¿×¢ÊÍ¡£

	//ÕâÊÇÒ»¸öÈ«¾Ö£¨bTypeÎª1£©ÌõÄ¿£¬Ñ¡ÔñÓÃÍ¾Ò³ÎªÆÕÍ¨×ÀÃæGeneric Desktop Page(0x01)
	//ºóÃæ¸úÒ»×Ö½ÚÊý¾Ý£¨bSizeÎª1£©£¬ºóÃæµÄ×Ö½ÚÊý¾Í²»×¢ÊÍÁË£¬
	//×Ô¼º¸ù¾ÝbSizeÀ´ÅÐ¶Ï¡£
	HID_RPT_USAGE_PAGE(8, 0x01), /* Generic Desktop */

	//ÕâÊÇÒ»¸ö¾Ö²¿£¨bTypeÎª2£©ÌõÄ¿£¬ËµÃ÷½ÓÏÂÀ´µÄÓ¦ÓÃ¼¯ºÏÓÃÍ¾ÓÃÓÚÊó±ê
    HID_RPT_USAGE(8, 0x02)		, /* Mouse */

	//ÕâÊÇÒ»¸öÖ÷ÌõÄ¿£¨bTypeÎª0£©ÌõÄ¿£¬¿ª¼¯ºÏ£¬ºóÃæ¸úµÄÊý¾Ý0x01±íÊ¾
	//¸Ã¼¯ºÏÊÇÒ»¸öÓ¦ÓÃ¼¯ºÏ¡£ËüµÄÐÔÖÊÔÚÇ°ÃæÓÉÓÃÍ¾Ò³ºÍÓÃÍ¾¶¨ÒåÎª
	//ÆÕÍ¨×ÀÃæÓÃµÄÊó±ê¡£
    HID_RPT_COLLECTION(8, 0x01)		, /* Application */

    HID_RPT_REPORT_ID(8, USB_HID_MOUSE)		, /*Report ID*/

    HID_RPT_USAGE_PAGE(8, 0x09)		, /* Button */

	//ÕâÊÇÒ»¸ö¾Ö²¿ÌõÄ¿£¬ËµÃ÷ÓÃÍ¾µÄ×îÐ¡ÖµÎª1¡£Êµ¼ÊÉÏÊÇÊó±ê×ó¼ü¡£
	// 1 is mouse left button,2 is mouse right button,3 is central buuton
    HID_RPT_USAGE_MINIMUM(8, 0x01)		,
	//ÕâÊÇÒ»¸ö¾Ö²¿ÌõÄ¿£¬ËµÃ÷ÓÃÍ¾µÄ×î´óÖµ
    HID_RPT_USAGE_MAXIMUM(8, 0x05),

	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷·µ»ØµÄÊý¾ÝµÄÂß¼­Öµ£¨¾ÍÊÇÎÒÃÇ·µ»ØµÄÊý¾ÝÓòµÄÖµÀ²£©
	//×îÐ¡Îª0¡£ÒòÎªÎÒÃÇÕâÀïÓÃBitÀ´±íÊ¾Ò»¸öÊý¾ÝÓò£¬Òò´Ë×îÐ¡Îª0£¬×î´óÎª1¡£
    HID_RPT_LOGICAL_MINIMUM(8, 0x00)		,
	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷Âß¼­Öµ×î´óÎª1¡£
    HID_RPT_LOGICAL_MAXIMUM(8, 0x01),

	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷Ã¿¸öÊý¾ÝÓòµÄ³¤¶ÈÎª1¸öbit¡£
	HID_RPT_REPORT_SIZE(8, 0x01),
	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷total button ÊýÁ¿Îª5¸ö¡£
    HID_RPT_REPORT_COUNT(8, 0x05), /* debug note: 3->5*/

	//ÕâÊÇÒ»¸öÖ÷ÌõÄ¿£¬ËµÃ÷ÓÐ3¸ö³¤¶ÈÎª1bitµÄÊý¾ÝÓò£¨ÊýÁ¿ºÍ³¤¶È
	//ÓÉÇ°ÃæµÄÁ½¸öÈ«¾ÖÌõÄ¿Ëù¶¨Òå£©ÓÃÀ´×öÎªÊäÈë£¬
	//ÊôÐÔÎª£ºData,Var,Abs¡£Data±íÊ¾ÕâÐ©Êý¾Ý¿ÉÒÔ±ä¶¯£¬Var±íÊ¾
	//ÕâÐ©Êý¾ÝÓòÊÇ¶ÀÁ¢µÄ£¬Ã¿¸öÓò±íÊ¾Ò»¸öÒâË¼¡£Abs±íÊ¾¾ø¶ÔÖµ¡£
	//ÕâÑù¶¨ÒåµÄ½á¹û¾ÍÊÇ£¬µÚÒ»¸öÊý¾ÝÓòbit0±íÊ¾°´¼ü1£¨×ó¼ü£©ÊÇ·ñ°´ÏÂ£¬
	//µÚ¶þ¸öÊý¾ÝÓòbit1±íÊ¾°´¼ü2£¨ÓÒ¼ü£©ÊÇ·ñ°´ÏÂ£¬µÚÈý¸öÊý¾ÝÓòbit2±íÊ¾
	//°´¼ü3£¨ÖÐ¼ü£©ÊÇ·ñ°´ÏÂ¡£
    HID_RPT_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷Ã¿¸öÊý¾ÝÓòµÄ³¤¶ÈÎª
	HID_RPT_REPORT_SIZE(8, 0x03), /* debug note: 5->3*/
	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷Êý¾ÝÓòÊýÁ¿Îª1¸ö
    HID_RPT_REPORT_COUNT(8, 0x01),

	//ÕâÊÇÒ»¸öÖ÷ÌõÄ¿£¬ÊäÈëÓÃ£¬ÓÉÇ°ÃæÁ½¸öÈ«¾ÖÌõÄ¿¿ÉÖª£¬³¤¶ÈÎª3bit£¬
	//ÊýÁ¿Îª1¸ö¡£ËüµÄÊôÐÔÎª³£Á¿£¨¼´·µ»ØµÄÊý¾ÝÒ»Ö±ÊÇ0£©¡£
	//Õâ¸öÖ»ÊÇÎªÁË´ÕÆëÒ»¸ö×Ö½Ú£¨Ç°ÃæÓÃÁË3¸öbit£©¶øÌî³äµÄÒ»Ð©Êý¾Ý
	//¶øÒÑ£¬ËùÒÔËüÊÇÃ»ÓÐÊµ¼ÊÓÃÍ¾µÄ¡£
    HID_RPT_INPUT(8, HID_IOF_CONSTANT),

	//ÕâÊÇÒ»¸ö¾Ö²¿ÌõÄ¿¡£ËµÃ÷ÓÃÍ¾ÎªÖ¸Õë¼¯ºÏ
    HID_RPT_USAGE(8, 0x01), /* Pointer */

	//ÕâÊÇÒ»¸öÖ÷ÌõÄ¿£¬¿ª¼¯ºÏ£¬ºóÃæ¸úµÄÊý¾Ý0x00±íÊ¾¸Ã¼¯ºÏÊÇÒ»¸ö
	//ÎïÀí¼¯ºÏ£¬ÓÃÍ¾ÓÉÇ°ÃæµÄ¾Ö²¿ÌõÄ¿¶¨ÒåÎªÖ¸Õë¼¯ºÏ¡£
    HID_RPT_COLLECTION(8, 0x00), /* Physical */

	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬Ñ¡ÔñÓÃÍ¾Ò³ÎªÆÕÍ¨×ÀÃæGeneric Desktop Page(0x01)
    HID_RPT_USAGE_PAGE(8, 0x01),  /* Generic Desktop */

	//ÕâÊÇÒ»¸ö¾Ö²¿ÌõÄ¿£¬ËµÃ÷ÓÃÍ¾ÎªXÖá
    HID_RPT_USAGE(8, 0x30), /* Usage X */

	//ÕâÊÇÒ»¸ö¾Ö²¿ÌõÄ¿£¬ËµÃ÷ÓÃÍ¾ÎªYÖá
    HID_RPT_USAGE(8, 0x31), /* Usage Y */

	//ÏÂÃæÁ½¸öÎªÈ«¾ÖÌõÄ¿£¬ËµÃ÷·µ»ØµÄÂß¼­×îÐ¡ºÍ×î´óÖµ¡£
	//ÒòÎªÊó±êÖ¸ÕëÒÆ¶¯Ê±£¬Í¨³£ÊÇÓÃÏà¶ÔÖµÀ´±íÊ¾µÄ£¬
	//Ïà¶ÔÖµµÄÒâË¼¾ÍÊÇ£¬µ±Ö¸ÕëÒÆ¶¯Ê±£¬Ö»·¢ËÍÒÆ¶¯Á¿¡£
	//ÍùÓÒÒÆ¶¯Ê±£¬XÖµÎªÕý£»ÍùÏÂÒÆ¶¯Ê±£¬YÖµÎªÕý¡£
	//¶ÔÓÚ¹öÂÖ£¬µ±¹öÂÖÍùÉÏ¹öÊ±£¬ÖµÎªÕý¡£
    HID_RPT_LOGICAL_MINIMUM(8, 0x81), //     LOGICAL_MINIMUM (-127)
    HID_RPT_LOGICAL_MAXIMUM(8, 0x7f), //     LOGICAL_MAXIMUM (127)
	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷Êý¾ÝÓòµÄ³¤¶È¡£Èç¹ûÊ¹ÓÃ 16£¬¾Í¿ÉÒÔ±íÊ¾¾ø¶Ô×ø±ê
    HID_RPT_REPORT_SIZE(8, 0x08),
    HID_RPT_REPORT_COUNT(8, 0x02),
	//ÕâÊÇÒ»¸öÖ÷ÌõÄ¿¡£ËüËµÃ÷ÕâÈý¸ö8bitµÄÊý¾ÝÓòÊÇÊäÈëÓÃµÄ£¬
	//ÊôÐÔÎª£ºData,Var,Rel¡£DataËµÃ÷Êý¾ÝÊÇ¿ÉÒÔ±äµÄ£¬VarËµÃ÷
	//ÕâÐ©Êý¾ÝÓòÊÇ¶ÀÁ¢µÄ£¬¼´µÚÒ»¸ö8bit±íÊ¾XÖá£¬µÚ¶þ¸ö8bit±íÊ¾
	//YÖá£¬µÚÈý¸ö8bit±íÊ¾¹öÂÖ¡£Rel±íÊ¾ÕâÐ©ÖµÊÇÏà¶ÔÖµ¡£
    HID_RPT_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_RELATIVE),

    HID_RPT_USAGE(8, 0x38),  /* Usage Wheel */
    HID_RPT_LOGICAL_MINIMUM(8, 0x81),	 //LOGICAL_MINIMUM (-127)
    HID_RPT_LOGICAL_MAXIMUM(8, 0x7f),	 //LOGICAL_MAXIMUM (127)
    HID_RPT_REPORT_SIZE(8, 0x08),		 //REPORT_SIZE (8)
    HID_RPT_REPORT_COUNT(8, 0x01),		//REPORT_COUNT (1)
    HID_RPT_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_RELATIVE),

    HID_RPT_END_COLLECTION(0),
    HID_RPT_END_COLLECTION(0),
        
    HID_RPT_USAGE_PAGE(8, 0x0c),             //global, usage page (follow 1 bytes) consumer page
    HID_RPT_USAGE(8, 0x01),                  //local,  usage ID 01  Consumer Control
    HID_RPT_COLLECTION(8, 0x01),             //main, collection
    HID_RPT_REPORT_ID(8, USB_HID_KB_MEDIA),  //Report ID
    HID_RPT_REPORT_SIZE(8, 0x10),            //global, report size 16 bits
    HID_RPT_REPORT_COUNT(8, 0x02),           //global, report count 2
    HID_RPT_LOGICAL_MINIMUM(8, 0x01),        //global, min  0x01
    HID_RPT_LOGICAL_MAXIMUM(16, 0x02ff),     //global, max  0x028c
    HID_RPT_USAGE_MINIMUM(8, 0x01),          //local, min   0x01
    HID_RPT_USAGE_MAXIMUM(16, 0x02ff),       //local, max    0x28c
    HID_RPT_INPUT(8, HID_IOF_ABSOLUTE),      //main,  input data variable, absolute
    HID_RPT_END_COLLECTION(0),               //main, end collection

	HID_RPT_USAGE_PAGE(8, 0x01),             //gobal,  USAGE_PAGE 1 (Generic Desktop)
	HID_RPT_USAGE(8, 0x80),                  //local, usage ID 0x80 system control
	HID_RPT_COLLECTION(8, 0x01),             //main conllection
    HID_RPT_REPORT_ID(8, USB_HID_KB_SYS),    //Report ID
	HID_RPT_REPORT_SIZE(8, 0x01),            //global, report size 2
	HID_RPT_REPORT_COUNT(8, 0x03),           //report count  1
	HID_RPT_LOGICAL_MINIMUM(8, 0x00),        //global min 01
    HID_RPT_LOGICAL_MAXIMUM(8, 0x01),        //gobal, max 3
	HID_RPT_USAGE(8, 0x81),					//local usage ID 0x81 system power down
    HID_RPT_USAGE(8, 0x82),                  //local usage ID 0x82 system sleep
    HID_RPT_USAGE(8, 0x83),                  //local usage ID 0x83 system wakeup
    HID_RPT_INPUT(8, HID_IOF_ABSOLUTE|HID_IOF_NO_PREFERRED_STATE|HID_IOF_NULLSTATE),
    HID_RPT_REPORT_SIZE(8, 0x05),            //global report size 6
	HID_RPT_REPORT_COUNT(8, 0x01),			//report count	1
    HID_RPT_INPUT(8, HID_IOF_CONSTANT|HID_IOF_VARIABLE),
    HID_RPT_END_COLLECTION(0),               //end of collection

#else

	//ÕâÊÇÒ»¸öÈ«¾Ö£¨bTypeÎª1£©ÌõÄ¿£¬Ñ¡ÔñÓÃÍ¾Ò³ÎªÆÕÍ¨×ÀÃæGeneric Desktop Page(0x01)
	//ºóÃæ¸úÒ»×Ö½ÚÊý¾Ý£¨bSizeÎª1£©£¬ºóÃæµÄ×Ö½ÚÊý¾Í²»×¢ÊÍÁË£¬
	//×Ô¼º¸ù¾ÝbSizeÀ´ÅÐ¶Ï¡£
	0x05, 0x01, // USAGE_PAGE (Generic Desktop)

	//ÕâÊÇÒ»¸ö¾Ö²¿£¨bTypeÎª2£©ÌõÄ¿£¬ËµÃ÷½ÓÏÂÀ´µÄÓ¦ÓÃ¼¯ºÏÓÃÍ¾ÓÃÓÚÊó±ê
	0x09, 0x02, // USAGE (Mouse)

	//ÕâÊÇÒ»¸öÖ÷ÌõÄ¿£¨bTypeÎª0£©ÌõÄ¿£¬¿ª¼¯ºÏ£¬ºóÃæ¸úµÄÊý¾Ý0x01±íÊ¾
	//¸Ã¼¯ºÏÊÇÒ»¸öÓ¦ÓÃ¼¯ºÏ¡£ËüµÄÐÔÖÊÔÚÇ°ÃæÓÉÓÃÍ¾Ò³ºÍÓÃÍ¾¶¨ÒåÎª
	//ÆÕÍ¨×ÀÃæÓÃµÄÊó±ê¡£
	0xa1, 0x01, // COLLECTION (Application)

	0x85, USB_HID_MOUSE, //report ID 01

	//ÕâÊÇÒ»¸ö¾Ö²¿ÌõÄ¿¡£ËµÃ÷ÓÃÍ¾ÎªÖ¸Õë¼¯ºÏ
	0x09, 0x01, //   USAGE (Pointer)

	//ÕâÊÇÒ»¸öÖ÷ÌõÄ¿£¬¿ª¼¯ºÏ£¬ºóÃæ¸úµÄÊý¾Ý0x00±íÊ¾¸Ã¼¯ºÏÊÇÒ»¸ö
	//ÎïÀí¼¯ºÏ£¬ÓÃÍ¾ÓÉÇ°ÃæµÄ¾Ö²¿ÌõÄ¿¶¨ÒåÎªÖ¸Õë¼¯ºÏ¡£
	0xa1, 0x00, //   COLLECTION (Physical)


	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬Ñ¡ÔñÓÃÍ¾Ò³Îª°´¼ü£¨Button Page(0x09)£©
	0x05, 0x09, //     USAGE_PAGE (Button)

	//ÕâÊÇÒ»¸ö¾Ö²¿ÌõÄ¿£¬ËµÃ÷ÓÃÍ¾µÄ×îÐ¡ÖµÎª1¡£Êµ¼ÊÉÏÊÇÊó±ê×ó¼ü¡£
	// 1 is mouse left button,2 is mouse right button,3 is central buuton
	0x19, 0x01, //     USAGE_MINIMUM (Button 1)

	//ÕâÊÇÒ»¸ö¾Ö²¿ÌõÄ¿£¬ËµÃ÷ÓÃÍ¾µÄ×î´óÖµÎª3¡£Êµ¼ÊÉÏÊÇÊó±êÖÐ¼ü¡£
	0x29, 0x05, //     USAGE_MAXIMUM (Button 5)

	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷·µ»ØµÄÊý¾ÝµÄÂß¼­Öµ£¨¾ÍÊÇÎÒÃÇ·µ»ØµÄÊý¾ÝÓòµÄÖµÀ²£©
	//×îÐ¡Îª0¡£ÒòÎªÎÒÃÇÕâÀïÓÃBitÀ´±íÊ¾Ò»¸öÊý¾ÝÓò£¬Òò´Ë×îÐ¡Îª0£¬×î´óÎª1¡£
	0x15, 0x00, //     LOGICAL_MINIMUM (0)

	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷Âß¼­Öµ×î´óÎª1¡£
	0x25, 0x01, //     LOGICAL_MAXIMUM (1)

	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷total button ÊýÁ¿Îª5¸ö¡£
	0x95, 0x05, //     REPORT_COUNT (3)
	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷Ã¿¸öÊý¾ÝÓòµÄ³¤¶ÈÎª1¸öbit¡£
	0x75, 0x01, //     REPORT_SIZE (1)

	//ÕâÊÇÒ»¸öÖ÷ÌõÄ¿£¬ËµÃ÷ÓÐ5¸ö³¤¶ÈÎª1bitµÄÊý¾ÝÓò£¨ÊýÁ¿ºÍ³¤¶È
	//ÓÉÇ°ÃæµÄÁ½¸öÈ«¾ÖÌõÄ¿Ëù¶¨Òå£©ÓÃÀ´×öÎªÊäÈë£¬
	//ÊôÐÔÎª£ºData,Var,Abs¡£Data±íÊ¾ÕâÐ©Êý¾Ý¿ÉÒÔ±ä¶¯£¬Var±íÊ¾
	//ÕâÐ©Êý¾ÝÓòÊÇ¶ÀÁ¢µÄ£¬Ã¿¸öÓò±íÊ¾Ò»¸öÒâË¼¡£Abs±íÊ¾¾ø¶ÔÖµ¡£
	//ÕâÑù¶¨ÒåµÄ½á¹û¾ÍÊÇ£¬µÚÒ»¸öÊý¾ÝÓòbit0±íÊ¾°´¼ü1£¨×ó¼ü£©ÊÇ·ñ°´ÏÂ£¬
	//µÚ¶þ¸öÊý¾ÝÓòbit1±íÊ¾°´¼ü2£¨ÓÒ¼ü£©ÊÇ·ñ°´ÏÂ£¬µÚÈý¸öÊý¾ÝÓòbit2±íÊ¾
	//°´¼ü3£¨ÖÐ¼ü£©ÊÇ·ñ°´ÏÂ¡£
	0x81, 0x02, //     INPUT (Data,Var,Abs)

	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷Êý¾ÝÓòÊýÁ¿Îª1¸ö
	0x95, 0x01, //     REPORT_COUNT (1)
	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷Ã¿¸öÊý¾ÝÓòµÄ³¤¶ÈÎª3bit¡£
	0x75, 0x03, //     REPORT_SIZE (3)


	//ÕâÊÇÒ»¸öÖ÷ÌõÄ¿£¬ÊäÈëÓÃ£¬ÓÉÇ°ÃæÁ½¸öÈ«¾ÖÌõÄ¿¿ÉÖª£¬³¤¶ÈÎª3bit£¬
	//ÊýÁ¿Îª1¸ö¡£ËüµÄÊôÐÔÎª³£Á¿£¨¼´·µ»ØµÄÊý¾ÝÒ»Ö±ÊÇ0£©¡£
	//Õâ¸öÖ»ÊÇÎªÁË´ÕÆëÒ»¸ö×Ö½Ú£¨Ç°ÃæÓÃÁË3¸öbit£©¶øÌî³äµÄÒ»Ð©Êý¾Ý
	//¶øÒÑ£¬ËùÒÔËüÊÇÃ»ÓÐÊµ¼ÊÓÃÍ¾µÄ¡£
	0x81, 0x01, //     INPUT (Cnst,Var,Abs)


	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬Ñ¡ÔñÓÃÍ¾Ò³ÎªÆÕÍ¨×ÀÃæGeneric Desktop Page(0x01)
	0x05, 0x01, //     USAGE_PAGE (Generic Desktop)

	//ÕâÊÇÒ»¸ö¾Ö²¿ÌõÄ¿£¬ËµÃ÷ÓÃÍ¾ÎªXÖá
	0x09, 0x30, //     USAGE (X)

	//ÕâÊÇÒ»¸ö¾Ö²¿ÌõÄ¿£¬ËµÃ÷ÓÃÍ¾ÎªYÖá
	0x09, 0x31, //     USAGE (Y)
	//ÏÂÃæÁ½¸öÎªÈ«¾ÖÌõÄ¿£¬ËµÃ÷·µ»ØµÄÂß¼­×îÐ¡ºÍ×î´óÖµ¡£
	//ÒòÎªÊó±êÖ¸ÕëÒÆ¶¯Ê±£¬Í¨³£ÊÇÓÃÏà¶ÔÖµÀ´±íÊ¾µÄ£¬
	//Ïà¶ÔÖµµÄÒâË¼¾ÍÊÇ£¬µ±Ö¸ÕëÒÆ¶¯Ê±£¬Ö»·¢ËÍÒÆ¶¯Á¿¡£
	//ÍùÓÒÒÆ¶¯Ê±£¬XÖµÎªÕý£»ÍùÏÂÒÆ¶¯Ê±£¬YÖµÎªÕý¡£
	//¶ÔÓÚ¹öÂÖ£¬µ±¹öÂÖÍùÉÏ¹öÊ±£¬ÖµÎªÕý¡£
	0x15, 0x81, //     LOGICAL_MINIMUM (-127)
	0x25, 0x7f, //     LOGICAL_MAXIMUM (127)

	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷Êý¾ÝÓòµÄ³¤¶ÈÎª8bit¡£
	0x75, 0x08, //     REPORT_SIZE (16)

	//ÕâÊÇÒ»¸öÈ«¾ÖÌõÄ¿£¬ËµÃ÷Êý¾ÝÓòµÄ¸öÊýÎª2¸ö¡£
	0x95, 0x02, //     REPORT_COUNT (2)

	//ÕâÊÇÒ»¸öÖ÷ÌõÄ¿¡£ËüËµÃ÷ÕâÈý¸ö8bitµÄÊý¾ÝÓòÊÇÊäÈëÓÃµÄ£¬
	//ÊôÐÔÎª£ºData,Var,Rel¡£DataËµÃ÷Êý¾ÝÊÇ¿ÉÒÔ±äµÄ£¬VarËµÃ÷
	//ÕâÐ©Êý¾ÝÓòÊÇ¶ÀÁ¢µÄ£¬¼´µÚÒ»¸ö8bit±íÊ¾XÖá£¬µÚ¶þ¸ö8bit±íÊ¾
	//YÖá£¬µÚÈý¸ö8bit±íÊ¾¹öÂÖ¡£Rel±íÊ¾ÕâÐ©ÖµÊÇÏà¶ÔÖµ¡£
	0x81, 0x06, //     INPUT (Data,Var,Rel)

	//ÕâÊÇÒ»¸ö¾Ö²¿ÌõÄ¿£¬ËµÃ÷ÓÃÍ¾Îª¹öÂÖ
	0x09, 0x38, //     USAGE (Wheel)
	0x15, 0x81, //LOGICAL_MINIMUM (-127)
	0x25, 0x7f, //LOGICAL_MAXIMUM (127)
	0x75, 0x08, //REPORT_SIZE (16)
	0x95, 0x01, //REPORT_COUNT (1)
	0x81, 0x06, //INPUT (Data,Var,Rel)


	//ÏÂÃæÕâÁ½¸öÖ÷ÌõÄ¿ÓÃÀ´¹Ø±ÕÇ°ÃæµÄ¼¯ºÏÓÃ¡£
	//ÎÒÃÇ¿ªÁËÁ½¸ö¼¯ºÏ£¬ËùÒÔÒª¹ØÁ½´Î¡£bSizeÎª0£¬ËùÒÔºóÃæÃ»Êý¾Ý¡£
	0xc0, //   END_COLLECTION
	0xc0, // END_COLLECTION

#if (ONEKEY_WEB==0)
	// begin of media key
	0x05,0x0c,     //global, usage page (follow 1 bytes) consumer page
	0x09,0x01,     //local,  usage ID 01  Consumer Control
	0xA1,0x01,     //main, collection
	0x85,USB_HID_KB_MEDIA,     //global, report ID 0x03
	0x75,0x10,     //global, report size 16 bits
	0x95,0x02,     //global, report count 2
	0x15,0x01,     //global, min  0x01
	0x26,0x9c,0x02,  //global, max  0x29c
#if CHIP_EOP_ERROR
	0x19,0x01,     //local, min   0x01
	0x2a,0xff,0x02,  //local, max    0x2ff
#else
	0x19,0x01,     //local, min   0x01
	0x2a,0x8c,0x02,  //local, max    0x28c
#endif
	0x81,0x00,     //main,  input data variable, absolute
	0xc0,        //main, end collection

	0x05,0x01,     //gobal,  USAGE_PAGE 1 (Generic Desktop)
	0x09,0x80,     //local, usage ID 0x80 system control
	0xa1,0x01,     //main conllection
	0x85,USB_HID_KB_SYS,     //global report ID 0x4
	0x75,0x02,     //global, report size 2
	0x95,0x01,     //report count  1
	0x15,0x01,     //global min 01
	0x25,0x03,     //gobal, max 3
	0x09,0x82,     //local usage ID 0x82 system sleep
	0x09,0x81,     //local usage ID 0x81 system power down
	0x09,0x83,     //local usage ID 0x83 system wakeup
	0x81,0x60,     //main, input data, var, abs, No Prefer, NULL state
	0x75,0x06,     //global report size 6
	0x81,0x03,     //main input, constant, array
	0xc0,        //end of collection
	// end of media key
#endif
#endif
    //need Jensen's help: report ID 5
    HID_RPT_USAGE_PAGE(8, 0x01),     //global,  USAGE_PAGE 1 (Generic Desktop)
	0x09,0x00,     //usage undefined
	0xa1,0x01,     //main collection
	0x85,0x05,     //global report ID 0x5
	0x06,0x00,0xff,  //global usage page
	0x09,0x01,     //local,  usage ID 01  Consumer Control
	0x15,0x81,     //global min 81
	0x25,0x7f,     //global, max 7f
	0x75,0x08,     //global, report size 8
	0x95,0x07,     //report count  7
	0xb1,0x02,     //feature (data, var, abs)
	HID_RPT_END_COLLECTION(0),         //main, end collection
};

static inline u8* usbmouse_get_report_desc(void) {
	return (u8*) (mouse_report_desc);
}

static inline u16 usbmouse_get_report_desc_size(void) {
	return sizeof(mouse_report_desc);
}

