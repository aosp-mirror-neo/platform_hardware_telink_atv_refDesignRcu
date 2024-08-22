/******************************************************************************
 * @file     app_att.h
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
#ifndef APP_ATT_H_
#define APP_ATT_H_

#include "app_config.h"
#include "app_fmr/app_fmr.h"

///////////////////////////////////// ATT  HANDLER define ///////////////////////////////////////
typedef enum
{
    ATT_H_START = 0,


    //// Gap ////
    /**********************************************************************************************/
    GenericAccess_PS_H,                     //UUID: 2800,     VALUE: uuid 1800
    GenericAccess_DeviceName_CD_H,          //UUID: 2803,     VALUE:              Prop: Read | Notify
    GenericAccess_DeviceName_DP_H,          //UUID: 2A00,     VALUE: device name
    GenericAccess_Appearance_CD_H,          //UUID: 2803,     VALUE:              Prop: Read
    GenericAccess_Appearance_DP_H,          //UUID: 2A01,     VALUE: appearance
    CONN_PARAM_CD_H,                        //UUID: 2803,     VALUE:              Prop: Read
    CONN_PARAM_DP_H,                        //UUID: 2A04,     VALUE: connParameter


    //// gatt ////
    /**********************************************************************************************/
    GenericAttribute_PS_H,                  //UUID: 2800,     VALUE: uuid 1801
    GenericAttribute_ServiceChanged_CD_H,   //UUID: 2803,     VALUE:              Prop: Indicate
    GenericAttribute_ServiceChanged_DP_H,   //UUID: 2A05,     VALUE: service change
    GenericAttribute_ServiceChanged_CCB_H,  //UUID: 2902,     VALUE: serviceChangeCCC


    //// device information ////
    /**********************************************************************************************/
    DeviceInformation_PS_H,                 //UUID: 2800,     VALUE: uuid 180A
    DeviceInformation_pnpID_CD_H,           //UUID: 2803,     VALUE:              Prop: Read
    DeviceInformation_pnpID_DP_H,           //UUID: 2A50,     VALUE: PnPtrs

    DeviceInformation_firmwareRev_CD_H,     //UUID: 2803,     VALUE:              Prop: Read
    DeviceInformation_firmwareRev_DP_H,     //UUID: 2A50,     VALUE: PnPtrs

    DeviceInformation_softwareRev_CD_H,     //UUID: 2803,     VALUE:              Prop: Read
    DeviceInformation_softwareRev_DP_H,     //UUID: 2A50,     VALUE: PnPtrs

    DeviceInformation_hardwareRev_CD_H,     //UUID: 2803,     VALUE:              Prop: Read
    DeviceInformation_hardwareRev_DP_H,     //UUID: 2A50,     VALUE: PnPtrs

    //// HID ////
    /**********************************************************************************************/
    HID_PS_H,                               //UUID: 2800,     VALUE: uuid 1812

    //include
    HID_INCLUDE_H,                          //UUID: 2802,     VALUE: include

    //protocol
    HID_PROTOCOL_MODE_CD_H,                 //UUID: 2803,     VALUE:              Prop: read | write_without_rsp
    HID_PROTOCOL_MODE_DP_H,                 //UUID: 2A4E,     VALUE: protocolMode

    //boot keyboard input report
    HID_BOOT_KB_REPORT_INPUT_CD_H,          //UUID: 2803,     VALUE:              Prop: Read | Notify
    HID_BOOT_KB_REPORT_INPUT_DP_H,          //UUID: 2A22,     VALUE: bootKeyInReport
    HID_BOOT_KB_REPORT_INPUT_CCB_H,         //UUID: 2902,     VALUE: bootKeyInReportCCC

    //boot keyboard output report
    HID_BOOT_KB_REPORT_OUTPUT_CD_H,         //UUID: 2803,     VALUE:              Prop: Read | write| write_without_rsp
    HID_BOOT_KB_REPORT_OUTPUT_DP_H,         //UUID: 2A32,     VALUE: bootKeyOutReport

    //consume report in
    HID_CONSUME_REPORT_INPUT_CD_H,          //UUID: 2803,     VALUE:              Prop: Read | Notify
    HID_CONSUME_REPORT_INPUT_DP_H,          //UUID: 2A4D,     VALUE: reportConsumerIn
    HID_CONSUME_REPORT_INPUT_CCB_H,         //UUID: 2902,     VALUE: reportConsumerInCCC
    HID_CONSUME_REPORT_INPUT_REF_H,         //UUID: 2908      VALUE: REPORT_ID_CONSUMER, TYPE_INPUT

    //keyboard report in
    HID_NORMAL_KB_REPORT_INPUT_CD_H,        //UUID: 2803,     VALUE:              Prop: Read | Notify
    HID_NORMAL_KB_REPORT_INPUT_DP_H,        //UUID: 2A4D,     VALUE: reportKeyIn
    HID_NORMAL_KB_REPORT_INPUT_CCB_H,       //UUID: 2902,     VALUE: reportKeyInInCCC
    HID_NORMAL_KB_REPORT_INPUT_REF_H,       //UUID: 2908      VALUE: REPORT_ID_KEYBOARD, TYPE_INPUT

    //keyboard report out
    HID_NORMAL_KB_REPORT_OUTPUT_CD_H,       //UUID: 2803,     VALUE:              Prop: Read | write| write_without_rsp
    HID_NORMAL_KB_REPORT_OUTPUT_DP_H,       //UUID: 2A4D,     VALUE: reportKeyOut
    HID_NORMAL_KB_REPORT_OUTPUT_REF_H,      //UUID: 2908      VALUE: REPORT_ID_KEYBOARD, TYPE_OUTPUT

    // report map
    HID_REPORT_MAP_CD_H,                    //UUID: 2803,     VALUE:              Prop: Read
    HID_REPORT_MAP_DP_H,                    //UUID: 2A4B,     VALUE: reportKeyIn
    HID_REPORT_MAP_EXT_REF_H,               //UUID: 2907      VALUE: extService

    //hid information
    HID_INFORMATION_CD_H,                   //UUID: 2803,     VALUE:              Prop: read
    HID_INFORMATION_DP_H,                   //UUID: 2A4A      VALUE: hidInformation

    //control point
    HID_CONTROL_POINT_CD_H,                 //UUID: 2803,     VALUE:              Prop: write_without_rsp
    HID_CONTROL_POINT_DP_H,                 //UUID: 2A4C      VALUE: controlPoint
#if (BLE_AUDIO_ENABLE)
    #if (TL_AUDIO_MODE & TL_AUDIO_MASK_HID_SERVICE_CHANNEL)
    //audio report in 1
    HID_AUDIO_REPORT_INPUT_FIRST_CD_H,      //UUID: 2803,     VALUE:              Prop: Read | Notify
    HID_AUDIO_REPORT_INPUT_FIRST_DP_H,      //UUID: 2A4D,     VALUE: reportKeyIn
    HID_AUDIO_REPORT_INPUT_FIRST_CCB_H,     //UUID: 2902,     VALUE: reportKeyInInCCC
    HID_AUDIO_REPORT_INPUT_FIRST_REF_H,     //UUID: 2908      VALUE: REPORT_ID_KEYBOARD, TYPE_INPUT
    //audio report in 2
    HID_AUDIO_REPORT_INPUT_SECND_CD_H,      //UUID: 2803,     VALUE:              Prop: Read | Notify
    HID_AUDIO_REPORT_INPUT_SECND_DP_H,      //UUID: 2A4D,     VALUE: reportKeyIn
    HID_AUDIO_REPORT_INPUT_SECND_CCB_H,     //UUID: 2902,     VALUE: reportKeyInInCCC
    HID_AUDIO_REPORT_INPUT_SECND_REF_H,     //UUID: 2908      VALUE: REPORT_ID_KEYBOARD, TYPE_INPUT
    //audio report in 3
    HID_AUDIO_REPORT_INPUT_THIRD_CD_H,      //UUID: 2803,     VALUE:              Prop: Read | Notify
    HID_AUDIO_REPORT_INPUT_THIRD_DP_H,      //UUID: 2A4D,     VALUE: reportKeyIn
    HID_AUDIO_REPORT_INPUT_THIRD_CCB_H,     //UUID: 2902,     VALUE: reportKeyInInCCC
    HID_AUDIO_REPORT_INPUT_THIRD_REF_H,     //UUID: 2908      VALUE: REPORT_ID_KEYBOARD, TYPE_INPUT
    #endif
#endif
    //// battery service ////
    /**********************************************************************************************/
    BATT_PS_H,                              //UUID: 2800,     VALUE: uuid 180f
    BATT_LEVEL_INPUT_CD_H,                  //UUID: 2803,     VALUE:              Prop: Read | Notify
    BATT_LEVEL_INPUT_DP_H,                  //UUID: 2A19      VALUE: batVal
    BATT_LEVEL_INPUT_CCB_H,                 //UUID: 2902,     VALUE: batValCCC


    //// Ota ////
    /**********************************************************************************************/
    OTA_PS_H,                               //UUID: 2800,     VALUE: telink ota service uuid
    OTA_CMD_OUT_CD_H,                       //UUID: 2803,     VALUE:              Prop: read | write_without_rsp
    OTA_CMD_OUT_DP_H,                       //UUID: telink ota uuid,  VALUE: otaData
    OTA_CMD_OUT_CCC_H,
    OTA_CMD_OUT_DESC_H,                     //UUID: 2901,     VALUE: otaName



#if (BLE_AUDIO_ENABLE)
    /********************************************************************************************
    * GOOGLE AUDIO service 9:
    ********************************************************************************************/
    AUDIO_GOOGLE_PS_H ,
    AUDIO_GOOGLE_TX_CD_H,
    AUDIO_GOOGLE_TX_DP_H ,
    AUDIO_GOOGLE_RX_CD_H ,
    AUDIO_GOOGLE_RX_DP_H,
    AUDIO_GOOGLE_RX_CCC_H,
    AUDIO_GOOGLE_CTL_CD_H,
    AUDIO_GOOGLE_CTL_DP_H,
    AUDIO_GOOGLE_CTL_CCC_H,
#endif

#if APP_IR_OVER_BLE

    //ir
    IR_PS_H,                                //UUID: 2800,     VALUE: telink audio service uuid
    IR_PROG_OUT_CONTROL_CD_H,               //UUID: 2800,     VALUE: telink audio service uui
    IR_PROG_OUT_CONTROL_H,
    IR_KEY_ID_CD_H,
    IR_KEY_ID_H,
    IR_CODE_CD_H,
    IR_CODE_H,
    IR_SUPPRESS_CD_H,
    IR_SUPPRESS_H,
    IR_KEY_EVENT_CD_H,
    IR_KEY_EVENT_H,
    IR_KEY_EVENT_CCC_H,

#endif


#if FIND_ME_ENABLE
    FMS_GOOGLE_PS_H ,
    FMS_GOOGLE_CTL_CD_H,
    FMS_GOOGLE_CTL_DP_H,
    FMS_GOOGLE_DATA_CD_H ,
    FMS_GOOGLE_DATA_DP_H,
    FMS_GOOGLE_DATA_CCC_H,
#endif

    APP_FMR_HANDLE


#if (MP_TEST_MODE)
    //// Test Mode ////
    /**********************************************************************************************/
    TEST_MODE_PS_H,                         //UUID: 2800,     VALUE: test mode service uuid
    TEST_MODE_CD_H,                         //UUID: 2803,     VALUE: Prop: read | write | write_without_rsp | notify
    TEST_MODE_DP_H,                         //UUID: test mode uuid,  VALUE: testData
    TEST_MODE_CCB_H,                        //UUID: 2902,     VALUE: testDataCCC
#endif


    ATT_END_H,

}ATT_HANDLE;


#endif /* APP_ATT_H_ */
