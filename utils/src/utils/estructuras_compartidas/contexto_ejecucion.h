#ifndef CONTEXTO_EJECUCION_H
#define CONTEXTO_EJECUCION_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <utils/comunicacion/comunicacion.h>
#include <utils/estructuras_compartidas/registros_cpu.h>

typedef enum
{
    DESALOJO_IO,
    DESALOJO_WAIT,
    DESALOJO_SIGNAL,
    DESALOJO_EXIT_SUCCESS,
    DESALOJO_EXIT_INTERRUPTED,
    DESALOJO_FIN_QUANTUM,
    DESALOJO_OUT_OF_MEMORY
} motivo_desalojo;

typedef struct
{
    uint32_t PID;
    t_dictionary *registros_cpu;
} t_contexto;

// Se inicia contexto de ejecucion
t_contexto *iniciar_contexto(void);

// Destruir contexto de ejecución
void destruir_contexto(t_contexto *contexto);

// Agregado de valores a paquete
void agregar_a_paquete_registros_cpu(t_paquete *paquete, t_dictionary *registros_cpu);

// Agrego PID, registros
void agregar_a_paquete_contexto(t_paquete *paquete, t_contexto *contexto);

// Serializa y envía contexto a través del socket.
void enviar_contexto(int socket, t_contexto *contexto);

// ---------- FUNCIONES PARA MOTIVOS DE DESALOJO ---------- //

motivo_desalojo string_interrupcion_to_enum_motivo(char *interrupcion);

// ---------- FUNCIONES PARA RECIBO DE CONTEXTO ---------- //

t_contexto *recibir_contexto(int socket);

#endif
