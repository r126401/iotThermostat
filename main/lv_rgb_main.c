/*
 * lv_rgb_main.c
 *
 *  Created on: Jul 4, 2023
 *      Author: t126401
 */


#include "lv_rgb_main.h"
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"

#include "driver/spi_master.h"
#include "esp_lcd_touch_gt911.h"

#include "lv_factory_reset.h"
#include "lv_thermostat.h"
#include "lv_init_thermostat.h"

#include "driver/i2c.h"

static const char *TAG = "iotThermostat";
#define	I2C_HOST	0
esp_lcd_touch_handle_t tp = NULL;

#ifndef CONFIG_LCD_H_RES

#define CONFIG_LCD_H_RES 800
#define CONFIG_LCD_V_RES 480

#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define CONFIG_LCD_PIXEL_CLOCK_HZ     (14 * 1000 * 1000)
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



static bool on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data, void *user_data)
{
    BaseType_t high_task_awoken = pdFALSE;
#if CONFIG_AVOID_TEAR_EFFECT_WITH_SEM
    if (xSemaphoreTakeFromISR(sem_gui_ready, &high_task_awoken) == pdTRUE) {
        xSemaphoreGiveFromISR(sem_vsync_end, &high_task_awoken);
    }
#endif
    return high_task_awoken == pdTRUE;
}

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
#if CONFIG_AVOID_TEAR_EFFECT_WITH_SEM
    xSemaphoreGive(sem_gui_ready);
    xSemaphoreTake(sem_vsync_end, portMAX_DELAY);
#endif
    // pass the draw buffer to the driver
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

static void increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(CONFIG_LVGL_TICK_PERIOD_MS);
}




static void lv_init_app(DATOS_APLICACION *datosApp) {

	switch (datosApp->datosGenerales->estadoApp) {

	case ARRANQUE_FABRICA:
			lv_screen_factory_reset(datosApp);
			break;
	case ESPERA_FIN_ARRANQUE:
	case NORMAL_ARRANCANDO:
		lv_init_data_init_thermostat(datosApp);
		break;

	default:
		lv_screen_thermostat(datosApp);
		break;

	}


}




void lv_app_rgb_main(DATOS_APLICACION *datosApp)
{
    static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
    static lv_disp_drv_t disp_drv;      // contains callback functions

#if CONFIG_AVOID_TEAR_EFFECT_WITH_SEM
    ESP_LOGI(TAG, "Create semaphores");
    sem_vsync_end = xSemaphoreCreateBinary();
    assert(sem_vsync_end);
    sem_gui_ready = xSemaphoreCreateBinary();
    assert(sem_gui_ready);
#endif

#if CONFIG_PIN_NUM_BK_LIGHT >= 0
    ESP_LOGI(TAG, "Turn off LCD backlight");
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << CONFIG_PIN_NUM_BK_LIGHT
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
#endif

    ESP_LOGI(TAG, "Install RGB LCD panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 16, // RGB565 in parallel mode, thus 16bit in width
        .psram_trans_align = 64,
        .num_fbs = CONFIG_LCD_NUM_FB,
#if CONFIG_USE_BOUNCE_BUFFER
        .bounce_buffer_size_px = 10 * CONFIG_LCD_H_RES,
#endif
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .disp_gpio_num = CONFIG_PIN_NUM_DISP_EN,
        .pclk_gpio_num = CONFIG_PIN_NUM_PCLK,
        .vsync_gpio_num = CONFIG_PIN_NUM_VSYNC,
        .hsync_gpio_num = CONFIG_PIN_NUM_HSYNC,
        .de_gpio_num = CONFIG_PIN_NUM_DE,
        .data_gpio_nums = {
            CONFIG_PIN_NUM_DATA0,
            CONFIG_PIN_NUM_DATA1,
            CONFIG_PIN_NUM_DATA2,
            CONFIG_PIN_NUM_DATA3,
            CONFIG_PIN_NUM_DATA4,
            CONFIG_PIN_NUM_DATA5,
            CONFIG_PIN_NUM_DATA6,
            CONFIG_PIN_NUM_DATA7,
            CONFIG_PIN_NUM_DATA8,
            CONFIG_PIN_NUM_DATA9,
            CONFIG_PIN_NUM_DATA10,
            CONFIG_PIN_NUM_DATA11,
            CONFIG_PIN_NUM_DATA12,
            CONFIG_PIN_NUM_DATA13,
            CONFIG_PIN_NUM_DATA14,
            CONFIG_PIN_NUM_DATA15,
        },
        .timings = {
            .pclk_hz = CONFIG_LCD_PIXEL_CLOCK_HZ,
            .h_res = CONFIG_LCD_H_RES,
            .v_res = CONFIG_LCD_V_RES,
            // The following parameters should refer to LCD spec
            .hsync_back_porch = 8,
            .hsync_front_porch = 8,
            .hsync_pulse_width = 4,
            .vsync_back_porch = 8,
            .vsync_front_porch = 8,
            .vsync_pulse_width = 4,
            .flags.pclk_active_neg = true,
        },
        .flags.fb_in_psram = true, // allocate frame buffer in PSRAM
    };
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));

    ESP_LOGI(TAG, "Register event callbacks");
    esp_lcd_rgb_panel_event_callbacks_t cbs = {
        .on_vsync = on_vsync_event,
    };
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, &disp_drv));

    ESP_LOGI(TAG, "Initialize RGB LCD panel");
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

#if CONFIG_PIN_NUM_BK_LIGHT >= 0
    ESP_LOGI(TAG, "Turn on LCD backlight");
    gpio_set_level(CONFIG_PIN_NUM_BK_LIGHT, CONFIG_LCD_BK_LIGHT_ON_LEVEL);
#endif

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    void *buf1 = NULL;
    void *buf2 = NULL;
#if CONFIG_DOUBLE_FB
    ESP_LOGI(TAG, "Use frame buffers as LVGL draw buffers");
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2));
    // initialize LVGL draw buffers
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, CONFIG_LCD_H_RES * CONFIG_LCD_V_RES);
#else
    ESP_LOGI(TAG, "Allocate separate LVGL draw buffers from PSRAM");
    buf1 = heap_caps_malloc(CONFIG_LCD_H_RES * 100 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf1);
    buf2 = heap_caps_malloc(CONFIG_LCD_H_RES * 100 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    assert(buf2);
    // initialize LVGL draw buffers
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, CONFIG_LCD_H_RES * 100);
#endif // CONFIG_DOUBLE_FB

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = CONFIG_LCD_H_RES;
    disp_drv.ver_res = CONFIG_LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
#if CONFIG_DOUBLE_FB
    disp_drv.full_refresh = true; // the full_refresh mode can maintain the synchronization between the two frame buffers
#endif
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);


    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, CONFIG_LVGL_TICK_PERIOD_MS * 1000));

    ESP_LOGI(TAG, "Display LVGL Scatter Chart");
    init_app_touch_gt911(disp);
    
    lv_init_app(datosApp);


    while (1) {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
    }
}







static void lvgl_touch_cb(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;
    static bool hecho = false;



    /* Read touch controller data */
    esp_lcd_touch_read_data(drv->user_data);


    /* Get coordinates */
    bool touchpad_pressed = esp_lcd_touch_get_coordinates(drv->user_data, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

    if (touchpad_pressed && touchpad_cnt > 0) {
        data->point.x = touchpad_x[0];
        data->point.y = touchpad_y[0];
        data->state = LV_INDEV_STATE_PRESSED;
        ESP_LOGE(TAG, "pulsado");
        if (hecho == false) {
        	touch_gt911_print_config();
        	//touch_gt911_change_resolution(drv->user_data, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);
        	touch_gt911_print_config();
        	hecho = true;
        } else {
        	touch_gt911_get_config(tp);
        	touch_gt911_print_config();
        	touch_gt911_calculate_checksum();
        }


        ESP_LOGI(TAG, "presionado x= %d, y=%d", data->point.x, data->point.y);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}


void init_app_touch_gt911(lv_disp_t *disp) {



    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_TOUCH_GT911_SDA,
        .scl_io_num = CONFIG_TOUCH_GT911_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = CONFIG_TOUCH_GT911_CLOCK_HZ * 1000,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_HOST, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_HOST, I2C_MODE_MASTER, 0, 0, 0));

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();

    // Attach the TOUCH to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)I2C_HOST, &tp_io_config, &tp_io_handle));

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = CONFIG_LCD_H_RES,
        .y_max = CONFIG_LCD_V_RES,
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };

    ESP_LOGI(TAG, "Initialize touch controller GT911");
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &tp));
    // Danny

    static lv_indev_drv_t indev_drv;    // Input device driver (Touch)
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.disp = disp;
    indev_drv.read_cb = lvgl_touch_cb;
    indev_drv.user_data = tp;

    lv_indev_drv_register(&indev_drv);




}


