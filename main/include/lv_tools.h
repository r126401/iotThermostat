/*
 * lv_tools.h
 *
 *  Created on: Jul 10, 2023
 *      Author: t126401
 */

#ifndef LV_SRC_INCLUDE_LV_TOOLS_H_
#define LV_SRC_INCLUDE_LV_TOOLS_H_

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "lvgl.h"

lv_coord_t calculate_center_obj_x(lv_coord_t width, bool offset_left);
lv_obj_t* lv_anim_wait_event(char* message, lv_obj_t *parent, lv_point_t pos);
void lv_set_style_to_text(lv_obj_t *obj, lv_style_t *style, lv_font_t *font, lv_coord_t border_width, uint32_t color_text);



#endif /* LV_SRC_INCLUDE_LV_TOOLS_H_ */
