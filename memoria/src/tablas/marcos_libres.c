#include "marcos_libres.h"

int cant_marcos_memoria;

t_bitarray *crear_marcos_libres()
{
    cant_marcos_memoria = (obtener_tam_memoria() + obtener_tam_pagina() - 1) / obtener_tam_pagina();
    size_t bytes_necesarios = (cant_marcos_memoria + 8 - 1) / 8;
    void *espacio_bitarray = malloc(bytes_necesarios);
    t_bitarray *marcos = bitarray_create_with_mode(espacio_bitarray, bytes_necesarios, LSB_FIRST);
    for (int i = 0; i < cant_marcos_memoria; i++)
        bitarray_set_bit(marcos, i);

    return marcos;
}

void marcar_como_libre(int marco)
{
    bitarray_set_bit(marcos_libres, marco);
}

void marcar_como_ocupado(int marco)
{
    bitarray_clean_bit(marcos_libres, marco);
}

int cantidad_marcos_libres()
{
    int cant_marcos_libres = 0;

    for (int i = 0; i < cant_marcos_memoria; i++)
    {
        if (bitarray_test_bit(marcos_libres, i))
        {
            cant_marcos_libres++;
        }
    }

    return cant_marcos_libres;
}

int obtener_marco_libre()
{
    int marco = -1;

    for (int i = 0; i < cant_marcos_memoria && marco < 0; i++)
    {
        if (bitarray_test_bit(marcos_libres, i))
        {
            marco = i;
        }
    }

    return marco;
}