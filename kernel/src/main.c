#include "consola/consola.h"
#include <utils/funcionalidades_basicas.h>
#include <utils/comunicacion/comunicacion.h>
#include "conexiones/conexiones.h"

t_log *logger_obligatorio;
t_log *logger_propio;
t_config *config;

int main(int argc, char *argv[])
{
    logger_obligatorio = crear_logger("kernel_obligatorio");
    logger_propio = crear_logger("kernel_propio");
    log_info(logger_propio, "Iniciando Kernel ...");

    config = iniciar_config(logger_propio, "kernel.config");

    iniciar_conexiones();
    iniciar_planificacion();
    consola_interactiva();

    destruir_semaforos_planificacion();
    destruir_listas_planificacion();
    destruir_mutex_por_colas_de_recurso();

    close(conexion_kernel_cpu_dispatch);
    close(conexion_kernel_cpu_interrupt);
    close(conexion_kernel_memoria);
    log_destroy(logger_obligatorio);
    log_destroy(logger_propio);
    config_destroy(config);

    return 0;
}