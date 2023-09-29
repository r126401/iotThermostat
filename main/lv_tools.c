/*
 * lv_tools.c
 *
 *  Created on: Jul 10, 2023
 *      Author: t126401
 */


#include "lv_tools.h"
#include "logging.h"
#include "esp_log.h"


static const char *TAG = "lv_tools";

#define CONFIG_LCD_H_RES_BASE 480
#define CONFIG_LCD_V_RES_BASE 272


#define SCALE_X CONFIG_LCD_H_RES_BASE/CONFIG_LCD_H_RES_BASE
#define SCALE_Y CONFIG_LCD_V_RES_BASE/CONFIG_LCD_V_RES_BASE

lv_coord_t calculate_center_obj_x(lv_coord_t width, bool offset_left) {


	lv_coord_t desp;

	if (offset_left) {
		desp = -(width/2);
	} else {
		desp = width/2;
	}


	return desp;





}


bool  scale_dimensions(lv_coord_t *x, lv_coord_t *y) {


	ESP_LOGE(TAG, ""TRAZAR" antes: x:%d, y:%d", INFOTRAZA, *x, *y);

	*x = *x/SCALE_X;
	*y = *y/SCALE_Y;

	ESP_LOGE(TAG, ""TRAZAR" despues: x:%d, y:%d", INFOTRAZA, *x, *y);



	return false;

}



