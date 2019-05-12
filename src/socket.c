/**
 * socket.c
 * Librería que contiene las funciones de los sockets
 * Lucía Rivas Molina <lucia.rivasmolina@estudiante.uam.es>
 * Daniel Santo-Tomás López <daniel.santo-tomas@estudiante.uam.es>
 */
#include "socket.h"
#include "picohttpparser.h"
#include "http.h"


/**
* Funcion : iniciar_servidor
* Inicia un servidor dados un puerto y un maximo de clientes
* INPUT : int puerto -> puerto del servidor
*         int max_clients -> máx de clientes que acepta el servidor
* OUTPUT : ERROR o el descriptor del socket
*/
int iniciar_servidor(int puerto, int max_clients) {
  int sockval;
  struct sockaddr_in Direccion;

 /* Creamos el socket, escribiendo en LOG_ERR en caso de error*/
  syslog (LOG_INFO, "Creando un socket");
  if ( (sockval = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
    syslog(LOG_ERR, "Error creating socket");
    return ERROR;
  }


  /* familia TCP/IP  */
  Direccion.sin_family = AF_INET;
  /*Asignando puerto*/
  Direccion.sin_port = htons(puerto);
  /* Acepta todas las direciones */
  Direccion.sin_addr.s_addr = htonl(INADDR_ANY);
  /* Sustituye los 8 primeros bytes de sin_zero por ceros*/
  bzero((void *)&(Direccion.sin_zero), 8);

  /* Asociamos la información de Dirección al socket creado*/
  syslog (LOG_INFO, "Binding socket");
  if (bind (sockval, (struct sockaddr *)&Direccion, sizeof(Direccion)) < 0){
    syslog(LOG_ERR, "Error binding socket");
    return ERROR;
  }

  /*Establecemos el servidor en modo escucha*/
  syslog (LOG_INFO, "Escuchando conexiones");
  if (listen (sockval, max_clients) < 0){
    syslog(LOG_ERR, "Error escuchando conexiones");
    return ERROR;
  }
  return sockval;

}


/**
* Funcion : aceptar_conexion
* Acepta las conexiones recibidas registrando un socket
* INPUT :  sockval -> socket que se registra
* OUTPUT : el descriptor de socket o en caso de error ERROR
*/
int aceptar_conexion(int sockval){
  int desc;
  socklen_t len;
  struct sockaddr Conexion;

  len = sizeof(Conexion);
  syslog (LOG_INFO, "Aceptando coexion por socket");

  /*Registra el socket sockval en el servidor*/
  if ((desc = accept(sockval, &Conexion, &len)) < 0){
      syslog(LOG_ERR, "Error aceptando conexion");
      return ERROR;
  }
  syslog (LOG_INFO, "Aceptada conexion por socket");
  /* Devuelve el descriptor del socket */
  return desc;
}

/**
* Funcion : aceptar_conexion
* Acepta las conexiones recibidas registrando un socket
* INPUT :  desc -> descriptor de socket devuelto por accept
*          root -> directorio donde está guardado el servidor
*          nombre -> nombre del servidor
* OUTPUT : OK/ERROR
*/
int asociar_socket(int desc, char *root, char *nombre){
   /*Procesa y parsea la peticion recibida*/
   if (procesar(desc, root, nombre) == ERROR){
     syslog(LOG_ERR, "Error procesando petición");
     close(desc);
     return ERROR;
   }
   /* Antes de salir cierra el socket */
   close(desc);
   return OK;
}


/*
* Funcion: procesar
* Función que procesa y parsea las peticiones que le llegan, llamando a la
* funcion GET, POST u OPTIONS dependiendo de la peticion
*
* INPUT :  desc -> descriptor del socket por el que procesamos la peticion
*          root -> directorio donde se almacena el servidor
*          nombre -> nombre del servidor
* OUTPUT : OK/ERROR
*/
int procesar(int desc, char *root, char *nombre){

  char buf[4096], root_aux[1024], num[20] = {0};
  char *path_aux, *token = NULL, *args = NULL;
  const char *method, *path;
  int pret, minor_version, error, tam, j;
  struct phr_header headers[100];
  size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers, i ;
  ssize_t rret;

  syslog (LOG_INFO, "Leyendo y parseando peticiones");
  while (1) {
      /* Lee la peticion */
      while ((rret = read(desc, buf + buflen, sizeof(buf) - buflen)) == -1);
      if (rret <= 0){
         syslog (LOG_ERR, "Error leyendo peticion");
      }

      prevbuflen = buflen;
      buflen += rret;

      /* Parsea la peticion */
      num_headers = sizeof(headers) / sizeof(headers[0]);

      pret = phr_parse_request(buf, buflen, &method, &method_len, &path, &path_len,
                               &minor_version, headers, &num_headers, prevbuflen);

      /* Si se ha parseado bien  salimos */
      if (pret > 0) break;
      /* En caso de error */
      else if (pret == -1) {
         syslog (LOG_ERR, "Error parseando peticion");
         return ERROR;
      }
      /* Si todavía no ha acabado de parsear continua */
      assert(pret == -2);
      if (buflen == sizeof(buf)) {
         syslog (LOG_ERR, "Error parseando peticion");
         return ERROR;
      }
  }

  /* En caso de no recibir bien la respuesta mandamos un 400 Bad Request */
    if (!method || !path){
      syslog(LOG_ERR, "Error 400");
      buflen = sprintf(buf,"HTTP/1.%d 400 Bad Request\r\n",minor_version);
      send(desc, buf, buflen, 0);
      return OK;
    }
    /* Creamos una copia para el path */
    path_aux = (char*)calloc((path_len + 1),sizeof(char));
    sprintf(path_aux, "%.*s", (int)path_len, path);
    strcpy(root_aux, root);

    /*Se pide un GET*/
    if (*method == 71){
        syslog (LOG_INFO, "Procesando peticion GET");
        error = http_get(desc, path_aux, minor_version, root_aux, nombre);
    }

    /*Se pide un options */
    else if (*method == 79){
        syslog (LOG_INFO, "Procesando peticion OPTIONS");
        error = http_options(desc, minor_version, nombre);
    }

    /*Se pide un post */
    else if (*method == 80){

      syslog (LOG_INFO, "Procesando peticion POST");
      /* Obtenemos el número de caracteres del cuerpo a raiz del header 2*/
      for( i = 0 ; i < headers[2].value_len; i++) num[i] = headers[2].value[i];

      tam = atoi(num);
      args = (char*)calloc(1024, sizeof(char));
      /*Guardamos el contenido del cuerpo */
      for( j = 0; j < tam; j++){
        args[j] = (char)buf[buflen-tam+j];
      }

      /* Comprobamos si se pasa algún argumento por el root*/
      token = strtok(path_aux, "?");
      /* Segundo token */
      token = strtok(NULL, "?");

      /* Si no hay argumentos en el root, añade el \n a args*/
      if(!token){
        strcat(args,"\n");
      }
      /* Si hay argumentos, los concatena y añade un \n al final*/
      else{
        strcat(args, "\n");
        strcat(args, token);
        strcat(args, "\n");
      }
      /* Ejecuta la funcion post y comprueba si hay algun error */
      error = http_post(desc, path_aux, minor_version, root_aux, args);
    }

    /* En otro caso, no hemos implementado más métodos luego devuelve 501 Not Implemented*/
    else {
      free(path_aux);
      free(args);
      syslog(LOG_ERR, "Error 501 Mtehod Not Implemented");
      buflen = sprintf(buf,"HTTP/1.%d 501 Method Not Implemented\r\n",minor_version);
      send(desc, buf, buflen, 0);
      return OK;
   }

    /* Devolvemos el error recibido en caso de devolver alguno */
    if (error == ERROR){
      free(path_aux);
      free(args);
      syslog(LOG_ERR, "Error procesando la peticion");
      return ERROR;
    }
    /* Si devuelve un error 404 */
    else if (error == ERROR_404){
        free(path_aux);
        free(args);
        syslog(LOG_ERR, "Error 404 Not Found");
        buflen = sprintf(buf, "HTTP/1.%d 404 Not Found\r\n", minor_version);
        send(desc, buf, buflen, 0);
        return OK;
    }
    /* Si devuelve error 501 */
    else if (error == ERROR_501 ){
      free(path_aux);
      free(args);
      syslog(LOG_ERR, "Error 501 Mtehod Not Implemented");
      buflen = sprintf(buf,"HTTP/1.%d 501 Method Not Implemented\r\n", minor_version);
      send(desc, buf, buflen, 0);
      return OK;
   }

    /* Liberamos memoria y salimos */
    free(path_aux);
    free(args);
    return OK;
}



/**
 * Funcion cerrar_socket
 * Funcion que cierra el socket pasado como parametro de entrada
 *
 * INPUT : socket -> id de socket a cerrar
 * OUTPUT : void
 */
void cerrar_socket(int socket){
   close(socket);
   return;
}
