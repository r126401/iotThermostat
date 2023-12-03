/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */


#include "esp_log.h"
#include "esp_err.h"
#include "esp_app_desc.h"
#include "logging.h"
#include "common_data.h"
#include "nvslib.h"
#include "configuracion.h"
#include "interfaz_usuario.h"
#include "conexiones.h"
#include "conexiones_mqtt.h"
#include "code_application.h"
#include "events_device.h"




#include "lv_rgb_main.h"
#include "lv_factory_reset.h"
#include "lv_thermostat.h"
#include "lv_init_thermostat.h"


static const char *TAG = "IotThermostat";
DATOS_APLICACION datosApp;
TaskHandle_t handle;




void app_main(void) {

	esp_err_t error = ESP_OK;

	ESP_LOGI(TAG, ""TRAZAR"COMIENZO DE LA APLICACION version", INFOTRAZA);

	error = init_code_application(&datosApp);
	create_event_task(&datosApp);

	error = init_application(&datosApp, CONFIG_CARGA_CONFIGURACION);
	if (error == ESP_OK) {
		ESP_LOGI(TAG, ""TRAZAR"INICIALIZACION CORRECTA", INFOTRAZA);
	} else {

	}

	



#ifdef CONFIG_USE_LCD

#ifdef CONFIG_RGB_LCD

	xTaskCreate(lv_init_lcd_application, "tarea LCD", 8192,  &datosApp, 4, NULL);
#endif
#endif


	xTaskCreate(task_iotThermostat, "tarea_lectura_temperatura", 8192, (void*) &datosApp, 1, NULL);

	if(is_factory() == ESP_OK) {

		send_event(__func__,EVENT_FACTORY);
	} else {

		conectar_dispositivo_wifi();
		ESP_LOGI(TAG, ""TRAZAR" ESTADO ANTES DE INICIAR GESTION: %d", INFOTRAZA, datosApp.datosGenerales->estadoApp);

	}










}
