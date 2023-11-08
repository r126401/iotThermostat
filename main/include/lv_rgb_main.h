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





#ifndef CONFIG_LCD_H_RES

#define CONFIG_LCD_H_RES 480
#define CONFIG_LCD_V_RES 272

#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define CONFIG_LCD_PIXEL_CLOCK_HZ     (7 * 1000 * 1000)
#define CONFIG_LCD_BK_LIGHT_ON_LEVEL  1
#define CONFIG_LCD_BK_LIGHT_OFF_LEVEL !CONFIG_LCD_BK_LIGHT_ON_LEVEL
#define CONFIG_PIN_NUM_BK_LIGHT       2
#define CONFIG_PIN_NUM_HSYNC          39
#define CONFIG_PIN_NUM_VSYNC          41
#define CONFIG_PIN_NUM_DE             40
#define CONFIG_PIN_NUM_PCLK           42
#define CONFIG_PIN_NUM_DATA0          8 // B0
#define CONFIG_PIN_NUM_DATA1          3 // B1
#define CONFIG_PIN_NUM_DATA2          46 // B2
#define CONFIG_PIN_NUM_DATA3          9 // B3
#define CONFIG_PIN_NUM_DATA4          1 // B4
#define CONFIG_PIN_NUM_DATA5          5 // G0
#define CONFIG_PIN_NUM_DATA6          6 // G1
#define CONFIG_PIN_NUM_DATA7          7 // G2
#define CONFIG_PIN_NUM_DATA8          15 // G3
#define CONFIG_PIN_NUM_DATA9          16 // G4
#define CONFIG_PIN_NUM_DATA10         4 // G5
#define CONFIG_PIN_NUM_DATA11         45  // R0
#define CONFIG_PIN_NUM_DATA12         48  // R1
#define CONFIG_PIN_NUM_DATA13         47 // R2
#define CONFIG_PIN_NUM_DATA14         21 // R3
#define CONFIG_PIN_NUM_DATA15         14 // R4
#define CONFIG_PIN_NUM_DISP_EN        -1

// The pixel number in horizontal and vertical
//#define CONFIG_LCD_H_RES              480
//#define CONFIG_LCD_V_RES              272

#if CONFIG_DOUBLE_FB
#define CONFIG_LCD_NUM_FB             2
#else
#define CONFIG_LCD_NUM_FB             1
#endif // CONFIG_DOUBLE_FB

#define CONFIG_LVGL_TICK_PERIOD_MS    2

// we use two semaphores to sync the VSYNC event and the LVGL task, to avoid potential tearing effect
#if CONFIG_AVOID_TEAR_EFFECT_WITH_SEM
SemaphoreHandle_t sem_vsync_end;
SemaphoreHandle_t sem_gui_ready;
#endif





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

void lv_init_lcd_application(void *arg);
void init_app_touch_xpt2046(lv_disp_t *disp);
void init_app_touch_gt911(lv_disp_t *disp);
void lv_command_list(DATOS_APLICACION *datosApp, LCD_COMMANDS *command);
void lv_receive_lcd_commands(DATOS_APLICACION *datosApp);
void lv_send_lcd_commands(LCD_COMMANDS command);
void timing_backlight();
esp_err_t backlight_on();
void lv_cancel_timing_backlight();
#endif /* MAIN_INCLUDE_LV_RGB_MAIN_H_ */
