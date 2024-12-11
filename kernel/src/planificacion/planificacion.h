#ifndef KERNEL_PLANIFICACION_LARGO_PLAZO_H
#define KERNEL_PLANIFICACION_LARGO_PLAZO_H

#include <pthread.h>
#include <semaphore.h>
#include <commons/temporal.h>
#include "pcb.h"
#include "../manejos/manejo_interfaces.h"
#include "../manejos/manejo_recursos.h"
#include <string.h>
#include <commons/collections/dictionary.h>
#include "../configuraciones.h"

extern t_list *pcbs_en_NEW;
extern t_list *pcbs_en_memoria;
extern t_list *pcbs_en_READY;
extern t_list *pcbs_en_aux_READY;
extern t_pcb *pcb_en_EXEC;
extern t_list *pcbs_en_BLOCKED;
extern t_list *pcbs_en_EXIT;

extern sem_t sem_grado_multiprogramacion;
extern int32_t grado_multiprogramacion_auxiliar;
extern pthread_mutex_t mutex_multiprogramacion_auxiliar;

extern sem_t hay_pcbs_NEW;
extern sem_t hay_pcbs_READY;
extern sem_t planificacion_largo_plazo_liberada;
extern sem_t planificacion_corto_plazo_liberada;
extern sem_t desalojo_liberado;
extern sem_t planificacion_pausada;
extern sem_t transicion_estados_corto_plazo_liberada;

extern pthread_mutex_t mutex_lista_NEW;
extern pthread_mutex_t mutex_cola_READY;
extern pthread_mutex_t mutex_cola_aux_READY;
extern pthread_mutex_t mutex_lista_BLOCKED;
extern pthread_mutex_t mutex_pcb_EXEC;
extern pthread_mutex_t mutex_lista_memoria;
extern pthread_mutex_t mutex_lista_EXIT;
extern pthread_mutex_t mutex_lista_PIDS;

extern int *instancias_recursos;
extern char *estadosProcesos[5];
extern int conexion_kernel_cpu_dispatch;
extern int conexion_kernel_cpu_interrupt;
extern int conexion_kernel_memoria;
extern char *algoritmo;

typedef enum
{
    SUCCESS,
    INVALID_RESOURCE,
    INVALID_INTERFACE,
    FINALIZACION_OUT_OF_MEMORY,
    INTERRUPTED_BY_USER
} motivo_finalizacion;

typedef struct
{
    t_contexto *contexto;
    t_pcb *pcb;
} t_args; // argumentos para el hilo de VRR

// para iniciar toda la planificacion, tanto a largo como a corto plazo
void iniciar_planificacion(void);

// largo plazo
void planificar_a_largo_plazo(void);
void ingresar_pcb_a_NEW(t_pcb *pcb);
t_pcb *obtener_siguiente_pcb_READY(void);
void ingresar_pcb_a_READY(t_pcb *pcb);
void inicializar_listas_planificacion(void);
void destruir_listas_planificacion(void);
void enviar_pcb_a_EXIT(t_pcb *pcb, int motivo);
void remover_pcb_de_listas_globales(t_pcb *pcb);

// manejo de semaforos
void inicializar_semaforos_planificacion(void);
void destruir_semaforos_planificacion(void);

// corto plazo
void planificar_a_corto_plazo_segun_algoritmo(void);
void planificar_a_corto_plazo(t_pcb *(*proximo_a_ejecutar)(void));
t_pcb *proximo_a_ejecutar_segun_FIFO(void);
t_pcb *proximo_a_ejecutar_segun_RR(void);
t_pcb *proximo_a_ejecutar_segun_VRR(void);
void esperar_contexto_y_manejar_desalojo(t_pcb *pcb, pthread_t *hilo_quantum);
void encolar_pcb_ready_segun_algoritmo(t_pcb *pcb);
t_contexto *obtener_contexto_de_paquete_desalojo(t_list *paquete);
t_list *obtener_parametros_de_paquete_desalojo(t_list *paquete);

// relacionado con la CPU
void procesar_pcb_segun_algoritmo(t_pcb *pcb, pthread_t *hilo_quantum);
void ejecutar_segun_FIFO(t_contexto *contexto);
void ejecutar_segun_RR(t_contexto *contexto);
void ejecutar_segun_VRR(t_args *args);
void enviar_interrupcion(char *motivo);

#endif
