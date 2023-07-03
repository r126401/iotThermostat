/*
 * datosapp.h
 *
 *  Created on: 5 jun 2023
 *      Author: t126401
 */

#ifndef LV_SRC_INCLUDE_DATOSAPP_H_

#define LV_SRC_INCLUDE_DATOSAPP_H_

#include <time.h>
#include "lvgl.h"

typedef enum TIPO_DISPOSITIVO {
	DESCONOCIDO = -1,
	INTERRUPTOR = 0,
	CRONOTERMOSTATO = 1,
	TERMOMETRO = 2
}TIPO_DISPOSITIVO;

typedef enum ESTADO_RELOJ {
	SIN_HORA, // El reloj no esta sincronizado y no esta operativo.
	SINCRONIZANDO, // El reloj esta en fase de sincronizacion
	SINCRONIZADO, // El reloj esta sincronizado pero hay operaciones a nivel de aplicacion pendientes. Es un estado transitorio.
	EN_HORA, // El reloj esta sincronizado y operativo
	DESAJUSTADO,  // el reloj ha presentado desviacion y necesita ser ajustado
	ERROR_NTP // El reloj no ha podido sincronizarse y por lo tanto la hora no es valida.
}ESTADO_RELOJ;


typedef struct NTP_CLOCK {
    //struct tm date; // Fecha en formato tm
    time_t time; // fecha en formato time_t
    char* ntpTimeZone; // Zona horaria a aplicar segun el pais.
    enum ESTADO_RELOJ estado;
    bool timeValid;

} NTP_CLOCK;


typedef enum ESTADO_APP {
    NORMAL_AUTO,
    NORMAL_AUTOMAN,
    NORMAL_MANUAL,
    NORMAL_ARRANCANDO,
    NORMAL_SIN_PROGRAMACION,
    UPGRADE_EN_PROGRESO,
	NORMAL_SINCRONIZANDO,
	ESPERA_FIN_ARRANQUE,
	ARRANQUE_FABRICA,
	NORMAL_FIN_PROGRAMA_ACTIVO



} ESTADO_APP;

typedef enum ESTADO_RELE {
    INDETERMINADO = -1,
    OFF = 0,
    ON = 1
}ESTADO_RELE;

typedef enum ESTADO_ALARMA {
	ALARMA_INDETERMINADA = -1,
	ALARMA_OFF,
	ALARMA_WARNING,
	ALARMA_ON,


}ESTADO_ALARMA;


typedef struct ALARMA {

	uint8_t tipo_alarma;
	enum ESTADO_ALARMA estado_alarma;
	time_t fecha_alarma;
	char nemonico[50];

} ALARMA;

typedef struct OTADATA {
    char server[100];
    char url[150];
    int puerto;
    char file[50];
    const char* swVersion;
    char newVersion[32];


}OTADATA;

typedef enum PROG_STATE {
    INVALID_PROG, // La programacion no es valida. Es un estado transitorio que solo debe pasar en el arranque
    VALID_PROG, // Se han ordenado las programaciones y se pueden usar dentro de la aplicacion.
    INH_PROG // Se han inhibido todos los programas y la aplicacion no ejecutara ninguna accion programada.

} PROG_STATE;


typedef struct MQTT_PARAM {
    char broker[100];
    int port;
    char user[25];
    char password[25];
    char publish[50];
    char subscribe[50];
    int qos;
    bool tls;
    char* cert;

} MQTT_PARAM;

typedef struct DATOS_GENERALES {
	enum TIPO_DISPOSITIVO tipoDispositivo;
    MQTT_PARAM parametrosMqtt;
    struct NTP_CLOCK clock;
    struct TIME_PROGRAM *programacion;
    uint8_t nProgramacion;
    uint8_t programa_estatico_real;
    int nProgramaCandidato;
    PROG_STATE estadoProgramacion;
    OTADATA ota;
    bool botonPulsado;
    //Estado de la aplicacion
    enum ESTADO_APP estadoApp;


}DATOS_GENERALES;


typedef struct DATOS_TERMOSTATO {
    float tempActual;
    float tempUmbral;
    float tempUmbralDefecto;
    float humedad;
    double margenTemperatura;
    uint8_t reintentosLectura;
    uint8_t intervaloReintentos;
    uint8_t intervaloLectura;
    double calibrado;
    bool master;
    char sensor_remoto[13];
    float incdec;

}DATOS_TERMOSTATO;

/**
 * @struct DATOS_APLICACION
 * @brief Estructura general de la aplicacion.
 *
 */
typedef struct DATOS_APLICACION {

	DATOS_GENERALES *datosGenerales;
    //nvs_handle handle;
    //esp_mqtt_event_handle_t handle_mqtt;
    ALARMA alarmas[5];
    DATOS_TERMOSTATO termostato;
    lv_disp_t *lcd;
    lv_group_t *iconos;
    lv_obj_t *popup;
    lv_group_t *zona_umbral;
    lv_group_t *zona_temperatura;




} DATOS_APLICACION;

typedef enum ESTADO_PROGAMA {
    INACTIVO,
	ACTIVO

} ESTADO_PROGAMA;

typedef enum TIPO_PROGRAMA {
                DIARIA,
                SEMANAL,
                FECHADA
}TIPO_PROGRAMA;

#define TERMOSTATO

#ifndef TERMOSTATO

typedef struct DATOS_PROGRAMAS {
	char idPrograma[19];
	struct tm programacion;
	time_t programa;
	enum TIPO_PROGRAMA tipo;
	uint8_t activo;
	uint8_t mascara;
	enum ESTADO_PROGAMA estadoPrograma;
}DATOS_PROGRAMAS;

typedef struct DATOS_PROGRAMAS_INTERRUPTOR {
	DATOS_PROGRAMAS datos_programa;
	uint8_t accion;
	uint8_t mascara;
	uint32_t duracion;


}DATOS_PROGRAMAS_INTERRUPTOR;

typedef struct TIME_PROGRAM {
                char idPrograma[19];
                struct tm programacion;
                enum TIPO_PROGRAMA tipo; // Indica el tipo de programa
                time_t programa; // fecha en formato time_t del programa. Tiene sentido cuando es fechado
                uint8_t activo; // cuando el programa es diario, marca segun la mascara si el programa se ejecutara o no
                int accion;
                uint8_t mascara; // mascara que mapea los dias activos cuando la programacion es diaria
                enum ESTADO_PROGAMA estadoPrograma; // Indica si el programa esta activo o inhibido
                uint32_t duracion;
                union {
                    double temperatura;
                    double humedad;

                };
}TIME_PROGRAM;

#else
typedef struct TIME_PROGRAM {
                char idPrograma[19];
                struct tm programacion;
                enum TIPO_PROGRAMA tipo;
                time_t programa;
                uint8_t activo;
                int accion;
                float temperatura;
                float humedad;
                uint8_t mascara;
                enum ESTADO_PROGAMA estadoPrograma;
                uint32_t duracion;

}TIME_PROGRAM;

#endif


#endif /* LV_SRC_INCLUDE_DATOSAPP_H_ */
