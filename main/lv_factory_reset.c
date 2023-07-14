/*
 * lv_factory_reset.c
 *
 *  Created on: 8 jun 2023
 *      Author: t126401
 */
/*
#include <include/datosapp.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "lvgl/lvgl.h"
#include "lvgl/examples/lv_examples.h"
#include "lvgl/demos/lv_demos.h"
#include <include/lv_factory_reset.h>

*/


#include "lv_factory_reset.h"
#include <stdlib.h>
#include "lv_tools.h"
lv_obj_t *lv_screen_factory;
lv_obj_t *lv_text_area_factory;
lv_obj_t *lv_img_app;
lv_obj_t *lv_imgbtn_reset;
lv_obj_t *lv_text_reset;



lv_style_t	lv_style_factory_screen;
lv_style_t	lv_style_text_area_factory;
lv_style_t	lv_style_button_reset;

LV_IMG_DECLARE(ic_app);
LV_IMG_DECLARE(ic_action_reset);



void lv_screen_factory_reset() {

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
	  lv_create_screen_factory(datosApp);


}



void lv_set_style_factory_screen() {

	lv_style_init(&lv_style_factory_screen);
	lv_theme_default_init(lv_obj_get_disp(lv_scr_act()),  /*Use the DPI, size, etc from this display*/
					lv_color_hex(0x0534F0), lv_color_hex(0x0534F0),   /*Primary and secondary palette*/
			                                        true,    /*Light or dark mode*/
			                                        &lv_font_montserrat_16); /*Small, normal, large fonts*/
	lv_obj_add_style(lv_screen_factory, &lv_style_factory_screen, LV_PART_MAIN);


}


void lv_set_style_text_area_factory() {

	lv_style_init(&lv_style_text_area_factory);
	lv_obj_add_style(lv_text_area_factory, &lv_style_text_area_factory, LV_STATE_DEFAULT);

    lv_style_set_bg_opa(&lv_style_text_area_factory, LV_OPA_COVER);
    lv_style_set_bg_color(&lv_style_text_area_factory, lv_color_hex(0));
    lv_style_set_bg_grad_color(&lv_style_text_area_factory, lv_color_hex(0x0534F0));
    lv_style_set_bg_grad_dir(&lv_style_text_area_factory, LV_GRAD_DIR_VER);
    lv_style_set_line_width(&lv_style_text_area_factory, 0);



}
void lv_set_style_button_reset() {



}


static void event_handler_reset(lv_event_t *event) {

	if(lv_text_reset == NULL) {
		lv_text_reset = lv_label_create(lv_screen_factory);
		lv_obj_set_pos(lv_text_reset, 180, 20);
		lv_label_set_text(lv_text_reset, "Reiniciando...");
		esp_restart();

	}
}


static void event_handler_focused(lv_event_t *e) {

	lv_obj_clear_state(lv_text_area_factory, LV_STATE_FOCUSED);


}


void lv_create_screen_factory(DATOS_APLICACION *datosApp) {



	lv_screen_factory = lv_obj_create(NULL);
	lv_scr_load(lv_screen_factory);
	lv_set_style_factory_screen();
	lv_create_text_area(datosApp);
	lv_obj_set_size(lv_text_area_factory, lv_pct(60), LV_SIZE_CONTENT);
	lv_textarea_set_align(lv_text_area_factory, LV_TEXT_ALIGN_CENTER);
	lv_textarea_add_text(lv_text_area_factory, "PUESTA EN SERVICIO...\n\n");
	lv_textarea_add_text(lv_text_area_factory, "Por favor, abre la aplicacion MyHomeIot e instala un nuevo dispositivo.\n\n");
	lv_textarea_add_text(lv_text_area_factory, "Una vez instalado el dispositivo se conectara a la red y comenzara a funcionar.");


	lv_img_app = lv_img_create(lv_screen_factory);
	lv_obj_set_pos(lv_img_app, lv_pct(5), lv_pct(5));
	lv_img_set_src(lv_img_app, &ic_app);



	lv_imgbtn_reset = lv_imgbtn_create(lv_screen_factory);
	lv_imgbtn_set_src(lv_imgbtn_reset, LV_IMGBTN_STATE_RELEASED, &ic_action_reset, NULL, NULL);
	lv_obj_add_event_cb(lv_imgbtn_reset, event_handler_reset, LV_EVENT_CLICKED, NULL);
	lv_obj_add_event_cb(lv_text_area_factory, event_handler_focused, LV_EVENT_FOCUSED, NULL);

	lv_obj_set_pos(lv_imgbtn_reset, 650 , lv_pct(5));









}
void lv_create_text_area(DATOS_APLICACION *datosApp){

	//creamos el text_area

	lv_text_area_factory = lv_textarea_create(lv_screen_factory);
	lv_obj_center(lv_text_area_factory);
	lv_set_style_text_area_factory();
	lv_obj_clear_state(lv_text_area_factory, LV_STATE_FOCUSED);



}
