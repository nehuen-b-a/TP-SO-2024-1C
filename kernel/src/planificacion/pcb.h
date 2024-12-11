#ifndef KERNEL_PCB_H_
#define KERNEL_PCB_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "../configuraciones.h"
#include <utils/estructuras_compartidas/contexto_ejecucion.h>
#include <utils/estructuras_compartidas/registros_cpu.h>

typedef enum estado
{
    NEW,
    READY,
    EXEC,
    BLOCKED,
    EXIT
} estado;

typedef struct
{
    uint32_t PID;
    int64_t quantum;
    t_dictionary *registros_cpu;
    estado estado;
    t_list *recursos_asignados;
    bool desencolado_de_aux_ready;
} t_pcb;

extern int32_t procesos_creados;
extern char *lista_PIDS;

t_pcb *crear_pcb();
void destruir_pcb(t_pcb *pcb);

t_pcb *desencolar_pcb(t_list *pcbs);
void encolar_pcb(t_list *pcbs, t_pcb *pcb);
void mostrar_PIDS(t_list *pcbs);
void agregar_PID_a_lista_PIDS(void *pcb);
t_pcb *buscar_pcb_por_PID(t_list *lista_pcbs, uint32_t PID);

// Actualizar PCB a partir de contexto de ejecución recibido de CPU
void actualizar_pcb(t_pcb *pcb, t_contexto *contexto, int64_t ms_en_ejecucion);

// Crear contexto de ejecucion a partir de PCB, para enviar a CPU
t_contexto *crear_contexto(t_pcb *pcb);

#endif