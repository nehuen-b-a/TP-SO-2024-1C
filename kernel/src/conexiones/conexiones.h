#ifndef KERNEL_CONEXIONES_H
#define KERNEL_CONEXIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../configuraciones.h"
#include "../planificacion/pcb.h"
#include <utils/comunicacion/comunicacion.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include "manejos/manejo_interfaces.h"

extern int conexion_kernel_cpu_dispatch;
extern int conexion_kernel_cpu_interrupt;
extern int conexion_kernel_memoria;
extern int servidor_kernel_fd;

void servidor(void);
void conexion_dispatch_con_CPU(void);
void conexion_interrupt_con_CPU(void);
void conexion_memoria(void);
void iniciar_conexiones(void);

#endif
