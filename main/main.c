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

	error = inicializar_nvs(CONFIG_NAMESPACE, &datosApp.handle);
	if (error != ESP_OK) {
		ESP_LOGW(TAG, ""TRAZAR" ERROR AL INICIALIZAR NVS", INFOTRAZA);
		error = ESP_FAIL;
	} else {
		error = ESP_OK;
	}

/*
#ifdef CONFIG_RGB_PANEL
	xTaskCreate(lv_app_rgb_main, "tarea LCD", 4096, (void*) &datosApp, 4, NULL);
#endif
	*/

	lv_app_rgb_main(&datosApp);

	if(configurado_de_fabrica() == ESP_OK) {

		datosApp.datosGenerales->estadoApp = ARRANQUE_FABRICA;
		ESP_LOGW(TAG, ""TRAZAR" ESTAMOS ARRANCANDO DE FABRICA", INFOTRAZA);
		lv_screen_factory_reset(datosApp);

	} else {
		datosApp.datosGenerales->estadoApp = NORMAL_ARRANCANDO;
		ESP_LOGE(TAG, ""TRAZAR" NORMAL ARRANCANDO...", INFOTRAZA);
		lv_init_thermostat();

	}

	lv_timer_handler();

	error = inicializacion(&datosApp, CONFIG_CARGA_CONFIGURACION);
	if (error == ESP_OK) {
		ESP_LOGI(TAG, ""TRAZAR"INICIALIZACION CORRECTA", INFOTRAZA);
	} else {
		if (datosApp.datosGenerales->estadoApp == ARRANQUE_FABRICA) {
			ESP_LOGI(TAG, ""TRAZAR"NO SE HA PODIDO INICIALIZAR EL DISPOSITIVO", INFOTRAZA);
		} else {
			ESP_LOGE(TAG, ""TRAZAR"NO SE HA PODIDO INICIALIZAR EL DISPOSITIVO", INFOTRAZA);
			return;
		}

	}


	lv_screen_thermostat(&datosApp);
	lv_timer_handler();

	ESP_LOGI(TAG, ""TRAZAR" vamos a conectar al wifi", INFOTRAZA);
	conectar_dispositivo_wifi();
	sync_app_by_ntp(&datosApp);
	crear_tarea_mqtt(&datosApp);


	ESP_LOGI(TAG, ""TRAZAR" ESTADO ANTES DE INICIAR GESTION: %d", INFOTRAZA, datosApp.datosGenerales->estadoApp);
	iniciar_gestion_programacion(&datosApp);

	xTaskCreate(tarea_lectura_temperatura, "tarea_lectura_temperatura", 8192, (void*) &datosApp, 1, NULL);

    //pintar_fecha();


    while (1) {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
    }

}
