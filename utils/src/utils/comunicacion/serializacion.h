#ifndef SERIALIZACION_H
#define SERIALIZACION_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h> // library for ssize_t data type
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <string.h>
#include <assert.h> // testing
#include "../funcionalidades_basicas.h"

// ------------------------ CLIENTE ------------------------ //

typedef struct
{
    int size;
    void *stream;
} t_buffer;

typedef enum
{
    MENSAJE,
    PAQUETE,
    CONTEXTO_EJECUCION,
    OPERACION_INVALIDA,
    OK,
    INSTRUCCION,
    INTERRUPCION,
    SOLICITUD_MARCO,
    MARCO,
    SOLICITUD_INSTRUCCION,
    ACCESO_ESPACIO_USUARIO_LECTURA,
    ACCESO_ESPACIO_USUARIO_ESCRITURA,
    ACCESO_TABLA_PAGINAS,
    AJUSTAR_TAMANIO_PROCESO,
    AMPLIAR_PROCESO,
    REDUCIR_PROCESO,
    CREAR_PROCESO_KERNEL,
    FINALIZAR_PROCESO_KERNEL,
    RESIZE_PROCESO,
    OUT_OF_MEMORY,
    CONEXION_CPU,
    CONEXION_KERNEL,
    CONEXION_MEMORIA,
    CONEXION_IO,
    CONEXION_INTERFAZ_KERNEL,
    ERROR
} op_code;

typedef struct
{
    op_code codigo_operacion;
    t_buffer *buffer;

} t_paquete;

void enviar_mensaje(char *mensaje, int socket_cliente);
t_paquete *crear_paquete(int codigo_operacion);
void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio);
void enviar_paquete(t_paquete *paquete, int socket_cliente);
int enviar_cod_op_con_retorno(op_code codigo_de_operacion, int socket);
void eliminar_paquete(t_paquete *paquete);
void enviar_cod_op(op_code codigo_de_operacion, int socket);

/* Funciones Privadas (?) */

void crear_buffer(t_paquete *paquete);
void *serializar_paquete(t_paquete *paquete, int bytes);
void agregar_a_paquete_uint32(t_paquete *paquete, uint32_t data);
void agregar_a_paquete_uint8(t_paquete *paquete, uint8_t data);
void agregar_a_paquete_string(t_paquete *paquete, char *data);
int enviar_paquete_interfaz(t_paquete *paquete, int conexion);
// ------------------------ SERVIDOR ------------------------ //

int recibir_operacion(int socketCliente);
void *recibir_buffer(int socket, int *size);
t_list *recibir_paquete(int socket_cliente);
char *recibir_string(int socket_cliente);

void agregar_parametros_a_paquete(t_paquete *paquete, t_list *parametros);

#endif /* SERIALIZACION_H */
