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



#if CONFIG_RGB_PANEL
#include "lv_rgb_main.h"
#include "lv_factory_reset.h"
#include "lv_thermostat.h"
#include "lv_init_thermostat.h"
#endif

static const char *TAG = "IotThermostat";
DATOS_APLICACION datosApp;
TaskHandle_t handle;




void app_main(void) {



	const esp_app_desc_t *aplicacion;
	esp_err_t error = ESP_OK;
	ESP_LOGI(TAG, ""TRAZAR"COMIENZO DE LA APLICACION version", INFOTRAZA);
	DATOS_GENERALES *datosGenerales;
	datosGenerales = (DATOS_GENERALES*) calloc(1, sizeof(DATOS_GENERALES));
	datosApp.datosGenerales = datosGenerales;





	aplicacion = calloc(1, sizeof(esp_app_desc_t));
	aplicacion = esp_app_get_description();
	ESP_LOGW(TAG, ""TRAZAR" app:%s, version: %s, hora: %s, dia:%s, idfver:%s", INFOTRAZA,
			aplicacion->project_name, aplicacion->version, aplicacion->time, aplicacion->date, aplicacion->idf_ver);

	init_code_application(&datosApp);
	create_event_task(&datosApp);
	error = inicializar_nvs(CONFIG_NAMESPACE, &datosApp.handle);
	if (error != ESP_OK) {
		ESP_LOGW(TAG, ""TRAZAR" ERROR AL INICIALIZAR NVS", INFOTRAZA);
		error = ESP_FAIL;
	} else {
		error = ESP_OK;
	}







	//init_nvs e init_app
	error = init_application(&datosApp, CONFIG_CARGA_CONFIGURACION);
	if (error == ESP_OK) {
		ESP_LOGI(TAG, ""TRAZAR"INICIALIZACION CORRECTA", INFOTRAZA);
	} else {


	}

/*

	if (lv_init_lcd_application(&datosApp) != ESP_OK) {
		send_event(__func__,EVENT_ERROR_LCD);
		return;
	} else {
		send_event(__func__,EVENT_LCD_OK);
	}
*/

#ifdef CONFIG_RGB_PANEL
	xTaskCreate(lv_init_lcd_application, "tarea LCD", 8192,  &datosApp, 4, NULL);
#endif

	//lv_screen_thermostat(&datosApp);
	//lv_timer_handler();

	xTaskCreate(task_iotThermostat, "tarea_lectura_temperatura", 8192, (void*) &datosApp, 1, NULL);

	if(is_factory() == ESP_OK) {

		send_event(__func__,EVENT_FACTORY);
	} else {

		conectar_dispositivo_wifi();
		ESP_LOGI(TAG, ""TRAZAR" ESTADO ANTES DE INICIAR GESTION: %d", INFOTRAZA, datosApp.datosGenerales->estadoApp);

	}










}
