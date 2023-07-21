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
#include "esp_wifi.h"


typedef struct station_t {

	char ssid[33];
	bool open_ssid;


}station_t;




void lv_set_style_layout_wifi_stations();
void lv_set_style_button_reset_init_thermostat();
void lv_set_style_screen_init_thermostat();
void lv_back_wifi_layout(DATOS_APLICACION *datosApp);

void lv_create_layout_init_thermostat(DATOS_APLICACION *datosApp);
void lv_init_data_init_thermostat(DATOS_APLICACION *datosApp);
station_t* lv_get_list_stations(uint16_t *size);



void lv_create_layout_password_wifi(DATOS_APLICACION datosApp);
void lv_delete_objects_layout_wifi();
void lv_connecting_to_wifi_station(DATOS_APLICACION *datosApp);
void lv_create_layout_search_ssid(DATOS_APLICACION *datosApp, wifi_ap_record_t *ap_info, uint16_t *ap_count);
void lv_set_button_wifi(DATOS_APLICACION *datosApp, bool fail);

#endif /* LV_SRC_INCLUDE_LV_INIT_THERMOSTAT_H_ */
