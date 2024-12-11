#ifndef KERNEL_MANEJO_INTERFACES_H
#define KERNEL_MANEJO_INTERFACES_H

#include <commons/collections/list.h>
#include <commons/string.h>
#include <stdlib.h>
#include "../configuraciones.h"
#include <utils/comunicacion/comunicacion.h>
#include "../planificacion/pcb.h"
#include "../planificacion/planificacion.h"
#include <fcntl.h>

extern int fd_servidor;
extern t_list *interfaces;
extern int conexion_kernel_cpu_dispatch;
extern int servidor_kernel_fd;
extern pthread_mutex_t mutex_interfaces;

typedef struct
{
    int fd;
    char *nombre;
    char *tipo;
    t_list *procesos_bloqueados;
    pthread_mutex_t cola_bloqueados;
    sem_t procesos_en_cola;
    pthread_t hilo_interfaz;
    bool desconectada;
} t_io;

typedef struct
{
    t_pcb *pcb;
    t_list *parametros;
} t_proceso_bloqueado;

typedef enum
{
    IO_GEN_SLEEP = 10,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ
} t_id_io;

// funciones para el manejo de interfaces
void ejecutar_espera_interfaces(void);
void inicializar_interfaces();
bool puede_realizar_operacion(t_io *io, char *operacion);
void agregar_a_lista_io_global(char *nombre, char *tipo, int fd);
void manejador_interfaz(t_pcb *pcb, t_list *parametros);
void atender_interfaz(void *interfaz);

// funciones de manejo de t_proceso_bloqueado
void eliminar_proceso_bloqueado(t_proceso_bloqueado *proceso);
t_proceso_bloqueado *crear_proceso_bloqueado(t_pcb *pcb, t_list *parametros);

// funciones de manejo de t_io
t_io *crear_interfaz(char *nombre, char *tipo, int fd);
t_io *buscar_interfaz(char *nombre_io);
void liberar_interfaz(t_io *io);
void liberar_procesos_io(t_list *procesos_io);
bool socket_desconectado(int socket);

#endif