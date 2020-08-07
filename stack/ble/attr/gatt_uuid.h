/******************************************************************************
 * @file     gatt_uuid.h
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
#ifndef GATT_UUID_H_
#define GATT_UUID_H_

/** @addtogroup  GATT_Common_Module GATT UUID
 *  @{
 */


/** @addtogroup GATT_UUID GATT 16 bit UUID definition
 * @{
 */
#define GATT_UUID_PRIMARY_SERVICE        0x2800     //!< Primary Service
#define GATT_UUID_SECONDARY_SERVICE      0x2801     //!< Secondary Service
#define GATT_UUID_INCLUDE                0x2802     //!< Include
#define GATT_UUID_CHARACTER              0x2803     //!< Characteristic
#define GATT_UUID_CHAR_EXT_PROPS         0x2900     //!< Characteristic Extended Properties
#define GATT_UUID_CHAR_USER_DESC         0x2901     //!< Characteristic User Description
#define GATT_UUID_CLIENT_CHAR_CFG        0x2902     //!< Client Characteristic Configuration
#define GATT_UUID_SERVER_CHAR_CFG        0x2903     //!< Server Characteristic Configuration
#define GATT_UUID_CHAR_PRESENT_FORMAT    0x2904     //!< Characteristic Present Format
#define GATT_UUID_CHAR_AGG_FORMAT        0x2905     //!< Characteristic Aggregate Format
#define GATT_UUID_VALID_RANGE            0x2906     //!< Valid Range
#define GATT_UUID_EXT_REPORT_REF         0x2907     //!< External Report Reference
#define GATT_UUID_REPORT_REF             0x2908     //!< Report Reference

#define GATT_UUID_DEVICE_NAME            0x2a00     //!< Report Reference
#define GATT_UUID_APPEARANCE             0x2a01
#define GATT_UUID_PERI_CONN_PARAM          0x2a04
#define GATT_UUID_SERVICE_CHANGE         0x2a05
#define    GATT_UUID_BATTERY_LEVEL             0x2A19
#define    GATT_UUID_FIRMWARE_VER             0x2a26        //!<Firmware Version
/** @} end of group GATT_UUID */


///////////////////////////
// Service UUID
///////////////////////////
/**
 *  @brief  Definition for Services UUID
 */
#define SERVICE_UUID_ALERT_NOTIFICATION                           0x1811
#define SERVICE_UUID_BATTERY                                      0x180F
#define SERVICE_UUID_BLOOD_PRESSURE                               0x1810
#define SERVICE_UUID_CURRENT_TIME                                 0x1805
#define SERVICE_UUID_CYCLING_POWER                                0x1818
#define SERVICE_UUID_CYCLING_SPEED_AND_CADENCE                    0x1816
#define SERVICE_UUID_DEVICE_INFORMATION                           0x180A
#define SERVICE_UUID_GENERIC_ACCESS                               0x1800
#define SERVICE_UUID_GENERIC_ATTRIBUTE                            0x1801
#define SERVICE_UUID_GLUCOSE                                      0x1808
#define SERVICE_UUID_HEALTH_THERMOMETER                           0x1809
#define SERVICE_UUID_HEART_RATE                                   0x180D
#define SERVICE_UUID_HUMAN_INTERFACE_DEVICE                       0x1812
#define SERVICE_UUID_IMMEDIATE_ALERT                              0x1802
#define SERVICE_UUID_LINK_LOSS                                    0x1803
#define SERVICE_UUID_LOCATION_AND_NAVIGATION                      0x1819
#define SERVICE_UUID_NEXT_DST_CHANGE                              0x1807
#define SERVICE_UUID_PHONE_ALERT_STATUS                           0x180E
#define SERVICE_UUID_REFERENCE_TIME_UPDATE                        0x1806
#define SERVICE_UUID_RUNNING_SPEED_AND_CADENCE                    0x1814
#define SERVICE_UUID_SCAN_PARAMETER                               0x1813
#define SERVICE_UUID_TX_POWER                                     0x1804
#define SERVICE_UUID_USER_DATA                                    0x181C
#define SERVICE_UUID_CONTINUOUS_GLUCOSE_MONITORING                0x181F
#define SERVICE_UUID_WEIGHT_SCALE                                 0x181D

////////////////////////////////////
// Telink Service
////////////////////////////////////
#define TELINK_SPP_UUID_SERVICE               {0x10,0x19,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00}        //!< TELINK_SPP service
#define TELINK_AUDIO_UUID_SERVICE             {0x11,0x19,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00}        //!< TELINK_SPP service
#define TELINK_OTA_UUID_SERVICE               {0x12,0x19,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00}        //!< TELINK_OTA service
#define TELINK_MESH_UUID_SERVICE              {0x20,0x19,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00}        //!< TELINK_SPP service
#define TELINK_MESH_LT_UUID_SERVICE           {0x21,0x19,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00}        //!< TELINK_SPP service

#define TELINK_SPP_DATA_SERVER2CLIENT         {0x10,0x2B,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00}         //!< TELINK_SPP data from server to client
#define TELINK_SPP_DATA_CLIENT2SERVER         {0x11,0x2B,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00}         //!< TELINK_SPP data from client to server
#define TELINK_SPP_DATA_OTA                    0x12,0x2B,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00         //!< TELINK_SPP data for ota
#define TELINK_SPP_DATA_PAIR                  {0x13,0x2B,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00}         //!< TELINK_SPP data for ota
#define TELINK_SPP_DATA_DEFINE                {0x14,0x2B,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00}         //!< TELINK_SPP data for userdefine data


#define TELINK_MIC_DATA                       {0x18,0x2B,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00}         //!< TELINK_SPP data from server to client
#define TELINK_SPEAKER_DATA                   {0x19,0x2B,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00}         //!< TELINK_SPP data from client to server


#define AUDIO_GOOGLE_SERVICE_UUID              0x64,0xB6,0x17,0xF6,0x01,0xAF,0x7D,0xBC,0x05,0x4F,0x21,0x5A,0x01,0x00,0x5E,0xAB
#define AUDIO_GOOGL_TX_CHAR_UUID               0x64,0xB6,0x17,0xF6,0x01,0xAF,0x7D,0xBC,0x05,0x4F,0x21,0x5A,0x02,0x00,0x5E,0xAB
#define AUDIO_GOOGL_RX_CHAR_UUID               0x64,0xB6,0x17,0xF6,0x01,0xAF,0x7D,0xBC,0x05,0x4F,0x21,0x5A,0x03,0x00,0x5E,0xAB
#define AUDIO_GOOGL_CTL_CHAR_UUID              0x64,0xB6,0x17,0xF6,0x01,0xAF,0x7D,0xBC,0x05,0x4F,0x21,0x5A,0x04,0x00,0x5E,0xAB

#define TELINK_IR_UUID_SERVICE                 0x64,0xb6,0x17,0xf6,0x01,0xaf,0x7d,0xbc,0x05,0x4f,0x21,0x5a,0xc0,0xbf,0x43,0xd3
#define TELINK_IR_PROG_CONTROL_UUID_SERVICE    0x64,0xb6,0x17,0xf6,0x01,0xaf,0x7d,0xbc,0x05,0x4f,0x21,0x5a,0xc1,0xbf,0x43,0xd3
#define TELINK_IR_KEY_ID_UUID_SERVICE          0x64,0xb6,0x17,0xf6,0x01,0xaf,0x7d,0xbc,0x05,0x4f,0x21,0x5a,0xc2,0xbf,0x43,0xd3
#define TELINK_IR_CODE_UUID_SERVICE            0x64,0xb6,0x17,0xf6,0x01,0xaf,0x7d,0xbc,0x05,0x4f,0x21,0x5a,0xc3,0xbf,0x43,0xd3
#define TELINK_IR_SUPPRESS_UUID_SERVICE        0x64,0xb6,0x17,0xf6,0x01,0xaf,0x7d,0xbc,0x05,0x4f,0x21,0x5a,0xc4,0xbf,0x43,0xd3
#define TELINK_IR_KEY_EVENT_UUID_SERVICE       0x64,0xb6,0x17,0xf6,0x01,0xaf,0x7d,0xbc,0x05,0x4f,0x21,0x5a,0xc5,0xbf,0x43,0xd3


/** @addtogroup  GATT_UUID_Variables GATT UUID variables
 *  @{
 */
/**
 *  @brief  External variable for GATT UUID which can be used in service definition
 */
/** @} end of group GATT_UUID_Variables */

/** @} end of group GATT_Common_Module */


#define CHARACTERISTIC_UUID_BATTERY_LEVEL                    0x2A19
#define CHARACTERISTIC_UUID_ALERT_LEVEL                       0x2A06

//#include <stack/ble/service/hids.h>
#include "att.h"
#include <stack/ble/service/device_information.h>

#endif
