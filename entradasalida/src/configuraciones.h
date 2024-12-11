#ifndef ENTRADASALIDA_CONFIGURACIONES_H
#define ENTRADASALIDA_CONFIGURACIONES_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <utils/funcionalidades_basicas.h>
#include <utils/comunicacion/comunicacion.h>
#include <commons/string.h>
#include <utils/estructuras_compartidas/instrucciones_cpu.h>
#include "conexiones/conexiones.h"
#include <readline/readline.h>

#define obtener_tipo_interfaz() config_get_string_value(config, "TIPO_INTERFAZ")
#define obtener_tiempo_unidad_trabajo() config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO")
#define obtener_ip_kernel() config_get_string_value(config, "IP_KERNEL")
#define obtener_puerto_kernel() config_get_string_value(config, "PUERTO_KERNEL")
#define obtener_ip_memoria() config_get_string_value(config, "IP_MEMORIA")
#define obtener_puerto_memoria() config_get_string_value(config, "PUERTO_MEMORIA")
#define obtener_path_base_dialfs() config_get_string_value(config, "PATH_BASE_DIALFS")
#define obtener_block_size() config_get_int_value(config, "BLOCK_SIZE")
#define obtener_block_count() config_get_int_value(config, "BLOCK_COUNT")
#define obtener_retraso_compactacion() config_get_int_value(config, "RETRASO_COMPACTACION")

extern t_config *config;
extern t_log *logger_propio;
extern t_log *logger_obligatorio;
extern char *nombres_de_instrucciones[];

// Logs minimos y obligatorios
void loggear_operacion(uint32_t PID, char *operacion);
void loggear_dialfs_crear_archivo(uint32_t PID, char *nombre);
void loggear_dialfs_eliminar_archivo(uint32_t PID, char *nombre);
void loggear_dialfs_truncar_archivo(uint32_t PID, char *nombre, int tam);
void loggear_dialfs_leer_archivo(uint32_t PID, char *nombre, int tam, int puntero);
void loggear_dialfs_escribir_archivo(uint32_t PID, char *nombre, int tam, int puntero);
void loggear_dialfs_inicio_compactacion(uint32_t PID);
void loggear_dialfs_fin_compactacion(uint32_t PID);

#endif