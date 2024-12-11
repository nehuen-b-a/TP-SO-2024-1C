#ifndef CPU_ATENDER_CONEXIONES_H
#define CPU_ATENDER_CONEXIONES_H

#include "conexiones.h"
#include <utils/estructuras_compartidas/contexto_ejecucion.h>
#include "interface_cpu.h"
#include "ciclo_de_instruccion.h" //Compartir hay_interrupcion y motivo de instrucccion

void iniciar_servidor_dispatch(void);
void iniciar_servidor_interrupt(void);
void iniciar_conexion_con_memoria(void);

// Inicia conexiones kernel y memoria. Crea hilos para cada conexion con kernel.
void iniciar_conexiones();

#endif