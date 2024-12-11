#include "mmu.h"

int tamanio_pagina = 0; // TODO: temporal hasta que se consiga este dato desde la memoria

uint32_t calcular_direccion_fisica(t_TLB *tlb, uint32_t PID, uint32_t direccion_logica)
{
    int pagina = floor(direccion_logica / tamanio_pagina);
    int desplazamiento = direccion_logica - pagina * tamanio_pagina;
    int marco = buscar_marco(tlb, PID, pagina);
    return (tamanio_pagina * marco) + desplazamiento;
}

int buscar_marco(t_TLB *tlb, uint32_t PID, int pagina)
{
    int marco = buscar_en_TLB(tlb, PID, pagina);

    if (marco == -1) // TLB MISS
    {
        loggear_tlb_miss(PID, pagina);
        solicitar_marco_memoria(PID, pagina);
        marco = recibir_marco_memoria();
        agregar_pagina_TLB(tlb, PID, pagina, marco);
    }
    else
    { // TLB hit
        loggear_tlb_hit(PID, pagina);
    }

    loggear_obtener_marco(PID, pagina, marco);

    return marco;
}

// TLB

t_TLB *crear_TLB()
{
    int max_entradas = obtener_cantidad_entradas_tlb();

    t_TLB *tlb = malloc(sizeof(t_TLB));
    tlb->entradas = malloc(sizeof(t_registro_TLB) * max_entradas);
    tlb->max_entradas = max_entradas;
    tlb->entradas_utilizadas = 0;
    tlb->tiempo_actual = 0;
    return tlb;
}

void destruir_TLB(t_TLB *tlb)
{
    free(tlb->entradas);
    free(tlb);
}

// retorna marco correspondiente
int buscar_en_TLB(t_TLB *tlb, uint32_t PID, int pagina)
{
    for (int i = 0; i < tlb->entradas_utilizadas; i++)
    {
        if (tlb->entradas[i].PID == PID && tlb->entradas[i].pagina == pagina)
        {
            if (!strcmp(obtener_algoritmo_tlb(), "LRU")) // LRU
            {
                tlb->entradas[i].tiempo_utilizo = tlb->tiempo_actual++;
            }
            return tlb->entradas[i].marco;
        }
    }
    return -1; // Fallo de TLB, buscar el Memoria
}

void agregar_pagina_TLB(t_TLB *tlb, uint32_t PID, int pagina, int marco)
{
    if (tlb->max_entradas)
    {
        if (tlb->entradas_utilizadas < tlb->max_entradas)
        {
            int tiempo_actual = tlb->tiempo_actual++;
            t_registro_TLB nueva_entrada = {PID, pagina, marco, tiempo_actual, tiempo_actual};
            tlb->entradas[tlb->entradas_utilizadas++] = nueva_entrada;
        }
        else
        {
            reemplazar_pagina_TLB(tlb, PID, pagina, marco);
        }
    }
}

void reemplazar_pagina_TLB(t_TLB *tlb, uint32_t PID, int pagina, int marco)
{
    int indice = obtener_indice_para_reemplazo(tlb);
    int tiempo_actual = tlb->tiempo_actual++;
    t_registro_TLB nueva_entrada = {PID, pagina, marco, tiempo_actual, tiempo_actual};
    tlb->entradas[indice] = nueva_entrada;
}

int obtener_indice_para_reemplazo(t_TLB *tlb)
{
    if (!strcmp(obtener_algoritmo_tlb(), "FIFO")) // FIFO
    {
        int indice_fifo = 0;
        for (int i = 1; i < tlb->entradas_utilizadas; i++)
        {
            if (tlb->entradas[i].tiempo_nacimiento < tlb->entradas[indice_fifo].tiempo_nacimiento)
            {
                indice_fifo = i;
            }
        }
        return indice_fifo;
    }
    else if (!strcmp(obtener_algoritmo_tlb(), "LRU")) // LRU
    {
        // Encuentra la entrada con el menor valor de tiempo
        int indice_lru = 0;
        for (int i = 1; i < tlb->entradas_utilizadas; i++)
        {
            if (tlb->entradas[i].tiempo_utilizo < tlb->entradas[indice_lru].tiempo_utilizo)
            {
                indice_lru = i;
            }
        }
        return indice_lru;
    }
    return 0;
}
