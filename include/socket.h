/**
 * socket.h
 * Librería que contiene las funciones de los sockets
 *
 * Lucía Rivas Molina <lucia.rivasmolina@estudiante.uam.es>
 * Daniel Santo-Tomás López <daniel.santo-tomas@estudiante.uam.es>
 */
#define  _GNU_SOURCE
#ifndef SOCKET_H
#define SOCKET_H

#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>
#include <syslog.h>


/**
* Funcion : iniciar_servidor
* Inicia un servidor dados un puerto y un maximo de clientes
* INPUT : int puerto -> puerto del servidor
*         int max_clients -> máx de clientes que acepta el servidor
* OUTPUT : ERROR o el descriptor del socket
*/
int iniciar_servidor(int puerto, int max_clients);

/**
* Funcion : aceptar_conexion
* Acepta las conexiones recibidas registrando un socket
* INPUT :  sockval -> socket que se registra
* OUTPUT : el descriptor de socket o en caso de error ERROR
*/
int aceptar_conexion(int sockval);

/**
* Funcion : aceptar_conexion
* Acepta las conexiones recibidas registrando un socket
* INPUT :  desc -> descriptor de socket devuelto por accept
*          root -> directorio donde está guardado el servidor
*          nombre -> nombre del servidor
* OUTPUT : OK/ERROR
*/
int asociar_socket(int desc, char *root, char *nombre);

/**
* Funcion: procesar
* Función que procesa y parsea las peticiones que le llegan, llamando a la
* funcion GET, POST u OPTIONS dependiendo de la peticion
*
* INPUT :  desc -> descriptor del socket por el que procesamos la peticion
*          root -> directorio donde se almacena el servidor
*          nombre -> nombre del servidor
* OUTPUT : OK/ERROR
*/
int procesar(int desc, char *root, char *nombre);

/**
 * Funcion cerrar_socket
 * Funcion que cierra el socket pasado como parametro de entrada
 *
 * INPUT : socket -> id de socket a cerrar
 * OUTPUT : void
 */
void cerrar_socket(int socket);

#endif
