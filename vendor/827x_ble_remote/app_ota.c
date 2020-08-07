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
#include "stack/ble/ble.h"
#include "battery_check.h"
#include "stack/ble/service/ble_ll_ota.h"

extern u8  mac_public[6];

extern void ota_set_flag();
extern void ota_save_data(u32 adr, u8 * data);

extern int ota_adr_index;

extern ota_startCb_t		otaStartCb;
extern ota_versionCb_t 	otaVersionCb;
extern ota_resIndicateCb_t otaResIndicateCb;



_attribute_data_retention_  u8  ota_ctl_flag = 0;


static const unsigned long crc32_half_tbl[16] = {
	0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
	0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
	0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
	0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

void google_ota_pause(){
	printf("google_ota_pause\n");
	gpio_write(GPIO_LED2, 0);

	u8 report_data[3] = {SLAVE_REPORT_CMD,TYPE_CONTROL_DATA,OTA_PAUSE};
	bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, report_data, 3);
}

void google_ota_continue(){
	printf("google_ota_pause\n");
	gpio_write(GPIO_LED2, 1);
	u8 report_data[3] = {SLAVE_REPORT_CMD,TYPE_CONTROL_DATA,OTA_CONTINUE};
	bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, report_data, 3);
}

int google_ota_write(void * p)
{
	rf_packet_att_data_t *req = (rf_packet_att_data_t*)p;
	u8 count = ((req->l2cap - 3)/20)?(req->l2cap - 3)/20:1;
//	printf("count: %d len:%d  RF_data:",count,(req->l2cap - 3));
//	array_printf(req->dat,(req->l2cap - 3));

	for(u8 i=0;i<count;i++){
		int err_flg = OTA_SUCCESS;
		u8 *data;
		data = req->dat + i*20;
		u16 ota_adr =  data[0] | (data[1]<<8);
		u16 crc;
		if(ota_adr == CMD_OTA_FW_VERSION){

			ota_ctl_flag = 0;
			ota_ctl_flag |= FW_VER_CHECK;

			printf("CMD_OTA_FW_VERSION\n");
			if(otaVersionCb){
				otaVersionCb();
			}

			u8 FW_version[6] = {0};
			FW_version[0] = SLAVE_REPORT_CMD;
			FW_version[1] = TYPE_VERSION;
			FW_version[2] = *(volatile u8 *)0x02;
			FW_version[3] = *(volatile u8 *)0x03;
			FW_version[4] = *(volatile u8 *)0x04;
			FW_version[5] = *(volatile u8 *)0x05;

			bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, FW_version, 6);
			//check battery voltage
			u16 ota_battery_voltage = get_battery_voltage();
			printf("get_battery_voltage:%d\n",ota_battery_voltage);

			if(ota_battery_voltage < OTA_PROTECT_VOLTAGE){

				u8 report_data[3];
				report_data[0] = SLAVE_REPORT_CMD;
				report_data[1] = TYPE_OTA_RESULT;
				report_data[2] = OTA_LOW_VOLTAGE;

				bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, report_data, 3);
				printf("OTA_LOW_VOLTAGE\n");

				ota_ctl_flag = 0;
			}else{
				ota_ctl_flag |= BATT_CHECK_OK;
				printf("OTA_VOLTAGE_OK. 0xota_ctl_flag:%x \n",ota_ctl_flag);
			}
		}else if(ota_adr == CMD_OTA_SIGNATURE){
			if((ota_ctl_flag & 0x03) != 0x03)	return 0;	//check FW_VER_CHECK & BATT_CHECK_OK

			u8 aes_plaintext[16] = {'g','o','o','g','l','e','G','1','0','R','C','U',0x00,0x00,0x00,0x00,};
			u8 signature_key[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f',};	//OTA key;
			u8 notify_data[20] = {0};

			aes_plaintext[12] = mac_public[0] ^ mac_public[1];
			aes_plaintext[13] = mac_public[1] ^ mac_public[2];
			aes_plaintext[14] = mac_public[2] ^ mac_public[3];
			aes_plaintext[15] = mac_public[3] ^ mac_public[4];

			printf("aes_plaintext:");
			array_printf(aes_plaintext,16);
			printf("signature_key:");
			array_printf(signature_key,16);

			aes_ll_encryption(signature_key,aes_plaintext,notify_data+4);
			notify_data[2] = SLAVE_REPORT_CMD;
			notify_data[3] = TYPE_SINATURE_DATA;

			printf("notify_data:");
			array_printf(notify_data+2,18);
			bls_att_pushNotifyData (OTA_CMD_OUT_DP_H, notify_data+2, 18);

			ota_ctl_flag |= OTA_SIGNATURE;

		}else if(ota_adr == CMD_OTA_START){
			if((ota_ctl_flag & 0x07) != 0x07) return 0;	//check FW_VER_CHECK & BATT_CHECK_OK &  OTA_SIGNATURE

			gpio_write(GPIO_LED2, 1);
			printf("start data:");
			array_printf(data + 2,4);
			u8 check_data[4];

			check_data[0] = mac_public[0] ^ mac_public[1];
			check_data[1] = mac_public[1] ^ mac_public[2];
			check_data[2] = mac_public[2] ^ mac_public[3];
			check_data[3] = mac_public[3] ^ mac_public[4];

			if(memcmp(check_data,data + 2,4 )){

				u8 report_data[3];
				report_data[0] = SLAVE_REPORT_CMD;
				report_data[1] = TYPE_OTA_RESULT;
				report_data[2] = OTA_START_KEY_ERR;

				bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, report_data, 3);
				printf("OTA_START_KEY_ERR\n");

				ota_ctl_flag = 0;
				return 0;
			}

			ota_ctl_flag |= OTA_START;

			blcOta.ota_start_flag = 1;   //set flag
			blt_ota_start_tick = clock_time();  //mark time
			ota_adr_index = -1;
			if(otaStartCb){
				otaStartCb();
			}

	#if (BLE_OTA_FW_CHECK_EN)
			blcOta.fw_check_en = 0;
			blcOta.fw_crc_last_index = (FW_MAX_SIZE>>4);
	#endif
			//log_event(TR_T_ota_start);
		}
		else if(ota_adr == CMD_OTA_END){

			//log_event(TR_T_ota_end);
			if((ota_ctl_flag & 0x0f) != 0x0f) return 0; //check FW_VER_CHECK & BATT_CHECK_OK & OTA_SIGNATURE & OTA_START

			u16 adrIndex_max	   = data[2] | (data[3]<<8);
			u16 adrIndex_max_check = data[4] | (data[5]<<8);

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

			if(!err_flg){

//				u8 report_data[3];
//				report_data[0] = SLAVE_REPORT_CMD;
//				report_data[1] = TYPE_OTA_RESULT;
//				report_data[2] = OTA_SUCCESS;
//
//				bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, report_data, 3);
//				printf("TYPE_OTA_RESULT:0x%x\n", err_flg);
//				bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN);

				if(otaResIndicateCb){
					otaResIndicateCb(OTA_SUCCESS);  //OTA successed indicate
				}

//				while(blc_ll_getTxFifoNumber()){
//					sleep_us(10000);//printf("waiting TX fifo is empty\n"); //waiting TX fifo is empty
//				}

				ota_set_flag ();
				start_reboot();
			}else{
				u8 report_data[3];
				report_data[0] = SLAVE_REPORT_CMD;
				report_data[1] = TYPE_OTA_RESULT;
				report_data[2] = err_flg;

				bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, report_data, 3);
				printf("TYPE_OTA_RESULT:0x%x\n", err_flg);
			}
		}
		else{

			//log_task_begin(TR_T_ota_data);
			if((ota_ctl_flag & 0x0f) != 0x0f) return 0; //check FW_VER_CHECK & BATT_CHECK_OK & OTA_SIGNATURE & OTA_START


			if(ota_adr_index + 1 == ota_adr){   //correct OTA data index
				crc = (data[19]<<8) | data[18];
				if(crc == crc16(data, 18)){

	#if(BLE_OTA_FW_CHECK_EN)

					int fw_check_err = 0;

					//////////////读取OTA数据的adr_0x06~adr_0x07，判断是否需要做fw check ////////////
					if(!ota_adr){  //第一笔OTA数据
						printf("count: %d len:%d  RF_data:",count,(req->l2cap - 3));
						if(data[8] == FLAG_FW_CHECK && data[9] == FW_CHECK_AGTHM2){ //adr_0x06的值为0x5D，adr_0x07的值为0x02
							blcOta.fw_check_en = 1;
						}
						else if(data[8] == 0){ //adr_0x06 is 0, no need check
							blcOta.fw_check_en = 0;
						}
						else{ //adr_0x06 is unexpected data, firmware is err
							blcOta.fw_check_en = 0;
							fw_check_err = 1;
						}

						blcOta.fw_crc_init = 0xFFFFFFFF;  //crc init set to 0xFFFFFFFF
					}

					if(blcOta.fw_check_en && ota_adr<=blcOta.fw_crc_last_index){
						if(ota_adr == 1){ //第2笔数据，在adr_0x18 ~ adr_0x1b 获取fw_size
							u32 fw_size = data[10] | data[11]<<8 | data[12]<<16 | data[13]<<24;

							//firmware size none 0, and smaller than 256k, and (fw_size % 16 == 4)
							if(fw_size && (fw_size < FW_MAX_SIZE) && ((fw_size & 0x0f) == 4) ){
								blcOta.fw_crc_last_index = (fw_size>>4) - 1;
							}
							else{ //adr_0x18 ~ adr_0x1b firmware size err
								blcOta.fw_crc_last_index = (FW_MAX_SIZE>>4);
								blcOta.fw_check_en = 0;
								fw_check_err = 1;
							}
						}


						u8 ota_dat[32];
						for(int i=0;i<16;i++){
							ota_dat[i*2] = data[i+2]&0x0f;
							ota_dat[i*2+1] = data[i+2]>>4;
						}
						blcOta.fw_crc_init = crc32_half_cal(blcOta.fw_crc_init, ota_dat, (unsigned long* )crc32_half_tbl, 32);
					}




					if(blcOta.fw_check_en && ota_adr == (blcOta.fw_crc_last_index + 1) ){  //比较check值
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
			}
			//log_task_end(TR_T_ota_data);

		}




		if(err_flg){
			//log_event(TR_T_ota_err);

//			u8 report_data[3];
//			report_data[0] = SLAVE_REPORT_CMD;
//			report_data[1] = TYPE_OTA_RESULT;
//			report_data[2] = err_flg;
//
//			bls_att_pushNotifyData(OTA_CMD_OUT_DP_H, report_data, 3);
//			printf("TYPE_OTA_RESULT:0x%x\n", err_flg);
//			bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN);

			if(otaResIndicateCb){
				otaResIndicateCb(err_flg);   //OTA fail indicate
			}

//			while(blc_ll_getTxFifoNumber()){
//				sleep_us(10000);//printf("waiting TX fifo is empty\n"); //waiting TX fifo is empty
//			}

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
