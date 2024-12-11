#include "atender_conexiones.h"

int conexion_cpu_memoria;
int conexion_cpu_kernel_dispatch;
int conexion_cpu_kernel_interrupt;

pthread_t th_dispatch;
pthread_t th_interrupt;

t_TLB *tlb;

void iniciar_servidor_dispatch(void)
{
    int server_fd = iniciar_servidor(logger_propio, obtener_puerto_escucha_dispatch());
    log_info(logger_propio, "CPU lista para recibir clientes (dispatch)");
    conexion_cpu_kernel_dispatch = esperar_cliente(logger_propio, server_fd);
    log_info(logger_propio, "Se conectó un cliente!");
    // int32_t handshake_esperado = 4;
    // int handshake_respuesta = handshake_servidor(logger_propio, conexion_cpu_kernel_dispatch, handshake_esperado);
}

void iniciar_servidor_interrupt(void)
{
    int server_fd = iniciar_servidor(logger_propio, obtener_puerto_escucha_interrupt());
    log_info(logger_propio, "CPU lista para recibir clientes (interrupt)");
    conexion_cpu_kernel_interrupt = esperar_cliente(logger_propio, server_fd);
    log_info(logger_propio, "Se conectó un cliente!");
    // int32_t handshake_esperado = 4;
    // int handshake_respuesta = handshake_servidor(logger_propio, conexion_cpu_kernel_interrupt, handshake_esperado);
}

void iniciar_conexion_memoria(void)
{
    conexion_cpu_memoria = crear_conexion(logger_propio, obtener_ip_memoria(), obtener_puerto_memoria());
    enviar_cod_op(CONEXION_CPU, conexion_cpu_memoria);
    if (recibir_operacion(conexion_cpu_memoria) == MENSAJE)
    {
        t_list *valores = recibir_paquete(conexion_cpu_memoria);
        tamanio_pagina = *(int *)list_get(valores, 0);
        list_destroy_and_destroy_elements(valores, free);
    }
    else
    {
        log_info(logger_propio, "No se logró conectar adecuadamente con la memoria");
        exit(1);
    }
}

char *recibir_interrupcion() // TODO F:
{
    if (recibir_operacion(conexion_cpu_kernel_interrupt) == INTERRUPCION)
    {
        char *interrupcion = recibir_string(conexion_cpu_kernel_interrupt);
        log_info(logger_propio, "recibir_interrupcion(): motivo interrupcion: %s", interrupcion);
        return interrupcion;
    }
    else
    {
        log_info(logger_propio, "Error: recibir_interrupcion(): Op Code != INTERRUPCION");
        return NULL; // TODO F ?
    }
}

void atender_dispatch()
{
    iniciar_servidor_dispatch();

    log_info(logger_propio, "CPU escuchando puerto dispatch");

    tlb = crear_TLB();
    while (1)
    {
        int opCode = recibir_operacion(conexion_cpu_kernel_dispatch);
        log_info(logger_propio, "opCode: %d", opCode);
        if (opCode == CONTEXTO_EJECUCION)
        {
            log_info(logger_propio, "Contexto recibido por CPU");
            t_contexto *contexto = recibir_contexto(conexion_cpu_kernel_dispatch);
            ciclo_de_instruccion(&contexto);
            destruir_contexto(contexto);
        }
        else
        {
            log_info(logger_propio, "Enviaste a CPU algo que no es un contexto");
            break;
        }
    }
}

void atender_interrupt()
{
    iniciar_servidor_interrupt();

    log_info(logger_propio, "CPU escuchando puerto interrupt");
    while (1)
    {
        pthread_mutex_lock(&mutex_interrupt);
        while (!hay_interrupcion)
        {
            pthread_mutex_unlock(&mutex_interrupt);
            char *motivo = recibir_interrupcion();
            if(motivo == NULL){abort();}

            pthread_mutex_lock(&mutex_interrupt);
            motivo_interrupcion = motivo;
            hay_interrupcion = true;
        }
        pthread_mutex_unlock(&mutex_interrupt);
    }
}

void iniciar_conexiones()
{
    iniciar_conexion_memoria();

    pthread_mutex_init(&mutex_interrupt, NULL);

    pthread_create(&th_dispatch, NULL, (void *)atender_dispatch, NULL);
    pthread_create(&th_interrupt, NULL, (void *)atender_interrupt, NULL);

    pthread_join(th_dispatch, NULL);
    pthread_join(th_interrupt, NULL);
}
