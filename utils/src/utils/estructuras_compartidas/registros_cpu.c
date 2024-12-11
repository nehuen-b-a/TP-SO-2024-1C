#include "registros_cpu.h"

// REGISTROS DE CPU

t_dictionary *crear_registros_cpu()
{
    t_dictionary *registros_cpu = dictionary_create();
    if (!dictionary_size(registros_cpu))
    {
        dictionary_put(registros_cpu, "PC", memset(malloc(sizeof(uint32_t *)), 0, 4));
        dictionary_put(registros_cpu, "AX", memset(malloc(sizeof(uint8_t *)), 0, 1));
        dictionary_put(registros_cpu, "BX", memset(malloc(sizeof(uint8_t *)), 0, 1));
        dictionary_put(registros_cpu, "CX", memset(malloc(sizeof(uint8_t *)), 0, 1));
        dictionary_put(registros_cpu, "DX", memset(malloc(sizeof(uint8_t *)), 0, 1));
        dictionary_put(registros_cpu, "EAX", memset(malloc(sizeof(uint32_t *)), 0, 4));
        dictionary_put(registros_cpu, "EBX", memset(malloc(sizeof(uint32_t *)), 0, 4));
        dictionary_put(registros_cpu, "ECX", memset(malloc(sizeof(uint32_t *)), 0, 4));
        dictionary_put(registros_cpu, "EDX", memset(malloc(sizeof(uint32_t *)), 0, 4));
        dictionary_put(registros_cpu, "SI", memset(malloc(sizeof(uint32_t *)), 0, 4));
        dictionary_put(registros_cpu, "DI", memset(malloc(sizeof(uint32_t *)), 0, 4));
    }

    return registros_cpu;
}

uint32_t obtener_valor_registro(t_dictionary *registros_cpu, char *nombre_registro)
{
    uint32_t valor;

    if (strlen(nombre_registro) == 3 || !strcmp(nombre_registro, "SI") || !strcmp(nombre_registro, "DI") || !strcmp(nombre_registro, "PC")) // caso registros de 4 bytes
    {
        uint32_t *registro = dictionary_get(registros_cpu, nombre_registro);
        valor = *registro;
    }

    else if (strlen(nombre_registro) == 2) // caso registros de 1 bytes
    {
        uint8_t *registro = dictionary_get(registros_cpu, nombre_registro);
        valor = *registro;
    }

    return valor;
}

void destruir_registros_cpu(t_dictionary *registros_cpu)
{
    dictionary_destroy_and_destroy_elements(registros_cpu, free);
}

t_dictionary *copiar_registros_cpu(t_dictionary *a_copiar) // TODO:Liberar a a_copiar
{
    t_dictionary *copia = dictionary_create();

    dictionary_put(copia, "AX", memcpy(malloc(sizeof(uint8_t)), dictionary_get(a_copiar, "AX"), sizeof(uint8_t)));
    dictionary_put(copia, "EAX", memcpy(malloc(sizeof(uint32_t)), dictionary_get(a_copiar, "EAX"), sizeof(uint32_t)));
    dictionary_put(copia, "BX", memcpy(malloc(sizeof(uint8_t)), dictionary_get(a_copiar, "BX"), sizeof(uint8_t)));
    dictionary_put(copia, "EBX", memcpy(malloc(sizeof(uint32_t)), dictionary_get(a_copiar, "EBX"), sizeof(uint32_t)));
    dictionary_put(copia, "CX", memcpy(malloc(sizeof(uint8_t)), dictionary_get(a_copiar, "CX"), sizeof(uint8_t)));
    dictionary_put(copia, "ECX", memcpy(malloc(sizeof(uint32_t)), dictionary_get(a_copiar, "ECX"), sizeof(uint32_t)));
    dictionary_put(copia, "DX", memcpy(malloc(sizeof(uint8_t)), dictionary_get(a_copiar, "DX"), sizeof(uint8_t)));
    dictionary_put(copia, "EDX", memcpy(malloc(sizeof(uint32_t)), dictionary_get(a_copiar, "EDX"), sizeof(uint32_t)));
    dictionary_put(copia, "PC", memcpy(malloc(sizeof(uint32_t)), dictionary_get(a_copiar, "PC"), sizeof(uint32_t)));
    dictionary_put(copia, "SI", memcpy(malloc(sizeof(uint32_t)), dictionary_get(a_copiar, "SI"), sizeof(uint32_t)));
    dictionary_put(copia, "DI", memcpy(malloc(sizeof(uint32_t)), dictionary_get(a_copiar, "DI"), sizeof(uint32_t)));

    return copia;
}
