#ifndef CPU_CONFIGURACIONES_H
#define CPU_CONFIGURACIONES_H

#include <stdlib.h>
#include <stdio.h>
#include <utils/funcionalidades_basicas.h>
#include <utils/comunicacion/comunicacion.h>

#define obtener_ip_memoria() config_get_string_value(config, "IP_MEMORIA")
#define obtener_puerto_memoria() config_get_string_value(config, "PUERTO_MEMORIA")
#define obtener_puerto_escucha_dispatch() config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH")
#define obtener_puerto_escucha_interrupt() config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT")
#define obtener_cantidad_entradas_tlb() config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB")
#define obtener_algoritmo_tlb() config_get_string_value(config, "ALGORITMO_TLB")

extern t_config *config;
extern t_log *logger_obligatorio;
extern t_log *logger_propio;

// Logs minimos y obligatorios
void loggear_fetch_instrucccion(uint32_t PID, uint32_t PC);
// void loggear_instrucion_ejecutada(uint32_t PID, t_instruccion instruccion);
void loggear_tlb_hit(uint32_t PID, int numero_pagina);
void loggear_tlb_miss(uint32_t PID, int numero_pagina);
void loggear_obtener_marco(uint32_t PID, int numero_pagina, int numero_marco);
void loggear_lectura_memoria_num(uint32_t PID, uint32_t direccion_fisica, uint32_t valor);
void loggear_lectura_memoria_char(uint32_t PID, uint32_t direccion_fisica, char *valor);
void loggear_escritura_memoria_num(uint32_t PID, uint32_t direccion_fisica, uint32_t valor);
void loggear_escritura_memoria_char(uint32_t PID, uint32_t direccion_fisica, char *valor);

#endif