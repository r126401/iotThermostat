/*
 * lv_rgb_main.h
 *
 *  Created on: Jul 4, 2023
 *      Author: t126401
 */

#ifndef MAIN_INCLUDE_LV_RGB_MAIN_H_
#define MAIN_INCLUDE_LV_RGB_MAIN_H_

#include "lvgl.h"
#include "common_data.h"
#

typedef enum LCD_COMMANDS {

	LCD_ON,
	LCD_OFF,
	UPDATE_DATE,
	UPDATE_COMMUNICATION,
	UPDATE_ALARM,
	UPPDATE_TEMPERATURE,
	SHOW_BUTTON_WIFI,
	HIDE_BUTTON_WIFI,
	SMARTCONFIG_NOTIFY,
	CONNECTING_WIFI,
	UPDATE_DEVICE,
	CREATE_LAYOUT_WIFI,
	SCREEN_FACTORY,
	SCREEN_INIT,
	SCREEN_NORMAL



} LCD_COMMANDS;

esp_err_t lv_app_rgb_main(DATOS_APLICACION *datosApp);
void init_app_touch_xpt2046(lv_disp_t *disp);
void init_app_touch_gt911(lv_disp_t *disp);
void init_app_touch_xpt2046(lv_disp_t *disp);
void lv_command_list(DATOS_APLICACION *datosApp, LCD_COMMANDS *command);
void lv_receive_lcd_commands(DATOS_APLICACION *datosApp);
void lv_send_lcd_commands(LCD_COMMANDS command);

#endif /* MAIN_INCLUDE_LV_RGB_MAIN_H_ */
