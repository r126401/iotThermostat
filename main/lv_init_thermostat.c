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
#include "conexiones.h"
#include "esp_wifi.h"
#include "lv_tools.h"

static const char *TAG = "lv_factory_reset";

lv_obj_t *lv_screen_init_thermostat = NULL;
lv_obj_t *lv_img_reset_init = NULL;
lv_obj_t *lv_list_wifi_station = NULL;
lv_obj_t *lv_text_ssid = NULL;
lv_obj_t *lv_layout_wifi_stations = NULL;
lv_obj_t *lv_text_init_thermostat = NULL;
lv_obj_t *lv_button_wifi_stations = NULL;

lv_obj_t *lv_box_connecting_wifi = NULL;


lv_style_t lv_style_screen_init_thermostat;
lv_style_t lv_style_button_reset_init;
lv_style_t lv_style_wifi_stations;
lv_style_t lv_style_text_init_termostat;
lv_style_t lv_style_text_fail;
lv_style_t lv_style_text_password;

lv_style_t style;


lv_obj_t *lv_label_reset = NULL;
lv_obj_t *lv_button_reset = NULL;
lv_obj_t *lv_label_password = NULL;
lv_obj_t *lv_password_text = NULL;
lv_obj_t *lv_btn_ok = NULL;
lv_obj_t *lv_text_button = NULL;
lv_obj_t *lv_keyboard = NULL;
lv_obj_t *lv_label_connection_wifi = NULL;



lv_obj_t *lv_anim_wait = NULL;


static void lv_event_handler_button_reset(lv_event_t *e);
static void lv_event_handler_wifi_stations(lv_event_t *e);
static void lv_event_handler_list(lv_event_t *e);
static void lv_handler_keyboard(lv_event_t *e);






lv_style_t style_anim;
lv_obj_t *spinner = NULL;
lv_obj_t *lv_label_message = NULL;


void lv_set_style_to_text(lv_obj_t *obj, lv_style_t *style, lv_font_t *font, lv_coord_t border_width, uint32_t color_text) {

	lv_style_init(style);
	lv_style_set_text_font(style, font);
    lv_style_set_bg_opa(style, LV_OPA_TRANSP);
    lv_style_set_border_opa(style, LV_OPA_TRANSP);
    lv_style_set_border_width(style, border_width);
    lv_style_set_text_color(style, lv_color_hex(color_text));
    lv_obj_add_style(obj, style, LV_STATE_DEFAULT);





}




void lv_set_style_anim_wait_event(lv_obj_t *obj) {

	lv_style_init(&style_anim);
	lv_style_set_text_font(&style_anim, &lv_font_montserrat_26);
    lv_style_set_bg_opa(&style_anim, LV_OPA_TRANSP);
    lv_obj_add_style(obj, &style_anim, LV_STATE_DEFAULT);
    lv_style_set_border_opa(&style_anim, LV_OPA_TRANSP);



}





void lv_create_anim_wait_event(char* message) {


	lv_anim_wait = lv_obj_create(lv_screen_init_thermostat);
	lv_obj_center(lv_anim_wait);


	//lv_obj_set_pos(lv_layout_anim, pos.x, pos.y);
	lv_obj_set_size(lv_anim_wait, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_set_style_anim_wait_event(lv_anim_wait);

	if (spinner == NULL) {
		spinner = lv_spinner_create(lv_anim_wait, 5000, 60);
	}
	lv_obj_set_size(spinner, 24,24);
	lv_obj_center(spinner);


	if (lv_label_message == NULL) {
		lv_label_message = lv_label_create(lv_anim_wait);
	}
	lv_obj_align_to(lv_label_message, spinner, LV_ALIGN_OUT_BOTTOM_MID, 0, 50);
	lv_label_set_text(lv_label_message, message);
	lv_set_style_anim_wait_event(lv_label_message);


}

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
	lv_style_set_text_font(&lv_style_text_init_termostat, &lv_font_montserrat_26);
    lv_style_set_bg_opa(&lv_style_text_init_termostat, LV_OPA_TRANSP);
    //lv_style_set_shadow_width(&lv_style_text_init_termostat, 20);
    //lv_style_set_shadow_color(&lv_style_text_init_termostat, lv_palette_main(LV_PALETTE_BLUE));
    lv_obj_add_style(lv_text_init_thermostat, &lv_style_text_init_termostat, LV_STATE_DEFAULT);

}


void lv_set_style_screen_init_thermostat() {

	lv_style_init(&lv_style_screen_init_thermostat);
	lv_theme_default_init(lv_obj_get_disp(lv_scr_act()),  /*Use the DPI, size, etc from this display*/
					lv_color_hex(0x0534F0), lv_color_hex(0x0534F0),   /*Primary and secondary palette*/
			                                        true,    /*Light or dark mode*/
			                                        &lv_font_montserrat_16); /*Small, normal, large fonts*/

	lv_obj_add_style(lv_screen_init_thermostat, &lv_style_screen_init_thermostat, LV_PART_MAIN);
	lv_obj_clear_flag(lv_screen_init_thermostat, LV_OBJ_FLAG_SCROLLABLE);



    //lv_style_set_shadow_ofs_x(&lv_style_screen_init_thermostat, 0);
    //lv_style_set_shadow_ofs_y(&lv_style_screen_init_thermostat, 20);





}

void lv_delete_list_wifi_stations() {



	if (lv_list_wifi_station != NULL) {
		ESP_LOGI(TAG, ""TRAZAR"VAMOS A BORRAR LA LISTA", INFOTRAZA);
		lv_obj_del(lv_list_wifi_station);
		ESP_LOGI(TAG, ""TRAZAR"LISTA BORRADA", INFOTRAZA);
		lv_list_wifi_station = NULL;
	}else {
		ESP_LOGW(TAG, ""TRAZAR"la lista no existe", INFOTRAZA);
	}



}


void lv_back_wifi_layout() {


	ESP_LOGI(TAG, ""TRAZAR"Pulsado el boton atras", INFOTRAZA);

	//Escenario lista de redes visible
	if (lv_list_wifi_station != NULL) {
		ESP_LOGI(TAG, ""TRAZAR"ESCONDEMOS LAS REDES Y VOLVEMOS A ENSEÑAR LOS TEXTOS INICIALIZANDO", INFOTRAZA);
		if(!lv_obj_has_flag(lv_list_wifi_station, LV_OBJ_FLAG_HIDDEN)) {
			lv_obj_add_flag(lv_anim_wait, LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(lv_list_wifi_station, LV_OBJ_FLAG_HIDDEN);

		}
	}


	//Escenario ssid con password para introducir
	if (lv_text_ssid != NULL) {

		if (!lv_obj_has_flag(lv_text_ssid, LV_OBJ_FLAG_HIDDEN)) {
			ESP_LOGI(TAG, ""TRAZAR"ESCONDEMOS EL TEXTO PARA LA PASSWORD Y EL TECLADO", INFOTRAZA);
		}
		lv_obj_add_flag(lv_text_ssid, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(lv_password_text, LV_OBJ_FLAG_HIDDEN);
		if (lv_keyboard != NULL) {
			lv_obj_add_flag(lv_keyboard, LV_OBJ_FLAG_HIDDEN);

		}
	}


	lv_obj_clear_flag(lv_button_wifi_stations, LV_OBJ_FLAG_HIDDEN);
	lv_connecting_to_wifi_station();
	lv_label_set_text(lv_label_reset, LV_SYMBOL_REFRESH);




}

static void lv_event_handler_button_reset(lv_event_t *e) {



	if (strcmp(lv_label_get_text(lv_label_reset), LV_SYMBOL_REFRESH )) {

		ESP_LOGI(TAG, ""TRAZAR"SIMBOLO REINICIAR", INFOTRAZA);
		lv_back_wifi_layout();



	} else {
		ESP_LOGI(TAG, ""TRAZAR"SIMBOLO BACK", INFOTRAZA);
		lv_label_set_text(lv_text_init_thermostat, "Reiniciando...");
		esp_restart();

	}








}

void show_animation() {



		lv_obj_clear_flag(lv_anim_wait, LV_OBJ_FLAG_HIDDEN);
		ESP_LOGI(TAG, ""TRAZAR"ANIMACION ENSEÑADA", INFOTRAZA);





}

void hide_animation() {



		ESP_LOGI(TAG, ""TRAZAR"ESCONDEMOS LA ANIMACION", INFOTRAZA);
		lv_obj_add_flag(lv_anim_wait, LV_OBJ_FLAG_HIDDEN);




}






/**
 * Esta funcion es la que dispara la busqueda de redes wifi tras pulsar el boton de wifi.
 * @param e
 */
static void lv_event_handler_wifi_stations(lv_event_t *e) {


	uint16_t ap_count = 10;
	//Escondemos el texto inicializando y el conectando wifi
	lv_obj_add_flag(lv_box_connecting_wifi, LV_OBJ_FLAG_HIDDEN);
	//Escondemos el boton de estaciones wifi
	lv_obj_add_flag(lv_button_wifi_stations, LV_OBJ_FLAG_HIDDEN);
	lv_label_set_text(lv_label_reset, LV_SYMBOL_BACKSPACE);
	if (lv_anim_wait == NULL) {
		lv_create_anim_wait_event( "Buscando redes...");

	}
	//Mostramos la animacion para scanear redes
	show_animation();
	//Escaneamos las redes disponibles
	if (lv_list_wifi_station == NULL) {
		get_scan_station_list();
	} else {
		lv_create_layout_search_ssid(NULL, &ap_count);
	}


}






void lv_init_thermostat() {


	ESP_LOGI(TAG, ""TRAZAR"LEVANTAMOS LA PANTALLA NORMAL DE INICIALIZANDO...", INFOTRAZA);

	lv_anim_wait = NULL;


	//Creamos objetos
	//lv_screen_init_thermostat = lv_obj_create(NULL);
	lv_screen_init_thermostat = lv_scr_act();
	lv_scr_load(lv_screen_init_thermostat);
	lv_button_reset = lv_btn_create(lv_screen_init_thermostat);
	lv_label_reset = lv_label_create(lv_button_reset);
	lv_label_set_text(lv_label_reset, LV_SYMBOL_REFRESH);

	lv_button_wifi_stations = lv_btn_create(lv_screen_init_thermostat);
	lv_obj_t *lv_label_button = lv_label_create(lv_button_wifi_stations);
	lv_label_set_text(lv_label_button, LV_SYMBOL_WIFI);
	lv_obj_add_flag(lv_button_wifi_stations, LV_OBJ_FLAG_HIDDEN);


	//Creamos estilos
	lv_set_style_screen_init_thermostat();
	lv_set_style_button_reset_init_thermostat();


	//position objects
	lv_obj_set_pos(lv_button_reset, lv_pct(5), lv_pct(5));
	lv_obj_set_pos(lv_button_wifi_stations, lv_pct(90), lv_pct(5));



	//sizing objects



	//callback functions
	lv_obj_add_event_cb(lv_button_reset, lv_event_handler_button_reset, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event_cb(lv_button_wifi_stations, lv_event_handler_wifi_stations, LV_EVENT_CLICKED, NULL);


	  //create_screen_layout_search_ssid(datosApp);
	ESP_LOGI(TAG, ""TRAZAR"FIN DE INIT THERMOSTAT...", INFOTRAZA);


}


station_t* lv_get_list_stations(uint16_t *size) {

	wifi_ap_record_t list_stations[CONFIG_DEFAULT_SCAN_LIST_SIZE];
	int i;
	station_t *list;

	get_scan_station_list(list_stations, size);


	if (*size > 0) {
		list = (station_t*) calloc(*size, sizeof(station_t));

		for (i=0;i<*size;i++) {

			sprintf(list[i].ssid, "%s", list_stations[i].ssid);
			ESP_LOGI(TAG, ""TRAZAR"estacion: %s",INFOTRAZA, list[i].ssid);
			if (list_stations[i].authmode == WIFI_AUTH_OPEN) {
				list[i].open_ssid = true;
			} else {
				list[i].open_ssid = false;
			}

		}
		return list;
	} else {
		return NULL;
	}




}





static void lv_set_style_text(lv_obj_t *obj, lv_style_t *style) {

	lv_style_init(style);
	lv_obj_add_style(obj, style, LV_STATE_DEFAULT);
	lv_style_set_text_font(&lv_style_text_fail, &lv_font_montserrat_26);
    lv_style_set_bg_opa(style, LV_OPA_TRANSP);
    lv_style_set_text_color(style, lv_color_hex(0x0000FF));




}

static void lv_event_handler_list(lv_event_t *e) {


	lv_obj_t * obj = lv_event_get_target(e);
	if (lv_text_ssid == NULL) {
		lv_text_ssid = lv_label_create(lv_screen_init_thermostat);
	}
	lv_label_set_text(lv_text_ssid, lv_list_get_btn_text(lv_list_wifi_station, obj));
	lv_obj_add_flag(lv_list_wifi_station, LV_OBJ_FLAG_HIDDEN);
	hide_animation();
	lv_create_layout_password_wifi();



}


/**
 * @fn void lv_create_layout_search_ssid(wifi_ap_record_t*, uint16_t*)
 * @brief Se llama desde appuser_notify_scan_done
 *
 * @param ap_info
 * @param ap_count
 */
void lv_create_layout_search_ssid(wifi_ap_record_t *ap_info, uint16_t *ap_count) {


	int i;
	ESP_LOGI(TAG, ""TRAZAR"VAMOS A PINTAR  REDES", INFOTRAZA);
	// consultamos la lista de estaciones escaneadas

	lv_obj_t *btn;

	if (*ap_count == 0 ) {

		ESP_LOGW(TAG, ""TRAZAR"No hay redes disponibles", INFOTRAZA);
		if (lv_anim_wait != NULL) hide_animation();
		lv_obj_clear_flag(lv_label_connection_wifi, LV_OBJ_FLAG_HIDDEN);
		lv_label_set_text_fmt(lv_label_connection_wifi, "No hay redes disponibles...");
		lv_obj_clear_flag(lv_button_wifi_stations, LV_OBJ_FLAG_HIDDEN);

		return;
	}

	if (lv_list_wifi_station == NULL) {

		ESP_LOGI(TAG, ""TRAZAR"Se van a pintar %d redes", INFOTRAZA, *ap_count);
		lv_list_wifi_station = lv_list_create(lv_screen_init_thermostat);
		lv_set_style_text(lv_list_wifi_station, &style);
		lv_obj_invalidate(lv_list_wifi_station);
		lv_obj_set_pos(lv_list_wifi_station, 20, lv_pct(20));
		lv_obj_set_size(lv_list_wifi_station, CONFIG_LCD_H_RES - 40, CONFIG_LCD_V_RES - 30);
		//lv_obj_center(lv_list_wifi_station);
		lv_list_add_text(lv_list_wifi_station, "Estaciones wifi");
		for (i=0;i<*ap_count;i++) {
			ESP_LOGW(TAG, ""TRAZAR"creando boton %d", INFOTRAZA, i);
			btn = lv_list_add_btn(lv_list_wifi_station, LV_SYMBOL_WIFI, (char*) ap_info[i].ssid);
		    lv_obj_add_event_cb(btn, lv_event_handler_list, LV_EVENT_CLICKED, NULL);
		    lv_set_style_text(btn, &style);
		    ESP_LOGW(TAG, ""TRAZAR"creado boton %d", INFOTRAZA, i);
		}


		if (lv_anim_wait != NULL) hide_animation();

	} else {
		ESP_LOGI(TAG, ""TRAZAR"Solo enseñamos las redes", INFOTRAZA);
		hide_animation();
		lv_obj_clear_flag(lv_list_wifi_station, LV_OBJ_FLAG_HIDDEN);
	}



}



static void lv_handler_keyboard(lv_event_t *e) {


	uint16_t kcode;
	int length;
	wifi_config_t wifi_config;

    bzero(&wifi_config, sizeof(wifi_config_t));



    lv_obj_t * kb = lv_event_get_user_data(e);
    kcode = lv_keyboard_get_selected_btn(kb);


    length = strlen(lv_textarea_get_text(lv_password_text));
    if (length >= 8) {
    	lv_obj_set_style_text_color(lv_password_text, lv_color_hex(0xccaaaa), LV_PART_MAIN);
        if ((kcode == 40) || (kcode == 22)) {
        	strcpy((char*) wifi_config.sta.ssid, lv_label_get_text(lv_text_ssid));
        	strcpy((char*) wifi_config.sta.password, lv_textarea_get_text(lv_password_text));
        	ESP_LOGI(TAG, ""TRAZAR"SSID: %s, PASSWORD: %s", INFOTRAZA,  (char*) wifi_config.sta.ssid, (char*) wifi_config.sta.password);
        	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        	esp_restart();
        	//reiniciar el dispositivo
        }

    } else {
    	lv_obj_set_style_text_color(lv_password_text, lv_color_hex(0xffffff), LV_PART_MAIN);
    }


}


void lv_create_layout_password_wifi() {


	lv_obj_center(lv_text_ssid);
	lv_obj_align_to(lv_text_ssid, lv_screen_init_thermostat,LV_ALIGN_TOP_MID, 0, 50);
	//lv_set_style_to_text(lv_text_ssid, &lv_style_text_password, &lv_font_montserrat_26, 0, 0x0534F0);
	lv_obj_invalidate(lv_text_ssid);



	//caja password
	lv_password_text = lv_textarea_create(lv_screen_init_thermostat);
    lv_textarea_set_text(lv_password_text, "");
    lv_textarea_set_password_mode(lv_password_text, true);
    lv_textarea_set_one_line(lv_password_text, true);
	lv_obj_align_to(lv_password_text, lv_text_ssid, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
	lv_obj_set_width(lv_password_text, lv_pct(25));
	lv_obj_set_style_bg_opa(lv_password_text, LV_OPA_50, LV_PART_MAIN);
	lv_obj_set_style_border_width(lv_password_text, 2, LV_PART_MAIN);
	lv_obj_add_style(lv_password_text, &lv_style_text_password, LV_PART_MAIN);
	//lv_set_style_to_text(lv_text_ssid, &style_text_password, &lv_font_montserrat_26, 0, 0x0534F0);
	//lv_obj_invalidate(lv_text_ssid);


	if (lv_keyboard == NULL) {
	    lv_keyboard = lv_keyboard_create(lv_screen_init_thermostat);
	    lv_obj_set_size(lv_keyboard,  LV_HOR_RES , LV_VER_RES / 2);
	    lv_obj_add_event_cb(lv_keyboard, lv_handler_keyboard, LV_EVENT_CLICKED, lv_keyboard);
	}

    lv_keyboard_set_textarea(lv_keyboard, lv_password_text); /*Focus it on one of the text areas to start*/
    lv_obj_clear_flag(lv_keyboard, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(lv_password_text, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(lv_text_ssid, LV_OBJ_FLAG_HIDDEN);

}

void lv_delete_objects_layout_wifi() {

	lv_obj_del(lv_layout_wifi_stations);
	lv_layout_wifi_stations = NULL;

}


static void lv_set_style_text_connecting() {

	lv_style_init(&lv_style_text_fail);
	lv_style_set_text_font(&lv_style_text_fail, &lv_font_montserrat_16);
    lv_style_set_bg_opa(&lv_style_text_fail, LV_OPA_TRANSP);
	lv_obj_add_style(lv_label_connection_wifi, &lv_style_text_fail, LV_STATE_DEFAULT);



}


void lv_connecting_to_wifi_station() {

	wifi_config_t conf_wifi;

	ESP_LOGI(TAG, ""TRAZAR"CONECTANDO A LA RED WIFI", INFOTRAZA);

	//create objects

	if (lv_box_connecting_wifi == NULL) {

		lv_box_connecting_wifi = lv_obj_create(lv_screen_init_thermostat);
		lv_text_init_thermostat = lv_label_create(lv_box_connecting_wifi);
		lv_label_connection_wifi = lv_label_create(lv_box_connecting_wifi);

	}


	lv_obj_set_size(lv_box_connecting_wifi, 300, 100);


	//lv_obj_invalidate(lv_text_init_thermostat);
	lv_obj_clear_flag(lv_box_connecting_wifi, LV_OBJ_FLAG_HIDDEN);
	lv_label_set_text(lv_text_init_thermostat, "Inicializando...");

	//style objects
	lv_set_style_text_init_thermostat();
	lv_set_style_text_connecting();
	lv_obj_add_style(lv_box_connecting_wifi, &lv_style_text_init_termostat, LV_STATE_DEFAULT);
	//position objects
	lv_obj_set_pos(lv_box_connecting_wifi, lv_pct(25), lv_pct(25));
	lv_obj_set_pos(lv_text_init_thermostat, 0, 0);
	lv_obj_set_pos(lv_label_connection_wifi, 0, 30);

	//sizing objects



	//callback functions





	esp_wifi_get_config(WIFI_IF_STA, &conf_wifi);
	ESP_LOGI(TAG, ""TRAZAR"CONECTANDO A LA RED WIFI", INFOTRAZA);




	lv_label_set_text_fmt(lv_label_connection_wifi, "Conectando a : %s", conf_wifi.sta.ssid );
	lv_obj_clear_flag(lv_label_connection_wifi, LV_OBJ_FLAG_HIDDEN);
	//lv_obj_invalidate(lv_screen_init_thermostat);
	ESP_LOGI(TAG, ""TRAZAR"INVALIDADA LA PANTALLA", INFOTRAZA);


}




void lv_update_button_wifi(bool fail) {

	wifi_config_t conf_wifi;
	esp_wifi_get_config(WIFI_IF_STA, &conf_wifi);

	if (fail == false) {
		lv_style_set_text_color(&lv_style_text_fail, lv_color_hex(0xFFFF00));
		//lv_label_set_text_fmt(lv_label_connection_wifi, "Conectando a : %s", conf_wifi.sta.ssid );
		lv_obj_invalidate(lv_label_connection_wifi);
		ESP_LOGW(TAG, ""TRAZAR"WARNING", INFOTRAZA);
	} else {

		if (lv_obj_has_flag(lv_button_wifi_stations, LV_OBJ_FLAG_HIDDEN)) {
			lv_obj_clear_flag(lv_button_wifi_stations, LV_OBJ_FLAG_HIDDEN);

			lv_label_set_text_fmt(lv_label_connection_wifi, "Error al conectar a : %s", conf_wifi.sta.ssid);
			lv_style_set_text_color(&lv_style_text_fail, lv_color_hex(0xFF0000));
			ESP_LOGW(TAG, ""TRAZAR"FALLO", INFOTRAZA);

		}
	}
}

void lv_delete_init_thermostat() {

	if (lv_screen_init_thermostat != NULL)
	lv_obj_del(lv_screen_init_thermostat);
}





