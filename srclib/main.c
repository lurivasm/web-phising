/**
 * main.c
 * Main que ejecuta el servidor de la práctica 1 de Redes de Comunicación
 *
 * Lucía Rivas Molina <lucia.rivasmolina@estudiante.uam.es>
 * Daniel Santo-Tomás López <daniel.santo-tomas@estudiante.uam.es>
 */
 #define _GNU_SOURCE
 #include <signal.h>

#include "demonio.h"
#include "socket.h"
#include "picohttpparser.h"
#include "confuse.h"
#include "thpool.h"

/**
 * Variables globales del server.conf para liberar la memoria al pulsar Ctrl-c
 */
int sockt;
static char *server_root = NULL;
static char *server_signature = NULL;
static long int listen_port = 0;
static long int max_clients = 0;
threadpool pool;

/**
 * captura()
 * Funcion que captura la señal SIGINT para liberar la memoria antes de acabar
 */
void captura();

/**
 * handler
 * Añade como trabajo el asociar un socket a un thread del pool
 * INPUT : arg -> puntero a void que contiene el descriptor de un socket
 */
void handler(void *arg);

/**
 * Main que configura y lanza el servidor y va aceptando conexiones
 */
int main(void){

   int desc;
   cfg_t *cfg;
   sigset_t set;

   /* Estructura con los campos del server.conf */
   cfg_opt_t opts[] = {
        CFG_SIMPLE_STR("server_root", &server_root),
        CFG_SIMPLE_STR("server_signature", &server_signature),
        CFG_SIMPLE_INT("listen_port", &listen_port),
        CFG_SIMPLE_INT("max_clients", &max_clients),
        CFG_END()
    };

   /* Parseamos la informacion */
   cfg = cfg_init(opts, 0);
   if (cfg_parse(cfg, "server.conf") == CFG_PARSE_ERROR){
      syslog(LOG_ERR, "Error parseando el servidor");
      return -1;
   }
   cfg_free(cfg);


   /* Iniciamos el servidor */
   syslog(LOG_INFO, "Iniciando el servidor");
   sockt = iniciar_servidor(listen_port, max_clients);
   if(sockt < 0){
      syslog(LOG_ERR, "Error inciando servidor");
      free(server_root);
      free(server_signature);
      return ERROR;
   }

   /* Iniciamos un pool de threads */
   pool = thpool_init(max_clients);

   /* Creamos una máscara de señales */
   sigemptyset(&set);
   /* Añadimos la señal SIGINT */
   sigaddset(&set, SIGINT);
   /* Bloqueamos la señal SIGINT para los hilos de modo que solo la capture el principal */
   pthread_sigmask(SIG_UNBLOCK, &set, NULL);

   /* Asociamos la funcion captura a la señal SIGINT */
   if(signal(SIGINT, captura) == SIG_ERR){
      syslog(LOG_ERR, "Error asociando un manejador a la señal SIGINT");
      free(server_root);
      free(server_signature);
      return ERROR;
   }

   /* Aceptamos las conexiones hasta pulsar ctrl-c */
   syslog(LOG_INFO, "Comenzamos a aceptar conexiones");
   while(1) {
      /* En caso de devolver un descriptor no valido cerramos todo */
      desc = aceptar_conexion(sockt);
      if (desc == ERROR){
         syslog(LOG_ERR, "Descriptor de socket no válido");
         cerrar_socket(sockt);
         thpool_destroy(pool);
         free(server_root);
         free(server_signature);
         exit(EXIT_SUCCESS);
      }
      /* Si encuentra conexion se la añade como trabajo al pool */
      thpool_add_work(pool, (void (*)(void *))handler, (void *) (intptr_t) desc);
   }

   /* Nunca deberia llegar aqui */
   thpool_destroy(pool);
   return OK;
}


/**
 * captura()
 * Funcion que captura la señal SIGINT para liberar la memoria antes de acabar
 */
void captura(){
   syslog(LOG_INFO, "Cerrando servidor... Por favor, espere y no pulse más teclas");
   cerrar_socket(sockt);
   thpool_destroy(pool);
   free(server_root);
   free(server_signature);
   exit(EXIT_SUCCESS);
}

/**
 * handler
 * Añade como trabajo el asociar un socket a un thread del pool
 * INPUT : arg -> puntero a void que contiene el descriptor de un socket
 */
void handler(void *arg){
   syslog(LOG_INFO, "Asociando socket a un thread");
   asociar_socket((int) (intptr_t) arg, server_root, server_signature);
}
