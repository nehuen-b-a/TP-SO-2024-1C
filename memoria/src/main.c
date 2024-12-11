
#include "configuraciones.h"
#include <utils/comunicacion/comunicacion.h>
#include <utils/estructuras_compartidas/contexto_ejecucion.h>
#include "conexiones/conexiones.h"

t_log *logger_obligatorio;
t_log *logger_propio;
t_config *config;

void test_resize();

int main(int argc, char *argv[])
{
    logger_obligatorio = crear_logger("memoria_obligatorio");
    logger_propio = crear_logger("memoria_propio");
    log_info(logger_propio, "Iniciando Memoria ...");

    config = iniciar_config(logger_propio, "memoria.config");

    inicializar_memoria();
    iniciar_conexiones();
    // test_resize();
    // testeaba cosas de bitarray
    // int cantidad_marcos = (obtener_tam_memoria() + obtener_tam_pagina() - 1) / obtener_tam_pagina();
    // size_t bytes_necesarios = (cantidad_marcos + 8 - 1) / 8;
    // void *espacio_bitarray = malloc(bytes_necesarios);
    // memset(espacio_bitarray, 1, bytes_necesarios);
    // t_bitarray *bitarray = bitarray_create_with_mode(espacio_bitarray, bytes_necesarios, LSB_FIRST);
    // log_info(logger_propio, "test:%d", bitarray_test_bit(bitarray, 0));
    // bitarray_set_bit(bitarray, 0);
    // log_info(logger_propio, "test:%d", bitarray_test_bit(bitarray, 0));
    // bitarray_clean_bit(bitarray, 1);
    // log_info(logger_propio, "test:%d", bitarray_test_bit(bitarray, 0));

    // terminar_programa(server_fd, logger_propio, config);
    //  free(sockets);
    // log_destroy(logger_obligatorio);

    return 0;
}

// el test se hizo con tam pagina = 16 y tam memoria = 200
void test_resize()
{
    /*inicializar_memoria();
    uint32_t PID1 = 3, PID2 = 4, PID3 = 5;
    agregar_proceso_al_indice(PID1);
    agregar_proceso_al_indice(PID2);
    agregar_proceso_al_indice(PID3);

    agregar_pagina(obtener_tp_de_proceso(PID1));
    agregar_pagina(obtener_tp_de_proceso(PID1));
    for (int i = 0; i < 13; i++)
    {
        log_info(logger_propio, "test bit %d: %d", i, bitarray_test_bit(marcos_libres, i));
    }
    agregar_pagina(obtener_tp_de_proceso(PID2));
    agregar_pagina(obtener_tp_de_proceso(PID2));
    agregar_pagina(obtener_tp_de_proceso(PID3));
    agregar_pagina(obtener_tp_de_proceso(PID3));

    // caso reducción
    log_info(logger_propio, "Páginas antes de reducción (deben ser 2): %d", list_size(obtener_tp_de_proceso(PID1)));
    log_info(logger_propio, "Resultado de reducción: %d", resize(PID1, 14));
    log_info(logger_propio, "Páginas luego de reducción (debe ser 1): %d", list_size(obtener_tp_de_proceso(PID1)));

    // caso ampliación
    log_info(logger_propio, "Páginas antes de ampliación (deben ser 2): %d", list_size(obtener_tp_de_proceso(PID2)));
    log_info(logger_propio, "Resultado de ampliación: %d", resize(PID2, 60));
    log_info(logger_propio, "Páginas luego de ampliación (debe ser 4): %d", list_size(obtener_tp_de_proceso(PID2)));

    // caso no hay cambios
    log_info(logger_propio, "Páginas antes de no hacer nada (deben ser 4): %d", list_size(obtener_tp_de_proceso(PID2)));
    log_info(logger_propio, "Resultado de no hacer nada: %d", resize(PID2, 60));
    log_info(logger_propio, "Páginas luego de no hacer nada (debe ser 4): %d", list_size(obtener_tp_de_proceso(PID2)));

    // caso out of memory
    log_info(logger_propio, "Páginas antes de out of memory (deben ser 2): %d", list_size(obtener_tp_de_proceso(PID3)));
    log_info(logger_propio, "Resultado de out of memory: %d", resize(PID3, 255));
    log_info(logger_propio, "Páginas luego de out of memory: %d", list_size(obtener_tp_de_proceso(PID3)));*/
}
