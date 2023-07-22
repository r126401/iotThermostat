/*
 * lv_tools.c
 *
 *  Created on: Jul 10, 2023
 *      Author: t126401
 */


#include "lv_tools.h"


lv_style_t style_anim;

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





lv_obj_t* lv_anim_wait_event(char* message, lv_obj_t *parent, lv_point_t pos) {


	lv_obj_t *lv_layout_anim = lv_obj_create(parent);
	if ((pos.x == 0) && (pos.y == 0)) {
		lv_obj_center(lv_layout_anim);
	}

	//lv_obj_set_pos(lv_layout_anim, pos.x, pos.y);
	lv_obj_set_size(lv_layout_anim, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_set_style_anim_wait_event(lv_layout_anim);

	lv_obj_t *spinner = lv_spinner_create(lv_layout_anim, 5000, 60);
	lv_obj_set_size(spinner, 48,48);
	lv_obj_center(spinner);


	lv_obj_t *lv_label_message = lv_label_create(lv_layout_anim);
	lv_obj_align_to(lv_label_message, spinner, LV_ALIGN_OUT_BOTTOM_MID, 0, 50);
	lv_label_set_text(lv_label_message, message);
	lv_set_style_anim_wait_event(lv_label_message);

	return lv_layout_anim;

}



lv_coord_t calculate_center_obj_x(lv_coord_t width, bool offset_left) {


	lv_coord_t desp;

	if (offset_left) {
		desp = -(width/2);
	} else {
		desp = width/2;
	}


	return desp;





}
