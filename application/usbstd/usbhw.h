/******************************************************************************
 * @file     usbhw.h
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

enum {
    USB_EDP_PRINTER_IN = 8, // endpoint 8 is alias of enpoint 0,  becareful.  // default hw buf len = 64
    USB_EDP_MOUSE = 2,            // default hw buf len = 8
    USB_EDP_KEYBOARD_IN = 1,    // default hw buf len = 8
    USB_EDP_KEYBOARD_OUT = 3,    // default hw buf len = 16
    USB_EDP_AUDIO_IN = 4,        // default hw buf len = 64
    USB_EDP_PRINTER_OUT = 5,    // default hw buf len = 64
    USB_EDP_SPEAKER = 6,        // default hw buf len = 16
    USB_EDP_MIC = 7,            // default hw buf len = 16
#if(USB_GAMEPAD_ENABLE)
    USB_EDP_GAMEPAD_IN = 3,        // default hw buf len = 64
    USB_EDP_GAMEPAD_OUT = 5,        // default hw buf len = 64
#endif
    USB_EDP_MS_IN = USB_EDP_PRINTER_IN,        // mass storage
    USB_EDP_MS_OUT = USB_EDP_PRINTER_OUT,
    USB_EDP_SOMATIC_IN = USB_EDP_AUDIO_IN,        //  when USB_SOMATIC_ENABLE, USB_EDP_PRINTER_OUT disable
    USB_EDP_SOMATIC_OUT = USB_EDP_PRINTER_OUT,
    USB_EDP_CDC_IN = 4,
    USB_EDP_CDC_OUT = 5,
};

// #defined in the standard spec
enum {
    USB_HID_AUDIO           = 2,
    USB_HID_MOUSE           = 1,
    USB_HID_KB_MEDIA        = 3,// media
    USB_HID_KB_SYS          = 4,// system : power,sleep,wakeup
#if(USB_GAMEPAD_ENABLE)
    USB_HID_GAMEPAD            = 5,// somatic sensor,  may have many report ids
#endif
    USB_HID_SOMATIC            = 5,// somatic sensor,  may have many report ids
};

void usbhw_disable_manual_interrupt(int m);
void usbhw_enable_manual_interrupt(int m);
void usbhw_write_ep(u32 ep, u8 * data, int len);
void usbhw_write_ctrl_ep_u16(u16 v);
u16 usbhw_read_ctrl_ep_u16(void);

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif

