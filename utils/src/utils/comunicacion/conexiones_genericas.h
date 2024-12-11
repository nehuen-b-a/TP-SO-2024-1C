#ifndef UTILS_CONEXIONES_GENERICAS_H_
#define UTILS_CONEXIONES_GENERICAS_H_

#include <errno.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define obtener_configuracion(clave) config_get_string_value(config, clave)

extern t_config *config;
extern t_log *logger_propio;

typedef struct
{
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    socklen_t ai_addrlen;
    struct sockaddr *ai_addr;
    char *ai_canonname;
    struct addrinfo *ai_next;
} addrinfo;

void terminar_programa(int conexion, t_log *logger, t_config *config);

//----SERVIDOR----
int iniciar_servidor(t_log *logger, char *puerto);
int esperar_cliente(t_log *logger, int server_socket);
int handshake_servidor(t_log *logger, int conexion, int32_t handshake);
void esperar_a(char *tipo, int *socket, int server);

//----CLIENTE----
int crear_conexion(t_log *logger, char *ip, char *puerto);
int handshake_cliente(t_log *logger, int conexion, int32_t handshake);
int conectar_a(char *servidor, t_log *logger, int tiempo_de_reintento_seg);

#endif