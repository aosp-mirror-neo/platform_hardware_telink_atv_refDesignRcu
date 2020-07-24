/******************************************************************************
 * @file     app_info.c
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
#include "app_flash_write.h"


_attribute_data_retention_ u8 flag_ccc_data = 0;



/**********************************************************
 **** operate  flash function
 **********************************************************/

/**
 * @brief This function can make more effective use of flash.
 * @param[in]   addr the start address of the page
 * @param[in]   Number of sectors used to save this data type
 * @param[in]   Space occupied by a piece of data.Less than 256£¬
 * @return -1:No data found,others:address offset
 */
s16 bsearch_without_recursion(unsigned long addr, u8 used_sector, u8 used_block)
{
    s16 low=0, high=(4*1024/used_block)*used_sector -1;
    u8 rbuf[2];
    u8 cmp1[] = {U16_LO(FLASH_FLAG),U16_HI(FLASH_FLAG)};//{0xa5,0xff};
    u8 cmp2[] = {0x00,0x00};
    u8 cmp3[] = {0xff,0xff};
    while(low<=high)
    {
        //DEBUG("low: %d  high: %d\n",low,high);
        s16 mid=(low+high)/2;

        flash_read_page(addr + mid*used_block,2,rbuf);
        //printf("bsearchWithoutRecursion tmp:");
        //arrayPrint(tmp,2);
        //printf("\n:");
        if(!memcmp(rbuf, cmp3, 2))    high = mid - 1;
        else if(!memcmp(rbuf, cmp2, 2))    low = mid + 1;
        else if(!memcmp(rbuf, cmp1, 2))     return mid;//return mid*used_block;
        else return -1;
    }
    return -1;
}

/**
 * @brief This function can make more effective use of flash.
 * @param[in]   addr the start address of the page
 * @param[in]   Number of sectors used to save this data type
 * @param[in]   Space occupied by a piece of data.Less than 256£¬
 * @param[in]   len the length(in byte) of content needs to read out from the page
 * @param[out]  buf the start address of the buffer
 * @return 0:Success,1:flash is empty,2:sector data error
 */
u8 flash_pos_info(unsigned long addr, u8 used_sector, u8 used_block){
    u8 ret = 0;
    u8 begin_data[2];    //beginning of sector data.byte 0,1 is 1st sector data,byte 2,3 is 2nd sector data.

    s16 high=(4*1024/used_block)*used_sector -1;

    //Read the beginning and end data of the 1st sector
    flash_read_page(addr, 2, begin_data);
    //flash_read_page(addr, 2, end_data);

    u8 allff[2] = {0xff,0xff};

    /****  start check flash  ****/

    if(!memcmp(begin_data, allff, 2)){
        //beginning of 1st sector data(2byte) is all 0xff.
        //Indicates whether the RCU is brand new or factory set
        ret = 1;    //return 1;    //No data in flash
    }else{

        //search data in flash
        s16 offset = bsearch_without_recursion(addr,used_sector,used_block);
        printf("offset=%x \r\n", (u16)offset);
        if(offset == -1){

            ret = 2;    //return 2;    //sector data error
        }
        else
        {
             if(offset>(high/10*9))
                 return 3;
        }
    }

    return ret;
}


/**
 * @brief This function can make more effective use of flash.
 * @param[in]   addr the start address of the page
 * @param[in]   Number of sectors used to save this data type
 * @param[in]   Space occupied by a piece of data.Less than 256£¬
 * @param[in]   len the length(in byte) of content needs to read out from the page
 * @param[out]  buf the start address of the buffer
 * @return 0:Success,1:flash is empty,2:sector data error
 */
u8 flash_read_info(unsigned long addr, u8 used_sector, u8 used_block, unsigned long len, u8 *rbuf){
    u8 ret = 0;
    u8 begin_data[2];    //beginning of sector data.byte 0,1 is 1st sector data,byte 2,3 is 2nd sector data.
    //Read the beginning and end data of the 1st sector
    flash_read_page(addr, 2, begin_data);
    //flash_read_page(addr, 2, end_data);

    u8 allff[2] = {0xff,0xff};

    /****  start check flash  ****/

    if(!memcmp(begin_data, allff, 2)){
        //beginning of 1st sector data(2byte) is all 0xff.
        //Indicates whether the RCU is brand new or factory set
        ret = 1;    //return 1;    //No data in flash
    }else{

        //search data in flash
        s16 offset = bsearch_without_recursion(addr,used_sector,used_block);

        if(offset == -1){

            ret = 2;    //return 2;    //sector data error
        }else{

            flash_read_page(addr + offset*used_block, len, rbuf);
        }
    }

    return ret;
}

/**
 * @brief This function can make more effective use of flash.
 * @param[in]   addr the start address of the page
 * @param[in]   Number of sectors used to save this data type
 * @param[in]   Space occupied by a piece of data.Less than 256£¬
 * @param[in]   len the length(in byte) of content needs to read out from the page
 * @param[out]  buf the start address of the buffer
 * @return 0:Success,1:flash is empty,2:sector data error,3:read data error
 */
u8 flash_write_info(unsigned long addr, u8 used_sector, u8 used_block, unsigned long len, u8 *wbuf ){

    u8 ret = 0;
    u8 begin_data[2];    //beginning of sector data.byte 0,1 is 1st sector data,byte 2,3 is 2nd sector data.
    //u8 end_data[4];        //end of sector data.byte 0,1 is 1st sector data,byte 2,3 is 2nd sector data.

    //Read the beginning and end data of the 1st sector
    flash_read_page(addr, 2, begin_data);
    //flash_read_page(addr, 2, end_data);

    u8 allff[2] = {0xff,0xff};

    /****  start check flash  ****/

    if(!memcmp(begin_data, allff, 2)){
        //beginning of 1st sector data(2byte) is all 0xff.
        //Indicates whether the RCU is brand new or factory set
        ret = 1;    //No data in flash
    }else{

        //search data in flash
        s16 offset = bsearch_without_recursion(addr,used_sector,used_block);

        if(offset == -1){

            ret = 2;    //sector data error
        }else{
            u8 clr[2] = {0};
            flash_write_page(addr + offset*used_block, 2, clr);

            //if(offset + 1 >= 4*1024*used_sector/used_block) offtset = -1;    //2nd sector is full.so set offset to beginning of 1st sector
            //write data to flash
            flash_write_page(addr + (offset + 1)*used_block, len, wbuf);
            //Confirm that writing data is correct
            //u8 temp[256] = {0};
            u8 temp[16] = {0};
            flash_read_page(addr + (offset + 1)*used_block, len, temp);
            if(memcmp(wbuf, temp, len))     ret = 3;    //The data written and read does not match
        }
    }

    switch(ret){
        case 3:
        case 2:
            break;
        case 1:
        {
            flash_write_page(addr, len, wbuf);
            break;
        }
        case 0:
        default:
            break;
    }

    return ret;
}

void flash_check_area(unsigned long addr, u8 used_sector, u8 used_block, unsigned long len){


    u8 begin_data[2];    //beginning of sector data.byte 0,1 is 1st sector data,byte 2,3 is 2nd sector data.
    u8 allff[2] = {0xff,0xff};
    //Read the beginning and end data of the 1st sector
    flash_read_page(addr, 2, begin_data);

    if(!memcmp(begin_data, allff, 2)){
        //beginning of 1st sector data(2byte) is all 0xff.
        //Indicates whether the RCU is brand new or factory set
        return;
    }


    s16 offset = bsearch_without_recursion(addr,used_sector,used_block);
    if(offset == -1){
        //sector data error
        for(u8 i=0;i<used_sector;i++){
            flash_erase_sector(addr+i*0x1000);
        }
        return;
    }

    //If the area usage exceeds 70%, erase the area
//    if(offset > (4*1024/used_block)*used_sector/10*8){
//        u8 rbuf[256] = {0};
//        flash_read_page(addr + offset*used_block,len,rbuf);
//        //flash_read_page(addr + offset*used_block,used_block,rbuf);
//        for(u8 i=0;i<used_sector;i++){
//            flash_erase_sector(addr+i*0x1000);
//        }
//        //Move the data to the beginning of the 2st sector
//        flash_write_page(addr,len,rbuf);
//        //flash_write_page(addr,used_block,rbuf);
//    }
}


void write_ccc_info(u8 *ccc){

    u8 dat[4] ={0xA5,0xFF,0xFF,0xFF};
    memcpy(dat+2,ccc,1);
    u8 ret = flash_write_info(CCC_DATA_AREA,1,4,3,dat);
    printf("write_ccc_info:0x%x",ret);
}

u8 read_ccc_info(u8 *ccc){

    u8 dat[4] ={0};
    u8 ret = flash_read_info(CCC_DATA_AREA,1,4,3,dat);
    if(!ret){
        ccc[0] = dat[2];
    }
    //return 0:Success,1:flash is empty,2:sector data error
    return ret;
}

extern u16 atv_char_ctl_ccc;
extern u16 atv_char_rx_ccc;

void init_ccc_value(){

    read_ccc_info(&flag_ccc_data);

    atv_char_ctl_ccc = (flag_ccc_data & FLAG_GOOGLE_CTL_CCC)?1:0;
    atv_char_rx_ccc = (flag_ccc_data & FLAG_GOOGLE_RX_CCC)?1:0;

    printf("atv_char_ctl_ccc:%d\n",atv_char_ctl_ccc);
    printf("atv_char_rx_ccc:%d\n",atv_char_rx_ccc);
}


void is_flash_info_full(void)
{
    if(flash_pos_info(CCC_DATA_AREA,1,4) == 0x03)
    {
        flash_erase_sector(CCC_DATA_AREA);
    }

}


