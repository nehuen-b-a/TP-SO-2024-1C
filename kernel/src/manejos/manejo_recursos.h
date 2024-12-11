#ifndef KERNEL_RECURSOS_E_INTERFACES_H
#define KERNEL_RECURSOS_E_INTERFACES_H

#include <commons/collections/list.h>
#include <commons/string.h>
#include <stdlib.h>
#include "../configuraciones.h"
#include "../planificacion/pcb.h"
#include "../planificacion/planificacion.h"

extern int *instancias_recursos;
extern t_list *colas_de_recursos;
extern char **nombres_recursos;
extern pthread_mutex_t mutex_colas_de_recursos;
extern pthread_mutex_t mutex_instancias_recursos;
extern pthread_mutex_t *mutex_cola_recurso;

// funciones principales
void crear_colas_de_bloqueo(void);
void desbloquear_pcb_si_corresponde(int pos_recurso);
bool wait_recurso(char *recurso, t_pcb *pcb);
bool signal_recurso(char *recurso, t_pcb *pcb);
void liberar_recursos(t_pcb *pcb);

// funciones complementarias
void eliminar_pcb_de_colas_de_recursos(t_pcb *pcb);
bool existe_recurso(char *recurso);
int posicion_recurso(char *recurso);
int cantidad_recursos(void);
void destruir_lista_string(char **lista_string);
void destruir_colas_de_recursos(void);
void crear_mutex_por_cola_de_recurso(void);
void destruir_mutex_por_colas_de_recurso(void);

#endif