/*
 * codigo.h
 *
 *  Created on: 3 jun 2023
 *      Author: t126401
 */

#ifndef LV_SRC_INCLUDE_LV_THERMOSTAT_H_

#define LV_SRC_INCLUDE_LV_THERMOSTAT_H_ 1

//#define _DEFAULT_SOURCE /* needed for usleep() */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "lvgl.h"
#include "lv_examples.h"
#include "lv_demos.h"

#include "esp_lcd_touch.h"
#include "esp_lcd_panel_io.h"
#include "common_data.h"




void lv_set_style_screen(lv_obj_t *display);
void lv_set_style_layout_notification();
void lv_create_layout_nofitification(DATOS_APLICACION *datosApp);
void lv_create_layout_temperature(DATOS_APLICACION *datosApp);
void lv_set_style_status_application();
void lv_create_status_aplication(DATOS_APLICACION *datosApp);
void lv_screen_thermostat(DATOS_APLICACION *datosApp);
void lv_update_alarm_device(DATOS_APLICACION *datosApp);
void lv_status_device(DATOS_APLICACION *datosApp);
void lv_set_style_layout_temperature();
void lv_set_style_threshold();
void lv_create_layout_threshold(DATOS_APLICACION *datosApp);
void lv_update_status_application(DATOS_APLICACION *datosApp);
void lv_update_heating(DATOS_APLICACION *datosApp);
void lv_create_heating_icon(DATOS_APLICACION *datosApp);
void lv_create_layout_buttons_threshold(DATOS_APLICACION *datosApp);
void lv_set_style_layout_buttons_threshold();
void lv_create_layout_schedule(DATOS_APLICACION *datosApp);
void lv_set_style_layout_schedule();
void lv_set_style_bar_schedule();
void lv_update_bar_schedule(DATOS_APLICACION *datosApp);
void lv_update_threshold(DATOS_APLICACION *datosApp);
void example_lvgl_demo_ui(lv_disp_t *disp);
void lv_update_device(DATOS_APLICACION *datosApp);
void lv_update_error_temperature();
void lv_update_temperature(DATOS_APLICACION *datosApp);
bool lv_update_hour(char* hour);
void lv_update_relay();

#endif /* LV_SRC_INCLUDE_LV_THERMOSTAT_H_ */
