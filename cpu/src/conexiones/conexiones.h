#ifndef CPU_CONEXIONES_H
#define CPU_CONEXIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include "../configuraciones.h"
#include <utils/comunicacion/comunicacion.h>
//    #include <commons/collections/list.h>
//    #include <commons/collections/dictionary.h>

extern int conexion_cpu_memoria;
extern int conexion_cpu_kernel_dispatch;
extern int conexion_cpu_kernel_interrupt;
extern char *motivo_interrupcion;
extern bool hay_interrupcion;
extern int tamanio_pagina;

#endif
