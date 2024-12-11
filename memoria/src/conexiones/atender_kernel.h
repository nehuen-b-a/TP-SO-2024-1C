#ifndef MEMORIA_ATENDER_KERNEL_H
#define MEMORIA_ATENDER_KERNEL_H

#include <utils/comunicacion/serializacion.h>
#include "configuraciones.h"
#include "gestion_memoria.h"

void atender_kernel(int *socket_cliente);

bool crear_estructuras_administrativas(uint32_t PID, char *path);
void recibir_creacion_proceso(uint32_t *PID, char **ptr_path);
void atender_crear_proceso(void);

void liberar_estructuras_administrativas(uint32_t PID);
void recibir_pid(uint32_t *PID);
void atender_finalizar_proceso(void);

#endif