#include "configuraciones.h"
#include "conexiones/atender_conexiones.h"

t_log *logger_obligatorio;
t_log *logger_propio;
t_config *config;

int main(int argc, char *argv[])
{
    logger_obligatorio = crear_logger("cpu_obligatorio");
    logger_propio = crear_logger("cpu_propio");
    config = iniciar_config(logger_propio, "cpu.config");
    log_info(logger_propio, "Iniciando CPU ...");

    iniciar_conexiones();

    close(conexion_cpu_memoria);
    close(conexion_cpu_kernel_dispatch);
    close(conexion_cpu_kernel_interrupt);
    log_destroy(logger_obligatorio);
    log_destroy(logger_propio);
    config_destroy(config);
    return 0;
}