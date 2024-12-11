#ifndef CPU_MMU_H
#define CPU_MMU_H

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <utils/funcionalidades_basicas.h>
#include <utils/comunicacion/comunicacion.h>
#include "../configuraciones.h"
#include "interface_cpu.h"

typedef struct
{
    uint32_t PID;
    int pagina;
    int marco;
    int tiempo_nacimiento; // FIFO
    int tiempo_utilizo;    // LRU
} t_registro_TLB;

typedef struct
{
    t_registro_TLB *entradas;
    int entradas_utilizadas;
    int max_entradas;
    int tiempo_actual;
} t_TLB;

t_TLB *crear_TLB();
void destruir_TLB(t_TLB *tlb);
int buscar_marco(t_TLB *tlb, uint32_t PID, int pagina);
// Devuelve el marco de la página de un proceso o -1 si no la encuentra
int buscar_en_TLB(t_TLB *tlb, uint32_t PID, int pagina);
void agregar_pagina_TLB(t_TLB *tlb, uint32_t PID, int pagina, int marco);
void reemplazar_pagina_TLB(t_TLB *tlb, uint32_t PID, int pagina, int marco);
int obtener_indice_para_reemplazo(t_TLB *tlb);

uint32_t calcular_direccion_fisica(t_TLB *tlb, uint32_t PID, uint32_t direccion_logica);

#endif