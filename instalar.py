
import sys
import subprocess
import json
from _operator import sub



servidorMqtt = sys.argv[1]
puertoMqtt = sys.argv[2]
puertoMqtt = int(puertoMqtt)
nombreProyecto = sys.argv[3]
directorioRepositorio = sys.argv[4] # directorio de base del repositorio de firmware.
directorioImagen = sys.argv[5] # directorio del binario. Normalmento build
versionOta = sys.argv[6] # directorio de instalacion a partir del directorio del repositorio
prefijoOta = sys.argv[7] # prefijo que tiene el fichero imagen
directorioLastVersion = sys.argv[8] # directorio del fichero lastVersion a partir del directorio del repositorio
url = sys.argv[9]# ruta dentro del servidor web para encontrar el binario.
nombre_fichero_lastVersion = "lastVersion"
directorioInstalacion = "firmware"

def obtener_fecha():
    fecha = subprocess.getoutput('date +%y%m%d%H%M')
    return fecha


#versionOta = obtener_fecha()

ficheroOta = prefijoOta + "_" + versionOta +".bin"
ficheroBinarioOrigen = directorioImagen + "/" + nombreProyecto + ".bin"
ficheroBinarioDestino = directorioRepositorio + "/" + directorioInstalacion + "/" + nombreProyecto + "/" + prefijoOta + "_" + versionOta + ".bin"
ficheroLastVersionDestino = directorioRepositorio + "/" + directorioLastVersion + "/" + nombreProyecto + "/" +  nombre_fichero_lastVersion + "_" + versionOta + ".json"

print (ficheroBinarioOrigen)
print(ficheroBinarioDestino)





#ficheroBinario = origenOta + ".ota.bin"
#ficheroLastVersion = "lastVersion.json"






    
def crearJson(servidor, puerto, directorio, destinoOta, otaVersion) :
    
    fecha = subprocess.getoutput('date +%d/%m/%y\ %H:%M')
    cadena = {}
    cadena['token'] = '5586625c-d148-482b-a71d-662399f97414'
    cadena['date'] = fecha
    cadena['dlgComando'] = 100
    cadena['idDevice'] = "otaServer"
    cadena['dlgResultCode'] = 200
    cadena['otaServer'] = servidor
    cadena['otaPort'] = puerto
    cadena['otaUrl'] = directorio
    cadena['otaFile'] = destinoOta
    cadena['otaVersion'] = otaVersion
    cadena['device'] = 100

    
    
    return json.dumps(cadena)


def json_a_fichero(textoJson, ficheroJson):

    fichero = open(ficheroJson, "w")
    fichero.write(textoJson)
    fichero.write("\n")
    fichero.close





def copiaFichero(ficheroOrigen, directorioDestino):


    comando = "cp " + ficheroOrigen + " " + directorioDestino
    print(comando)
    subprocess.getoutput(comando)
    



def copiarFichero(textoJson, directorioInstalacion, origenOta, destinoOta, directorioLastVersion):
    
    nombre_fichero = ficheroLastVersion
    fichero = open(nombre_fichero, "w")
    fichero.write(textoJson)
    fichero.write("\n")
    fichero.close
    
    comando1 = "cp build/" + origenOta + " " + directorioInstalacion + destinoOta
    comando2 = "cp " + ficheroLastVersion + " " + directorioLastVersion
    print(comando1)
       
    subprocess.getoutput(comando1)
    subprocess.getoutput(comando2)
    
def copiarLastVersion(ficheroLastVersion, directorioLastVersion):
    
    
    comando = "cp " + ficheroLastVersion + " " + directorioLastVersion
    print (comando)
    subprocess.getoutput(comando)
    comando2 = "rm ./" + ficheroLastVersion
    print(comando2)
    subprocess.getoutput(comando2)

    
def enlacelastVersion(directorioRepositorio, directorioLastVersion, nombreProyecto, nombre_fichero_lastVersion, versionOta):
    

    nombre_json = nombre_fichero_lastVersion + ".json"


    comando = "cd " + directorioRepositorio + "/" + directorioLastVersion + "/" + nombreProyecto + ";rm " + nombre_json + ";ln -s " +  nombre_fichero_lastVersion + "_" + versionOta + ".json " + nombre_json
    print(comando)
    subprocess.getoutput(comando)






#Comienzo de la logica.

texto = crearJson(servidorMqtt, puertoMqtt, url, ficheroOta, versionOta)
json_a_fichero(texto, nombre_fichero_lastVersion)


print("el fichero es: \n" + texto)

copiaFichero(ficheroBinarioOrigen,ficheroBinarioDestino) 
copiaFichero(nombre_fichero_lastVersion, ficheroLastVersionDestino)
enlacelastVersion(directorioRepositorio, directorioLastVersion, nombreProyecto, nombre_fichero_lastVersion, versionOta)



   
