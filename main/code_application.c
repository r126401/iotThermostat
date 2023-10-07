/*
 * code_application.c
 *
 *  Created on: Jul 13, 2023
 *      Author: t126401
 */

#include "code_application.h"
#include "logging.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "interfaz_usuario.h"
#include "conexiones.h"
#include "conexiones_mqtt.h"
#include <math.h>
#include "cJSON.h"
#include "funciones_usuario.h"
#include "ds18x20.h"
#include "alarmas.h"
#include "dialogos_json.h"
#include "esp_interface.h"
#include "esp_netif.h"



static const char *TAG = "code_application";

static esp_timer_handle_t temporizador_lectura_remota;


#define TIMEOUT_LECTURA_REMOTA 5000 //ms
#ifdef CONFIG_GPIO_PIN_DS18B20
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<CONFIG_GPIO_PIN_DS18B20)
#endif










enum ESTADO_RELE operacion_rele(DATOS_APLICACION *datosApp, enum TIPO_ACTUACION_RELE tipo, enum ESTADO_RELE operacion) {

	enum ESTADO_RELE rele;


	//gpio_reset_pin(CONFIG_GPIO_PIN_RELE);
	//gpio_rele_in();
	switch (tipo) {
	case MANUAL:
		if (gpio_get_level(CONFIG_GPIO_PIN_RELE) == OFF) {
			ESP_LOGW(TAG, ""TRAZAR" ESTABA A OFF Y SE ENCIENDE", INFOTRAZA);
			rele = ON;
		} else {
			ESP_LOGW(TAG, ""TRAZAR" ESTABA A On Y SE APAGA", INFOTRAZA);
			rele = OFF;
		}
		break;
	default:
		rele = operacion;
		break;
	}
	//gpio_rele_out();
	gpio_set_level(CONFIG_GPIO_PIN_RELE, rele);
	ESP_LOGW(TAG, ""TRAZAR"EL RELE SE HA PUESTO A %d", INFOTRAZA, rele);
	//lv_actualizar_rele_lcd(rele);
	//gpio_set_direction(CONFIG_GPIO_PIN_RELE, GPIO_MODE_INPUT);

	return rele;
}



void accionar_termostato(DATOS_APLICACION *datosApp) {

	enum ESTADO_RELE accion_rele;
	enum TIPO_ACCION_TERMOSTATO accion_termostato;
	cJSON* informe = NULL;
	static float lecturaAnterior = -1000;

    if (((accion_termostato = calcular_accion_termostato(datosApp, &accion_rele)) == ACCIONAR_TERMOSTATO)) {
    	ESP_LOGI(TAG, ""TRAZAR"VAMOS A ACCIONAR EL RELE", INFOTRAZA);
    	operacion_rele(datosApp, TEMPORIZADA, accion_rele);
    }

    if ((accion_termostato == ACCIONAR_TERMOSTATO) || (lecturaAnterior != datosApp->termostato.tempActual)) {
    	//lv_actualizar_temperatura_lcd(datosApp);
    	ESP_LOGI(TAG, ""TRAZAR"HA HABIDO CAMBIO DE TEMPERATURA", INFOTRAZA);
        informe = appuser_send_spontaneous_report(datosApp, CAMBIO_TEMPERATURA, NULL);
        if (informe != NULL) {
        	ESP_LOGI(TAG, "mensaje: %s", cJSON_Print(informe));
        	publicar_mensaje_json(datosApp, informe, NULL);
        } else {
        	ESP_LOGI(TAG, "El informe iba vacio");
        }

    }
    lecturaAnterior = datosApp->termostato.tempActual;
}



void tarea_lectura_temperatura(void *parametros) {

    DATOS_APLICACION *datosApp = (DATOS_APLICACION*) parametros;


    ESP_LOGI(TAG, ""TRAZAR"COMIENZA LA TAREA DE LECTURA DE TEMPERATURA", INFOTRAZA);

    while(1) {
    	vTaskDelay(datosApp->termostato.intervaloLectura * 1000 / portTICK_RATE_MS);

    	ESP_LOGE(TAG, ""TRAZAR" tempUmbral %.02f", INFOTRAZA, datosApp->termostato.tempUmbral);
    	leer_temperatura(datosApp);
    	ESP_LOGE(TAG, ""TRAZAR" tempUmbral %.02f", INFOTRAZA, datosApp->termostato.tempUmbral);
    	accionar_termostato(datosApp);




    }

}







float redondear_temperatura(float temperatura) {

	float redondeado;
	float diferencia;
	float resultado = 0;
	float valor_absoluto;

	redondeado = lround(temperatura);
	diferencia = temperatura - redondeado;
	valor_absoluto = fabs(redondeado);
	if (diferencia < 0.25) resultado = valor_absoluto;
	if ((diferencia > 0.25 ) && (diferencia < 0.5)) resultado = valor_absoluto + 0.5;

	if ((diferencia < -0.25)) resultado = valor_absoluto - 0.5;

	return resultado;

}


enum TIPO_ACCION_TERMOSTATO calcular_accion_termostato(DATOS_APLICACION *datosApp, enum ESTADO_RELE *accion) {

    // El termostato esta apagado y la temperatura actual es menor o igual que la umbral.


	//gpio_rele_in();
    if (gpio_get_level(CONFIG_GPIO_PIN_RELE) == OFF) {
               if (datosApp->termostato.tempActual <= (datosApp->termostato.tempUmbral - datosApp->termostato.margenTemperatura)) {
            	   ESP_LOGI(TAG, ""TRAZAR"RELE OFF Y SE ENCIENDE. tempMedida: %.2f, tempUmbral: %.02f", INFOTRAZA, datosApp->termostato.tempActual, datosApp->termostato.tempUmbral);
                   *accion = ON;
                   return ACCIONAR_TERMOSTATO;
               } else {
            	   ESP_LOGI(TAG, ""TRAZAR"RELE OFF Y SIGUE OFF. tempMedida: %.2f, tempUmbral: %.02f", INFOTRAZA, datosApp->termostato.tempActual, datosApp->termostato.tempUmbral);
                   *accion = OFF;
                   return NO_ACCIONAR_TERMOSTATO;

               }
           } else {
               if (datosApp->termostato.tempActual >= (datosApp->termostato.tempUmbral + datosApp->termostato.margenTemperatura) ) {
            	   ESP_LOGI(TAG, ""TRAZAR"RELE ON Y SE ENCIENDE. tempMedida: %.2f, tempUmbral: %.02f", INFOTRAZA, datosApp->termostato.tempActual, datosApp->termostato.tempUmbral);
                   *accion = OFF;
                   return ACCIONAR_TERMOSTATO;
               } else {
            	   ESP_LOGI(TAG, ""TRAZAR"RELE ON Y SIGUE ON. tempMedida: %.2f, tempUmbral: %.02f", INFOTRAZA, datosApp->termostato.tempActual, datosApp->termostato.tempUmbral);
                   *accion = ON;
                   return NO_ACCIONAR_TERMOSTATO;

               }
           }

}

esp_err_t notify_end_starting(DATOS_APLICACION *datosApp) {

	cJSON *informe;


	if (datosApp->datosGenerales->estadoApp == ESPERA_FIN_ARRANQUE) {

		//lv_cargar_pantalla_principal();
		//calcular_estado_aplicacion(datosApp);
		//appuser_cambiar_modo_aplicacion(datosApp, estado_app);
		informe = appuser_send_spontaneous_report(datosApp, ARRANQUE_APLICACION, NULL);

		if (informe != NULL) {
			publicar_mensaje_json(datosApp, informe, NULL);
		}

	}



	return ESP_OK;

}



esp_err_t leer_temperatura_local(DATOS_APLICACION *datosApp) {

    esp_err_t error = ESP_FAIL;
    static uint8_t contador = 0;
    char temp[15]={0};
	float temperatura_a_redondear;


    ESP_LOGI(TAG, ""TRAZAR" Leyendo desde el sensor dht", INFOTRAZA);
    while (error != ESP_OK) {
#ifdef CONFIG_DHT22

    	error = dht_read_float_data(
    			DHT_TYPE_AM2301,
    			CONFIG_GPIO_PIN_DHT,
    			&datosApp->termostato.humedad,
    			&datosApp->termostato.tempActual);

    	float dato;
    	lectura_temperatura_ds18x20(&dato);
    	ESP_LOGI(TAG, ""TRAZAR" lectura ds18x20: %lf", INFOTRAZA, dato);

#endif
#ifdef CONFIG_DS18B20

    	error = lectura_temperatura_ds18x20(&datosApp->termostato.tempActual);


    	datosApp->termostato.humedad = 5;

#endif



    	if ((error == ESP_OK) && ((datosApp->termostato.humedad != 0) && (datosApp->termostato.tempActual != 0))) {
    		ESP_LOGI(TAG, ""TRAZAR" Lectura local correcta!. ", INFOTRAZA);
    		datosApp->termostato.tempActual = datosApp->termostato.tempActual + datosApp->termostato.calibrado;
    		temperatura_a_redondear = datosApp->termostato.tempActual;
          	datosApp->termostato.tempActual = redondear_temperatura(temperatura_a_redondear);
            	ESP_LOGI(TAG, ""TRAZAR" Temp sin redondeo %.01lf, Temp redondeada %.01lf ", INFOTRAZA, temperatura_a_redondear,datosApp->termostato.tempActual );
      		contador = 0;
    	} else {
    		contador++;
    		if (contador == 4)  {
    			registrar_alarma(datosApp, NOTIFICACION_ALARMA_SENSOR_DHT, ALARMA_SENSOR_DHT, ALARMA_WARNING, true);

    		}
    		if (contador == 10) {
    			registrar_alarma(datosApp, NOTIFICACION_ALARMA_SENSOR_DHT, ALARMA_SENSOR_DHT, ALARMA_ON, true);

    		}
    		ESP_LOGE(TAG, ""TRAZAR" ERROR AL TOMAR LA LECTURA. REINTENTAMOS EN %d SEGUNDOS", INFOTRAZA, datosApp->termostato.intervaloReintentos);
        	vTaskDelay(datosApp->termostato.intervaloReintentos * 1000 / portTICK_RATE_MS);
    	}

    }

    if (datosApp->alarmas[ALARMA_SENSOR_DHT].estado_alarma > ALARMA_OFF) {
    	registrar_alarma(datosApp, NOTIFICACION_ALARMA_SENSOR_DHT, ALARMA_SENSOR_DHT, ALARMA_OFF, true);
    }
    sprintf(temp,"%.02lf ºC", datosApp->termostato.tempActual);
    notify_end_starting(datosApp);

    return ESP_OK;

}

esp_err_t leer_temperatura(DATOS_APLICACION *datosApp) {

	if ((datosApp->termostato.master == false)) {
		if (datosApp->alarmas[ALARMA_SENSOR_REMOTO].estado_alarma == ALARMA_ON) {
			ESP_LOGW(TAG, ""TRAZAR" termostato en remoto. ADEMAS LA ALARMA ESTA A ON", INFOTRAZA);
			leer_temperatura_local(datosApp);
		}
		leer_temperatura_remota(datosApp);

	} else {
		leer_temperatura_local(datosApp);
	}




	return ESP_OK;
}

static void temporizacion_lectura_remota(void *arg) {

	static int contador = 0;
	DATOS_APLICACION *datosApp;
	datosApp = (DATOS_APLICACION*) arg;

	contador++;
	switch(datosApp->alarmas[ALARMA_SENSOR_REMOTO].estado_alarma) {

	case ALARMA_INDETERMINADA:
		registrar_alarma(datosApp, NOTIFICACION_ALARMA_SENSOR_REMOTO, ALARMA_SENSOR_REMOTO, ALARMA_WARNING, true);
		break;
	case ALARMA_OFF:
		contador = 0;
		break;
	case ALARMA_WARNING:
		ESP_LOGE(TAG, ""TRAZAR" ALARMA WARNING EN SENSOR REMOTO. CONTADOR %d ", INFOTRAZA, contador);
		if (contador == 5) {
			if (datosApp->alarmas[ALARMA_SENSOR_REMOTO].estado_alarma != ALARMA_ON) {
				registrar_alarma(datosApp, NOTIFICACION_ALARMA_SENSOR_REMOTO, ALARMA_SENSOR_REMOTO, ALARMA_ON, true);
			}
		}
		break;
	case ALARMA_ON:
		ESP_LOGE(TAG, ""TRAZAR" ALARMA ON EN SENSOR REMOTO. LEEMOS EN LOCAL", INFOTRAZA);
		break;
	}




}

esp_err_t leer_temperatura_remota(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR" Leyendo desde el sensor remoto %s", INFOTRAZA, datosApp->termostato.sensor_remoto);
	cJSON *objeto = NULL;
	cJSON *comando;
	comando = cJSON_CreateObject();
	char topic[50];
	ESP_LOGI(TAG, ""TRAZAR"debug 1", INFOTRAZA);
	strcpy(topic, "/sub_");
	strcat(topic, datosApp->termostato.sensor_remoto);
	ESP_LOGI(TAG, ""TRAZAR"debug 2", INFOTRAZA);
	objeto = cabeceraGeneral(datosApp);
	if (objeto != NULL) {


		cJSON_AddNumberToObject(objeto, DLG_COMANDO, STATUS_DISPOSITIVO);
		cJSON_AddItemToObject(comando, COMANDO, objeto);
		publicar_mensaje_json(datosApp, comando, topic);
	}


    if (datosApp->alarmas[ALARMA_SENSOR_REMOTO].estado_alarma == ALARMA_OFF) {
    	registrar_alarma(datosApp, NOTIFICACION_ALARMA_SENSOR_REMOTO, ALARMA_SENSOR_REMOTO, ALARMA_INDETERMINADA, false);
    }

    const esp_timer_create_args_t timer_remote_read_args = {
    		.callback = &temporizacion_lectura_remota,
			.name = "timer remote read",
			.arg = (void*) datosApp
    };

    ESP_ERROR_CHECK(esp_timer_create(&timer_remote_read_args, &temporizador_lectura_remota));
    ESP_ERROR_CHECK(esp_timer_start_once(temporizador_lectura_remota, TIMEOUT_LECTURA_REMOTA * 1000));


    ESP_LOGI(TAG, ""TRAZAR"debug 3", INFOTRAZA);


	return ESP_OK;
}


