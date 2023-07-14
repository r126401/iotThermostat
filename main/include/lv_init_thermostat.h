/*
 * lv_init_thermostat.h
 *
 *  Created on: 9 jun 2023
 *      Author: t126401
 */

#ifndef LV_SRC_INCLUDE_LV_INIT_THERMOSTAT_H_
#define LV_SRC_INCLUDE_LV_INIT_THERMOSTAT_H_

//#include "datosapp.h"
#include "lvgl.h"
#include "common_data.h"

void lv_set_style_layout_wifi_stations();
void lv_set_style_button_reset_init_thermostat();
void lv_set_style_screen_init_thermostat();
void lv_back_wifi_layout(DATOS_APLICACION *datosApp);

void lv_create_layout_init_thermostat(DATOS_APLICACION *datosApp);
void lv_init_data_init_thermostat(DATOS_APLICACION *datosApp);
void lv_create_items(char station_list[10][50]);

void lv_create_layout_search_ssid(DATOS_APLICACION *datosApp);

void lv_create_layout_password_wifi(DATOS_APLICACION datosApp);
void lv_delete_objects_layout_wifi();


#endif /* LV_SRC_INCLUDE_LV_INIT_THERMOSTAT_H_ */
