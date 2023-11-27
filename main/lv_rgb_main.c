
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


#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"



#include "driver/spi_master.h"
#include "esp_lcd_touch_gt911.h"
#include  "esp_lcd_touch_xpt2046.h"

#include "lv_factory_reset.h"
#include "lv_thermostat.h"
#include "lv_init_thermostat.h"

#include "driver/i2c.h"
#include "logging.h"

#include "configuracion.h"
#include "code_application.h"
#include "conexiones.h"
#include "conexiones_mqtt.h"

#include "events_device.h"

static const char *TAG = "lv_rgb_main";
#define	I2C_HOST	0
esp_lcd_touch_handle_t tp = NULL;
static esp_timer_handle_t timer_backlight;
//#define TIMING_BACKLIGHT 15

#include "esp_lcd_ili9341.h"









#ifdef CONFIG_RGB_LCD


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


void main_function(DATOS_APLICACION *datosApp) {

	esp_err_t error;

	if (init_code_application(datosApp) != ESP_OK) {
		ESP_LOGE(TAG, ""TRAZAR" FALLO LA INICIALIZACION DE LA APLICACION", INFOTRAZA);
	}




	error = init_application(datosApp, CONFIG_CARGA_CONFIGURACION);
	if (error == ESP_OK) {
		ESP_LOGI(TAG, ""TRAZAR"INICIALIZACION CORRECTA", INFOTRAZA);
	} else {
		if (datosApp->datosGenerales->estadoApp == FACTORY) {
			ESP_LOGI(TAG, ""TRAZAR"NO SE HA PODIDO INICIALIZAR EL DISPOSITIVO", INFOTRAZA);
		} else {
			ESP_LOGE(TAG, ""TRAZAR"NO SE HA PODIDO INICIALIZAR EL DISPOSITIVO", INFOTRAZA);
			return;
		}

	}




	ESP_LOGI(TAG, ""TRAZAR" vamos a conectar al wifi", INFOTRAZA);
	conectar_dispositivo_wifi();

	//sync_app_by_ntp(datosApp);
	crear_tarea_mqtt(datosApp);



}


void report_error_lcd() {

	send_event(__func__,EVENT_ERROR_LCD);
	vTaskDelete(NULL);

}




void lv_init_lcd_application(void *arg)
{
    static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
    static lv_disp_drv_t disp_drv;      // contains callback functions
    DATOS_APLICACION *datosApp = (DATOS_APLICACION *) arg;

#if CONFIG_AVOID_TEAR_EFFECT_WITH_SEM
    ESP_LOGI(TAG, "Create semaphores");
    sem_vsync_end = xSemaphoreCreateBinary();
    assert(sem_vsync_end);
    sem_gui_ready = xSemaphoreCreateBinary();
    assert(sem_gui_ready);
#endif

#if CONFIG_RGB_LCD_PIN_NUM_BK_LIGHT >= 0
    ESP_LOGI(TAG, "Turn off LCD backlight");
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_INPUT_OUTPUT,
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
        .disp_gpio_num = CONFIG_RGB_LCD_PIN_NUM_DISP_EN,
        .pclk_gpio_num = CONFIG_RGB_LCD_PIN_NUM_PCLK,
        .vsync_gpio_num = CONFIG_RGB_LCD_PIN_NUM_VSYNC,
        .hsync_gpio_num = CONFIG_RGB_LCD_PIN_NUM_HSYNC,
        .de_gpio_num = CONFIG_RGB_LCD_PIN_NUM_DE,
        .data_gpio_nums = {
            CONFIG_RGB_LCD_PIN_NUM_DATA0,
            CONFIG_RGB_LCD_PIN_NUM_DATA1,
            CONFIG_RGB_LCD_PIN_NUM_DATA2,
            CONFIG_RGB_LCD_PIN_NUM_DATA3,
            CONFIG_RGB_LCD_PIN_NUM_DATA4,
            CONFIG_RGB_LCD_PIN_NUM_DATA5,
            CONFIG_RGB_LCD_PIN_NUM_DATA6,
            CONFIG_RGB_LCD_PIN_NUM_DATA7,
            CONFIG_RGB_LCD_PIN_NUM_DATA8,
            CONFIG_RGB_LCD_PIN_NUM_DATA9,
            CONFIG_RGB_LCD_PIN_NUM_DATA10,
            CONFIG_RGB_LCD_PIN_NUM_DATA11,
            CONFIG_RGB_LCD_PIN_NUM_DATA12,
            CONFIG_RGB_LCD_PIN_NUM_DATA13,
            CONFIG_RGB_LCD_PIN_NUM_DATA14,
            CONFIG_RGB_LCD_PIN_NUM_DATA15,
        },
        .timings = {
            .pclk_hz = CONFIG_RGB_LCD_PIXEL_CLOCK_HZ,
            .h_res = CONFIG_LCD_H_RES,
            .v_res = CONFIG_LCD_V_RES,
            // The following parameters should refer to LCD spec
            .hsync_back_porch = 43,
            .hsync_front_porch = 8,
            .hsync_pulse_width = 4,
            .vsync_back_porch = 12,
            .vsync_front_porch = 8,
            .vsync_pulse_width = 4,
            .flags.pclk_active_neg = true,
        },
        .flags.fb_in_psram = true, // allocate frame buffer in PSRAM
    };

    if (esp_lcd_new_rgb_panel(&panel_config, &panel_handle) != ESP_OK) {

    	report_error_lcd();
    	return;

    }





    //ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));

    ESP_LOGI(TAG, "Register event callbacks");
    esp_lcd_rgb_panel_event_callbacks_t cbs = {
        .on_vsync = on_vsync_event,
    };

    if (esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, &disp_drv) != ESP_OK) {
    	report_error_lcd();
    	return;
    }



    //ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, &disp_drv));

    ESP_LOGI(TAG, "Initialize RGB LCD panel");
    if (esp_lcd_panel_reset(panel_handle) !=ESP_OK) {
    	report_error_lcd();
    	return;

    }



    //ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));

    if (esp_lcd_panel_init(panel_handle) != ESP_OK) {
    	report_error_lcd();
    	return;

    }
    //ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

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

    if (esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2) != ESP_OK) {
    	report_error_lcd();
    	return;

    }


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

    ESP_LOGI(TAG, "Display LVGL Scatter Chart. Resolution %d X %d", CONFIG_LCD_H_RES, CONFIG_LCD_V_RES );

#ifdef CONFIG_XPT2046
    init_app_touch_xpt2046(disp);
#endif

#ifdef CONFIG_GT911
    init_app_touch_gt911(disp);
#endif

    lv_screen_thermostat(datosApp);
    timing_backlight();
    send_event(__func__,EVENT_LCD_OK);


    while (1) {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
    }

}


#endif



#ifdef CONFIG_GT911
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

#endif

#ifdef CONFIG_XPT2046
static void lvgl_touch_cb2(lv_indev_drv_t * drv, lv_indev_data_t * data) {

    uint16_t x[1];
    uint16_t y[1];
    uint16_t strength[1];
    uint8_t count = 0;




    // Update touch point data.
   ESP_ERROR_CHECK(esp_lcd_touch_read_data(tp));

    data->state = LV_INDEV_STATE_REL;

    if (esp_lcd_touch_get_coordinates(tp, x, y, strength, &count, 1))
    {
        data->point.x = x[0];
        data->point.y = y[0];
        data->state = LV_INDEV_STATE_PR;
        ESP_LOGI(TAG, "XPT2046: x=%d, y=%d",data->point.x, data->point.y);
        //backlight_on();

    }

    data->continue_reading = false;


}


void init_app_touch_xpt2046(lv_disp_t *disp) {


	// Inicializamos el bus



    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num = CONFIG_SPI_TOUCH_SCLK,
        .mosi_io_num = CONFIG_SPI_TOUCH_MOSI,
        .miso_io_num = CONFIG_SPI_TOUCH_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = CONFIG_LCD_H_RES * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));


	esp_lcd_panel_io_handle_t tp_io_handle = NULL;
	esp_lcd_panel_io_spi_config_t tp_io_config = ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG(38);
	ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &tp_io_config, &tp_io_handle));

	    esp_lcd_touch_config_t tp_cfg = {
	        .x_max = CONFIG_LCD_H_RES,
	        .y_max = CONFIG_LCD_V_RES,
	        .rst_gpio_num = -1,
	        .int_gpio_num = 18,
	        .flags = {
	            .swap_xy = 0,
	            .mirror_x = 0,
	            .mirror_y = 0,
	        },
	    };

	    ESP_LOGI(TAG, "Initialize touch controller XPT2046");
	    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &tp_cfg, &tp));

	    ESP_LOGE(TAG, "tp es nulo y no puede ser");

	    static lv_indev_drv_t indev_drv;    // Input device driver (Touch)
	    lv_indev_drv_init(&indev_drv);
	    indev_drv.type = LV_INDEV_TYPE_POINTER;
	    indev_drv.disp = disp;
	    indev_drv.read_cb = lvgl_touch_cb2;
	    indev_drv.user_data = tp;
	    lv_indev_drv_register(&indev_drv);





}
#endif


esp_err_t backlight_on() {



	if (gpio_get_level(CONFIG_PIN_NUM_BK_LIGHT) == 0) {
		gpio_set_level(CONFIG_PIN_NUM_BK_LIGHT, 1);
		ESP_LOGE(TAG, "SE ENCIENDE LA PANTALLA");
	} else {
		if (esp_timer_is_active(timer_backlight)) {
			ESP_LOGE(TAG, "CANCELAMOS TEMPORIZADOS Y REINICIAMOS");
			ESP_ERROR_CHECK(esp_timer_stop(timer_backlight));
			ESP_ERROR_CHECK(esp_timer_delete(timer_backlight));
		} else {
			ESP_LOGE(TAG, "REBOTE");
		}


	}

	timing_backlight();




	return ESP_OK;
}



void backlight_off(void *arg) {



	//gpio_set_level(CONFIG_PIN_NUM_BK_LIGHT, 0);
	//ESP_LOGE(TAG, "PANTALLA APAGADA");



	if (gpio_get_level(CONFIG_PIN_NUM_BK_LIGHT) == 1) {
		gpio_set_level(CONFIG_PIN_NUM_BK_LIGHT, 0);
		ESP_LOGE(TAG, "PANTALLA APAGADA");
	} else {

		ESP_LOGE(TAG, "LA PANTALLA SIGUE ENCENDIDA");

	}



}


void timing_backlight() {


    const esp_timer_create_args_t backlight_shot_timer_args = {
            .callback = &backlight_off,
            /* name is optional, but may help identify the timer when debugging */
            .name = "end schedule"
    };



	    ESP_ERROR_CHECK(esp_timer_create(&backlight_shot_timer_args, &timer_backlight));
	    ESP_ERROR_CHECK(esp_timer_start_once(timer_backlight, (CONFIG_TIME_OFF_BACKLIGHT * 1000000)));


}


void lv_cancel_timing_backlight() {

	if (esp_timer_is_active(timer_backlight)) {
		ESP_LOGI(TAG, ""TRAZAR"se cancela temporizador", INFOTRAZA);
		esp_timer_stop(timer_backlight);
		esp_timer_delete(timer_backlight);
	}


}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Parte SPI_LCD
#ifdef CONFIG_SPI_LCD

static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

/* Rotate display and touch, when rotated screen in LVGL. Called when driver parameters are updated. */
static void example_lvgl_port_update_callback(lv_disp_drv_t *drv)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;

    ESP_LOGI(TAG, "LA ROTACION VALE: %d", drv->rotated);

    switch (drv->rotated) {
    case LV_DISP_ROT_NONE:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, false);
        esp_lcd_panel_mirror(panel_handle, true, false);
#ifdef CONFIG_XPT2046A
        // Rotate LCD touch
        esp_lcd_touch_set_mirror_y(tp, false);
        esp_lcd_touch_set_mirror_x(tp, false);
#endif
        break;
    case LV_DISP_ROT_90:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, true);
        esp_lcd_panel_mirror(panel_handle, true, true);
#ifdef CONFIG_XPT2046A
        // Rotate LCD touch
        esp_lcd_touch_set_mirror_y(tp, false);
        esp_lcd_touch_set_mirror_x(tp, false);
#endif
        break;
    case LV_DISP_ROT_180:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, false);
        esp_lcd_panel_mirror(panel_handle, false, true);
#ifdef CONFIG_XPT2046A
        // Rotate LCD touch
        esp_lcd_touch_set_mirror_y(tp, false);
        esp_lcd_touch_set_mirror_x(tp, false);
#endif
        break;
    case LV_DISP_ROT_270:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, true);
        esp_lcd_panel_mirror(panel_handle, false, false);
#ifdef CONFIG_XPT2046A
        // Rotate LCD touch
        esp_lcd_touch_set_mirror_y(tp, false);
        esp_lcd_touch_set_mirror_x(tp, false);
#endif
        break;
    }
}

#ifdef CONFIG_XPT2046
static void example_lvgl_touch_cb(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    /* Read touch controller data */
    esp_lcd_touch_read_data(drv->user_data);

    /* Get coordinates */
    bool touchpad_pressed = esp_lcd_touch_get_coordinates(drv->user_data, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

    if (touchpad_pressed && touchpad_cnt > 0) {
        data->point.x = touchpad_x[0];
        data->point.y = touchpad_y[0];
        data->state = LV_INDEV_STATE_PRESSED;
        ESP_LOGI(TAG, "XPT2046: x=%d, y=%d",data->point.x, data->point.y);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
#endif

static void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(CONFIG_LVGL_TICK_PERIOD_MS);
}




void init_app_touch_spi_xpt2046(lv_disp_t *disp) {


	// Inicializamos el bus



    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num = CONFIG_SPI_TOUCH_SCLK,
        .mosi_io_num = CONFIG_SPI_TOUCH_MOSI,
        .miso_io_num = CONFIG_SPI_TOUCH_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = CONFIG_LCD_H_RES * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));


	esp_lcd_panel_io_handle_t tp_io_handle = NULL;
	esp_lcd_panel_io_spi_config_t tp_io_config = ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG(CONFIG_SPI_TOUCH_CS);
	ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI3_HOST, &tp_io_config, &tp_io_handle));

	    esp_lcd_touch_config_t tp_cfg = {
	        .x_max = CONFIG_LCD_H_RES,
	        .y_max = CONFIG_LCD_V_RES,
	        .rst_gpio_num = -1,
	        .int_gpio_num = CONFIG_SPI_TOUCH_INTR,
	        .flags = {
	            .swap_xy = 1,
	            .mirror_x = 0,
	            .mirror_y = 0,
	        },
	    };

	    ESP_LOGI(TAG, "Initialize touch controller XPT2046");
	    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &tp_cfg, &tp));

	    ESP_LOGE(TAG, "tp es nulo y no puede ser");

	    static lv_indev_drv_t indev_drv;    // Input device driver (Touch)
	    lv_indev_drv_init(&indev_drv);
	    indev_drv.type = LV_INDEV_TYPE_POINTER;
	    indev_drv.disp = disp;
	    indev_drv.read_cb = example_lvgl_touch_cb;
	    indev_drv.user_data = tp;
	    lv_indev_drv_register(&indev_drv);





}






void lv_init_spi_lcd(void *arg) {

	DATOS_APLICACION *datosApp = (DATOS_APLICACION *) arg;
    static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
    static lv_disp_drv_t disp_drv;      // contains callback functions


    ESP_LOGI(TAG, "Turn off LCD backlight");
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << CONFIG_PIN_NUM_BK_LIGHT
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num = CONFIG_PIN_SPI_LCD_NUM_SCLK,
        .mosi_io_num = CONFIG_PIN_SPI_LCD_NUM_MOSI,
        .miso_io_num = CONFIG_PIN_SPI_LCD_NUM_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = CONFIG_LCD_H_RES * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = CONFIG_PIN_SPI_LCD_NUM_DC,
        .cs_gpio_num = CONFIG_PIN_SPI_LCD_NUM_LCD_CS,
        .pclk_hz = CONFIG_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = CONFIG_SPI_LCD_CMD_BITS,
        .lcd_param_bits = CONFIG_SPI_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = example_notify_lvgl_flush_ready,
        .user_ctx = &disp_drv,
    };
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = CONFIG_PIN_SPI_LCD_NUM_LCD_RST,
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,

    };

#if CONFIG_SPI_LCD_ILI9341
    ESP_LOGI(TAG, "Install ILI9341 panel driver");
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle));
#endif

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, true));
    esp_lcd_panel_swap_xy(panel_handle, true);


    // user can flush pre-defined pattern to the screen before we turn on the screen or backlight
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));


    ESP_LOGI(TAG, "Turn on LCD backlight");
    gpio_set_level(CONFIG_PIN_NUM_BK_LIGHT, CONFIG_LCD_BK_LIGHT_ON_LEVEL);

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    // alloc draw buffers used by LVGL
    // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
    lv_color_t *buf1 = heap_caps_malloc(CONFIG_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1);
    lv_color_t *buf2 = heap_caps_malloc(CONFIG_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);
    // initialize LVGL draw buffers
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, CONFIG_LCD_H_RES * 20);

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = CONFIG_LCD_H_RES;
    disp_drv.ver_res = CONFIG_LCD_V_RES;
    disp_drv.flush_cb = example_lvgl_flush_cb;
    disp_drv.drv_update_cb = example_lvgl_port_update_callback;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    //disp_drv.rotated = LV_DISP_ROT_180;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, CONFIG_LVGL_TICK_PERIOD_MS * 1000));



    ESP_LOGI(TAG, "Display LVGL Meter Widget");
#ifdef CONFIG_XPT2046
    init_app_touch_spi_xpt2046(disp);

#endif

#ifdef CONFIG_GT911
    init_app_touch_gt911(disp);
#endif

    lv_screen_thermostat(datosApp);
    timing_backlight();
    send_event(__func__,EVENT_LCD_OK);

    while (1) {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
    }


}

#endif
