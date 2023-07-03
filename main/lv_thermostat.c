/*
 * codigo.c
 *
 *  Created on: 3 jun 2023
 *      Author: t126401
 */



#include "lv_thermostat.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "lvgl.h"




//#define CONFIG_LV_DISPLAY_WIDTH 480
//#define CONFIG_LV_DISPLAY_height 272

lv_obj_t *lv_main_screen;
lv_obj_t *lv_layout_notification;
lv_obj_t *lv_layout_temperature;
lv_obj_t *lv_layout_threshold;
lv_obj_t *lv_layout_buttons_threshold;
lv_obj_t *lv_layout_schedule;
lv_obj_t *lv_text_temperature;
lv_obj_t *lv_icon_themometer;
lv_obj_t *lv_icon_heating;
lv_obj_t *lv_icon_threshold;
lv_obj_t *lv_text_threshold;
lv_obj_t *lv_text_status_application;
lv_obj_t *lv_text_from_schedule;
lv_obj_t *lv_text_to_schedule;
lv_obj_t *lv_progress_schedule;


lv_style_t lv_style_layout_temperature;
lv_style_t lv_style_layout_threshold;
lv_style_t lv_style_status_application;
lv_style_t lv_style_main_screen;
lv_style_t lv_style_layout_notification;
lv_style_t lv_style_lv_layout_buttons_threshold;
lv_style_t lv_style_button_threshold;
lv_style_t lv_style_layout_schedule;
lv_style_t lv_style_bar_schedule;


lv_obj_t *lv_date_text;
lv_obj_t *lv_icon_broker_status;
lv_obj_t *lv_icon_device_status_communication;
lv_obj_t *lv_icon_waiting_response;
lv_obj_t *lv_icon_action_upgrade;
lv_obj_t *lv_icon_ntp;
lv_obj_t *lv_icon_alarm;
lv_obj_t *lv_button_up_threshold;
lv_obj_t *lv_button_down_threshold;
lv_obj_t *lv_icon_up_threshold;
lv_obj_t *lv_icon_down_threshold;

LV_IMG_DECLARE(ic_action_online);
LV_IMG_DECLARE(ic_action_offline);
LV_IMG_DECLARE(ic_wifi_on);
LV_IMG_DECLARE(ic_wifi_off);
LV_IMG_DECLARE(ic_action_waiting_response);
LV_IMG_DECLARE(ic_action_upgrade);
LV_IMG_DECLARE(ic_ntp_off);
LV_IMG_DECLARE(ic_warning);
LV_IMG_DECLARE(ic_heating);
LV_IMG_DECLARE(ic_thermometer);
LV_IMG_DECLARE(ic_threshold);
LV_IMG_DECLARE(ic_up_threshold);
LV_IMG_DECLARE(ic_down_threshold);

//LV_FONT_CUSTOM_DECLARE(lv_font_led_30);
LV_FONT_DECLARE(russo48);
LV_FONT_DECLARE(russo36);






lv_theme_t *th;






static lv_obj_t *meter;
static lv_obj_t * btn;
static lv_disp_rot_t rotation = LV_DISP_ROT_NONE;

static void set_value(void *indic, int32_t v)
{
    lv_meter_set_indicator_end_value(meter, indic, v);
}

static void btn_cb(lv_event_t * e)
{
    lv_disp_t *disp = lv_event_get_user_data(e);
    rotation++;
    if (rotation > LV_DISP_ROT_270) {
        rotation = LV_DISP_ROT_NONE;
    }
    lv_disp_set_rotation(disp, rotation);
}

void lv_set_style_screen(lv_disp_t *display) {


	lv_theme_default_init(lv_obj_get_disp(lv_scr_act()),  /*Use the DPI, size, etc from this display*/
					lv_color_hex(0x0534F0), lv_color_hex(0x0534F0),   /*Primary and secondary palette*/
			                                        true,    /*Light or dark mode*/
			                                        &lv_font_montserrat_16); /*Small, normal, large fonts*/


	lv_style_init(&lv_style_main_screen);
	lv_obj_add_style(lv_main_screen, &lv_style_main_screen, LV_STATE_DEFAULT);
	//lv_style_set_bg_color(&lv_style_main_screen, lv_color_hex(0xcccccc));
	//lv_style_set_bg_grad_color(&lv_style_main_screen, lv_color_hex(0xdddddd));
    /*Make a gradient*/
/*
    lv_style_set_bg_opa(&lv_style_main_screen, LV_OPA_COVER);
    lv_style_set_bg_color(&lv_style_main_screen, lv_color_hex(0));
    lv_style_set_bg_grad_color(&lv_style_main_screen, lv_color_hex(0x0534F0));
    lv_style_set_bg_grad_dir(&lv_style_main_screen, LV_GRAD_DIR_VER);
*/
	//lv_style_set_border_color(&lv_style_main_screen, lv_color_hex(0xcccccc));





}

void lv_set_style_layout_notification() {



	lv_style_init(&lv_style_layout_notification);
	lv_obj_add_style(lv_layout_notification, &lv_style_layout_notification, LV_STATE_DEFAULT);
	lv_obj_set_style_base_dir(lv_layout_notification, LV_BASE_DIR_RTL, 1);
    lv_style_set_bg_opa(&lv_style_layout_notification, LV_OPA_TRANSP);
    lv_style_set_bg_color(&lv_style_layout_notification, lv_color_hex(0));
    lv_style_set_bg_grad_color(&lv_style_layout_notification, lv_color_hex(0x0534F0));
    lv_style_set_bg_grad_dir(&lv_style_layout_notification, LV_GRAD_DIR_HOR);
    lv_style_set_border_width(&lv_style_layout_notification, 0);

}



void lv_create_layout_nofitification(DATOS_APLICACION *datosApp) {

	lv_layout_notification = lv_obj_create(lv_main_screen);
	lv_set_style_layout_notification();
	lv_obj_set_size(lv_layout_notification, CONFIG_LCD_H_RES, LV_SIZE_CONTENT);
	lv_obj_set_pos(lv_layout_notification,0, 0);
	lv_obj_set_flex_flow(lv_layout_notification, LV_FLEX_FLOW_ROW_REVERSE);
	lv_obj_set_flex_align(lv_layout_notification, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
	lv_obj_set_style_pad_all(lv_layout_notification, 5, LV_PART_MAIN);

	//Etiqueta hora
	lv_date_text = lv_label_create(lv_layout_notification);
	lv_label_set_text_fmt(lv_date_text, "12:05");
	lv_obj_set_style_text_font(lv_date_text, &lv_font_montserrat_16, LV_STATE_DEFAULT);
	//lv_obj_center(lv_date_text);
	lv_obj_align(lv_date_text, LV_ALIGN_TOP_MID, 0, 0);

	//icono conexion al broker
	lv_icon_broker_status = lv_img_create(lv_layout_notification);
	lv_img_set_src(lv_icon_broker_status, &ic_wifi_off);


	//Icono conexion del dispositivo
	lv_icon_device_status_communication = lv_img_create(lv_layout_notification);
	lv_img_set_src(lv_icon_device_status_communication, &ic_action_offline);

	lv_icon_action_upgrade = lv_img_create(lv_layout_notification);
	lv_img_set_src(lv_icon_action_upgrade, &ic_action_upgrade);
	lv_obj_add_flag(lv_icon_action_upgrade, LV_OBJ_FLAG_HIDDEN);

	lv_icon_ntp = lv_img_create(lv_layout_notification);
	lv_img_set_src(lv_icon_ntp, &ic_ntp_off);
	lv_obj_add_flag(lv_icon_ntp, LV_OBJ_FLAG_HIDDEN);

	lv_icon_alarm = lv_img_create(lv_layout_notification);
	lv_img_set_src(lv_icon_alarm, &ic_warning);
	lv_obj_add_flag(lv_icon_alarm, LV_OBJ_FLAG_HIDDEN);




}










void lv_create_layout_temperature(DATOS_APLICACION *datosApp) {

	char temperature[10];
	lv_layout_temperature = lv_obj_create(lv_main_screen);
	lv_obj_set_size(lv_layout_temperature, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_align_to(lv_layout_temperature, lv_layout_notification, LV_ALIGN_OUT_BOTTOM_LEFT, 120, 50);
	//lv_obj_set_pos(lv_layout_temperature, 100, 100);
	//lv_obj_center(lv_layout_temperature);
	lv_obj_set_flex_align(lv_layout_temperature, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

	// icono temperatura
	lv_icon_themometer = lv_img_create(lv_layout_temperature);
	lv_img_set_src(lv_icon_themometer, &ic_thermometer);

	// texto temperatura
	lv_text_temperature = lv_label_create(lv_layout_temperature);
	sprintf(temperature, "%.1f", datosApp->termostato.tempActual);

	lv_label_set_text_fmt(lv_text_temperature, "%s ºC", temperature);
    lv_obj_set_style_text_font(lv_text_temperature, &russo48, LV_PART_MAIN);
	lv_set_style_layout_temperature();
	lv_obj_center(lv_text_temperature);





}

void lv_set_style_status_application() {

	lv_style_init(&lv_style_status_application);
	lv_style_set_text_font(&lv_style_status_application, &lv_font_montserrat_22);
	lv_obj_add_style(lv_text_status_application, &lv_style_status_application, LV_STATE_DEFAULT);
	//lv_obj_align_to(lv_text_status_application, lv_layout_temperature, LV_ALIGN_OUT_TOP_MID, 0, 0);



}

void lv_create_status_aplication(DATOS_APLICACION *datosApp) {

	lv_text_status_application = lv_label_create(lv_main_screen);
	lv_obj_align_to(lv_text_status_application, lv_layout_temperature, LV_ALIGN_OUT_TOP_RIGHT, 65,10);
	lv_set_style_status_application();
	lv_status_application(datosApp);


}








void lv_status_device(DATOS_APLICACION *datosApp) {

	lv_status_communication(datosApp);
}

void lv_set_style_layout_temperature() {

	lv_style_init(&lv_style_layout_temperature);
	lv_obj_add_style(lv_layout_temperature, &lv_style_layout_temperature, LV_STATE_DEFAULT);
	lv_style_set_text_font(&lv_style_layout_temperature, &lv_font_montserrat_48);
	lv_obj_clear_flag(lv_layout_temperature, LV_OBJ_FLAG_SCROLLABLE);
    lv_style_set_bg_opa(&lv_style_layout_temperature, LV_OPA_TRANSP);
    lv_style_set_border_width(&lv_style_layout_temperature, 0);
    lv_style_set_pad_all(&lv_style_layout_temperature, 0);



}

void lv_set_style_threshold() {

	lv_style_init(&lv_style_layout_threshold);
	lv_obj_add_style(lv_layout_threshold, &lv_style_layout_threshold, LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(lv_layout_threshold, &russo36, LV_PART_MAIN);
	lv_obj_clear_flag(lv_layout_threshold, LV_OBJ_FLAG_SCROLLABLE);
    lv_style_set_bg_opa(&lv_style_layout_threshold, LV_OPA_TRANSP);
    lv_style_set_border_width(&lv_style_layout_threshold, 0);
    lv_style_set_pad_all(&lv_style_layout_threshold, 0);



}


void lv_create_layout_threshold(DATOS_APLICACION *datosApp) {

	lv_layout_threshold =  lv_obj_create(lv_main_screen);
	lv_obj_align_to(lv_layout_threshold, lv_layout_temperature, LV_ALIGN_OUT_BOTTOM_LEFT, 15, 0);
	lv_obj_set_flex_align(lv_layout_threshold, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_icon_threshold = lv_img_create(lv_layout_threshold);
	lv_img_set_src(lv_icon_threshold, &ic_threshold);
	lv_text_threshold = lv_label_create(lv_layout_threshold);
	lv_obj_set_y(lv_text_threshold, 25);
	//lv_set_text_threshold(datosApp);
	//sprintf(temperature, "%.1f ºC", datosApp->termostato.tempUmbral);
	//lv_label_set_text(lv_text_threshold, temperature);
	lv_set_style_threshold();
	lv_obj_set_size(lv_layout_threshold, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	//lv_obj_align(lv_text_threshold, LV_ALIGN_OUT_BOTTOM_MID, 40, 25);





}



void lv_status_application(DATOS_APLICACION *datosApp) {


	switch(datosApp->datosGenerales->estadoApp) {

	case NORMAL_AUTO:
	case NORMAL_AUTOMAN:
		lv_label_set_text(lv_text_status_application, "AUTO");
		break;
	case NORMAL_MANUAL:
		lv_label_set_text(lv_text_status_application, "MANUAL");
		break;
	case UPGRADE_EN_PROGRESO:
		lv_label_set_text(lv_text_status_application, "ACTUALIZANDO VERSION");
		break;
	default:
		lv_label_set_text(lv_text_status_application, "----");
		break;


	}

}

void lv_create_heating_icon(DATOS_APLICACION *datosApp) {

	lv_icon_heating = lv_img_create(lv_main_screen);
	lv_img_set_src(lv_icon_heating, &ic_heating);
	lv_obj_align_to(lv_icon_heating, lv_layout_temperature, LV_ALIGN_OUT_RIGHT_MID, 50, 30);



}

void lv_set_status_heating(DATOS_APLICACION *datosApp, enum ESTADO_RELE rele) {

	if (rele == ON) {

		lv_obj_clear_flag(lv_icon_heating, LV_OBJ_FLAG_HIDDEN);
	} else {
		lv_obj_add_flag(lv_icon_heating, LV_OBJ_FLAG_HIDDEN);
	}




}


void lv_set_style_buttons_threshold() {

	lv_style_init(&lv_style_button_threshold);
	lv_obj_add_style(lv_button_up_threshold, &lv_style_button_threshold, LV_STATE_DEFAULT);
	lv_obj_add_style(lv_button_down_threshold, &lv_style_button_threshold, LV_STATE_DEFAULT);
	lv_style_set_pad_all(&lv_style_button_threshold, 0);
	lv_style_set_border_width(&lv_style_button_threshold, 0);

}

static void event_handler_up_threshold(lv_obj_t *obj, lv_event_t *event) {

	DATOS_APLICACION *datosApp;
	datosApp = (DATOS_APLICACION*) 	lv_event_get_user_data(event);
	datosApp->termostato.tempUmbral += datosApp->termostato.incdec;
	lv_set_text_threshold(datosApp);
	//appuser_send_update_threshold

}


static void event_handler_down_threshold(lv_obj_t *obj, lv_event_t *event) {

	DATOS_APLICACION *datosApp;
	datosApp = (DATOS_APLICACION*) 	lv_event_get_user_data(event);
	datosApp->termostato.tempUmbral -= datosApp->termostato.incdec;
	lv_set_text_threshold(datosApp);

}


void lv_create_layout_buttons_threshold(DATOS_APLICACION *datosApp) {


	lv_layout_buttons_threshold = lv_obj_create(lv_main_screen);
	lv_set_style_layout_buttons_threshold();
	lv_obj_set_flex_flow(lv_layout_buttons_threshold, LV_FLEX_FLOW_COLUMN);

	lv_button_up_threshold = lv_btn_create(lv_layout_buttons_threshold);
	lv_button_down_threshold = lv_btn_create(lv_layout_buttons_threshold);
	lv_obj_add_event_cb(lv_button_up_threshold, event_handler_up_threshold, LV_EVENT_CLICKED, datosApp);
	lv_obj_add_event_cb(lv_button_down_threshold, event_handler_down_threshold, LV_EVENT_CLICKED, datosApp);
	lv_icon_up_threshold = lv_img_create(lv_button_up_threshold);
	lv_img_set_src(lv_icon_up_threshold, &ic_up_threshold);
	lv_icon_down_threshold = lv_img_create(lv_button_down_threshold);
	lv_img_set_src(lv_icon_down_threshold, &ic_down_threshold);
	lv_obj_set_size(lv_layout_buttons_threshold, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_pos(lv_layout_buttons_threshold, 10, 60);
	lv_set_style_buttons_threshold();



}
void lv_set_style_layout_buttons_threshold() {

	lv_style_init(&lv_style_lv_layout_buttons_threshold);
	lv_obj_add_style(lv_layout_buttons_threshold, &lv_style_lv_layout_buttons_threshold, LV_STATE_DEFAULT);
	lv_style_set_pad_all(&lv_style_lv_layout_buttons_threshold, 0);
	lv_style_set_border_width(&lv_style_lv_layout_buttons_threshold, 2);
	lv_style_set_border_color(&lv_style_lv_layout_buttons_threshold, lv_color_hex(0xcccccc));

}


void lv_create_layout_schedule(DATOS_APLICACION *datosApp) {


	lv_layout_schedule = lv_obj_create(lv_main_screen);
	lv_obj_set_flex_flow(lv_layout_schedule, LV_FLEX_FLOW_ROW);
	lv_text_from_schedule = lv_label_create(lv_layout_schedule);
	lv_progress_schedule = lv_bar_create(lv_layout_schedule);
	lv_text_to_schedule = lv_label_create(lv_layout_schedule);
	lv_label_set_text(lv_text_from_schedule, "07:00");
	lv_label_set_text(lv_text_to_schedule, "18:50");
	lv_obj_set_size(lv_layout_schedule, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_align_to(lv_layout_schedule, lv_main_screen, LV_ALIGN_BOTTOM_MID, 0, 20);
	lv_set_style_layout_schedule();
	lv_bar_set_range(lv_progress_schedule, 0, 100);
	lv_update_bar_schedule(datosApp);
	lv_set_style_bar_schedule();




}

void lv_set_style_layout_schedule() {

	lv_style_init(&lv_style_layout_schedule);
	lv_obj_add_style(lv_layout_schedule, &lv_style_layout_schedule, LV_STATE_DEFAULT);
    lv_style_set_bg_opa(&lv_style_layout_schedule, LV_OPA_TRANSP);
    lv_style_set_border_width(&lv_style_layout_schedule, 0);
    lv_style_set_pad_all(&lv_style_layout_schedule, 0);
    lv_set_style_bar_schedule();

}

void lv_set_style_bar_schedule() {

	lv_style_init(&lv_style_bar_schedule);
	lv_obj_add_style(lv_progress_schedule, &lv_style_bar_schedule, LV_STATE_DEFAULT);


}


void lv_update_bar_schedule(DATOS_APLICACION *datosApp) {

	uint8_t hour_from = 12;
	uint8_t minute_from = 35;
	uint8_t hour_to = 18;
	uint8_t minute_to = 0;
	TIME_PROGRAM schedules;
	int current_schedule;
	int current_value =35;
	int progress;
	bool active_schedule = true;

	time_t interval_from = 25;
	time_t interval_to = 250;

	//pendiente determinar si el schedule esta activo o no
	if (!active_schedule) {
		lv_obj_add_flag(lv_layout_schedule, LV_OBJ_FLAG_HIDDEN);

		return;
	}

	lv_obj_clear_flag(lv_layout_schedule, LV_OBJ_FLAG_HIDDEN);

	lv_label_set_text_fmt(lv_text_from_schedule, "%02d:%02d", hour_from, minute_from);
	lv_label_set_text_fmt(lv_text_to_schedule, "%02d:%02d", hour_to, minute_to);

	progress = (current_value * 100) / (interval_to - interval_from);

	lv_bar_set_value(lv_progress_schedule, progress, LV_ANIM_OFF);

}

// BROKER STATUS
//UPGRADE STATUS
//ALARMA_STATUS
//STATUS APP
// TEMPERATURE & THRESHOLD & RELAY
// UPDATE_SCHEDULE
// UPDATE TIME


void lv_status_communication(DATOS_APLICACION *datosApp) {

#define ALARMA_WIFI 0
#define ALARMA_MQTT 1
#define ALARMA_NTP 2
#define ALARMA_NVS 3
#define ALARMA_SENSOR_DHT 4
#define ALARMA_SENSOR_REMOTO 5

	// ALARMA WIFI
	if (datosApp->alarmas[0].estado_alarma == ALARMA_ON) {

		lv_img_set_src(lv_icon_device_status_communication, &ic_action_offline);


	} else {
		lv_img_set_src(lv_icon_device_status_communication, &ic_action_online);

	}

	//ALARMA MQTT

	if(datosApp->alarmas[1].estado_alarma == ALARMA_ON) {
		lv_img_set_src(lv_icon_broker_status, &ic_wifi_off);

	} else {
		lv_img_set_src(lv_icon_broker_status, &ic_wifi_on);
	}

	//ALARMA NTP

	if(datosApp->alarmas[2].estado_alarma == ALARMA_ON) {

		lv_obj_clear_flag(lv_icon_ntp, LV_OBJ_FLAG_HIDDEN);
	} else {
		lv_obj_add_flag(lv_icon_ntp, LV_OBJ_FLAG_HIDDEN);
	}

	// ALARMA NVS
	if((datosApp->alarmas[3].estado_alarma == ALARMA_ON) ||
			(datosApp->alarmas[4].estado_alarma == ALARMA_ON) ||
					((datosApp->alarmas[5].estado_alarma == ALARMA_ON))){
		lv_img_set_src(lv_icon_alarm, &ic_warning);
		lv_obj_clear_flag(lv_icon_alarm, LV_OBJ_FLAG_HIDDEN);
	} else {
		lv_obj_add_flag(lv_icon_alarm, LV_OBJ_FLAG_HIDDEN);
	}

/*
	//ALARMA SENSOR TEMPERATURA LOCAL
	if(datosApp->alarmas[4].estado_alarma == ALARMA_ON) {
		lv_img_set_src(lv_icon_alarm, &ic_warning);
		lv_obj_clear_flag(lv_icon_alarm, LV_OBJ_FLAG_HIDDEN);
	} else {
		lv_obj_add_flag(lv_icon_alarm, LV_OBJ_FLAG_HIDDEN);
	}

	//ALARMA SENSOR_TEMPERATURA_REMOTO

	if(datosApp->alarmas[5].estado_alarma == ALARMA_ON) {
		lv_img_set_src(lv_icon_alarm, &ic_warning);
		lv_obj_clear_flag(lv_icon_alarm, LV_OBJ_FLAG_HIDDEN);
	} else {
		lv_obj_add_flag(lv_icon_alarm, LV_OBJ_FLAG_HIDDEN);
	}
	*/

}

void example_lvgl_demo_ui(lv_disp_t *disp)
{
    lv_obj_t *scr = lv_disp_get_scr_act(disp);
    meter = lv_meter_create(scr);
    lv_obj_center(meter);
    lv_obj_set_size(meter, 200, 200);

    /*Add a scale first*/
    lv_meter_scale_t *scale = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale, 41, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(meter, scale, 8, 4, 15, lv_color_black(), 10);

    lv_meter_indicator_t *indic;

    /*Add a blue arc to the start*/
    indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 20);

    /*Make the tick lines blue at the start of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_BLUE), false, 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 20);

    /*Add a red arc to the end*/
    indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_set_indicator_start_value(meter, indic, 80);
    lv_meter_set_indicator_end_value(meter, indic, 100);

    /*Make the tick lines red at the end of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false, 0);
    lv_meter_set_indicator_start_value(meter, indic, 80);
    lv_meter_set_indicator_end_value(meter, indic, 100);

    /*Add a needle line indicator*/
    indic = lv_meter_add_needle_line(meter, scale, 4, lv_palette_main(LV_PALETTE_GREY), -10);

    btn = lv_btn_create(scr);
    lv_obj_t * lbl = lv_label_create(btn);
    lv_label_set_text_static(lbl, LV_SYMBOL_REFRESH" ROTATE");
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 30, -30);
    /*Button event*/
    lv_obj_add_event_cb(btn, btn_cb, LV_EVENT_CLICKED, disp);

    /*Create an animation to set the value*/
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, set_value);
    lv_anim_set_var(&a, indic);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_repeat_delay(&a, 100);
    lv_anim_set_playback_time(&a, 500);
    lv_anim_set_playback_delay(&a, 100);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
}




void lv_thermostat_code(lv_disp_t *display) {

	  DATOS_APLICACION *datosApp;
	  DATOS_GENERALES *datos;

	  datosApp = (DATOS_APLICACION*) calloc(1, sizeof(DATOS_APLICACION));
	  datos = (DATOS_GENERALES*) calloc(1, sizeof(DATOS_GENERALES));
	  datosApp->datosGenerales = datos;
	  datosApp->datosGenerales->estadoApp = NORMAL_AUTO;
	  datosApp->datosGenerales->estadoProgramacion = VALID_PROG;
	  datosApp->termostato.tempActual = 21.254;
	  datosApp->termostato.tempUmbral = 20;
	  datosApp->alarmas[0].estado_alarma = ALARMA_OFF;
	  datosApp->alarmas[1].estado_alarma = ALARMA_OFF;
	  datosApp->alarmas[2].estado_alarma = ALARMA_ON;
	  datosApp->alarmas[3].estado_alarma = ALARMA_ON;
	  datosApp->alarmas[4].estado_alarma = ALARMA_OFF;
	  datosApp->termostato.incdec = 0.5;

	  //lv_main_screen =lv_scr_act();
	  lv_main_screen = lv_disp_get_scr_act(display);
	  lv_set_style_screen(display);

	lv_create_layout_nofitification(datosApp);
	lv_create_layout_temperature(datosApp);
	lv_create_layout_threshold(datosApp);
	lv_create_status_aplication(datosApp);

	enum ESTADO_RELE estado = ON;
	lv_create_heating_icon(datosApp);

	lv_create_layout_buttons_threshold(datosApp);
	lv_create_layout_schedule(datosApp);
	lv_status_device(datosApp);
	lv_set_status_heating(datosApp, estado);
	lv_set_text_threshold(datosApp);

}


void lv_set_text_threshold(DATOS_APLICACION *datosApp) {

	char threshold[10];

	sprintf(threshold, "%.1f ºC", datosApp->termostato.tempUmbral);


	lv_label_set_text(lv_text_threshold, threshold);

}

