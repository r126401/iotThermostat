/*
 * lv_init_thermostat.c
 *
 *  Created on: 9 jun 2023
 *      Author: t126401
 */



#include "lvgl.h"
#include "lv_init_thermostat.h"
#include <stdlib.h>
#include "logging.h"
#include "esp_log.h"

static const char *TAG = "lv_factory_reset";

lv_obj_t *lv_screen_init_thermostat;
lv_obj_t *lv_img_reset_init;
lv_obj_t *lv_list_wifi_station;
lv_obj_t *lv_text_ssid;
lv_obj_t *lv_layout_wifi_stations;
lv_obj_t *lv_text_init_thermostat;
lv_obj_t *lv_button_wifi_stations;
lv_style_t lv_style_screen_init_thermostat;
lv_style_t lv_style_button_reset_init;
lv_style_t lv_style_wifi_stations;
lv_style_t lv_style_text_init_termostat;

lv_obj_t *lv_label_reset;
lv_obj_t *lv_button_reset;
lv_obj_t *lv_label_password = NULL;
lv_obj_t *lv_password_text = NULL;
lv_obj_t *lv_btn_ok = NULL;
lv_obj_t *lv_text_button = NULL;
lv_obj_t *lv_keyboard;
lv_obj_t *lv_label_connection_wifi;


static void lv_event_handler_button_reset(lv_event_t *e);
static void lv_event_handler_wifi_stations(lv_event_t *e);
static void lv_event_handler_list(lv_event_t *e);
static void lv_handler_keyboard(lv_event_t *e);


void lv_set_style_layout_wifi_stations() {

	lv_style_init(&lv_style_wifi_stations);
	lv_obj_add_style(lv_layout_wifi_stations, &lv_style_wifi_stations, LV_STATE_DEFAULT);


	lv_style_set_bg_opa(&lv_style_wifi_stations, LV_OPA_TRANSP);
    lv_style_set_border_width(&lv_style_wifi_stations, 0);
    lv_style_set_pad_all(&lv_style_wifi_stations, 0);

}


void lv_set_transition(lv_obj_t *obj) {


    static const lv_style_prop_t props[] = {LV_STYLE_BG_COLOR, LV_STYLE_BORDER_COLOR, LV_STYLE_BORDER_WIDTH, 0};


    static lv_style_transition_dsc_t trans_def;
    lv_style_transition_dsc_init(&trans_def, props, lv_anim_path_linear, 100, 0, NULL);

    /* A special transition when going to pressed state
     * Make it slow (500 ms) but start  without delay*/
    static lv_style_transition_dsc_t trans_pr;
    lv_style_transition_dsc_init(&trans_pr, props, lv_anim_path_linear, 500, 0, NULL);

    static lv_style_t style_def;
    lv_style_init(&style_def);
    lv_style_set_transition(&style_def, &trans_def);

    static lv_style_t style_pr;
    lv_style_init(&style_pr);
    lv_style_set_bg_color(&style_pr, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_border_width(&style_pr, 6);
    lv_style_set_border_color(&style_pr, lv_palette_darken(LV_PALETTE_RED, 3));
    lv_style_set_transition(&style_pr, &trans_pr);

    lv_obj_add_style(obj, &style_def, 0);
    lv_obj_add_style(obj, &style_pr, LV_STATE_PRESSED);


}

void lv_set_style_button_reset_init_thermostat() {

	lv_style_init(&lv_style_button_reset_init);
	lv_obj_add_style(lv_button_reset, &lv_style_button_reset_init, LV_STATE_DEFAULT);
	lv_obj_add_style(lv_button_wifi_stations, &lv_style_button_reset_init, LV_STATE_DEFAULT);
	lv_style_set_pad_all(&lv_style_button_reset_init, 0);
	lv_style_set_text_font(&lv_style_button_reset_init, &lv_font_montserrat_26);
    lv_style_set_bg_opa(&lv_style_button_reset_init, LV_OPA_TRANSP);
    lv_style_set_text_color(&lv_style_button_reset_init, lv_color_hex(0x2A8AD2));
    lv_style_set_shadow_width(&lv_style_button_reset_init, 55);
    lv_style_set_shadow_color(&lv_style_button_reset_init, lv_palette_main(LV_PALETTE_BLUE));

    lv_set_transition(lv_button_reset);
    lv_set_transition(lv_button_wifi_stations);






}



void lv_set_style_text_init_thermostat() {

	lv_style_init(&lv_style_text_init_termostat);
	lv_obj_add_style(lv_text_init_thermostat, &lv_style_text_init_termostat, LV_STATE_DEFAULT);
	lv_style_set_text_font(&lv_style_text_init_termostat, &lv_font_montserrat_48);
    lv_style_set_bg_opa(&lv_style_text_init_termostat, LV_OPA_TRANSP);
    lv_style_set_shadow_width(&lv_style_text_init_termostat, 100);
    lv_style_set_shadow_color(&lv_style_text_init_termostat, lv_palette_main(LV_PALETTE_BLUE));

}


void lv_set_style_screen_init_thermostat() {

	lv_style_init(&lv_style_screen_init_thermostat);
	lv_theme_default_init(lv_obj_get_disp(lv_scr_act()),  /*Use the DPI, size, etc from this display*/
					lv_color_hex(0x0534F0), lv_color_hex(0x0534F0),   /*Primary and secondary palette*/
			                                        true,    /*Light or dark mode*/
			                                        &lv_font_montserrat_16); /*Small, normal, large fonts*/

	lv_obj_add_style(lv_screen_init_thermostat, &lv_style_screen_init_thermostat, LV_PART_MAIN);



    //lv_style_set_shadow_ofs_x(&lv_style_screen_init_thermostat, 0);
    //lv_style_set_shadow_ofs_y(&lv_style_screen_init_thermostat, 20);





}



void lv_back_wifi_layout(DATOS_APLICACION *datosApp) {

	lv_obj_add_flag(lv_layout_wifi_stations, LV_OBJ_FLAG_HIDDEN);
	if (lv_text_ssid != NULL) {
		lv_obj_add_flag(lv_text_ssid, LV_OBJ_FLAG_HIDDEN);
	}

	lv_create_layout_init_thermostat(datosApp);
	lv_obj_clear_flag(lv_button_wifi_stations, LV_OBJ_FLAG_HIDDEN);
	if (lv_keyboard != NULL) {
		lv_obj_add_flag(lv_keyboard, LV_OBJ_FLAG_HIDDEN);

	}


}

static void lv_event_handler_button_reset(lv_event_t *e) {

	DATOS_APLICACION *datosApp;
	datosApp = (DATOS_APLICACION*) 	lv_event_get_user_data(e);

	if (lv_layout_wifi_stations != NULL) {
		if (lv_obj_has_flag(lv_layout_wifi_stations, LV_OBJ_FLAG_HIDDEN)) {
			lv_label_set_text(lv_text_init_thermostat, "Reiniciando...");
			//ejectuar reset del dispositivo
			//appuser_reset_device();

		} else {
			lv_back_wifi_layout(datosApp);

		}


	} else {

		lv_label_set_text(lv_text_init_thermostat, "Reiniciando...");
		//ejectuar reset del dispositivo
		//appuser_reset_device(

	}







}

static void lv_event_handler_wifi_stations(lv_event_t *e) {

	DATOS_APLICACION *datosApp;
	datosApp = (DATOS_APLICACION*) 	lv_event_get_user_data(e);

	lv_obj_add_flag(lv_text_init_thermostat, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(lv_label_connection_wifi, LV_OBJ_FLAG_HIDDEN);
	lv_label_set_text(lv_label_reset, LV_SYMBOL_BACKSPACE);
	lv_create_layout_search_ssid(datosApp);



}





void lv_create_layout_init_thermostat(DATOS_APLICACION *datosApp) {


	if (lv_screen_init_thermostat == NULL) {
		lv_screen_init_thermostat = lv_obj_create(NULL);
		lv_scr_load(lv_screen_init_thermostat);
		lv_obj_clear_flag(lv_screen_init_thermostat, LV_OBJ_FLAG_SCROLLABLE);
	}

	lv_set_style_screen_init_thermostat();

	if (lv_text_init_thermostat == NULL) {
		lv_text_init_thermostat = lv_label_create(lv_screen_init_thermostat);
		lv_set_style_text_init_thermostat();
		lv_obj_center(lv_text_init_thermostat);
	}

	lv_label_set_text(lv_text_init_thermostat, "Inicializando...");
	lv_obj_clear_flag(lv_text_init_thermostat, LV_OBJ_FLAG_HIDDEN);


	if (lv_button_reset == NULL) {
		lv_button_reset = lv_btn_create(lv_screen_init_thermostat);
		lv_label_reset = lv_label_create(lv_button_reset);
		lv_obj_align_to(lv_button_reset, lv_screen_init_thermostat, LV_ALIGN_TOP_LEFT, 15, 15);
		lv_obj_add_event_cb(lv_button_reset, lv_event_handler_button_reset, LV_EVENT_CLICKED, datosApp);
	}


	lv_label_set_text(lv_label_reset, LV_SYMBOL_REFRESH);



	if (lv_button_wifi_stations == NULL) {
		lv_button_wifi_stations = lv_btn_create(lv_screen_init_thermostat);
		lv_obj_t *lv_label_button = lv_label_create(lv_button_wifi_stations);
		lv_label_set_text(lv_label_button, LV_SYMBOL_WIFI);
		lv_obj_align_to(lv_button_wifi_stations, lv_screen_init_thermostat, LV_ALIGN_TOP_RIGHT, -40, 15);
		lv_obj_add_event_cb(lv_button_wifi_stations, lv_event_handler_wifi_stations, LV_EVENT_CLICKED, datosApp);
		//lv_set_style_button_reset_init_thermostat();
	}




	if (lv_label_connection_wifi == NULL) {
		lv_label_connection_wifi = lv_label_create(lv_screen_init_thermostat);
		lv_obj_align_to(lv_label_connection_wifi, lv_text_init_thermostat, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
		lv_label_set_text_fmt(lv_label_connection_wifi, "Conectando a : %s", "NuevoSalon");
	}
	lv_obj_clear_flag(lv_label_connection_wifi, LV_OBJ_FLAG_HIDDEN);









}



void lv_init_data_init_thermostat(DATOS_APLICACION *datosApp) {


	ESP_LOGI(TAG, ""TRAZAR"LEVANTAMOS LA PANTALLA NORMAL DE INICIALIZANDO...", INFOTRAZA);

	  lv_create_layout_init_thermostat(datosApp);
	  //create_screen_layout_search_ssid(datosApp);



}


void lv_create_items(char station_list[10][50]) {

	strncpy(station_list[0], "Salon", 50);
	strncpy(station_list[1], "NuevoSalon", 50);
	strncpy(station_list[2], "patio", 50);
	strncpy(station_list[3], "comedor", 50);
	strncpy(station_list[4], "despacho", 50);
	strncpy(station_list[5], "Salon2", 50);
	strncpy(station_list[6], "NuevoSalon2", 50);
	strncpy(station_list[7], "patio2", 50);
	strncpy(station_list[8], "comedor2", 50);
	strncpy(station_list[9], "despacho2", 50);


}


static void lv_event_handler_list(lv_event_t *e) {

	DATOS_APLICACION *datosApp;

	datosApp = (DATOS_APLICACION*) 	lv_event_get_user_data(e);
	lv_obj_t * obj = lv_event_get_target(e);
	if (lv_text_ssid == NULL) {
		lv_text_ssid = lv_label_create(lv_screen_init_thermostat);
	}
	lv_label_set_text(lv_text_ssid, lv_list_get_btn_text(lv_list_wifi_station, obj));
	lv_create_layout_password_wifi(*datosApp);



}



void lv_create_layout_search_ssid(DATOS_APLICACION *datosApp) {

	int i;
	int size=10;
	char station_list[10][50];
	lv_obj_t * btn;




	//Creamos el layout donde estaran todos los componentes para buscar ssid wifi
	if (lv_layout_wifi_stations == NULL) {
		lv_layout_wifi_stations = lv_obj_create(lv_screen_init_thermostat);
		lv_obj_set_pos(lv_layout_wifi_stations, 50, 30);
		if (lv_list_wifi_station == NULL) {
			lv_list_wifi_station = lv_list_create(lv_layout_wifi_stations);
			lv_create_items(station_list);
			lv_list_add_text(lv_list_wifi_station, "Estaciones wifi");
		}
		lv_obj_set_size(lv_layout_wifi_stations, lv_pct(100), lv_pct(70));
		for (i=0;i<size;i++) {
			btn = lv_list_add_btn(lv_list_wifi_station, LV_SYMBOL_WIFI, station_list[i]);
		    lv_obj_add_event_cb(btn, lv_event_handler_list, LV_EVENT_CLICKED, datosApp);
		}
		lv_obj_set_size(lv_list_wifi_station, lv_pct(50), lv_pct(100));
		lv_set_style_layout_wifi_stations();

	} else {
		lv_obj_clear_flag(lv_layout_wifi_stations, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(lv_list_wifi_station, LV_OBJ_FLAG_HIDDEN);
		if (lv_password_text != NULL) {
			lv_obj_add_flag(lv_password_text, LV_OBJ_FLAG_HIDDEN);
		}

		if (lv_label_password != NULL) {
			lv_obj_add_flag(lv_label_password, LV_OBJ_FLAG_HIDDEN);
		}

		if (lv_text_ssid != NULL) {
			lv_obj_add_flag(lv_text_ssid, LV_OBJ_FLAG_HIDDEN);
		}


	}
	lv_obj_add_flag(lv_button_wifi_stations, LV_OBJ_FLAG_HIDDEN);
}



static void lv_handler_keyboard(lv_event_t *e) {


	uint16_t kcode;
	int length;
    lv_obj_t * kb = lv_event_get_user_data(e);
    kcode = lv_keyboard_get_selected_btn(kb);


    length = strlen(lv_textarea_get_text(lv_password_text));
    if (length >= 8) {
    	lv_obj_set_style_text_color(lv_password_text, lv_color_hex(0xccaaaa), LV_PART_MAIN);
        if ((kcode == 40) || (kcode == 22)) {
        	lv_label_set_text_fmt(lv_text_ssid, "%d", kcode);
        	//reiniciar el dispositivo
        }

    } else {
    	lv_obj_set_style_text_color(lv_password_text, lv_color_hex(0xffffff), LV_PART_MAIN);
    }


}


void lv_create_layout_password_wifi(DATOS_APLICACION datosApp) {

	//posicionamiento de la etiqueta ssid


	lv_obj_add_flag(lv_list_wifi_station, LV_OBJ_FLAG_HIDDEN);
	lv_obj_align_to(lv_text_ssid, lv_layout_wifi_stations, LV_ALIGN_TOP_MID, -50, 0);
	//lv_obj_set_pos(lv_text_ssid, lv_pct(70), lv_pct(20));


	//etiqueta password
	if (lv_label_password == NULL) {
		lv_label_password = lv_label_create(lv_layout_wifi_stations);
		lv_obj_align_to(lv_label_password, lv_text_ssid, LV_ALIGN_OUT_BOTTOM_MID, -100, 10);
	}
	//caja password
	if (lv_password_text == NULL) {
		lv_password_text = lv_textarea_create(lv_layout_wifi_stations);
	    lv_textarea_set_text(lv_password_text, "");
	    lv_textarea_set_password_mode(lv_password_text, true);
	    lv_textarea_set_one_line(lv_password_text, true);
		lv_label_set_text(lv_label_password, "Password:");
		lv_obj_align_to(lv_password_text, lv_label_password, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
		lv_obj_set_width(lv_password_text, lv_pct(25));
		lv_obj_set_style_bg_opa(lv_password_text, LV_OPA_TRANSP, 0);
		lv_obj_set_style_border_width(lv_password_text, 0, LV_PART_MAIN);

	}


	if (lv_keyboard == NULL) {
	    lv_keyboard = lv_keyboard_create(lv_screen_init_thermostat);
	    lv_obj_set_size(lv_keyboard,  LV_HOR_RES , LV_VER_RES / 2);
	    lv_obj_add_event_cb(lv_keyboard, lv_handler_keyboard, LV_EVENT_CLICKED, lv_keyboard);
	}

    lv_keyboard_set_textarea(lv_keyboard, lv_password_text); /*Focus it on one of the text areas to start*/
    lv_obj_clear_flag(lv_keyboard, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(lv_password_text, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(lv_label_password, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(lv_text_ssid, LV_OBJ_FLAG_HIDDEN);

}

void lv_delete_objects_layout_wifi() {

	lv_obj_del(lv_layout_wifi_stations);
	lv_layout_wifi_stations = NULL;

}





