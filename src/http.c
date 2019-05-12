/**
 * http.c
 * Librería que contiene las funciones que procesa las peticiones HTTP
 *
 * Lucía Rivas Molina <lucia.rivasmolina@estudiante.uam.es>
 * Daniel Santo-Tomás López <daniel.santo-tomas@estudiante.uam.es>
 */
#include "http.h"


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
int http_get(int desc, char *path, int version, char *root, char *nombre){
  char buf[4096], aux[1024];
  char *fech, *root_aux, *tipo, *token;
  size_t buflen = 0;
  int f, a;

  /*Si path es / concatenamos index.html*/
  if (strcmp("/", path) == 0){
    strcat(root, "/index.html");
  }
  else{
    strcat(root, path);
  }

  /* Vemos qué tipo de fichero se ha pedido */
  root_aux = (char*)malloc(sizeof(char)*(strlen(root) + 1));
  if(!root_aux){
      syslog (LOG_ERR, "Error reservando memoria http_get");
      return ERROR;
 }
  strcpy(root_aux, root);
  tipo = content_type(root_aux);
  free(root_aux);

  /* Si no existe el tipo devolvemos error 501 */
  if(!tipo){
     syslog(LOG_ERR, "Error 501 Not Implemented : Content Type");
     return ERROR_501;
 }

  /* Si se ha pedido un script en python o php ejecutamos los scripts */
  if (strcmp(tipo, "py") == 0 || strcmp(tipo, "php") == 0){

    /*Obtenemos los argumentos directamente del root*/
    token = strtok(root, "?");
    /* Segundo token */
    token = strtok(NULL, "?");
    strcat(token, "\n");

    syslog (LOG_INFO, "Ejecutando scripts http_get");
    return ejecuta_scripts(root, token, tipo, version, desc);
  }

  /* Abrimos el fichero para comprobar si existe*/
  f = open(root, O_RDONLY);
  if (f < 0 ){
     syslog (LOG_ERR, "Error 404 Not Found");
    return ERROR_404;
  }

  /* Enviamos la cabecera HTTP con la version */
  buflen = sprintf(buf, "HTTP/1.%d 200 OK\r\n", version);
  send(desc, buf, buflen, 0);
  syslog (LOG_INFO, "Enviando cabecera http_get");

  /*Creamos los headers*/
  fech = fecha();
  buflen = sprintf(buf, "Date: %s\r\n", fech);
  free(fech);

  buflen += sprintf(aux, "Server: %s\r\n", nombre);
  strcat(buf, aux);

  fech = fecha_ultima_mod(root);
  buflen += sprintf(aux, "Last-Modified: %s\r\n", fech);

  free(fech);
  strcat(buf, aux);

  buflen += sprintf(aux, "Content-Length: %d\r\n", tamanio_recurso(root));
  strcat(buf, aux);

  buflen += sprintf(aux, "Content-Type: %s\r\n\r\n",tipo );
  strcat(buf, aux);

  send(desc, buf, buflen, 0);
  syslog (LOG_INFO, "Enviando cuerpo http_get");

  memset(buf, 0, 4096);

  /*Enviamos el archivo de byte en byte*/
  while ((a = read(f, buf, 4096)) != 0){
      if (a < 0) {
         syslog (LOG_ERR, "Error leyendo fichero");
         return ERROR;
      }
      if (send(desc, buf, a, 0) < 0) {
         syslog (LOG_ERR, "Error enviando respuesta http_get");
         return ERROR;
      }
      memset(buf, 0, 4096);
  }
  close(f);
  return OK;
}

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
int http_post(int desc, char *path, int version, char *root, char* args){
  char* tipo, *root_aux;

  /* Concatenamos el root y el path */
  strcat(root, path);

  /* Sacamos el tipo de archivo que se pide */
  root_aux = (char*)malloc(sizeof(char)*(strlen(root) + 1));
  strcpy(root_aux, root);
  tipo = content_type(root_aux);
  free(root_aux);

  /* Si no existe el tipo devolvemos error 501 */
  if(!tipo){
     syslog(LOG_ERR, "Error 501 Not Implemented : Content Type");
     return ERROR_501;
 }

  /* Llama a la función que ejecuta los scripts */
  syslog (LOG_INFO, "Ejecutando scripts http_post");
  return ejecuta_scripts(root, args,tipo, version, desc);
}


/**
 * Funcion : ejecuta_scripts
 * Ejecuta los scripts http get (py o php)
 * INPUT : root -> directorio donde se almacena el servidor
 *         tipo -> py o php
 *         version -> version del HTTP
 *         desc -> descriptor del socket por el que mandamos la respuesta
 * OUTPUT : OK/ERROR
 */
int ejecuta_scripts(char *root, char* argum, char* tipo, int version, int desc){
  char *aux, *res;
  char buf[4096];
  char *arg[5];
  int pid, buflen;
  int pipe1[2], pipe2[2];
  ssize_t tam = 0, tam_res = 2;

  /* Inicializamos las tuberias */
  if( pipe(pipe1) == -1 || pipe(pipe2) == -1) {
     syslog (LOG_ERR, "Error en pipes ejecutando scripts");
     return ERROR;
  }

  /* Si es python */
  if(strcmp(tipo, "py") == 0){
    arg[0] = "python3";
    arg[1] = root;
    arg[2] = argum;
    arg[3] = NULL;
  }

  /* Si es php */
  else {
    arg[0] = "php";
    arg[1] = "-f";
    arg[2] = root;
    arg[3] = argum;
    arg[4] = NULL;
  }

  /* Creamos un hijo para procesar la información */
  pid = fork();
  if ( pid < 0 ) {
     syslog (LOG_ERR, "Error en fork ejecuta_scripts");
     return ERROR;
  }

  /*El hijo asocia los pipes a stdin y stdout*/
  else if (pid == 0){
    close(pipe1[1]);
    close(pipe2[0]);
    dup2(pipe1[0], STDIN_FILENO);  /*Asociamos stdin a la entrada de lectura de pipe1*/
    close(pipe1[0]);
    dup2(pipe2[1], STDOUT_FILENO);  /*Asociamos stdout a la entrada de escritura de pipe2*/
    close(pipe2[1]);
    if (execvp(arg[0], arg) == -1) {
      syslog (LOG_ERR, "Error 404 Not Found");
      return ERROR_404;
   }
    exit(EXIT_SUCCESS);
  }

  /* Enviamos desde el padre la respuesta */
  close(pipe1[0]);
  close(pipe2[1]);
  if( write(pipe1[1], argum, strlen(argum) + 1) < 1) {
     syslog (LOG_ERR, "Error escribiendo en pipe ejecuta_scripts");
     return ERROR;
  }
  wait(NULL);

  aux = (char*)calloc(2, sizeof(char));
  if(!aux) {
    syslog (LOG_ERR, "Error reservando memoria ejecuta_scripts");
    return ERROR;
  }

  res = (char*)calloc(2, sizeof(char));
  if(!res) {
     syslog (LOG_ERR, "Error reservando memoria ejecuta_scripts");
     free(aux);
     return ERROR;
  }
  memset(res, 0, 2);

  /* Leemos de byte en byte la tuberia */
  while ((tam = read(pipe2[0], aux, 1)) == 1){
    res = (char*)realloc(res, sizeof(char)*(tam + tam_res));
    if(!res) {
       syslog (LOG_ERR, "Error reservando memoria ejecuta_scripts");
       free(aux);
       return ERROR;
    }
    /* Concatenamos aux y res y ponemos aux a 0 */
    strcat(res, aux);
    memset(aux, 0, 2);
    tam_res += tam;
  }
  tam_res += 2;
  res = (char*)realloc(res, sizeof(char)*(tam + tam_res + 2));
  if(!res) {
     syslog (LOG_ERR, "Error reservando memoria ejecuta_scripts");
     free(aux);
     return ERROR;
  }
  strcat(res, "\r\n\r\n");

  buflen = sprintf(buf, "HTTP/1.%d 200 OK\r\n", version);
  send(desc, buf, buflen, 0);
  if(strcmp(tipo, "py") == 0){
    syslog (LOG_INFO, "Ejecutando script python");
    send(desc, res, tam_res, 0);
  }
  /* Si es php mandamos menos tamaño */
  else {
    syslog (LOG_INFO, "Ejecutando script php");
    send(desc, res, tam_res-2, 0);
  }

  free(res);
  free(aux);
  return OK;
}


/*
* Funcion: http_options
* Función que procesa las petciones HTTP options y construye la cabecera
* INPUT :  desc -> descriptor del socket por el que enviamos la cabecera
*          version -> version del HTTP
*          nombre -> nombre del servidor
* OUTPUT : OK/ERROR
*/
int http_options(int desc, int version, char *nombre){
  char buf[4096];
  char *fech;
  size_t buflen = 0;

  /* Mandamos la cabecera */
  buflen = sprintf(buf,"HTTP/1.%d 200 OK\r\n",version);
  send(desc, buf, buflen, 0);

  fech = fecha();

  /* Mandamos los datos con las respuestas permitidas */
  buflen = sprintf(buf, "Date: %s\r\nServer: %s\r\nAllow: OPTIONS, GET, POST\r\nContent-Length: 0\r\nConnection: close\r\nContent-Type: text/plain\r\n\r\n", fech, nombre);
  send(desc, buf, buflen, 0);
  syslog (LOG_INFO, "Enviando respuesta http_options");

  free(fech);
  return OK;
}


/*
* Funcion: dia_semana(int dia)
* INPUT : dia -> numero de la semana del 0 al 6
* OUTPUT : dia de la semana como char
*/
char *dia_semana(int dia){

  switch (dia) {
    case 0: return "Sun";
    case 1: return "Mon";
    case 2: return "Tue";
    case 3: return "Wed";
    case 4: return "Thu";
    case 5: return "Fri";
    case 6: return "Sat";
    default : return NULL;
  }
}

/*
* Funcion: mes(int mes)
* INPUT : mes -> numero del mes del 0 al 11
* OUTPUT : dia de la semana como char
*/
char *mes(int mes){

  switch (mes) {
    case 0: return "Jan";
    case 1: return "Feb";
    case 2: return "Mar";
    case 3: return "Apr";
    case 4: return "May";
    case 5: return "Jun";
    case 6: return "Jul";
    case 7: return "Aug";
    case 8: return "Sep";
    case 9: return "Oct";
    case 10: return "Nov";
    case 11: return "Dec";
    default : return NULL;
  }
}

/*
* Funcion: fecha()
* INPUT : void
* OUTPUT : devuelve el formato de la fecha en GMT
*/
char *fecha(void){

    time_t time_raw_format;
    struct tm * ptr_time;
    char *tiempo = (char*)malloc(sizeof(char)*50);

    time ( &time_raw_format );
    ptr_time = localtime ( &time_raw_format );

    sprintf(tiempo, "%s, %d %s %d %d:%d:%d GMT", dia_semana(ptr_time->tm_wday),
              ptr_time->tm_mday, mes(ptr_time->tm_mon), ptr_time->tm_year+1900,
              ptr_time->tm_hour, ptr_time->tm_min, ptr_time->tm_sec);
    return tiempo;
}

/*
* Funcion: fecha_ultima_mod
* INPUT : root -> directorio donde se almacena el servidor
* OUTPUT : fecha de ultima modificacion del path
*/
char *fecha_ultima_mod(char *root){
  struct stat status;
  struct tm * tm_time;
  char *tiempo = (char*)malloc(sizeof(char)*50);

  stat(root, &status);
  tm_time = localtime(&(status.st_mtim.tv_nsec));

  sprintf(tiempo, "%s, %d %s %d %d:%d:%d GMT", dia_semana(tm_time->tm_wday),
            tm_time->tm_mday, mes(tm_time->tm_mon), tm_time->tm_year+1900,
            tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);

  return tiempo;
}

/*
* Funcion: tamanio_recurso
* INPUT : root -> directorio donde se almacena el servidor
* OUTPUT : devuelve el tamaño del root
*/
int tamanio_recurso(char *root){
  int tamanio;
  FILE* arch;

  arch = fopen(root, "rb");
  fseek(arch, 0, SEEK_END);
  tamanio = ftell(arch);
  fclose(arch);
  return tamanio;
}

/*
* Funcion: content_type
* INPUT : path con la extension a descubrir
* OUTPUT : devuelve la extensión del path recibido
*/
char *content_type(char *path){
  char *token;

  /* Primer token */
  token = strtok(path, ".");

  /* Segundo token */
  token = strtok(NULL, ".");

  /* En caso de pedir algo inexistente que no contenga ese "." devuelve error 405*/
  if(!token) return NULL;

  if (strcmp(token, "html") == 0 || strcmp(token, "htm") == 0) return "text/html; charset=utf-8";
  else if (strcmp(token, "txt") == 0) return "text/plain";
  else if (strcmp(token, "gif") == 0) return "image/gif";
  else if (strcmp(token, "ico") == 0) return "image/vnd.microsoft.icon";
  else if (strcmp(token, "jpeg") == 0 || strcmp(token, "jpg") == 0) return "image/jpeg";
  else if (strcmp(token, "mpeg") == 0 || strcmp(token, "mpg") == 0) return "video/mpeg";
  else if (strcmp(token, "doc") == 0 || strcmp(token, "docx") == 0) return "application/msword";
  else if (strcmp(token, "pdf") == 0) return "application/pdf";
  else if (strcmp(token, "css") == 0) return "text/css";
  else if (strcmp(token, "js") == 0) return "text/javascript";
  else if (token[0] == 'p' && token[1] == 'y') return "py";
  else if (token[0] == 'p' && token[1] == 'h') return "php";
  else return NULL;
}
