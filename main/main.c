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

#if CONFIG_RGB_PANEL
#include "lv_rgb_main.h"
#include "lv_factory_reset.h"
#include "lv_thermostat.h"

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

	error = inicializar_nvs(CONFIG_NAMESPACE, &datosApp.handle);
	if (error != ESP_OK) {
		ESP_LOGE(TAG, ""TRAZAR" ERROR AL INICIALIZAR NVS", INFOTRAZA);
		error = ESP_FAIL;
	} else {
		error = ESP_OK;
	}



	if(configurado_de_fabrica() == ESP_OK) {

		datosApp.datosGenerales->estadoApp = ARRANQUE_FABRICA;

	}

#ifdef CONFIG_RGB_PANEL
	xTaskCreate(lv_app_rgb_main, "tarea LCD", 4096, (void*) &datosApp, 4, NULL);
#endif

	if (init_code_application(&datosApp) != ESP_OK) {
		ESP_LOGE(TAG, ""TRAZAR" FALLO LA INICIALIZACION DE LA APLICACION", INFOTRAZA);
	}

	ESP_LOGI(TAG, ""TRAZAR" vamos a conectar al wifi", INFOTRAZA);

	conectar_dispositivo_wifi();

	error = inicializacion(&datosApp, CONFIG_CARGA_CONFIGURACION);
	if (error == ESP_OK) {
		ESP_LOGI(TAG, ""TRAZAR"INICIALIZACION CORRECTA", INFOTRAZA);
	} else {
		ESP_LOGE(TAG, ""TRAZAR"NO SE HA PODIDO INICIALIZAR EL DISPOSITIVO", INFOTRAZA);
	}

	handle = NULL;
	crear_tarea_mqtt(&datosApp);


    iniciar_gestion_programacion(&datosApp);
    //xTaskCreate(tarea_lectura_temperatura, "tarea_lectura_temperatura", 4096, (void*) &datosApp, 4, NULL);

    pintar_fecha();


}
