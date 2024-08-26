/*
 * app_backlight.h
 *
 *  Created on: 2023-12-1
 *      Author: name
 */

#ifndef APP_BACKLIGHT_H_
#define APP_BACKLIGHT_H_


void app_backlight_set_ability(unsigned char en, unsigned short boost_pin);

unsigned char app_backlight_is_busy(void);
void app_backlight_start(void);
void app_backlight_task(void);

#endif /* APP_BACKLIGHT_H_ */
