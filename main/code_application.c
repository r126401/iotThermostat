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
#include "events_device.h"
#include "dialogos_json.h"
#include "esp_interface.h"
#include "esp_netif.h"
#include "lv_thermostat.h"
#include "alarmas.h"



static const char *TAG = "code_application";




#define TIMEOUT_LECTURA_REMOTA 5000 //ms
#ifdef CONFIG_GPIO_PIN_DS18B20
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<CONFIG_GPIO_PIN_DS18B20)
#endif










enum ESTADO_RELE relay_operation(DATOS_APLICACION *datosApp, enum TIPO_ACTUACION_RELE tipo, enum ESTADO_RELE operacion) {

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
	lv_update_relay(rele);

	return rele;
}



void thermostat_action(DATOS_APLICACION *datosApp) {

	enum ESTADO_RELE accion_rele;
	enum TIPO_ACCION_TERMOSTATO accion_termostato;
	cJSON* informe = NULL;
	static float lecturaAnterior = -1000;

	ESP_LOGI(TAG, ""TRAZAR"accionar_termostato: LECTURA ANTERIOR: %.2f, LECTURA POSTERIOR: %.2f HA HABIDO CAMBIO DE TEMPERATURA", INFOTRAZA,
			lecturaAnterior, datosApp->termostato.tempActual);

    if (((accion_termostato = calcular_accion_termostato(datosApp, &accion_rele)) == ACCIONAR_TERMOSTATO)) {
    	ESP_LOGI(TAG, ""TRAZAR"VAMOS A ACCIONAR EL RELE", INFOTRAZA);
    	relay_operation(datosApp, TEMPORIZADA, accion_rele);
    }

    if ((accion_termostato == ACCIONAR_TERMOSTATO) || (lecturaAnterior != datosApp->termostato.tempActual)) {
    	//lv_actualizar_temperatura_lcd(datosApp);
    	ESP_LOGI(TAG, ""TRAZAR"HA HABIDO CAMBIO DE TEMPERATURA", INFOTRAZA);
        informe = appuser_send_spontaneous_report(datosApp, CAMBIO_TEMPERATURA, NULL);
        if (informe != NULL) {
        	publicar_mensaje_json(datosApp, informe, NULL);
        } else {
        	ESP_LOGI(TAG, "El informe iba vacio");
        }

    }
    lecturaAnterior = datosApp->termostato.tempActual;
}


void update_thermostat_device(DATOS_APLICACION *datosApp) {

	lv_update_temperature(datosApp);
	ESP_LOGE(TAG, ""TRAZAR" tempUmbral %.02f", INFOTRAZA, datosApp->termostato.tempUmbral);
	thermostat_action(datosApp);


}


void task_iotThermostat(void *parametros) {

    DATOS_APLICACION *datosApp = (DATOS_APLICACION*) parametros;
    EVENT_DEVICE event = EVENT_NONE;


    ESP_LOGI(TAG, ""TRAZAR"COMIENZA LA TAREA DE LECTURA DE TEMPERATURA", INFOTRAZA);

    lv_update_relay(gpio_get_level(CONFIG_GPIO_PIN_RELE));

    while(1) {
		if (get_status_alarm(datosApp, ALARM_APP) == ALARM_ON) {
			vTaskDelay(datosApp->termostato.read_interval * 1000 / portTICK_RATE_MS);
		} else {
			vTaskDelay(datosApp->termostato.retry_interval * 1000 / portTICK_RATE_MS);
			ESP_LOGI(TAG, ""TRAZAR"ERROR EN LA LECTURA LOCAL, PASAMOS A LA LOGICA DE REINTENTOS", INFOTRAZA);
		}
/*


    	if (datosApp->termostato.master) {
    		if (get_status_alarm(datosApp, ALARM_APP) == ALARM_ON) {
    			vTaskDelay(datosApp->termostato.intervaloLectura * 1000 / portTICK_RATE_MS);
    		} else {
    			vTaskDelay(datosApp->termostato.intervaloReintentos * 1000 / portTICK_RATE_MS);
    			ESP_LOGI(TAG, ""TRAZAR"ERROR EN LA LECTURA LOCAL, PASAMOS A LA LOGICA DE REINTENTOS", INFOTRAZA);
    		}
    	} else {
    		if ((get_status_alarm(datosApp, ALARM_APP) == ALARM_ON) ||  (get_status_alarm(datosApp, ALARM_REMOTE_DEVICE) == ALARM_ON)) {
    			vTaskDelay(datosApp->termostato.intervaloLectura * 1000 / portTICK_RATE_MS);
    		} else {
    			vTaskDelay(datosApp->termostato.intervaloReintentos * 1000 / portTICK_RATE_MS);
    			ESP_LOGI(TAG, ""TRAZAR"ERROR EN LA LECTURA REMOTA, PASAMOS A LA LOGICA DE REINTENTOS", INFOTRAZA);
    		}
    	}
*/
    	//event = reading_temperature(datosApp);

		if ((datosApp->termostato.master == false)) {

			event = reading_remote_temperature(datosApp);
			if (get_status_alarm(datosApp, ALARM_REMOTE_DEVICE) == ALARM_ON) {
				ESP_LOGW(TAG, ""TRAZAR" termostato en remoto. ADEMAS LA ALARMA ESTA A ON", INFOTRAZA);
				event = reading_local_temperature(datosApp);
				if (event != EVENT_ANSWER_TEMPERATURE) {
					ESP_LOGE(TAG, ""TRAZAR"ERROR EN LA LECTURA EN LOCAL CUANDO ESTA SELECCIONADA LA LECTURA REMOTA", INFOTRAZA);
					event = EVENT_ERROR_READ_LOCAL_TEMPERATURE;
				}
			}


		} else {
			ESP_LOGW(TAG, ""TRAZAR" Leemos temperatura en local", INFOTRAZA);
			event = reading_local_temperature(datosApp);

		}



    	switch(event) {

    	case EVENT_ANSWER_TEMPERATURE:
    		send_event_device(EVENT_ANSWER_TEMPERATURE);
    		//update_thermostat_device(datosApp);
    		break;
    	case EVENT_ERROR_READ_LOCAL_TEMPERATURE:
    		send_event_device(EVENT_ERROR_READ_LOCAL_TEMPERATURE);
    		break;
    	case EVENT_WAITING_RESPONSE_TEMPERATURE:
    		send_event_device(EVENT_WAITING_RESPONSE_TEMPERATURE);
    	default:
    		break;

    	}
    	/*
    	if (reading_temperature(datosApp) == EVENT_ANSWER_TEMPERATURE) {
    		//send_event(EVENT_DEVICE_OK);



    	} else {
    		send_event(EVENT_ERROR_DEVICE);
    	}

*/


    }

}







float redondear_temperatura(float temperatura) {

	float redondeado;
	float diferencia;
	float resultado = 0;
	float valor_absoluto;

	redondeado = lround(temperatura);
	diferencia = temperatura - redondeado;
	ESP_LOGE(TAG, "temperatura: %.2f, redondeado: %.2f, diferencia: %.2f", temperatura, redondeado, diferencia);
	if (diferencia == 0) {
		resultado = temperatura;
		ESP_LOGI(TAG, ""TRAZAR"TEMPERATURA ORIGINAL: %.2f, TEMPERATURA REDONDEADA: %.2f,", INFOTRAZA, temperatura, resultado);
		return resultado;

	}
	valor_absoluto = fabs(redondeado);
	ESP_LOGE(TAG, "temperatura: %.2f, redondeado: %.2f, diferencia: %.2f, valor absoluto :%.2f", temperatura, redondeado, diferencia, valor_absoluto);
	if (diferencia <= 0.25) {
		ESP_LOGI(TAG, "diferencia <= 0.25");
		resultado = valor_absoluto;
	}
	if ((diferencia > 0.25 ) && (diferencia < 0.5)) {
		ESP_LOGI(TAG, "((diferencia > 0.25 ) && (diferencia < 0.5))");
		resultado = valor_absoluto + 0.5;
	}

	if ((diferencia < -0.25)) {
		ESP_LOGI(TAG, "diferencia < -0.25");
		resultado = valor_absoluto - 0.5;
	}

	ESP_LOGI(TAG, ""TRAZAR"TEMPERATURA ORIGINAL: %.2f, TEMPERATURA REDONDEADA: %.2f,", INFOTRAZA, temperatura, resultado);
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
            	   ESP_LOGI(TAG, ""TRAZAR"RELE OFF Y DEBE SEGUIR SIGUE OFF. tempMedida: %.2f, tempUmbral: %.02f", INFOTRAZA, datosApp->termostato.tempActual, datosApp->termostato.tempUmbral);
                   *accion = OFF;
                   return NO_ACCIONAR_TERMOSTATO;

               }
           } else {
               if (datosApp->termostato.tempActual >= (datosApp->termostato.tempUmbral + datosApp->termostato.margenTemperatura) ) {
            	   ESP_LOGI(TAG, ""TRAZAR"RELE ON Y SE APAGA. tempMedida: %.2f, tempUmbral: %.02f", INFOTRAZA, datosApp->termostato.tempActual, datosApp->termostato.tempUmbral);
                   *accion = OFF;
                   return ACCIONAR_TERMOSTATO;
               } else {
            	   ESP_LOGI(TAG, ""TRAZAR"RELE ON Y DEBE SEGUIR SIGUE ON. tempMedida: %.2f, tempUmbral: %.02f", INFOTRAZA, datosApp->termostato.tempActual, datosApp->termostato.tempUmbral);
                   *accion = ON;
                   return NO_ACCIONAR_TERMOSTATO;

               }
           }

}




EVENT_DEVICE reading_local_temperature(DATOS_APLICACION *datosApp) {

    esp_err_t error = ESP_FAIL;
    static uint8_t contador = 0;
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

    		ESP_LOGE(TAG, ""TRAZAR" ERROR AL TOMAR LA LECTURA", INFOTRAZA);
    		return EVENT_ERROR_READ_LOCAL_TEMPERATURE;

    		/*
    		contador++;
    		if (contador == 5) {
    			//registrar_alarma(datosApp, NOTIFICACION_ALARMA_SENSOR_DHT, ALARMA_SENSOR_DHT, ALARMA_ON, true);
    			return ESP_FAIL;

    		}
    		ESP_LOGE(TAG, ""TRAZAR" ERROR AL TOMAR LA LECTURA. REINTENTAMOS EN %d SEGUNDOS", INFOTRAZA, datosApp->termostato.intervaloReintentos);
        	vTaskDelay(datosApp->termostato.intervaloReintentos * 1000 / portTICK_RATE_MS);
        	*/
    	}

    }

 /*

    if (datosApp->alarmas[ALARMA_SENSOR_DHT].estado_alarma > ALARM_OFF) {
    	ESP_LOGE(TAG, ""TRAZAR"LA ALARMA DE SENSOR SE DESACTIVA", INFOTRAZA);
    	//registrar_alarma(datosApp, NOTIFICACION_ALARMA_SENSOR_DHT, ALARMA_SENSOR_DHT, ALARMA_OFF, true);
    	send_event(EVENT_ERROR_DEVICE);
    	lv_update_alarm_device(datosApp);
    }
*/



    return EVENT_ANSWER_TEMPERATURE;

}

EVENT_DEVICE reading_temperature(DATOS_APLICACION *datosApp) {


	EVENT_DEVICE event = EVENT_DEVICE_OK;

	ESP_LOGI(TAG, ""TRAZAR"LEEMOS TEMPERATURA!!!!!!!!!!!!!!!!!!!!!!!", INFOTRAZA);

	if ((datosApp->termostato.master == false)) {
		if (get_status_alarm(datosApp, ALARM_REMOTE_DEVICE) == ALARM_ON) {
			ESP_LOGW(TAG, ""TRAZAR" termostato en remoto. ADEMAS LA ALARMA ESTA A ON", INFOTRAZA);
			event = reading_local_temperature(datosApp);
			if (event != EVENT_ANSWER_TEMPERATURE) {
				ESP_LOGE(TAG, ""TRAZAR"ERROR EN LA LECTURA EN LOCAL CUANDO ESTA SELECCIONADA LA LECTURA REMOTA", INFOTRAZA);
				event = EVENT_ERROR_READ_LOCAL_TEMPERATURE;
			}
		}
		event = reading_remote_temperature(datosApp);

	} else {
		ESP_LOGW(TAG, ""TRAZAR" Leemos temperatura en local porque el remoto no esta disponible", INFOTRAZA);
		event = reading_local_temperature(datosApp);

	}



	ESP_LOGW(TAG,""TRAZAR"reading_temperature: RETURN %s", INFOTRAZA, local_event_2_mnemonic(event));


	return event;
}

/*
static void temporizacion_lectura_remota(void *arg) {

	static int contador = 0;
	DATOS_APLICACION *datosApp;
	datosApp = (DATOS_APLICACION*) arg;

	contador++;
	switch(datosApp->alarmas[ALARMA_SENSOR_REMOTO].estado_alarma) {

	case ALARM_UNDEFINED:
		//registrar_alarma(datosApp, NOTIFICACION_ALARMA_SENSOR_REMOTO, ALARMA_SENSOR_REMOTO, ALARMA_WARNING, true);
		send_event(EVENT_WARNING_DEVICE);
		break;
	case ALARM_OFF:
		contador = 0;
		break;
	case ALARM_WARNING:
		ESP_LOGE(TAG, ""TRAZAR" ALARMA WARNING EN SENSOR REMOTO. CONTADOR %d ", INFOTRAZA, contador);
		if (contador == 5) {
			if (datosApp->alarmas[ALARMA_SENSOR_REMOTO].estado_alarma != ALARM_ON) {
				//registrar_alarma(datosApp, NOTIFICACION_ALARMA_SENSOR_REMOTO, ALARMA_SENSOR_REMOTO, ALARMA_ON, true);
				send_event(EVENT_ERROR_DEVICE);
			}
		}
		break;
	case ALARM_ON:
		ESP_LOGE(TAG, ""TRAZAR" ALARMA ON EN SENSOR REMOTO. LEEMOS EN LOCAL", INFOTRAZA);
		break;
	}




}
*/
esp_err_t reading_remote_temperature(DATOS_APLICACION *datosApp) {

	ESP_LOGI(TAG, ""TRAZAR" Leyendo desde el sensor remoto %s", INFOTRAZA, datosApp->termostato.sensor_remoto);
	cJSON *objeto = NULL;
	cJSON *comando;
	comando = cJSON_CreateObject();
	objeto = cabeceraGeneral(datosApp);
	if (objeto != NULL) {
		cJSON_AddNumberToObject(objeto, DLG_COMANDO, STATUS_DISPOSITIVO);
		cJSON_AddItemToObject(comando, COMANDO, objeto);
		publicar_mensaje_json(datosApp, comando, datosApp->datosGenerales->parametrosMqtt.topics[CONFIG_INDEX_REMOTE_TOPIC_TEMPERATURE].publish);
	}
	return EVENT_WAITING_RESPONSE_TEMPERATURE;


}


void gpio_rele_in_out() {
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.pin_bit_mask = 1ULL<<CONFIG_GPIO_PIN_RELE;
	io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    ESP_LOGW(TAG, ""TRAZAR" gpio rele en E/S", INFOTRAZA);

}



esp_err_t init_code_application(DATOS_APLICACION *datosApp) {

	gpio_rele_in_out();
	gpio_set_level(CONFIG_GPIO_PIN_RELE, OFF);

	return ESP_OK;

}



