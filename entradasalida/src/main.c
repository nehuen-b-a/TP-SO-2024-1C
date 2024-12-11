#include "gestor_interfaces.h"
#include "conexiones/conexiones.h"
#include <signal.h>

t_log *logger_obligatorio;
t_log *logger_propio;
t_config *config;
char *nombre;

int main(int argc, char *argv[])
{
    logger_propio = crear_logger("entradasalida_propio");

    if (argc < 3)
    {
        log_info(logger_propio, "Uso: %s [nombre de interfaz] [ruta de archivo de configuracion]\n", argv[0]);
        return EXIT_FAILURE;
    }

    nombre = malloc(strlen(argv[1]) + 1);
    strcpy(nombre, argv[1]);
    logger_obligatorio = crear_logger("entradasalida_obligatorio");
    log_info(logger_propio, "Iniciando Interfaz de I/O %s", nombre);

    setear_config(argv[2]);

    // conectar con kernel y memoria (en caso de que corresponda)
    conexion_con_kernel();
    conexion_con_memoria();

    atender_segun_tipo_interfaz();
    recibir_peticiones_del_kernel();

    close(conexion_memoria);
    close(conexion_kernel);
    log_destroy(logger_obligatorio);
    log_destroy(logger_propio);
    config_destroy(config);
    free(nombre);

    return 0;
}