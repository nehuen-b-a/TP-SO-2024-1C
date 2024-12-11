#include "conexiones.h"

int conexion_kernel_cpu_dispatch;
int conexion_kernel_cpu_interrupt;
int conexion_kernel_memoria;
int servidor_kernel_fd;
pthread_t hilo_servidor;

void servidor(void)
{
    servidor_kernel_fd = iniciar_servidor(logger_propio, obtener_puerto_escucha());
    log_info(logger_propio, "Kernel listo para recibir clientes");
    ejecutar_espera_interfaces();
}

void conexion_interrupt_con_CPU(void)
{
    conexion_kernel_cpu_interrupt = crear_conexion(logger_propio, obtener_ip_cpu(), obtener_puerto_cpu_interrupt());
}

void conexion_dispatch_con_CPU(void)
{
    conexion_kernel_cpu_dispatch = crear_conexion(logger_propio, obtener_ip_cpu(), obtener_puerto_cpu_dispatch());
}

void conexion_memoria(void)
{
    conexion_kernel_memoria = crear_conexion(logger_propio, obtener_ip_memoria(), obtener_puerto_memoria());
    enviar_cod_op(CONEXION_KERNEL, conexion_kernel_memoria);
}

void iniciar_conexiones(void)
{
    if (pthread_create(&hilo_servidor, NULL, (void *)servidor, NULL))
        log_error(logger_propio, "Error creando el hilo servidor para recibir las interfaces.");

    conexion_dispatch_con_CPU();
    conexion_interrupt_con_CPU();
    conexion_memoria();
}