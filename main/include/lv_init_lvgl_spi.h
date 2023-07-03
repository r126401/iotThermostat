/*
 * lv_init_lvgl_spi.h
 *
 *  Created on: Jun 26, 2023
 *      Author: t126401
 */

#ifndef MAIN_INCLUDE_LV_INIT_LVGL_SPI_H_
#define MAIN_INCLUDE_LV_INIT_LVGL_SPI_H_



#include <stdio.h>
#include "lvgl.h"
#include "esp_lcd_ili9341.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
/*
void increase_lvgl_tick(void *arg);
bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
void lvgl_port_update_callback(lv_disp_drv_t *drv);
void init_lvgl_library();
*/

#endif /* MAIN_INCLUDE_LV_INIT_LVGL_SPI_H_ */
