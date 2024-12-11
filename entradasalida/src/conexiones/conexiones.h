#ifndef ENTRADASALIDA_CONEXIONES_H
#define ENTRADASALIDA_CONEXIONES_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <utils/funcionalidades_basicas.h>
#include <utils/comunicacion/comunicacion.h>
#include <commons/string.h>
#include <utils/estructuras_compartidas/instrucciones_cpu.h>
#include "gestor_interfaces.h"
#include "configuraciones.h"

extern int conexion_memoria;
extern int conexion_kernel;
extern char *nombre;

void conexion_con_kernel(void);
void conexion_con_memoria(void);
void recibir_peticiones_del_kernel(void);

#endif