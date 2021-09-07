/******************************************************************************
 * @file     app_att.c
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
#include "application/audio/audio_config.h"
#include "stack/ble/ble.h"
#include "app_audio.h"
#include "app_ui.h"
#include "app_ota.h"
#include "app_ir.h"

#if (MP_TEST_MODE)
#include "app_test.h"
#endif

typedef struct
{
  /** Minimum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  u16 intervalMin;
  /** Maximum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  u16 intervalMax;
  /** Number of LL latency connection events (0x0000 - 0x03e8) */
  u16 latency;
  /** Connection Timeout (0x000A - 0x0C80 * 10 ms) */
  u16 timeout;
} gap_periConnectParams_t;

static const u16 clientCharacterCfgUUID = GATT_UUID_CLIENT_CHAR_CFG;

static const u16 extReportRefUUID = GATT_UUID_EXT_REPORT_REF;

static const u16 reportRefUUID = GATT_UUID_REPORT_REF;

static const u16 characterPresentFormatUUID = GATT_UUID_CHAR_PRESENT_FORMAT;

static const u16 userdesc_UUID    = GATT_UUID_CHAR_USER_DESC;

static const u16 serviceChangeUUID = GATT_UUID_SERVICE_CHANGE;

static const u16 my_primaryServiceUUID = GATT_UUID_PRIMARY_SERVICE;

static const u16 my_characterUUID = GATT_UUID_CHARACTER;

static const u16 my_devServiceUUID = SERVICE_UUID_DEVICE_INFORMATION;

static const u16 my_PnPUUID = CHARACTERISTIC_UUID_PNP_ID;

static const u16 my_SysIDUUID = CHARACTERISTIC_UUID_SYSTEM_ID;

static const u16 my_ModelNUMUUID = CHARACTERISTIC_UUID_MODEL_NUM_STRING;

static const u16 my_SerialNUMUUID = CHARACTERISTIC_UUID_SERIAL_NUM_STRING;

static const u16 my_FWRevUUID = CHARACTERISTIC_UUID_FW_REVISION_STRING;

static const u16 my_HWRevUUID = CHARACTERISTIC_UUID_HW_REVISION_STRING;

static const u16 my_SWRevUUID = CHARACTERISTIC_UUID_SW_REVISION_STRING;

static const u16 my_ManuNameUUID = CHARACTERISTIC_UUID_MANU_NAME_STRING;

static const u16 my_IEEE11073UUID = CHARACTERISTIC_UUID_IEEE_11073_CERT_LIST;


static const u16 my_devNameUUID = GATT_UUID_DEVICE_NAME;

static const u16 my_gapServiceUUID = SERVICE_UUID_GENERIC_ACCESS;

static const u16 my_appearanceUIID = GATT_UUID_APPEARANCE;

static const u16 my_periConnParamUUID = GATT_UUID_PERI_CONN_PARAM;

static const u16 my_appearance = GAP_APPEARE_UNKNOWN;

static const u16 my_gattServiceUUID = SERVICE_UUID_GENERIC_ATTRIBUTE;

_attribute_data_retention_ u8 my_devName[20];
_attribute_data_retention_ u8 my_devNamelen;

static u16 serviceChangeVal[2] = {0};

static u8 serviceChangeCCC[2] = {0,0};

static const gap_periConnectParams_t my_periConnParameters = {8, 11, 0, 1000};
_attribute_data_retention_ u8 my_PnPtrs []       = {0x02, 0x8a, 0x24, 0x66, 0x82, 0x01, 0x00};


 _attribute_data_retention_ u8 my_FWRevtrs []     = {'V','1','.','1','1'};
 _attribute_data_retention_ u8 my_SWRevtrs []     = {'V','1','.','0','2'};
 static const u8 my_HWRevtrs []     = {'V','1','.','0'};
 static const u8 my_ManuNametrs []  = {'T','e','l','i','n','k'};

//////////////////////// Battery /////////////////////////////////////////////////
static const u16 my_batServiceUUID        = SERVICE_UUID_BATTERY;
static const u16 my_batCharUUID             = CHARACTERISTIC_UUID_BATTERY_LEVEL;
static u8 batteryValueInCCC[2];
_attribute_data_retention_ u8 my_batVal[1]     = {99};

//////////////////////// HID /////////////////////////////////////////////////////

static const u16 my_hidServiceUUID        = SERVICE_UUID_HUMAN_INTERFACE_DEVICE;

static const u16 hidServiceUUID           = SERVICE_UUID_HUMAN_INTERFACE_DEVICE;
static const u16 hidProtocolModeUUID      = CHARACTERISTIC_UUID_HID_PROTOCOL_MODE;
static const u16 hidReportUUID            = CHARACTERISTIC_UUID_HID_REPORT;
static const u16 hidReportMapUUID         = CHARACTERISTIC_UUID_HID_REPORT_MAP;
static const u16 hidbootKeyInReportUUID   = CHARACTERISTIC_UUID_HID_BOOT_KEY_INPUT;
static const u16 hidbootKeyOutReportUUID  = CHARACTERISTIC_UUID_HID_BOOT_KEY_OUTPUT;
static const u16 hidbootMouseInReportUUID = CHARACTERISTIC_UUID_HID_BOOT_MOUSE_INPUT;
static const u16 hidinformationUUID       = CHARACTERISTIC_UUID_HID_INFORMATION;
static const u16 hidCtrlPointUUID         = CHARACTERISTIC_UUID_HID_CONTROL_POINT;
static const u16 hidIncludeUUID           = GATT_UUID_INCLUDE;

static const u8 protocolMode               = DFLT_HID_PROTOCOL_MODE;

// Key in Report characteristic variables
static u8 reportKeyIn[8];
static u8 reportKeyInCCC[2];
// HID Report Reference characteristic descriptor, key input
static const u8 reportRefKeyIn[2] =
             { HID_REPORT_ID_KEYBOARD_INPUT, HID_REPORT_TYPE_INPUT };

// Key out Report characteristic variables
static u8 reportKeyOut[1];
static const u8 reportRefKeyOut[2] =
             { HID_REPORT_ID_KEYBOARD_INPUT, HID_REPORT_TYPE_OUTPUT };

// Consumer Control input Report
static u8 reportConsumerControlIn[2];
static u8 reportConsumerControlInCCC[2];
static const u8 reportRefConsumerControlIn[2] =
             { HID_REPORT_ID_CONSUME_CONTROL_INPUT, HID_REPORT_TYPE_INPUT };

// Boot Keyboard Input Report
static u8 bootKeyInReport;
static u8 bootKeyInReportCCC[2];

// Boot Keyboard Output Report
static u8 bootKeyOutReport;

// HID Information characteristic
static const u8 hidInformation[] =
{
  U16_LO(0x0111), U16_HI(0x0111),             // bcdHID (USB HID version)
  0x00,                                       // bCountryCode
  0x01                                        // Flags
};

// HID Control Point characteristic
static u8 controlPoint;

// HID Report Map characteristic
// Keyboard report descriptor (using format for Boot interface descriptor)

static const u8 reportMap[] =
{
    //keyboard report in
    0x05, 0x01,     // Usage Pg (Generic Desktop)
    0x09, 0x06,     // Usage (Keyboard)
    0xA1, 0x01,     // Collection: (Application)
    0x85, HID_REPORT_ID_KEYBOARD_INPUT,     // Report Id (keyboard)
                  //
    0x05, 0x07,     // Usage Pg (Key Codes)
    0x19, 0xE0,     // Usage Min (224)  VK_CTRL:0xe0
    0x29, 0xE7,     // Usage Max (231)  VK_RWIN:0xe7
    0x15, 0x00,     // Log Min (0)
    0x25, 0x01,     // Log Max (1)
                  //
                  // Modifier byte
    0x75, 0x01,     // Report Size (1)   1 bit * 8
    0x95, 0x08,     // Report Count (8)
    0x81, 0x02,     // Input: (Data, Variable, Absolute)
                  //
                  // Reserved byte
    0x95, 0x01,     // Report Count (1)
    0x75, 0x08,     // Report Size (8)
    0x81, 0x01,     // Input: (static constant)

    //keyboard output
    //5 bit led ctrl: NumLock CapsLock ScrollLock Compose kana
    0x95, 0x05,    //Report Count (5)
    0x75, 0x01,    //Report Size (1)
    0x05, 0x08,    //Usage Pg (LEDs )
    0x19, 0x01,    //Usage Min
    0x29, 0x05,    //Usage Max
    0x91, 0x02,    //Output (Data, Variable, Absolute)
    //3 bit reserved
    0x95, 0x01,    //Report Count (1)
    0x75, 0x03,    //Report Size (3)
    0x91, 0x01,    //Output (static constant)

    // Key arrays (6 bytes)
    0x95, 0x06,     // Report Count (6)
    0x75, 0x08,     // Report Size (8)
    0x15, 0x00,     // Log Min (0)
    0x25, 0xF1,     // Log Max (241)
    0x05, 0x07,     // Usage Pg (Key Codes)
    0x19, 0x00,     // Usage Min (0)
    0x29, 0xf1,     // Usage Max (241)
    0x81, 0x00,     // Input: (Data, Array)

    0xC0,            // End Collection

#if (TL_AUDIO_MODE & TL_AUDIO_MASK_HID_SERVICE_CHANNEL)                            //HID Service
    //consumer report in
    0x05, 0x0C,   // Usage Page (Consumer)
    0x09, 0x01,   // Usage (Consumer Control)
    0xA1, 0x01,   // Collection (Application)
    0x85, HID_REPORT_ID_CONSUME_CONTROL_INPUT,   //     Report Id
    0x75,0x10,     //global, report size 16 bits
    0x95,0x0A,     //global, report count 1
    0x15,0x01,     //global, min  0x01
    0x26,0x8c,0x02,  //global, max  0x28c
    0x19,0x01,     //local, min   0x01
    0x2a,0x8c,0x02,  //local, max    0x28c
    0x81,0x00,     //main,  input data variable, absolute
    0xc0,        //main, end collection

    //audio 3
    0x05, 0x0c,
    0x09, 0x01,
    0xA1, 0x01,
    0x85, HID_REPORT_ID_AUDIO_THIRD_INPUT,
    0x95, 0x14,
    0x75, 0x08,
    0x15, 0x00,
    0x26, 0xff, 0x00,
    0x81, 0x00,
    0xC0,
    //audio 1
    0x05, 0x0c,
    0x09, 0x01,
    0xA1, 0x01,
    0x85, HID_REPORT_ID_AUDIO_FIRST_INPUT,
    0x95, 0x14,
    0x75, 0x08,
    0x15, 0x00,
    0x26, 0xff, 0x00,
    0x81, 0x00,
    0xC0,
    //audio 2
    0x05, 0x0c,
    0x09, 0x01,
    0xA1, 0x01,
    0x85, HID_REPORT_ID_AUDIO_SECND_INPUT,
    0x95, 0x14,
    0x75, 0x08,
    0x15, 0x00,
    0x26, 0xff, 0x00,
    0x81, 0x00,
    0xC0,
#else
    //consumer report in
    0x05, 0x0C,   // Usage Page (Consumer)
    0x09, 0x01,   // Usage (Consumer Control)
    0xA1, 0x01,   // Collection (Application)
    0x85, HID_REPORT_ID_CONSUME_CONTROL_INPUT,   //     Report Id
    0x75,0x10,     //global, report size 16 bits
    0x95,0x02,     //global, report count 2
    0x15,0x01,     //global, min  0x01
    0x26,0x8c,0x02,  //global, max  0x28c
    0x19,0x01,     //local, min   0x01
    0x2a,0x8c,0x02,  //local, max    0x28c
    0x81,0x00,     //main,  input data variable, absolute
    0xc0,        //main, end collection
#endif


};

// HID External Report Reference Descriptor for report map
static u16 extServiceUUID;


/////////////////////////////////////////////////////////

static const u8 my_OtaUUID[16]           = {TELINK_SPP_DATA_OTA};
static const u8 my_OtaServiceUUID[16]    = TELINK_OTA_UUID_SERVICE;

static u8 my_OtaData         = 0x00;
_attribute_data_retention_ u16 my_Otaccc = 0;

static const u8  my_OtaName[] = {'O', 'T', 'A'};

#if (MP_TEST_MODE)
static const u8 my_TestModeServiceUUID[16] = {TEST_MODE_SERVICE_UUID};
static const u8 my_TestDataUUID[16] = {TEST_MODE_TEST_DATA_UUID};

static u8 my_TestData = 0x00;
static u8 my_TestDataCCC[2] = { 0x00, };
#endif


// Include attribute (Battery service)
static const u16 include[3] = {BATT_PS_H, BATT_LEVEL_INPUT_CCB_H, SERVICE_UUID_BATTERY};


//// GAP attribute values
static const u8 my_devNameCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_NOTIFY,
    U16_LO(GenericAccess_DeviceName_DP_H), U16_HI(GenericAccess_DeviceName_DP_H),
    U16_LO(GATT_UUID_DEVICE_NAME), U16_HI(GATT_UUID_DEVICE_NAME)
};
static const u8 my_appearanceCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(GenericAccess_Appearance_DP_H), U16_HI(GenericAccess_Appearance_DP_H),
    U16_LO(GATT_UUID_APPEARANCE), U16_HI(GATT_UUID_APPEARANCE)
};
static const u8 my_periConnParamCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(CONN_PARAM_DP_H), U16_HI(CONN_PARAM_DP_H),
    U16_LO(GATT_UUID_PERI_CONN_PARAM), U16_HI(GATT_UUID_PERI_CONN_PARAM)
};


//// GATT attribute values
static const u8 my_serviceChangeCharVal[5] = {
    CHAR_PROP_INDICATE,
    U16_LO(GenericAttribute_ServiceChanged_DP_H), U16_HI(GenericAttribute_ServiceChanged_DP_H),
    U16_LO(GATT_UUID_SERVICE_CHANGE), U16_HI(GATT_UUID_SERVICE_CHANGE)
};

//// device Information  attribute values
static const u8 my_PnCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(DeviceInformation_pnpID_DP_H), U16_HI(DeviceInformation_pnpID_DP_H),
    U16_LO(CHARACTERISTIC_UUID_PNP_ID), U16_HI(CHARACTERISTIC_UUID_PNP_ID)
};

static const u8 my_FWRevCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(DeviceInformation_firmwareRev_DP_H), U16_HI(DeviceInformation_firmwareRev_DP_H),
    U16_LO(CHARACTERISTIC_UUID_FW_REVISION_STRING), U16_HI(CHARACTERISTIC_UUID_FW_REVISION_STRING)
};


static const u8 my_HWRevCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(DeviceInformation_hardwareRev_DP_H), U16_HI(DeviceInformation_hardwareRev_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HW_REVISION_STRING), U16_HI(CHARACTERISTIC_UUID_HW_REVISION_STRING)
};

static const u8 my_SWRevCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(DeviceInformation_softwareRev_DP_H), U16_HI(DeviceInformation_softwareRev_DP_H),
    U16_LO(CHARACTERISTIC_UUID_SW_REVISION_STRING), U16_HI(CHARACTERISTIC_UUID_SW_REVISION_STRING)
};



//// HID attribute values
static const u8 my_hidProtocolModeCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP,
    U16_LO(HID_PROTOCOL_MODE_DP_H), U16_HI(HID_PROTOCOL_MODE_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_PROTOCOL_MODE), U16_HI(CHARACTERISTIC_UUID_HID_PROTOCOL_MODE)
};
static const u8 my_hidbootKeyInReporCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_NOTIFY,
    U16_LO(HID_BOOT_KB_REPORT_INPUT_DP_H), U16_HI(HID_BOOT_KB_REPORT_INPUT_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_BOOT_KEY_INPUT), U16_HI(CHARACTERISTIC_UUID_HID_BOOT_KEY_INPUT)
};
static const u8 my_hidbootKeyOutReporCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
    U16_LO(HID_BOOT_KB_REPORT_OUTPUT_DP_H), U16_HI(HID_BOOT_KB_REPORT_OUTPUT_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_BOOT_KEY_OUTPUT), U16_HI(CHARACTERISTIC_UUID_HID_BOOT_KEY_OUTPUT)
};
static const u8 my_hidReportCCinCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_NOTIFY,
    U16_LO(HID_CONSUME_REPORT_INPUT_DP_H), U16_HI(HID_CONSUME_REPORT_INPUT_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_REPORT), U16_HI(CHARACTERISTIC_UUID_HID_REPORT)
};
static const u8 my_hidReportKEYinCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_NOTIFY,
    U16_LO(HID_NORMAL_KB_REPORT_INPUT_DP_H), U16_HI(HID_NORMAL_KB_REPORT_INPUT_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_REPORT), U16_HI(CHARACTERISTIC_UUID_HID_REPORT)
};
static const u8 my_hidReportKEYoutCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
    U16_LO(HID_NORMAL_KB_REPORT_OUTPUT_DP_H), U16_HI(HID_NORMAL_KB_REPORT_OUTPUT_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_REPORT), U16_HI(CHARACTERISTIC_UUID_HID_REPORT)
};
static const u8 my_hidReportMapCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(HID_REPORT_MAP_DP_H), U16_HI(HID_REPORT_MAP_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_REPORT_MAP), U16_HI(CHARACTERISTIC_UUID_HID_REPORT_MAP)
};
static const u8 my_hidinformationCharVal[5] = {
    CHAR_PROP_READ,
    U16_LO(HID_INFORMATION_DP_H), U16_HI(HID_INFORMATION_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_INFORMATION), U16_HI(CHARACTERISTIC_UUID_HID_INFORMATION)
};
static const u8 my_hidCtrlPointCharVal[5] = {
    CHAR_PROP_WRITE_WITHOUT_RSP,
    U16_LO(HID_CONTROL_POINT_DP_H), U16_HI(HID_CONTROL_POINT_DP_H),
    U16_LO(CHARACTERISTIC_UUID_HID_CONTROL_POINT), U16_HI(CHARACTERISTIC_UUID_HID_CONTROL_POINT)
};


//// Battery attribute values
static const u8 my_batCharVal[5] = {
    CHAR_PROP_READ | CHAR_PROP_NOTIFY,
    U16_LO(BATT_LEVEL_INPUT_DP_H), U16_HI(BATT_LEVEL_INPUT_DP_H),
    U16_LO(CHARACTERISTIC_UUID_BATTERY_LEVEL), U16_HI(CHARACTERISTIC_UUID_BATTERY_LEVEL)
};


//// OTA attribute values
static const u8 my_OtaCharVal[19] = {
//    CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_NOTIFY,

    CHAR_PROP_READ | CHAR_PROP_NOTIFY | CHAR_PROP_WRITE_WITHOUT_RSP,
    U16_LO(OTA_CMD_OUT_DP_H), U16_HI(OTA_CMD_OUT_DP_H),
    TELINK_SPP_DATA_OTA,
};

#if (MP_TEST_MODE)
/// Test Mode attribute values
static const u8 my_TestCharVal[19] = {
    CHAR_PROP_READ | CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_NOTIFY,
    U16_LO(TEST_MODE_DP_H), U16_HI(TEST_MODE_DP_H),
    TEST_MODE_TEST_DATA_UUID,
};
#endif

static const u8 my_IrUUID[16]      = {TELINK_IR_UUID_SERVICE};
static const u8 my_Ir_ProgControlUUID[16]   = {TELINK_IR_PROG_CONTROL_UUID_SERVICE};
static const u8 my_Ir_KeyIdUUID[16]          = {TELINK_IR_KEY_ID_UUID_SERVICE};
static const u8 my_Ir_CodeUUID[16]          = {TELINK_IR_CODE_UUID_SERVICE};
static const u8 my_Ir_SuppressUUID[16]      = {TELINK_IR_SUPPRESS_UUID_SERVICE};
static const u8 my_Ir_KeyEventUUID[16]      = {TELINK_IR_KEY_EVENT_UUID_SERVICE};



static const u8 my_Ir_ProgControl_CharVal[19] = {
    CHAR_PROP_WRITE,
    U16_LO(IR_PROG_OUT_CONTROL_H), U16_HI(IR_PROG_OUT_CONTROL_H),
    TELINK_IR_PROG_CONTROL_UUID_SERVICE
};
static const u8 my_Ir_Key_Id_CharVal[19] = {
    CHAR_PROP_WRITE,
    U16_LO(IR_KEY_ID_H), U16_HI(IR_KEY_ID_H),
    TELINK_IR_KEY_ID_UUID_SERVICE
};
static const u8 my_Ir_Code_CharVal[19] = {
    CHAR_PROP_WRITE,
    U16_LO(IR_CODE_H), U16_HI(IR_CODE_H),
    TELINK_IR_CODE_UUID_SERVICE
};
static const u8 my_Ir_Suppress_CharVal[19] = {
    CHAR_PROP_WRITE,
    U16_LO(IR_SUPPRESS_H), U16_HI(IR_SUPPRESS_H),
    TELINK_IR_SUPPRESS_UUID_SERVICE
};
static const u8 my_Ir_Key_Event_CharVal[19] = {
    CHAR_PROP_NOTIFY,
    U16_LO(IR_KEY_EVENT_H), U16_HI(IR_KEY_EVENT_H),
    TELINK_IR_KEY_EVENT_UUID_SERVICE
};
u8      ir_data[1] = {0};


#if BLE_AUDIO_ENABLE
//// Audio attribute values
    #include "application/audio/gl_audio.h"

    u8      atv_char_tx_data[20] = {0};
    u8      atv_char_rx_data[1] = {0};
    u8      atv_char_ctl_data[1] = {0};

    static const u8 ATVV_Server_UUID[16]         = AUDIO_GOOGLE_SERVICE_UUID;

    static const u8 ATVV_Char_UUID_TX[16]         = {AUDIO_GOOGL_TX_CHAR_UUID};
    static const u8 ATVV_Char_UUID_RX[16]         = {AUDIO_GOOGL_RX_CHAR_UUID};
    static const u8 ATVV_Char_UUID_CTL[16]        = {AUDIO_GOOGL_CTL_CHAR_UUID};

    static const u8 my_MicCharTx[20] = {
        CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_NOTIFY,
        U16_LO(AUDIO_GOOGLE_TX_DP_H), U16_HI(AUDIO_GOOGLE_TX_DP_H),
        AUDIO_GOOGL_TX_CHAR_UUID,
    };

    static const u8 my_MicCharRx[20] = {
        CHAR_PROP_READ | CHAR_PROP_NOTIFY,
        U16_LO(AUDIO_GOOGLE_RX_DP_H), U16_HI(AUDIO_GOOGLE_RX_DP_H),
        AUDIO_GOOGL_RX_CHAR_UUID,
    };

    static const u8 my_MicCharCtl[20] = {
        CHAR_PROP_READ | CHAR_PROP_NOTIFY,
        U16_LO(AUDIO_GOOGLE_CTL_DP_H), U16_HI(AUDIO_GOOGLE_CTL_DP_H),
        AUDIO_GOOGL_CTL_CHAR_UUID,
    };

#endif

extern void app_set_mtusize(void);
extern void ir_fallback_process(u16 handle, u8* buf, u16 len);
int app_ir_callback(void *p)
{
    u16 handle,len;
    u16 printf_len;
    u16 i;

    rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;
    handle = pw->handle;

    //printf("handle = %x\r\n",handle);
    //printf("pw->l2cap=%x\r\n",pw->l2cap-3);
    printf("dat=\r\n");

    len = pw->l2cap-3;//pw->rf_len-7;
    printf_len = len;
    #if 1
    if(printf_len > 6)
        printf_len = 6;
    for(i=0;i<printf_len;i++)
        printf(" %x",pw->dat[i]);
    #endif
    printf("len=%x\r\n",len);
    app_set_mtusize();
    ir_fallback_process(handle,pw->dat,len);

    return 0;
}

int app_nec_ir_table_callback(void *p)
{
    u16 handle,len;
    u16 printf_len;
    u16 i;

    rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;
    handle = pw->handle;

    //printf("handle = %x\r\n",handle);
    //printf("pw->l2cap=%x\r\n",pw->l2cap-3);
    //printf("app_nec_ir_table_callback handle=%x\r\n",handle);

    len = pw->l2cap-3;//pw->rf_len-7;
    printf_len = len;

    #if 0
    printf("len=%x\r\n",len);
    for(i=0;i<printf_len;i++)
        printf(" %x",pw->dat[i]);
    #endif

    ir_nec_ir_table_process(pw->dat,len);
    return 0;
}

int app_ota_handle(void * p){

    int result=0;

    app_nec_ir_table_callback(p);
    result = app_ota_write(p);

    return result;
}



// TM : to modify
#if BLE_AUDIO_ENABLE
    static const attribute_t my_Attributes[] = {
#if (MP_TEST_MODE)
        {ATT_END_H - 1 - 4, 0,0,0,0,0},    // total num of attribute
#else
        {ATT_END_H - 1, 0,0,0,0,0},    // total num of attribute
#endif

        // 0001 - 0007  gap         (1-7)
        {7,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_gapServiceUUID), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_devNameCharVal),(u8*)(&my_characterUUID), (u8*)(my_devNameCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_devName), (u8*)(&my_devNameUUID), (u8*)(my_devName), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_appearanceCharVal),(u8*)(&my_characterUUID), (u8*)(my_appearanceCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof (my_appearance), (u8*)(&my_appearanceUIID),     (u8*)(&my_appearance), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_periConnParamCharVal),(u8*)(&my_characterUUID), (u8*)(my_periConnParamCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof (my_periConnParameters),(u8*)(&my_periConnParamUUID),     (u8*)(&my_periConnParameters), 0},


        // 0008 - 000b gatt         (8-11)
        {4,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_gattServiceUUID), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_serviceChangeCharVal),(u8*)(&my_characterUUID),         (u8*)(my_serviceChangeCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof (serviceChangeVal), (u8*)(&serviceChangeUUID),     (u8*)(&serviceChangeVal), 0},
        {0,ATT_PERMISSIONS_RDWR,2,sizeof (serviceChangeCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(serviceChangeCCC), 0},

        // 000c - 000e  device Information Service
        //PnP ID                    (12-14)

        {9,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_devServiceUUID), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_PnCharVal),(u8*)(&my_characterUUID), (u8*)(my_PnCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof (my_PnPtrs),(u8*)(&my_PnPUUID), (u8*)(my_PnPtrs), 0},

        //firmware revision string  (15-16)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_FWRevCharVal),(u8*)(&my_characterUUID), (u8*)(my_FWRevCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof (my_FWRevtrs),(u8*)(&my_FWRevUUID), (u8*)(my_FWRevtrs), 0},

        //software revision string  (17-18)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_SWRevCharVal),(u8*)(&my_characterUUID), (u8*)(my_SWRevCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof (my_SWRevtrs),(u8*)(&my_SWRevUUID), (u8*)(my_SWRevtrs), 0},
        
        //hardware revision string  (19-20)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_HWRevCharVal),(u8*)(&my_characterUUID), (u8*)(my_HWRevCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof (my_HWRevtrs),(u8*)(&my_HWRevUUID), (u8*)(my_HWRevtrs), 0},

        /////////////////////////////////// 4. HID Service /////////////////////////////////////////////////////////
        // 000f                     (21)
        //{27, ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_hidServiceUUID), 0},
        {HID_CONTROL_POINT_DP_H - HID_PS_H + 1, ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_hidServiceUUID), 0},

        // 0010  include battery service  (22)
        {0,ATT_PERMISSIONS_READ,2,sizeof(include),(u8*)(&hidIncludeUUID),     (u8*)(include), 0},

        // 0011 - 0012  protocol mode (23-24)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidProtocolModeCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidProtocolModeCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_READ,2, sizeof(protocolMode),(u8*)(&hidProtocolModeUUID),     (u8*)(&protocolMode), 0},    //value

        // 0013 - 0015  boot keyboard input report (char-val-client)   (25-27)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidbootKeyInReporCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidbootKeyInReporCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_READ,2,sizeof(bootKeyInReport),(u8*)(&hidbootKeyInReportUUID),     (u8*)(&bootKeyInReport), 0},    //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(bootKeyInReportCCC),(u8*)(&clientCharacterCfgUUID),     (u8*)(bootKeyInReportCCC), 0},    //value

        // 0016 - 0017   boot keyboard output report (char-val)        (28-29)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidbootKeyOutReporCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidbootKeyOutReporCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_RDWR,2, sizeof(bootKeyOutReport), (u8*)(&hidbootKeyOutReportUUID),     (u8*)(&bootKeyOutReport), 0},    //value


        // 0018 - 001b. consume report in: 4 (char-val-client-ref)     (30-33)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportCCinCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportCCinCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_READ,2, sizeof(reportConsumerControlIn),(u8*)(&hidReportUUID),     (u8*)(reportConsumerControlIn), 0},    //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportConsumerControlInCCC),(u8*)(&clientCharacterCfgUUID),     (u8*)(reportConsumerControlInCCC), 0},    //value
        {0,ATT_PERMISSIONS_READ,2,sizeof(reportRefConsumerControlIn),(u8*)(&reportRefUUID),     (u8*)(reportRefConsumerControlIn), 0},    //value

        // 001c - 001f . keyboard report in : 4 (char-val-client-ref)   (34-37)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportKEYinCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportKEYinCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_READ,2, sizeof(reportKeyIn),(u8*)(&hidReportUUID),     (u8*)(reportKeyIn), 0},    //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportKeyInCCC),(u8*)(&clientCharacterCfgUUID),     (u8*)(reportKeyInCCC), 0},    //value
        {0,ATT_PERMISSIONS_READ,2,sizeof(reportRefKeyIn),(u8*)(&reportRefUUID),     (u8*)(reportRefKeyIn), 0},    //value

        // 0020 - 0022 . keyboard report out: 3 (char-val-ref)         (38-40)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportKEYoutCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportKEYoutCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportKeyOut),(u8*)(&hidReportUUID),     (u8*)(reportKeyOut), 0},    //value
        {0,ATT_PERMISSIONS_READ,2,sizeof(reportRefKeyOut),(u8*)(&reportRefUUID),     (u8*)(reportRefKeyOut), 0},    //value
 
        // 0023 - 0025 . report map: 3                                  (41-43)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportMapCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportMapCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_READ,2,sizeof(reportMap),(u8*)(&hidReportMapUUID),     (u8*)(reportMap), 0},    //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(extServiceUUID),(u8*)(&extReportRefUUID),     (u8*)(&extServiceUUID), 0},    //value

        // 0026 - 0027 . hid information: 2                            (44-45)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidinformationCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidinformationCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_READ,2, sizeof(hidInformation),(u8*)(&hidinformationUUID),     (u8*)(hidInformation), 0},    //value

        // 0028 - 0029 . control point: 2                               (46-47)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidCtrlPointCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidCtrlPointCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_WRITE,2, sizeof(controlPoint),(u8*)(&hidCtrlPointUUID),     (u8*)(&controlPoint), 0},    //value


        ////////////////////////////////////// Battery Service /////////////////////////////////////////////////////
        // 002a - 002d                                                  (48-51)
        {4,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_batServiceUUID), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_batCharVal),(u8*)(&my_characterUUID), (u8*)(my_batCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_batVal),(u8*)(&my_batCharUUID),     (u8*)(my_batVal), 0},    //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(batteryValueInCCC),(u8*)(&clientCharacterCfgUUID),     (u8*)(batteryValueInCCC), 0},    //value

        ////////////////////////////////////// OTA /////////////////////////////////////////////////////
        // 002e - 0031                                                  (52-56)
        {8,ATT_PERMISSIONS_READ, 2,16,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_OtaServiceUUID), 0},
        {0,ATT_PERMISSIONS_READ, 2, sizeof(my_OtaCharVal),(u8*)(&my_characterUUID), (u8*)(my_OtaCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_RDWR,16,sizeof(my_OtaData),(u8*)(&my_OtaUUID),    (&my_OtaData), &app_ota_handle, 0},//&otaRead},            //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(my_Otaccc),(u8*)(&clientCharacterCfgUUID),(u8*)(&my_Otaccc), &att_ccc_control},
        {0,ATT_PERMISSIONS_READ, 2,sizeof (my_OtaName),(u8*)(&userdesc_UUID), (u8*)(my_OtaName), 0},

        ////////////////////////////////////// Audio /////////////////////////////////////////////////////
        /********************************************************************************************
        * GOOGLE AUDIO service
        ********************************************************************************************/
        //0032                                                           (57-65)
        {9,ATT_PERMISSIONS_READ,2,16, (u8*)(&my_primaryServiceUUID), (u8*)(&ATVV_Server_UUID), 0},
        //TX 0033-0034
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(my_MicCharTx),(u8*)(&my_characterUUID),(u8*)(my_MicCharTx), 0},
        {0,ATT_PERMISSIONS_RDWR,16,sizeof(atv_char_tx_data),(u8*)(&ATVV_Char_UUID_TX),(u8*)(&atv_char_tx_data), &app_auido_google_callback},    //value
        //RX 0035 - 0037
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_MicCharRx),(u8*)(&my_characterUUID),(u8*)(my_MicCharRx), 0},
        {0,ATT_PERMISSIONS_READ,16,sizeof(atv_char_rx_data),(u8*)(&ATVV_Char_UUID_RX),(u8*)(&atv_char_rx_data), 0},
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(atv_char_rx_ccc),(u8*)(&clientCharacterCfgUUID),(u8*)(&atv_char_rx_ccc), &att_ccc_control},
//        {0,ATT_PERMISSIONS_RDWR,2,sizeof(atv_char_rx_ccc),(u8*)(&clientCharacterCfgUUID),(u8*)(&atv_char_rx_ccc), 0},

        //CTRL 0038 -003A
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_MicCharCtl),(u8*)(&my_characterUUID),(u8*)(my_MicCharCtl), 0},
        {0,ATT_PERMISSIONS_READ,16,sizeof(atv_char_ctl_data),(u8*)(&ATVV_Char_UUID_CTL),(u8*)(&atv_char_ctl_data), 0},
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(atv_char_ctl_ccc),(u8*)(&clientCharacterCfgUUID),(u8*)(&atv_char_ctl_ccc), &att_ccc_control},
//        {0,ATT_PERMISSIONS_RDWR,2,sizeof(atv_char_ctl_ccc),(u8*)(&clientCharacterCfgUUID),(u8*)(&atv_char_ctl_ccc), 0},

        ////////////////////////////////////// IR /////////////////////////////////////////////////////
        //IR 003C                                                       (66-77)
        {12,ATT_PERMISSIONS_READ,2,16,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_IrUUID), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_Ir_ProgControl_CharVal),(u8*)(&my_characterUUID), (u8*)(my_Ir_ProgControl_CharVal), 0},        //prop
        {0,ATT_PERMISSIONS_RDWR,16,sizeof(ir_data),(u8*)(&my_Ir_ProgControlUUID), (u8*)(ir_data), &app_ir_callback},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_Ir_Key_Id_CharVal),(u8*)(&my_characterUUID), (u8*)(my_Ir_Key_Id_CharVal), 0},                //prop
        {0,ATT_PERMISSIONS_RDWR,16,sizeof(ir_data),(u8*)(&my_Ir_KeyIdUUID), (u8*)(ir_data), &app_ir_callback},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_Ir_Code_CharVal),(u8*)(&my_characterUUID), (u8*)(my_Ir_Code_CharVal), 0},                    //prop
        {0,ATT_PERMISSIONS_RDWR,16,sizeof(ir_data),(u8*)(&my_Ir_CodeUUID), (u8*)(ir_data),&app_ir_callback},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_Ir_Suppress_CharVal),(u8*)(&my_characterUUID),     (u8*)(my_Ir_Suppress_CharVal), 0},            //prop
        {0,ATT_PERMISSIONS_RDWR,16,sizeof(ir_data),(u8*)(&my_Ir_SuppressUUID), (u8*)(ir_data), &app_ir_callback},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_Ir_Key_Event_CharVal),(u8*)(&my_characterUUID),    (u8*)(my_Ir_Key_Event_CharVal), 0},            //prop
        {0,ATT_PERMISSIONS_RDWR,16,sizeof(ir_data),(u8*)(&my_Ir_KeyEventUUID), (u8*)(ir_data), 0},
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(ir_data),(u8*)(&clientCharacterCfgUUID), (u8*)(ir_data), &app_ir_callback},

    };

#if (MP_TEST_MODE)
    static const attribute_t my_test_dome_Attributes[] = {

        {ATT_END_H - 1, 0,0,0,0,0},    // total num of attribute


        /////////////////////////////////// 1. Gap /////////////////////////////////////////////////////////
        // 0001 - 0007              (1-7)
        {7,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_gapServiceUUID), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_devNameCharVal),(u8*)(&my_characterUUID), (u8*)(my_devNameCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_devName), (u8*)(&my_devNameUUID), (u8*)(my_devName), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_appearanceCharVal),(u8*)(&my_characterUUID), (u8*)(my_appearanceCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof (my_appearance), (u8*)(&my_appearanceUIID),     (u8*)(&my_appearance), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_periConnParamCharVal),(u8*)(&my_characterUUID), (u8*)(my_periConnParamCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof (my_periConnParameters),(u8*)(&my_periConnParamUUID),     (u8*)(&my_periConnParameters), 0},


        /////////////////////////////////// 2. Gatt ////////////////////////////////////////////////////////
        // 0008 - 000b              (8-11)
        {4,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_gattServiceUUID), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_serviceChangeCharVal),(u8*)(&my_characterUUID),         (u8*)(my_serviceChangeCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof (serviceChangeVal), (u8*)(&serviceChangeUUID),     (u8*)(&serviceChangeVal), 0},
        {0,ATT_PERMISSIONS_RDWR,2,sizeof (serviceChangeCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(serviceChangeCCC), 0},


        /////////////////////////////////// 3. Device Information Service //////////////////////////////////
        // 000c - 0014
        // PnP ID                   (12-14)
        {9,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_devServiceUUID), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_PnCharVal),(u8*)(&my_characterUUID), (u8*)(my_PnCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof (my_PnPtrs),(u8*)(&my_PnPUUID), (u8*)(my_PnPtrs), 0},
        
        // firmware revision string (15-16)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_FWRevCharVal),(u8*)(&my_characterUUID), (u8*)(my_FWRevCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof (my_FWRevtrs),(u8*)(&my_FWRevUUID), (u8*)(my_FWRevtrs), 0},

        // software revision string (17-18)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_SWRevCharVal),(u8*)(&my_characterUUID), (u8*)(my_SWRevCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof (my_SWRevtrs),(u8*)(&my_SWRevUUID), (u8*)(my_SWRevtrs), 0},

        // hardware revision string (19-20)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_HWRevCharVal),(u8*)(&my_characterUUID), (u8*)(my_HWRevCharVal), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof (my_HWRevtrs),(u8*)(&my_HWRevUUID), (u8*)(my_HWRevtrs), 0},
        

        /////////////////////////////////// 4. HID Service /////////////////////////////////////////////////
        // 0015 - 002F
        // primary                  (21)
        {HID_CONTROL_POINT_DP_H - HID_PS_H + 1, ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_hidServiceUUID), 0},

        // include battery service  (22)
        {0,ATT_PERMISSIONS_READ,2,sizeof(include),(u8*)(&hidIncludeUUID),     (u8*)(include), 0},

        // protocol mode            (23-24)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidProtocolModeCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidProtocolModeCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_RDWR,2, sizeof(protocolMode),(u8*)(&hidProtocolModeUUID),     (u8*)(&protocolMode), 0},    //value

        // boot keyboard input report (char-val-client)   (25-27)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidbootKeyInReporCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidbootKeyInReporCharVal), 0},            //prop
        {0,ATT_PERMISSIONS_READ,2,sizeof(bootKeyInReport),(u8*)(&hidbootKeyInReportUUID),     (u8*)(&bootKeyInReport), 0},    //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(bootKeyInReportCCC),(u8*)(&clientCharacterCfgUUID),     (u8*)(bootKeyInReportCCC), 0},    //value

        // boot keyboard output report (char-val)         (28-29)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidbootKeyOutReporCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidbootKeyOutReporCharVal), 0},          //prop
        {0,ATT_PERMISSIONS_RDWR,2, sizeof(bootKeyOutReport), (u8*)(&hidbootKeyOutReportUUID),     (u8*)(&bootKeyOutReport), 0},    //value

        // consume report in: 4 (char-val-client-ref)     (30-33)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportCCinCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportCCinCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_READ,2, sizeof(reportConsumerControlIn),(u8*)(&hidReportUUID),     (u8*)(reportConsumerControlIn), 0},    //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportConsumerControlInCCC),(u8*)(&clientCharacterCfgUUID),     (u8*)(reportConsumerControlInCCC), 0},    //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportRefConsumerControlIn),(u8*)(&reportRefUUID),     (u8*)(reportRefConsumerControlIn), 0},    //value

        // keyboard report in : 4 (char-val-client-ref)   (34-37)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportKEYinCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportKEYinCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_READ,2, sizeof(reportKeyIn),(u8*)(&hidReportUUID),     (u8*)(reportKeyIn), 0},    //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportKeyInCCC),(u8*)(&clientCharacterCfgUUID),     (u8*)(reportKeyInCCC), 0},    //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportRefKeyIn),(u8*)(&reportRefUUID),     (u8*)(reportRefKeyIn), 0},    //value

        // keyboard report out: 3 (char-val-ref)          (38-40)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportKEYoutCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportKEYoutCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportKeyOut),(u8*)(&hidReportUUID),     (u8*)(reportKeyOut), 0},    //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(reportRefKeyOut),(u8*)(&reportRefUUID),     (u8*)(reportRefKeyOut), 0},    //value
 
        // report map: 3            (41-43)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidReportMapCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidReportMapCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_READ,2,sizeof(reportMap),(u8*)(&hidReportMapUUID),     (u8*)(reportMap), 0},    //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(extServiceUUID),(u8*)(&extReportRefUUID),     (u8*)(&extServiceUUID), 0},    //value

        // hid information: 2       (44-45)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidinformationCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidinformationCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_READ,2, sizeof(hidInformation),(u8*)(&hidinformationUUID),     (u8*)(hidInformation), 0},    //value

        // control point: 2         (46-47)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_hidCtrlPointCharVal),(u8*)(&my_characterUUID), (u8*)(my_hidCtrlPointCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_WRITE,2, sizeof(controlPoint),(u8*)(&hidCtrlPointUUID),     (u8*)(&controlPoint), 0},    //value


        /////////////////////////////////// 5. Battery Service /////////////////////////////////////////////
        // 0030 - 0033              (48-51)
        {4,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_batServiceUUID), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_batCharVal),(u8*)(&my_characterUUID), (u8*)(my_batCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_batVal),(u8*)(&my_batCharUUID),     (u8*)(my_batVal), 0},    //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(batteryValueInCCC),(u8*)(&clientCharacterCfgUUID),     (u8*)(batteryValueInCCC), 0},    //value


        /////////////////////////////////// 6. OTA /////////////////////////////////////////////////////////
        // 0034 - 0038              (52-56)
        {5,ATT_PERMISSIONS_READ, 2,16,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_OtaServiceUUID), 0},
        {0,ATT_PERMISSIONS_READ, 2, sizeof(my_OtaCharVal),(u8*)(&my_characterUUID), (u8*)(my_OtaCharVal), 0},                //prop
        {0,ATT_PERMISSIONS_RDWR,16,sizeof(my_OtaData),(u8*)(&my_OtaUUID),    (&my_OtaData), &app_ota_write, 0},//&otaRead},            //value
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(my_Otaccc),(u8*)(&clientCharacterCfgUUID),(u8*)(&my_Otaccc), &att_ccc_control},
        {0,ATT_PERMISSIONS_READ, 2,sizeof (my_OtaName),(u8*)(&userdesc_UUID), (u8*)(my_OtaName), 0},


        /////////////////////////////////// 7. Audio ///////////////////////////////////////////////////////
        /********************************************************************************************
        * GOOGLE AUDIO service
        ********************************************************************************************/
        // 0039 - 0041
        // primary                  (57)
        {9,ATT_PERMISSIONS_READ,2,16, (u8*)(&my_primaryServiceUUID), (u8*)(&ATVV_Server_UUID), 0},

        // TX                       (58-59)
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(my_MicCharTx),(u8*)(&my_characterUUID),(u8*)(my_MicCharTx), 0},
        {0,ATT_PERMISSIONS_RDWR,16,sizeof(atv_char_tx_data),(u8*)(&ATVV_Char_UUID_TX),(u8*)(&atv_char_tx_data), &app_auido_google_callback},    //value

        // RX                       (60-62)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_MicCharRx),(u8*)(&my_characterUUID),(u8*)(my_MicCharRx), 0},
        {0,ATT_PERMISSIONS_READ,16,sizeof(atv_char_rx_data),(u8*)(&ATVV_Char_UUID_RX),(u8*)(&atv_char_rx_data), 0},
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(atv_char_rx_ccc),(u8*)(&clientCharacterCfgUUID),(u8*)(&atv_char_rx_ccc), &att_ccc_control},

        // CTRL                     (63-65)
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_MicCharCtl),(u8*)(&my_characterUUID),(u8*)(my_MicCharCtl), 0},
        {0,ATT_PERMISSIONS_READ,16,sizeof(atv_char_ctl_data),(u8*)(&ATVV_Char_UUID_CTL),(u8*)(&atv_char_ctl_data), 0},
        {0,ATT_PERMISSIONS_RDWR,2,sizeof(atv_char_ctl_ccc),(u8*)(&clientCharacterCfgUUID),(u8*)(&atv_char_ctl_ccc), &att_ccc_control},


        /////////////////////////////////// 8. IR //////////////////////////////////////////////////////////
        // 0042 - 004D              (66-77)
        {12,ATT_PERMISSIONS_READ,2,16,(u8*)(&my_primaryServiceUUID),     (u8*)(&my_IrUUID), 0},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_Ir_ProgControl_CharVal),(u8*)(&my_characterUUID), (u8*)(my_Ir_ProgControl_CharVal), 0},        //prop
        {0,ATT_PERMISSIONS_RDWR,16,0,(u8*)(&my_Ir_ProgControlUUID), 0, &app_ir_callback},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_Ir_Key_Id_CharVal),(u8*)(&my_characterUUID), (u8*)(my_Ir_Key_Id_CharVal), 0},                //prop
        {0,ATT_PERMISSIONS_RDWR,16,0,(u8*)(&my_Ir_KeyIdUUID), 0, &app_ir_callback},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_Ir_Code_CharVal),(u8*)(&my_characterUUID), (u8*)(my_Ir_Code_CharVal), 0},                    //prop
        {0,ATT_PERMISSIONS_RDWR,16,0,(u8*)(&my_Ir_CodeUUID), 0,&app_ir_callback},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_Ir_Suppress_CharVal),(u8*)(&my_characterUUID),     (u8*)(my_Ir_Suppress_CharVal), 0},            //prop
        {0,ATT_PERMISSIONS_RDWR,16,0,(u8*)(&my_Ir_SuppressUUID), 0, &app_ir_callback},
        {0,ATT_PERMISSIONS_READ,2,sizeof(my_Ir_Key_Event_CharVal),(u8*)(&my_characterUUID),    (u8*)(my_Ir_Key_Event_CharVal), 0},            //prop
        {0,ATT_PERMISSIONS_RDWR,16,0,(u8*)(&my_Ir_KeyEventUUID), 0, 0},
        {0,ATT_PERMISSIONS_RDWR,2,0,(u8*)(&clientCharacterCfgUUID), 0, &app_ir_callback},


        /////////////////////////////////// 9. TEST MODE ///////////////////////////////////////////////////
        // 004E - 0051              (78-81)
        {4, ATT_PERMISSIONS_READ, 2, 16, (u8*)(&my_primaryServiceUUID), (u8*)(&my_TestModeServiceUUID), 0},
        {0, ATT_PERMISSIONS_READ, 2, sizeof(my_TestCharVal), (u8*)(&my_characterUUID), (u8*)(my_TestCharVal), 0},        //prop
        {0, ATT_PERMISSIONS_WRITE, 16, sizeof(my_TestData), (u8*)(&my_TestDataUUID), (&my_TestData), &test_write_cb, 0}, //value
        {0, ATT_PERMISSIONS_RDWR, 2, sizeof(my_TestDataCCC), (u8*)(&clientCharacterCfgUUID), (u8*)(my_TestDataCCC), 0},

    };
#endif

#endif

void my_att_init (void)
{
#if (MP_TEST_MODE)
    if (test_get_mode() == MODE_TEST) bls_att_setAttributeTable((u8 *)my_test_dome_Attributes);
    else
#endif
    bls_att_setAttributeTable ((u8 *)my_Attributes);
}
#include "../../version.h"
void init_device_info (void)
{
    //u8 my_FWRevtrs []     = {'V','1','.','1','.','1'};
    //u8 my_SWRevtrs []     = {'V','1','.','0','2'};
    my_FWRevtrs[1] = ((FW_VERSION>>28)&0x0f) + 48;
    //my_FWRevtrs[3] = ((FW_VERSION>>24)&0x0f) + 48;
    //my_FWRevtrs[5] = ((FW_VERSION>>20)&0x0f) + 48;

    my_SWRevtrs[1] = ((FW_VERSION>>8)&0x0f) + 48;
    my_SWRevtrs[3] = ((FW_VERSION>>4)&0x0f) + 48;
    my_SWRevtrs[4] = (FW_VERSION&0x0f) + 48;
}
