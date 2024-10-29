/*
 * app_backlight_gatt.h
 *
 *  Created on: 2024-5-14
 *      Author: name
 */

#ifndef APP_BACKLIGHT_GATT_H_
#define APP_BACKLIGHT_GATT_H_

/**********************************************************************
 * GLOBAL DEFINES
 */

#define APP_BACKLIGHT_GATT_HANDLE                           \
            APP_BACKLIGHT_GATT_PS_H,                        \
            APP_BACKLIGHT_GATT_CMD_CD_H,                    \
            APP_BACKLIGHT_GATT_CMD_DP_H,                    \
            APP_BACKLIGHT_GATT_CMD_CCC_H,


#define APP_BACKLIGHT_GATT_SERVICE                                                                                                                                                      \
    {4,ATT_PERMISSIONS_READ, 2, 16,                                     (u8*)(&my_primaryServiceUUID),          (u8*)(&app_backlight_gatt_service_uuid),    0},                                 \
    {0,ATT_PERMISSIONS_READ, 2, sizeof(app_backlight_gatt_char_val),    (u8*)(&my_characterUUID),               (u8*)(app_backlight_gatt_char_val),         0},                                 \
    {0,ATT_PERMISSIONS_RDWR, 16,sizeof(app_backlight_gatt_data),        (u8*)(&app_backlight_gatt_char_uuid),   (u8*)(app_backlight_gatt_data),             &app_backlight_gatt_write_cb, 0},   \
    {0,ATT_PERMISSIONS_RDWR, 2, sizeof(app_backlight_gatt_ccc),         (u8*)(&clientCharacterCfgUUID),         (u8*)(&app_backlight_gatt_ccc),             0},                                 \


/**********************************************************************
 * ATT SERVICE VARIABLES
 */

extern const unsigned char app_backlight_gatt_service_uuid[16];
extern const unsigned char app_backlight_gatt_char_uuid[16];


extern const unsigned char app_backlight_gatt_char_val[19];

extern volatile unsigned char app_backlight_gatt_data[3];
extern unsigned short app_backlight_gatt_ccc;


/**********************************************************************
 * GLOBAL FUNCTIONS
 */

int app_backlight_gatt_write_cb(void *p);

unsigned char app_backlight_gatt_is_allow_backlight(void);


#endif /* APP_BACKLIGHT_GATT_H_ */
