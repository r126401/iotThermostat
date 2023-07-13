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
xQueueHandle cola_gpio = NULL;
static esp_timer_handle_t temporizador_refresco_led;
static esp_timer_handle_t temporizador_lectura_remota;

#define TASA_REFRESCO_LED 5 //ms
#define NUM_REPETICIONES    3
#define TIMEOUT_LECTURA_REMOTA 5000 //ms

#ifdef CONFIG_GPIO_PIN_DHT
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<CONFIG_GPIO_PIN_LED)| (1ULL<<CONFIG_GPIO_PIN_DHT) | (1ULL<<CONFIG_GPIO_PIN_LED_LCD))
#endif

#ifdef CONFIG_GPIO_PIN_DS18B20

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<CONFIG_GPIO_PIN_LED)| (1ULL<<CONFIG_GPIO_PIN_LED_LCD) | (1ULL<<CONFIG_GPIO_PIN_DS18B20))
#endif

//#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<CONFIG_GPIO_PIN_LED)| (1ULL<<CONFIG_GPIO_PIN_DHT) | (1ULL<<CONFIG_GPIO_PIN_LED_LCD) | (1ULL<<CONFIG_GPIO_PIN_DS18B20))
#define GPIO_INPUT_PIN_SEL  ( (1ULL<<CONFIG_GPIO_PIN_BOTON))

/*
static void gpio_rele_out() {

	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.pin_bit_mask = 1ULL<<CONFIG_GPIO_PIN_RELE;
	io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    ESP_LOGW(TAG, ""TRAZAR" gpio rele en salida", INFOTRAZA);

}




*/







static void gpio_rele_in_out() {
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

static void refresco_lcd(void *arg) {


	if (gpio_get_level(CONFIG_GPIO_PIN_LED_LCD) == OFF) {
		gpio_set_level(CONFIG_GPIO_PIN_LED_LCD, ON);

	} else {
		gpio_set_level(CONFIG_GPIO_PIN_LED_LCD, OFF);
	}


}

void accionar_termostato(DATOS_APLICACION *datosApp) {

	enum ESTADO_RELE accion_rele;
	enum TIPO_ACCION_TERMOSTATO accion_termostato;
	cJSON* informe = NULL;
	static float lecturaAnterior = -1000;

    if (((accion_termostato = calcular_accion_termostato(datosApp, &accion_rele)) == ACCIONAR_TERMOSTATO)) {
    	operacion_rele(datosApp, TEMPORIZADA, accion_rele);
    }

    if ((accion_termostato == ACCIONAR_TERMOSTATO) || (lecturaAnterior != datosApp->termostato.tempActual)) {
    	//lv_actualizar_temperatura_lcd(datosApp);
        informe = appuser_send_spontaneous_report(datosApp, CAMBIO_TEMPERATURA, NULL);
        publicar_mensaje_json(datosApp, informe, NULL);
    }
    lecturaAnterior = datosApp->termostato.tempActual;
}



void tarea_lectura_temperatura(void *parametros) {

    DATOS_APLICACION *datosApp = (DATOS_APLICACION*) parametros;



    while(1) {

    	ESP_LOGE(TAG, ""TRAZAR" tempUmbral %.02f", INFOTRAZA, datosApp->termostato.tempUmbral);
    	leer_temperatura(datosApp);
    	ESP_LOGE(TAG, ""TRAZAR" tempUmbral %.02f", INFOTRAZA, datosApp->termostato.tempUmbral);
    	accionar_termostato(datosApp);
    	/*
	    if (((accion_termostato = calcular_accion_termostato(datosApp, &accion_rele)) == ACCIONAR_TERMOSTATO)) {
	    	operacion_rele(datosApp, TEMPORIZADA, accion_rele);
	    }

	    if ((accion_termostato == ACCIONAR_TERMOSTATO) || (lecturaAnterior != datosApp->termostato.tempActual)) {
	    	lv_actualizar_temperatura_lcd(datosApp);
            informe = appuser_generar_informe_espontaneo(datosApp, CAMBIO_TEMPERATURA, NULL);
            publicar_mensaje_json(datosApp, informe, NULL);

            //lv_actualizar_humedad_lcd(datosApp);
	    }

	    lecturaAnterior = datosApp->termostato.tempActual;
	    */
	    vTaskDelay(datosApp->termostato.intervaloLectura * 1000 / portTICK_RATE_MS);


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


void pulsacion_modo_app(DATOS_APLICACION *datosApp) {

	time_t t_siguiente_intervalo;
	ESP_LOGI(TAG, ""TRAZAR" PONER QUITAR MODO MANUAL :%d", INFOTRAZA, datosApp->datosGenerales->estadoApp );
	switch (datosApp->datosGenerales->estadoApp) {

	case NORMAL_AUTO:
	case NORMAL_AUTOMAN:
		//datosApp->termostato.tempUmbral = datosApp->termostato.tempUmbralDefecto;
		appuser_notify_app_status(datosApp, NORMAL_MANUAL);
		break;
	case NORMAL_MANUAL:
		//ESP_LOGE(TAG, ""TRAZAR" pasamos a modo auto", INFOTRAZA);
		appuser_notify_app_status(datosApp, NORMAL_AUTO);
		calcular_programa_activo(datosApp, &t_siguiente_intervalo);
		break;
	default:
		//ESP_LOGE(TAG, ""TRAZAR" NO SE PUEDE CAMBIAR EL ESTADO POR NO SER CONSISTENTE. ESTADO %d", INFOTRAZA, datosApp->datosGenerales->estadoApp);
		break;

	}
	leer_temperatura(datosApp);


}

void pulsacion(void *arg) {


    static esp_timer_handle_t timer_pulse;
    static uint8_t rep=0;
    cJSON *informe;

    DATOS_APLICACION *datosApp;
    datosApp = (DATOS_APLICACION*) arg;



    const esp_timer_create_args_t timer_pulse_args = {
            .callback = &pulsacion,
            /* name is optional, but may help identify the timer when debugging */
            .name = "pulsacion",
			.arg = (void*) datosApp
    };


    ESP_LOGW(TAG, "pulsacion  %p", datosApp);
    ESP_LOGI(TAG, ""TRAZAR"RUTINA QUE TRATA LAS INTERRUPCIONES", INFOTRAZA);
    //gpio_rele_in();
    if (gpio_get_level(CONFIG_GPIO_PIN_BOTON) == OFF) {
    	ESP_ERROR_CHECK(esp_timer_create(&timer_pulse_args, &timer_pulse));
    	ESP_ERROR_CHECK(esp_timer_start_once(timer_pulse,  500*1000));

        rep++;
        ESP_LOGI(TAG, ""TRAZAR"repeticion %d", INFOTRAZA, rep);

    } else {
        datosApp->datosGenerales->botonPulsado = false;
        //*rebote = false;
        //printf("Rebote cancelado, rep = %d\n", rep);
        if (rep > NUM_REPETICIONES) {

            ESP_LOGI(TAG, ""TRAZAR"pulsacion larga", INFOTRAZA);



            if (esp_netif_is_netif_up(ESP_IF_WIFI_STA) == true) {
            	ESP_LOGI(TAG, ""TRAZAR"EJECUTAMOS RESTART", INFOTRAZA);
                esp_restart();
            } else {
                //smartconfig
                ESP_LOGI(TAG,"sin ip, entramos en smartconfig...");
                //smartconfig_set_type(SC_TYPE_ESPTOUCH);
                ESP_LOGI(TAG, ""TRAZAR"AQUI LANZARIAMOS LA RUTINA DE SMARTCONFIG", INFOTRAZA);
                appuser_notify_smartconfig(datosApp);
                xTaskCreate(tarea_smartconfig, "tarea_smart", 2048, (void*)&datosApp, tskIDLE_PRIORITY + 0, NULL);


            }

        } else {
            //ESP_LOGI(TAG, ""TRAZAR"pulsacion corta 0x%08x", INFOTRAZA, datosApp);
            ESP_LOGW(TAG, "pulsacion corta %p", datosApp);
            pulsacion_modo_app(datosApp);
            /*
            if (datosApp->datosGenerales->estadoApp < 2) {
            	appuser_cambiar_modo_aplicacion(datosApp, NORMAL_MANUAL);

            } else {
                if (datosApp->datosGenerales->estadoApp == NORMAL_MANUAL) {
            		appuser_cambiar_modo_aplicacion(datosApp, NORMAL_AUTO);


                }
            }*/
            informe = appuser_send_spontaneous_report(datosApp, CAMBIO_ESTADO_APLICACION, NULL);
            publicar_mensaje_json(datosApp, informe, NULL);
            //poner_quitar_modo_manual(datosApp);

            //operacion_rele(datosApp, MANUAL, INDETERMINADO);

        }

        rep=0;
    }
}

static void tratarInterrupcionesPulsador(void *arg) {


	DATOS_APLICACION *datosApp;
	datosApp = (DATOS_APLICACION*) arg;


    static esp_timer_handle_t timer_pulse;

    const esp_timer_create_args_t timer_pulse_args = {
    		.callback = &pulsacion,
    		.name = "tratarInterrupcionesPulsador",
			.arg = (void*) datosApp
    };


    if (datosApp->datosGenerales->botonPulsado == false) {
        datosApp->datosGenerales->botonPulsado = true;
        //ets_timer_disarm(&notificacionWifi);
        ESP_ERROR_CHECK(esp_timer_create(&timer_pulse_args, &timer_pulse));
        ESP_ERROR_CHECK(esp_timer_start_once(timer_pulse, 250*1000));


    }




    //GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status );
}

esp_err_t init_code_application(DATOS_APLICACION *datosApp) {



	esp_err_t error = ESP_OK;
	ESP_LOGW(TAG, "appuser_inicializar_aplicacion %p", datosApp);
    datosApp->datosGenerales->botonPulsado = false;

    ESP_LOGI(TAG, ""TRAZAR"INICIALIZAR PARAMETROS PARTICULARES DE LA APLICACION", INFOTRAZA);

    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    //ESP_LOGI(TAG, ""TRAZAR" VAMOS (0)", INFOTRAZA);
    gpio_config(&io_conf);
    //ESP_LOGI(TAG, ""TRAZAR" VAMOS (1)", INFOTRAZA);
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;

    gpio_config(&io_conf);

    //gpio_set_level(CONFIG_GPIO_PIN_RELE, OFF);
    //change gpio intrrupt type for one pin
    //gpio_set_intr_type(CONFIG_GPIO_PIN_BOTON, GPIO_INTR_ANYEDGE);


    gpio_rele_in_out();
#ifdef CONFIG_GPIO_PIN_DHT
    gpio_set_pull_mode(CONFIG_GPIO_PIN_DHT, GPIO_PULLUP_ONLY);
#endif

#ifdef CONFIG_GPIO_PIN_DS18B20
    gpio_set_pull_mode(CONFIG_GPIO_PIN_DS18B20, GPIO_PULLUP_ONLY);
#endif

    const esp_timer_create_args_t timer_refresh_led_args = {
            .callback = &refresco_lcd,
            /* name is optional, but may help identify the timer when debugging */
            .name = "led refresh",
			.arg = (void*) datosApp
    };

    ESP_ERROR_CHECK(esp_timer_create(&timer_refresh_led_args, &temporizador_refresco_led));
    ESP_ERROR_CHECK(esp_timer_start_periodic(temporizador_refresco_led, TASA_REFRESCO_LED * 1000));

    /*
    ets_timer_disarm(&temporizador_refresco_led);
    ets_timer_setfn(&temporizador_refresco_led, (ETSTimerFunc*) refresco_lcd, NULL);
    ets_timer_arm(&temporizador_refresco_led, TASA_REFRESCO_LED,true);
*/


    gpio_set_level(CONFIG_GPIO_PIN_RELE, OFF);
    //change gpio intrrupt type for one pin
    gpio_set_intr_type(CONFIG_GPIO_PIN_BOTON, GPIO_INTR_ANYEDGE);


    ESP_LOGI(TAG, ""TRAZAR" VAMOS A CREAR LA COLA GPIO", INFOTRAZA);
    //create a queue to handle gpio event from isr
    cola_gpio = xQueueCreate(3, sizeof(DATOS_APLICACION));
    ESP_LOGI(TAG, ""TRAZAR" COLA GPIO CREADA", INFOTRAZA);
    //xTaskCreate(tratar_interrupciones, "tratar_interrupciones", 4096, NULL, 10, NULL);
    ESP_LOGI(TAG, ""TRAZAR" RUTINA DE TRATAR INTERRUPCIONES LANZADA", INFOTRAZA);
    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(CONFIG_GPIO_PIN_BOTON, tratarInterrupcionesPulsador,datosApp);
    ESP_LOGI(TAG, ""TRAZAR"RUTINA ISR_HANDLER LANZADA", INFOTRAZA);


    return error;

}

