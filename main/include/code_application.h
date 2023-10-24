/*
 * code_application.h
 *
 *  Created on: Jul 13, 2023
 *      Author: t126401
 */

#ifndef MAIN_INCLUDE_CODE_APPLICATION_H_
#define MAIN_INCLUDE_CODE_APPLICATION_H_

#include "common_data.h"
#include "esp_log.h"
#include "esp_err.h"

/**
 * @fn esp_err_t appuser_init_application(DATOS_APLICACION*)
 * @brief Esta funcion es llamada desde la aplicacion de usuario y determina las acciones de inicializacion del despositivo especifico.
 *
 * @pre
 * @post
 * @param datosApp es la estructura de aplicacion
 * @return ESP_OK cuando la inicializacion del dispositivo especifico es correcta
 */
esp_err_t init_code_application(DATOS_APLICACION *datosApp);
void task_iotThermostat(void *parametros);
void thermostat_action(DATOS_APLICACION *datosApp);
float redondear_temperatura(float temperatura);



#endif /* MAIN_INCLUDE_CODE_APPLICATION_H_ */
