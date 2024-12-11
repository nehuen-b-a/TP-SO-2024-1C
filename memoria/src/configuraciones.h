#ifndef MEMORIA_CONFIGURACIONES_H
#define MEMORIA_CONFIGURACIONES_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/funcionalidades_basicas.h>
#include <utils/comunicacion/comunicacion.h>

#define obtener_puerto_escucha() config_get_string_value(config, "PUERTO_ESCUCHA")
#define obtener_tam_memoria() config_get_int_value(config, "TAM_MEMORIA")
#define obtener_tam_pagina() config_get_int_value(config, "TAM_PAGINA")
#define obtener_path_instrucciones() config_get_string_value(config, "PATH_INSTRUCCIONES")
#define obtener_retardo_respuesta() config_get_int_value(config, "RETARDO_RESPUESTA")

extern t_config *config;
extern t_log *logger_propio;
extern t_log *logger_obligatorio;

// Logs minimos y obligatorios
void loggear_creacion_destruccion_tabla_de_paginas(uint32_t PID, int cantidad_paginas);
void loggear_acceso_tablas_de_paginas(uint32_t PID, int pagina, int marco);
void loggear_ampliacion_de_proceso(uint32_t PID, int tamanio_actual, int tamanio_a_ampliar);
void loggear_reduccion_de_proceso(uint32_t PID, int tamanio_actual, int tamanio_a_reducir);
void loggear_lectura_espacio_de_usuario(uint32_t PID, uint32_t direccion_fisica, int tamanio);
void loggear_escritura_espacio_de_usuario(uint32_t PID, uint32_t direccion_fisica, int tamanio);

#endif