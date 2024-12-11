#ifndef KERNEL_CONSOLA_H_
#define KERNEL_CONSOLA_H_

#include <readline/readline.h>
#include <stdlib.h>
#include "../planificacion/pcb.h"
#include "../planificacion/planificacion.h"
#include <unistd.h>
#include <readline/history.h>

extern sem_t planificacion_pausada;
extern sem_t sem_grado_multiprogramacion;

extern int conexion_kernel_memoria;

typedef struct t_comando
{
    char *nombre;
    void (*funcion_de_comando)();
} t_comando;

void consola_interactiva(void);

// comandos
void ejecutar_script(char *path);
void iniciar_proceso(char *path);
void finalizar_proceso(char *PID);
void detener_planificacion(void);
void reanudar_planificacion(void);
void cambiar_grado_multiprogramacion(char *valor);
void listar_procesos_por_cada_estado(void);
void listar_recursos(void);

// comandos utiles pero que no estan en la consigna
void listar_recursos(void);
void multiprogramacion_actual(void);

// funciones utiles
void listar_procesos_por_estado(char *estado, t_list *lista);
void buscar_y_ejecutar_comando(char *token);
void cambiar_valor_de_semaforo(sem_t *sem, int valor_resultante);

#endif