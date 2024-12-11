#ifndef MEMORIA_TABLAS_H
#define MEMORIA_TABLAS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <utils/funcionalidades_basicas.h>
#include <commons/collections/list.h>
#include <configuraciones.h>
#include "marcos_libres.h"

extern t_dictionary *indice_tablas;

// INDICE DE TABLA DE PAGINAS ---------------------
t_dictionary *crear_indice_de_tablas();
void agregar_proceso_al_indice(uint32_t PID);
void quitar_proceso_del_indice(uint32_t PID);
t_list *obtener_tp_de_proceso(uint32_t PID); // TODO: facu fijate si te gusta el nombre ahora. Trabajo Práctico?
void destruir_tabla_de_paginas(void *tp);
void destruir_indice_de_tablas();

// TABLA DE PÁGINAS -------------------------------
// Crea una nueva página y le asigna un marco libre
void agregar_pagina(t_list *tp);
// Elimina la última página y libera su marco
void quitar_ultima_pagina(t_list *tp);
int obtener_marco(t_list *tp, int pagina);
int buscar_marco(uint32_t PID, int pagina);

void liberar_marcos_proceso(uint32_t PID);

#endif