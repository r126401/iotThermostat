cmake_minimum_required(VERSION 3.16)



#add_custom_target( ALL NAME prueba COMMAND date)
set (SERVIDOR_OTA jajicaiot.ddns.net)
set (PUERTO_OTA 8070)
set (DIR_REPOSITORIO /opt/trastero)
set (DIR_INSTALL firmware)
set (PREFIJO_OTA newVersion)
set (DIR_LASTVERSION lastVersion)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

string(TIMESTAMP variable "%y%m%d%H%M")


#include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(PROJECT_VER ${variable})
project(iotThermostat)
set (URL /firmware/${PROJECT_NAME}/)
add_custom_target( instalar ALL COMMAND 
python ${PROJECT_DIR}/instalar.py ${SERVIDOR_OTA} ${PUERTO_OTA} ${PROJECT_NAME} ${DIR_REPOSITORIO} 
${BUILD_DIR} ${PROJECT_VER} ${PREFIJO_OTA} ${DIR_LASTVERSION} ${URL} DEPENDS app)


