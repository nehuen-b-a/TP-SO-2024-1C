#ifndef UTILS_REGISTROS_CPU_H_
#define UTILS_REGISTROS_CPU_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <utils/comunicacion/comunicacion.h>

// REGISTROS DE CPU,  CPU-KERNEL
t_dictionary *crear_registros_cpu();
uint32_t obtener_valor_registro(t_dictionary *registros_cpu, char *nombre_registro);
void destruir_registros_cpu(t_dictionary *registros_cpu);

// Copia diccionario de registros de CPU de un dict a otro
t_dictionary *copiar_registros_cpu(t_dictionary *a_copiar);

#endif