/**
* http.h
* Librería que contiene las funciones que procesa las peticiones HTTP
*
* Lucía Rivas Molina <lucia.rivasmolina@estudiante.uam.es>
* Daniel Santo-Tomás López <daniel.santo-tomas@estudiante.uam.es>
*/
#define  _GNU_SOURCE
#ifndef HTTP_H
#define HTTP_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>

/**
 * Macro OK
 * Devolver OK en las funciones
 */
#define OK 0
/**
 * Macro ERROR
 * Devolver ERROR en las funciones
 */
#define ERROR -1
/**
 * Macro ERROR_404
 * Devolver el error 404 Not Found
 */
#define ERROR_404 -2
/**
 * Macro ERROR
 * Devolver el error 400
 */
#define ERROR_400 -3
/**
 * Macro ERROR_501
 * Devuelve el error 501 Method Not Implemented
 */
#define ERROR_501 -4

/*
* Funcion: http_get
* Función que procesa las petciones HTTP get y construye la cabecera
* INPUT :  desc -> descriptor del socket por el que enviamos la cabecera
*          path -> path de la petición
*          version -> version del HTTP
*          root -> directorio donde se almacena el socket
*          nombre -> nombre del servidor
* OUTPUT : OK/ERROR o ERROR_404 en caso de no encontrar lo que se ha pedido
*/
int http_get(int desc, char *path, int version, char *root, char *nombre);


/*
* Funcion: http_post
* Función que procesa las petciones HTTP post y llama a la funcion que ejecuta scripts
* INPUT :  desc -> descriptor del socket por el que enviamos la cabecera
*          path -> path de la petición
*          version -> version del HTTP
*          root -> directorio donde se almacena el socket
*          args -> argumentos de la peticion
* OUTPUT : OK/ERROR o ERROR_404 en caso de no encontrar lo que se ha pedido
*/
int http_post(int desc, char *path, int version, char *root, char* args);

/**
 * Funcion : ejecuta_scripts
 * Ejecuta los scripts http get (py o php)
 * INPUT : root -> directorio donde se almacena el servidor
 *         arg -> argumentos para el script
 *         tipo -> py o php
 *         version -> version del HTTP
 *         desc -> descriptor del socket por el que mandamos la respuesta
 * OUTPUT : OK/ERROR
 */
int ejecuta_scripts(char *root, char* argum, char* tipo, int version, int desc);

/*
* Funcion: http_options
* Función que procesa las petciones HTTP options y construye la cabecera
* INPUT :  desc -> descriptor del socket por el que enviamos la cabecera
*          version -> version del HTTP
*          nombre -> nombre del servidor
* OUTPUT : OK/ERROR
*/
int http_options(int desc, int version, char *nombre);

/*
* Funcion: dia_semana(int dia)
* INPUT : dia -> numero de la semana del 0 al 6
* OUTPUT : dia de la semana como char
*/
char *dia_semana(int dia);


/*
* Funcion: mes(int mes)
* INPUT : mes -> numero del mes del 0 al 11
* OUTPUT : dia de la semana como char
*/
char *mes(int mes);

/*
* Funcion: fecha()
* INPUT : void
* OUTPUT : devuelve el formato de la fecha en GMT
*/
char *fecha(void);

/*
* Funcion: fecha_ultima_mod
* INPUT : root -> directorio donde se almacena el servidor
* OUTPUT : fecha de ultima modificacion del path
*/
char *fecha_ultima_mod(char *root);

/*
* Funcion: tamanio_recurso
* INPUT : root -> directorio donde se almacena el servidor
* OUTPUT : devuelve el tamaño del root
*/
int tamanio_recurso(char *root);

/*
* Funcion: content_type
* INPUT : path con la extension a descubrir
* OUTPUT : devuelve la extensión del path recibido
*/
char *content_type(char *path);



#endif
