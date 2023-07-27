/*
 * lv_tools.c
 *
 *  Created on: Jul 10, 2023
 *      Author: t126401
 */


#include "lv_tools.h"





lv_coord_t calculate_center_obj_x(lv_coord_t width, bool offset_left) {


	lv_coord_t desp;

	if (offset_left) {
		desp = -(width/2);
	} else {
		desp = width/2;
	}


	return desp;





}
