#ifndef MEMORIA_ATENDER_CPU_H
#define MEMORIA_ATENDER_CPU_H

#include <utils/comunicacion/serializacion.h>
#include "configuraciones.h"
#include "gestion_memoria.h"

void atender_cpu(int *socket_cliente);

void enviar_tamanio_de_pagina();

void enviar_instruccion_a_cpu(char *instruccion);
char *obtener_instruccion_de_indice(uint32_t PID, uint32_t PC);
void recibir_solicitud_instruccion(uint32_t *PID, uint32_t *PC);
void atender_solicitud_instruccion(void);

void enviar_marco(int marco);
void recibir_solicitud_marco(uint32_t *PID, int *pagina);
void atender_solicitud_marco();

void recibir_solicitud_resize(uint32_t *PID, uint32_t *tamanio_proceso);
void atender_resize();
op_code resize(uint32_t PID, uint32_t tamanio_proceso);

#endif