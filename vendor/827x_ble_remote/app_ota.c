/******************************************************************************
 * @file     app_ota.c
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
#include "app_ota.h"
#include "sha256.h"
#include "app_ecdsa.h"
#include "stack/ble/ble.h"
#include "battery_check.h"
#include "stack/ble/service/ble_ll_ota.h"

#define APP_AUTHENTICATION_ADDR           0x78100
#define APP_ENCRYPTION_ADDR               0x78200
#define APP_ECDSA_PUBKEY_ADDR             0x78300

#define FW_ENC_FLAG                       0x5a
#define FW_ENC_INDEX                      0x01

#define FW_SIGN_FLAG                      0x5b
#define FW_SIGN_PROTECT_FLAG              0x5c
#define FW_SIGN_INDEX                     0x01

#define SIGN_PUBLIC_KEY 0x71,0xc9,0x86,0x93,0xcf,0x3e,0x01,0xcd,0x81,0x96,0x56,0x32,0x1d,0x2c,0x47,0xc2,\
                        0xfb,0x35,0x63,0xb6,0xed,0xe1,0x79,0xf7,0xd8,0xef,0x3d,0x20,0xcc,0x40,0x97,0x97,\
                        0x32,0x51,0xc5,0x38,0xa1,0x08,0xe3,0xeb,0xb0,0xbd,0xa7,0x0c,0x42,0x96,0x13,0xf8,\
                        0x2a,0xcd,0x5f,0x77,0xb3,0x02,0x7f,0xac,0xb5,0x13,0x05,0x69,0x8f,0xf7,0xf4,0x42

#define CRC32_TABLE     0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,\
                        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,\
                        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,\
                        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c

#define AUTH_PLAINTEXT  0xc2,0xca,0xca,0xc2,0xc9,0xc0,0xe2,0x94,0x95,0xf7,0xe6,0xf0,0x00,0x00,0x00,0x00     //googleG10RCU
#define AUTH_KEY        0x95,0x94,0x97,0x96,0x91,0x90,0x93,0x92,0x9d,0x9c,0xc4,0xc7,0xc6,0xc1,0xc0,0xc3     //0123456789abcdef

#define FW_DEC_KEY      0x3d,0x35,0x35,0x3d,0x36,0x3f,0x1c,0x0d,0x3f,0x34,0x39,0x31,0x3f,0x23,0x6a,0x6a     //googleFWenckey00

#define OTA_PRE_PROCESS_TIMEOUT           5000 //ms

#define DEBUG                             printf
#define DEBUG_ARRY                        array_printf

extern int ota_adr_index;

extern void ota_set_flag();
extern void ota_save_data(u32 adr, u8 * data);
extern unsigned long crc32_half_cal(unsigned long crc, unsigned char* input, unsigned long* table, int len);

/*****************************************************************
******************       Type definition        ******************
******************************************************************/
typedef enum{
    OTA_ENC_INDEX=0,
    OTA_SIGN_INDEX,
}ota_index_type_t;

typedef struct {
    u8 fw_dec:1;

    u8 fw_sign:1;
    u8 sign_protect:1;
    u8 ecdsa_en:1;

    u8 ota_pause:1;
}ota_flags_t;

typedef struct {
    ota_flags_t flags;
    ota_status_type_t status;
    u32 new_fw_size;
    u32 new_fw_last_index;
    u8 ota_random[4];
}ota_ctl_t;

typedef struct {
    u8 index;
    u8 sign_data[64];
    u8 pub_key[64];
}ota_sign_t;

typedef struct {
    u8 index;
    u8 dec_key[16];
}ota_dec_t;

/*****************************************************************
******************    Variable definition      ******************
******************************************************************/

_attribute_data_retention_ static ota_ctl_t ota_ctl = {
        flags:{0},
        status:0,
        new_fw_size:64,
        new_fw_last_index:64,
        ota_random:{0},
};

_attribute_data_retention_ static ota_sign_t ota_sign = {
        index:0,
        sign_data:{0},
        pub_key:{SIGN_PUBLIC_KEY},
};

_attribute_data_retention_ static ota_dec_t ota_dec = {
        index:0,
        dec_key:{FW_DEC_KEY},
};

static const unsigned long crc32_half_tbl[16] = {CRC32_TABLE};

//static struct tc_sha256_state_struct hash_sha256;
static mbedtls_sha256_context hash_sha256;

//_attribute_data_retention_  static u8 ota_random[4] = {0};
//_attribute_data_retention_  static u16 ota_ctl_flag = 0;
_attribute_data_retention_  static u16 ota_notify_hdl = 0;
_attribute_data_retention_  static u32 ota_pre_proc_tick = 0;


/*****************************************************************
******************        Weak function        ******************
******************************************************************/

u8 app_ota_ver_check(void) __attribute__((weak, alias("app_ota_default_ver_check")));
u8 app_ota_authentication(void) __attribute__((weak, alias("app_ota_default_authentication")));
u8 app_enter_ota_mode(u8 *data) __attribute__((weak, alias("app_default_enter_ota_mode")));
void app_ota_get_plaintext_key(u8 *plaintext,u8 *cus_auth_key) __attribute__((weak, alias("app_ota_get_default_plaintext_key")));
void app_ota_get_fw_decryption_key(u8 *dec_key) __attribute__((weak, alias("app_ota_get_default_fw_decryption_key")));
void app_ota_get_ecdsa_pub_key(u8 *pub_key) __attribute__((weak, alias("default_func3")));
void app_ota_decryption(u8 *data) __attribute__((weak, alias("app_ota_default_decryption")));
void app_fw_hash(u16 index,u8 *data,u8 len) __attribute__((weak, alias("app_default_fw_hash")));
void app_get_fw_hash(u8 *hash)  __attribute__((weak, alias("app_default_get_fw_hash")));
void app_ota_timeout(void) __attribute__((weak, alias("app_ota_default_timeout")));
void app_ota_result(u8 result) __attribute__((weak, alias("app_ota_default_result")));
void app_schedule_ota(u8 *data) __attribute__((weak, alias("default_func3")));


/*****************************************************************
******************             function        ******************
******************************************************************/
void app_ota_flag_check(u16 index,u8 *data);
void app_flash_fw_hash(u32 addr,u32 fw_size);

void bls_ota_init_handle(u16 handle){

    ota_notify_hdl = handle;
}

static void save_ota_index(ota_index_type_t type,u8 index){

    switch (type)
    {
    case OTA_ENC_INDEX:
        ota_dec.index = index;
        break;
    case OTA_SIGN_INDEX:
        ota_sign.index = index;
        break;
    default:
        break;
    }
}

u8 app_ota_get_dec_index(){

    return ota_dec.index;
}

u8 app_ota_get_sign_index(){

    return ota_sign.index;
}

void app_ota_set_en_ecdsa(u8 en){

    ota_ctl.flags.ecdsa_en = en?1:0;
}

ota_status_type_t app_get_ota_status(){

    return ota_ctl.status;
}

static void app_set_ota_status(ota_status_type_t status){

    ota_ctl.status = status;
}

static void init_ota_parameters(){

    ota_ctl.flags.fw_dec = 0;
    ota_ctl.flags.fw_sign = 0;
    ota_ctl.flags.sign_protect = 0;
    ota_ctl.flags.ota_pause = 0;
}

void reset_ota(){

    app_set_ota_status(IDLE_MODE);
    init_ota_parameters();

    ota_pre_proc_tick = 0;
    blt_ota_start_tick = 0;
}


int app_ota_write(void * p){

    rf_packet_att_data_t *req = (rf_packet_att_data_t*)p;
    u8 count = ((req->l2cap - 3)/20)?(req->l2cap - 3)/20:1;

    for(u8 i=0;i<count;i++){
        int err_flg = OTA_SUCCESS;
        u8 *data;
        data = req->dat + i*20;
        u16 ota_adr =  data[0] | (data[1]<<8);
        u16 crc;

//        DEBUG("data addr:0x%x\n",ota_adr);
//        DEBUG_ARRY(data,18);

        if(ota_adr == CMD_OTA_SCHEDULE){

            app_schedule_ota(data + 2);

        }else if(ota_adr == CMD_OTA_FW_VERSION){

            if(!ota_notify_hdl) return 0 ;

            reset_ota();

            if(app_ota_ver_check()) return 0;

            ota_pre_proc_tick = clock_time()|1;
            app_set_ota_status(OTA_VER_CHECK);

        }else if(ota_adr == CMD_OTA_AUTHENTICATION){

            if(app_get_ota_status() != OTA_VER_CHECK) return 0;
            app_ota_authentication();

            ota_pre_proc_tick = clock_time()|1;
            app_set_ota_status(OTA_AUTHENTICATION);

        }else if(ota_adr == CMD_OTA_START){
            //if((ota_ctl_flag & 0x07) != 0x07) return 0;//check FW_VER_CHECK & BATT_CHECK_OK &  OTA_AUTHENTICATION
            if(app_get_ota_status() != OTA_AUTHENTICATION) return 0;

            if(app_enter_ota_mode(data + 2)){

                reset_ota();
                return 0;
            }

            app_set_ota_status(OTA_PROCESS);
            init_ota_parameters();

            ota_pre_proc_tick = 0;
            blt_ota_start_tick = clock_time()|1;  //mark time
            ota_adr_index = -1;

    #if (BLE_OTA_FW_CHECK_EN)
            blcOta.fw_check_en = 0;
            blcOta.fw_crc_last_index = (FW_MAX_SIZE>>4);
    #endif
            //log_event(TR_T_ota_start);
        }else if(ota_adr == CMD_OTA_END){


            //log_event(TR_T_ota_end);
            if(app_get_ota_status() != OTA_PROCESS) return 0;
            //if((ota_ctl_flag & 0x0f) != 0x0f) return 0; //check FW_VER_CHECK & BATT_CHECK_OK & OTA_AUTHENTICATION & OTA_START
            DEBUG("CMD_OTA_END\n");
            u16 adrIndex_max       = data[2] | (data[3]<<8);
            u16 adrIndex_max_check = data[4] | (data[5]<<8);


            if(ota_ctl.flags.fw_sign ||ota_ctl.flags.sign_protect ){
                err_flg = OTA_SIGNATURE_ERR;
            }

    #if (BLE_OTA_FW_CHECK_EN)
            if(blcOta.fw_check_en && !blcOta.fw_check_match){
                err_flg = OTA_FW_CHECK_ERR;
            }
    #endif

            //if no index_max check, set ota success directly, otherwise we check if any index_max match
            if( req->l2cap == 9 && (adrIndex_max ^ adrIndex_max_check) == 0xffff){  //index_max valid, we can check
                if(adrIndex_max != ota_adr_index){  //last one or more packets missed
                    err_flg = OTA_DATA_UNCOMPLETE;
                }
            }

            if(err_flg == OTA_SUCCESS){

                app_ota_result(OTA_SUCCESS);
                ota_set_flag ();
                start_reboot();
            }

        }else{

            //log_task_begin(TR_T_ota_data);
            if(app_get_ota_status() != OTA_PROCESS) return 0;

            if(ota_adr_index + 1 == ota_adr){   //correct OTA data index
                crc = (data[19]<<8) | data[18];
                if(crc == crc16(data, 18)){

                    /* fw dec */
                    if(ota_ctl.flags.fw_dec) app_ota_decryption(data+2); //fw_dec

                    app_ota_flag_check(ota_adr,data+2);
                    //app_ctl_signature(ota_adr,data+2);

                    /* fw hash */
                    if(ota_adr < 2){
                        app_fw_hash(ota_adr,data+2,16);
                    }else if(ota_adr < ota_ctl.new_fw_last_index){//firmware data
                        if(ota_ctl.flags.fw_sign) app_fw_hash(ota_adr,data+2,16);
                    }else if(ota_adr == ota_ctl.new_fw_last_index){//last firmware data(crc)
                        if(ota_ctl.flags.fw_sign) app_fw_hash(ota_adr,data+2,4);
                    }else if(ota_adr <= (ota_ctl.new_fw_last_index + 4)){
                        if(ota_ctl.flags.fw_sign || ota_ctl.flags.sign_protect){
                            //save sign data
                            u8 shift = ota_adr - ota_ctl.new_fw_last_index - 1;
                            memcpy(ota_sign.sign_data + shift*16,data+2,16);
                            ota_adr_index = ota_adr;
                        }
                    }

                    //ECDSA
                    if(ota_adr == ota_ctl.new_fw_last_index + 4){

                        if(ota_ctl.flags.fw_sign || ota_ctl.flags.sign_protect){

                            if(!ota_ctl.flags.fw_sign && ota_ctl.flags.sign_protect){
                                u32 new_fw_addr_offset = ota_program_offset?ota_program_bootAddr:0;
                                app_flash_fw_hash(new_fw_addr_offset,ota_ctl.new_fw_size);
                            }

                            u8 hash[32] = {0};
                            app_get_fw_hash(hash);

                            u8 sign_res = ecdsa_verify(secp256r1, hash, 32, ota_sign.pub_key, ota_sign.sign_data);
                            if(ECDSA_SUCCESS == sign_res){
                                ota_ctl.flags.fw_sign = 0;
                                ota_ctl.flags.sign_protect = 0;
                                DEBUG("ECDSA OK!!!!!!\n");

                            }else{
                                DEBUG("ota_sign_ERROR:%d\n",sign_res);
                            }
                        }
                    }

                    if(ota_adr> ota_ctl.new_fw_last_index) continue;
    #if(BLE_OTA_FW_CHECK_EN)

                    int fw_check_err = 0;

                    //////////////��ȡOTA���ݵ�adr_0x06~adr_0x07���ж��Ƿ���Ҫ��fw check ////////////
                    if(!ota_adr){  //��һ��OTA����

                        if(data[8] == FLAG_FW_CHECK && data[9] == FW_CHECK_AGTHM2){ //adr_0x06��ֵΪ0x5D��adr_0x07��ֵΪ0x02
                            blcOta.fw_check_en = 1;

                            DEBUG_ARRY(data+2,16);
                        }
                        //else if(data[8] == 0){ //adr_0x06 is 0, no need check
                        //    blcOta.fw_check_en = 0;
                        //}
                        else{ //adr_0x06 is unexpected data, firmware is err
                            blcOta.fw_check_en = 0;
                            fw_check_err = 1;
                        }

                        blcOta.fw_crc_init = 0xFFFFFFFF;  //crc init set to 0xFFFFFFFF
                    }

                    if(blcOta.fw_check_en && ota_adr<=blcOta.fw_crc_last_index){
                        if(ota_adr == 1){ //��2�����ݣ���adr_0x18 ~ adr_0x1b ��ȡfw_size
                            u32 fw_size = data[10] | data[11]<<8 | data[12]<<16 | data[13]<<24;
                            ota_ctl.new_fw_size = fw_size;
                            ota_ctl.new_fw_last_index = fw_size>>4;

                            //firmware size none 0, and smaller than 256k, and (fw_size % 16 == 4)
                            if(fw_size && (fw_size < FW_MAX_SIZE) && ((fw_size & 0x0f) == 4) ){
                                blcOta.fw_crc_last_index = (fw_size>>4) - 1;
                            }
                            else{ //adr_0x18 ~ adr_0x1b firmware size err
                                blcOta.fw_crc_last_index = (FW_MAX_SIZE>>4);
                                blcOta.fw_check_en = 0;
                                fw_check_err = 1;
                            }

                            //check ecdsa flag
                            if(ota_ctl.flags.ecdsa_en && !ota_ctl.flags.fw_sign){
                                fw_check_err = 1;
                            }

                            DEBUG_ARRY(data+2,16);
                        }

                        u8 ota_dat[32];
                        for(int i=0;i<16;i++){
                            ota_dat[i*2] = data[i+2]&0x0f;
                            ota_dat[i*2+1] = data[i+2]>>4;
                        }
                        blcOta.fw_crc_init = crc32_half_cal(blcOta.fw_crc_init, ota_dat, (unsigned long* )crc32_half_tbl, 32);
                    }

                    if(blcOta.fw_check_en && ota_adr == (blcOta.fw_crc_last_index + 1) ){  //�Ƚ�checkֵ
                        u32 fw_check_value = data[2] | data[3]<<8 | data[4]<<16 | data[5]<<24;
                        if(fw_check_value == blcOta.fw_crc_init){  //crc match
                            blcOta.fw_check_match = 1;
                        }
                        else{
                            fw_check_err = 1;
                        }
                    }


                    if(fw_check_err){
                        err_flg = OTA_FW_CHECK_ERR;
                    }
                    else{
                        //log_data(TR_24_ota_adr,ota_adr);
                        ota_save_data (ota_adr<<4, data + 2);

                        u8 flash_check[16];

                        flash_read_page(ota_program_offset + (ota_adr<<4),16,flash_check);

                        if(!memcmp(flash_check,data + 2,16)){  //OK
                            ota_adr_index = ota_adr;
                        }
                        else{ //flash write err
                            err_flg = OTA_WRITE_FLASH_ERR;
                        }
                    }

    #else
                    //log_data(TR_24_ota_adr,ota_adr);
                    ota_save_data (ota_adr<<4, req->dat + 2);

                    u8 flash_check[16];

                    flash_read_page(ota_program_offset + (ota_adr<<4),16,flash_check);

                    if(!memcmp(flash_check,req->dat + 2,16)){  //OK
                        ota_adr_index = ota_adr;
                    }
                    else{ //flash write err
                        err_flg = OTA_WRITE_FLASH_ERR;
                    }

    #endif
                }
                else{  //crc err
                    err_flg = OTA_DATA_CRC_ERR;
                }
            }
            else if(ota_adr_index >= ota_adr){  //maybe repeated OTA data, we neglect it, do not consider it ERR

            }
            else{  //adr index err, missing at least one OTA data
                err_flg = OTA_PACKET_LOSS;
                DEBUG("OTA_PACKET_LOSS ota_adr:0x%x\n",ota_adr);
            }
            //log_task_end(TR_T_ota_data);
        }

        if(err_flg){
            //log_event(TR_T_ota_err);

            app_ota_result(err_flg);   //OTA fail indicate

            if(ota_adr_index>=0){
                irq_disable();

                //for(int i=0;i<=ota_adr_index;i+=256)
                for(int i=(ota_adr_index&0x3ff00); i>=0; i-=256) //erase from end to head
                {  //4K/16 = 256
                    flash_erase_sector(ota_program_offset + (i<<4));
                }
            }
            start_reboot();
        }

    }
    return 0;
}


static void ota_reboot(){

    if(blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){
        u16 i = 0;
        while(blc_ll_getTxFifoNumber() &&  i < 400){
            sleep_us(10000);//DEBUG("waiting TX fifo is empty\n"); //waiting TX fifo is empty
            i++;
        }
    }

    if(ota_adr_index>=0){
        irq_disable();

        //for(int i=0;i<=ota_adr_index;i+=256)
        for(int i=(ota_adr_index&0x3ff00); i>=0; i-=256){ //erase from end to head
            //4K/16 = 256
            flash_erase_sector(ota_program_offset + (i<<4));
        }
    }

    start_reboot();
}

void app_ota_proc(void){

    switch(app_get_ota_status())
    {
    case OTA_VER_CHECK:
    case OTA_AUTHENTICATION:
        if(ota_pre_proc_tick && clock_time_exceed(ota_pre_proc_tick , OTA_PRE_PROCESS_TIMEOUT*1000)){
            DEBUG("reset_ota!!!!!!!!!!!!!!!!!!\n");
            reset_ota();
        }
        break;
    case OTA_PROCESS:
        if(blt_ota_start_tick && clock_time_exceed(blt_ota_start_tick , blt_ota_timeout_us)){

            reset_ota();
            app_ota_timeout();
        }
        break;
    case IDLE_MODE:
        break;
    default:
        break;
    }
}

void app_ota_terminate(u8 reason){

    app_ota_result(reason);
    ota_reboot();
}


/*****************************************************************
************    Authentication related functions   ***************
******************************************************************/
void bls_ota_set_random(u8 *random){

    //memcpy(ota_random,random,4);
    ota_ctl.ota_random[0] = random[1] ^ random[2] ^ random[3];
    ota_ctl.ota_random[1] = random[2] ^ random[3] ^ random[0];
    ota_ctl.ota_random[2] = random[3] ^ random[0] ^ random[1];
    ota_ctl.ota_random[3] = random[0] ^ random[1] ^ random[2];
}

void bls_ota_get_random(u8 *random){

    memcpy(random,ota_ctl.ota_random,4);
}

/****************************************************
 *  OTA pause and resume related functions
 ****************************************************/
static char app_ota_flow_control(u8 data){

    if(app_get_ota_status() != OTA_PROCESS)return -2;    // ota not started

    u8 report_data[3] = {CLIENT_REPORT_CMD,TYPE_CONTROL_DATA,data};

    if(ota_notify_hdl){

        ble_sts_t ret = bls_att_pushNotifyData(ota_notify_hdl, report_data, 3);

        if(ret == BLE_SUCCESS){
            if(data == OTA_CTRL_PAUSE){
                ota_ctl.flags.ota_pause = 1;    //set ota pause flag
            }else if(data == OTA_CTRL_CONTINUE){
                ota_ctl.flags.ota_pause = 0;    //clear ota pause flag
            }else{

            }
        }
        return ret; //0 is success
    }
    return -1;    // ota_notify_hdl is empty
}

char app_ota_pause(){

    return app_ota_flow_control(OTA_CTRL_PAUSE);
}

char app_ota_continue(){

    return app_ota_flow_control(OTA_CTRL_CONTINUE);
}

u8 is_ota_paused(){

    return ota_ctl.flags.ota_pause;
}


static void app_read_flash_data(u32 addr,u8 *data,u8 len){

    u8 buf[64] = {0x00};
    u8 buf_ff[64] = {0x00};
    memset(buf_ff,0xff,len);
    flash_read_page(addr,len,buf);

    if(memcmp(buf,buf_ff,len)){
        memcpy(data,buf,len);   //if flash data is not all 0xff.use flash data to decrypt FW
    }
}


void app_ota_check_dec_flag(u8 *data){

    switch (data[0])
    {
    case FW_ENC_FLAG:
        DEBUG("encryption fw\n");
        ota_ctl.flags.fw_dec = 1;   //firmware decryption flag
        save_ota_index(OTA_ENC_INDEX,data[1]);  //save decryption method index

        data[0] = 0;    //clear data to 0x00
        data[1] = 0;    //clear data to 0x00

        app_ota_get_fw_decryption_key(ota_dec.dec_key); //set default dec_key
        app_read_flash_data(APP_ENCRYPTION_ADDR,ota_dec.dec_key,16); //if flash data is not all 0xff.use flash data to decrypt FW

        DEBUG_ARRY(ota_dec.dec_key,16);
        break;
    case 0:
        //original bin
        break;
    default:
        //unknown bin
        break;
    }
}

void app_ota_check_sign_flag(u8 *data){

    switch (data[0])
    {
    case FW_SIGN_FLAG:
        DEBUG("signature fw\n");
        ota_ctl.flags.fw_sign = 1;   //firmware signature flag
        save_ota_index(OTA_SIGN_INDEX,data[1]);      //save signature method index

        app_ota_get_ecdsa_pub_key(ota_sign.pub_key); //set default dec_key
        app_read_flash_data(APP_ECDSA_PUBKEY_ADDR,ota_sign.pub_key,64); //if flash data is not all 0xff.use flash data to decrypt FW

        data[0] = 0;
        data[1] = 0;
        break;
    case FW_SIGN_PROTECT_FLAG:
        DEBUG("signature protect fw\n");
        ota_ctl.flags.sign_protect = 1;   //firmware signature protect flag
        ota_sign.index = data[1];         //save signature method index
        data[0] = 0;
        data[1] = 0;
        break;
    case 0:
        //original bin
        break;
    default:
        //unknown bin
        break;
    }
}

void app_ota_flag_check(u16 index,u8 *data){

    switch (index)
    {
    case 0:
        break;
    case 1:
        app_ota_check_dec_flag(data+0x0c);
        app_ota_check_sign_flag(data+0x0e);
        break;
    default:
        if(index == ota_ctl.new_fw_last_index){
            ota_ctl.flags.fw_dec = 0;
            app_ota_check_sign_flag(data+0x04);
        }
        break;
    }
}


void app_flash_fw_hash(u32 addr,u32 fw_size){

    DEBUG("app_flash_fw_hash()\n");
    u8 buf[16];
    u32 last_index = fw_size/16;
    mbedtls_sha256_init(&hash_sha256);
    mbedtls_sha256_starts(&hash_sha256,0);
    flash_read_page(addr,16,buf);
    buf[8] = 0x4b;
    mbedtls_sha256_update(&hash_sha256,buf, 16);;

    for(u16 i=1;i<last_index;i++){
        flash_read_page(addr+i*16,16,buf);
        mbedtls_sha256_update(&hash_sha256,buf, 16);;
    }

    u8 mod = fw_size%16;
    flash_read_page(addr+last_index*16,mod,buf);
    app_fw_hash(last_index,buf,mod);
}


/****************************************************
 *  Some default functions
 ****************************************************/
static void app_ota_default_timeout(){

    app_ota_terminate(OTA_TIMEOUT);
}

static u8 app_ota_default_ver_check(){

    //notify software version
    u8 fw_version[6] = {0};

    flash_read_page(ota_program_offset?0:ota_program_bootAddr,8,fw_version);
    fw_version[0] = CLIENT_REPORT_CMD;
    fw_version[1] = TYPE_VERSION;
    ble_sts_t ret = bls_att_pushNotifyData(ota_notify_hdl, fw_version, 6);

    if(ret) return ret;

    //check battery voltage
    u16 ota_battery_voltage = get_battery_voltage();

    if(ota_battery_voltage < OTA_PROTECT_VOLTAGE){

        u8 report_data[3] = {CLIENT_REPORT_CMD,TYPE_OTA_RESULT,OTA_LOW_VOLTAGE};

        ret = bls_att_pushNotifyData(ota_notify_hdl, report_data, 3);

        return ret?ret:1;   //low battery
    }

    return 0;   //ok
}

static void app_ota_get_default_plaintext_key(u8 *plaintext,u8 *cus_auth_key){

    u8 aes_plaintext[16] = {AUTH_PLAINTEXT};
    u8 authentication_key[16] = {AUTH_KEY};

    for(u8 i=0;i<12;i++) aes_plaintext[i] = aes_plaintext[i]^0xA5;
    for(u8 i=0;i<16;i++) authentication_key[i] = authentication_key[i]^0xA5;

    memcpy(plaintext,aes_plaintext,16);
    memcpy(cus_auth_key,authentication_key,16);
}


static u8 app_ota_default_authentication(){

    u8 notify_data[20] = {0};
    u8 ota_plaintext[16] = {0};
    u8 ota_auth_key[16] = {0};
    u8 buffer[32];
    u8 buf_ff[16] = {0x00};

    app_ota_get_plaintext_key(ota_plaintext,ota_auth_key);

    flash_read_page(APP_AUTHENTICATION_ADDR, 32, buffer);
    memset(buf_ff,0xff,16);
    if(memcmp(buffer,buf_ff,12))    memcpy(ota_plaintext,buffer,12);
    if(memcmp(buffer+16,buf_ff,16)) memcpy(ota_auth_key,buffer+16,16);

    bls_ota_get_random(ota_plaintext+12);

    aes_ll_encryption(ota_auth_key,ota_plaintext,notify_data+4);
    notify_data[2] = CLIENT_REPORT_CMD;
    notify_data[3] = TYPE_SINATURE_DATA;

    return bls_att_pushNotifyData (ota_notify_hdl, notify_data+2, 18);
}

static u8 app_default_enter_ota_mode(u8 *data){

    u8 check_data[4] = {0};
    bls_ota_get_random(check_data);
    if(memcmp(check_data,data,4 )){

        u8 report_data[3] = {CLIENT_REPORT_CMD,TYPE_OTA_RESULT,OTA_START_KEY_ERR};

        ble_sts_t ret = bls_att_pushNotifyData(ota_notify_hdl, report_data, 3);

        return ret?ret:1;
    }

    //add user code
    bls_ota_setTimeout(90 * 1000 * 1000); //set OTA timeout  90 seconds

    return 0;
}

static void AES_ECB_Decryption(u8 *key, u8 *plaintext, u8 *encrypted_data){
    u8 rplaintext[16],rkey[16];
    swapX(plaintext, rplaintext, 16);
    swapX(key, rkey, 16);
    aes_ll_decryption(rkey, rplaintext, encrypted_data);
}

static void app_ota_get_default_fw_decryption_key(u8 *dec_key){
    for(u8 i=0;i<16;i++) dec_key[i] =  ota_dec.dec_key[i]^0x5A;
}

static void app_ota_default_decryption(u8 *data){

    if(app_ota_get_dec_index() == FW_ENC_INDEX){
        u8 fw_temp_data[16] = {0};
        AES_ECB_Decryption(ota_dec.dec_key,data,fw_temp_data);
        memcpy(data,fw_temp_data,16);
    }
}

void app_default_fw_hash(u16 index,u8 *data,u8 len){

    if(!index){
        mbedtls_sha256_init(&hash_sha256);
        mbedtls_sha256_starts(&hash_sha256,0);
        mbedtls_sha256_update(&hash_sha256,data, len);
    }else{
        mbedtls_sha256_update(&hash_sha256,data, len);
    }
}

void app_default_get_fw_hash(u8 *hash){
//    mbedtls_sha256_finish(hash, &hash_sha256);
    mbedtls_sha256_finish(&hash_sha256, hash);
}


void app_ota_default_result(u8 result){

    u8 report_data[3] = {CLIENT_REPORT_CMD, TYPE_OTA_RESULT, result};

    u16 i = 0;
    while(bls_att_pushNotifyData(ota_notify_hdl, report_data, 3) &&  i < 2000){
        sleep_us(100);
        i++;
    }

    i = 0;
    while(bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN) &&  i < 2000){
        sleep_us(100);
        i++;
    }

    i = 0;
    while(blc_ll_getTxFifoNumber() &&  i < 400){
        sleep_us(10000);//DEBUG("waiting TX fifo is empty\n"); //waiting TX fifo is empty
        i++;
    }
}



static void default_func3(u8 *data){

}
