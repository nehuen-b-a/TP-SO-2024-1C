#ifndef MEMORIA_MARCOS_H
#define MEMORIA_MARCOS_H

#include <commons/bitarray.h>
#include "configuraciones.h"

extern t_bitarray *marcos_libres;

t_bitarray *crear_marcos_libres();
void marcar_como_libre(int marco);
void marcar_como_ocupado(int marco);
int cantidad_marcos_libres();
int obtener_marco_libre();

#endif