#include "atender_cpu.h"

int socket_cpu;

void atender_cpu(int *socket_cliente)
{
    socket_cpu = *socket_cliente;
    enviar_tamanio_de_pagina();
    int op_code = 0;
    while (op_code != -1)
    {
        op_code = recibir_operacion(socket_cpu);
        retardo_de_peticion();
        switch (op_code)
        {
        case SOLICITUD_INSTRUCCION:
            atender_solicitud_instruccion(); // Para arrancar, el kernel tiene que mandar el archivo a abrir.
            break;

        case ACCESO_ESPACIO_USUARIO_ESCRITURA:
            atender_escritura_espacio_usuario(socket_cpu);
            break;

        case ACCESO_ESPACIO_USUARIO_LECTURA:
            atender_lectura_espacio_usuario(socket_cpu);
            break;

        case SOLICITUD_MARCO:
            atender_solicitud_marco();
            break;

        case RESIZE_PROCESO:
            atender_resize();
            break;

        case -1:
            log_info(logger_propio, "Conexión con CPU cerrada.");

        default:
            log_info(logger_propio, "Codigo de operacion incorrecto en atender_cpu");
            break;
        }
    }
    free(socket_cliente);
}

void enviar_tamanio_de_pagina()
{
    t_paquete *paquete = crear_paquete(MENSAJE);
    int tam_pagina = obtener_tam_pagina();
    agregar_a_paquete(paquete, &tam_pagina, sizeof(int));
    enviar_paquete(paquete, socket_cpu);
    eliminar_paquete(paquete);
}

void enviar_instruccion_a_cpu(char *instruccion)
{
    t_paquete *paquete_instruccion = crear_paquete(INSTRUCCION);
    agregar_a_paquete_string(paquete_instruccion, instruccion);
    enviar_paquete(paquete_instruccion, socket_cpu);
    eliminar_paquete(paquete_instruccion);
}

void atender_solicitud_instruccion(void)
{
    uint32_t PID, PC;
    recibir_solicitud_instruccion(&PID, &PC);
    char *instruccion = obtener_instruccion_de_indice(PID, PC);
    enviar_instruccion_a_cpu(instruccion);
}

void recibir_solicitud_instruccion(uint32_t *PID, uint32_t *PC)
{
    t_list *pid_y_pc = recibir_paquete(socket_cpu);
    *PID = *(uint32_t *)list_get(pid_y_pc, 0);
    *PC = *(uint32_t *)list_get(pid_y_pc, 1);
    list_destroy_and_destroy_elements(pid_y_pc, free);
}

char *obtener_instruccion_de_indice(uint32_t PID, uint32_t PC)
{
    t_list *instrucciones = dictionary_get_with_int_key(indice_instrucciones, PID);
    char *instruccion = list_get(instrucciones, PC);
    return instruccion;
}

void enviar_marco(int marco)
{
    t_paquete *paquete = crear_paquete(SOLICITUD_MARCO);
    agregar_a_paquete(paquete, &marco, sizeof(int));
    enviar_paquete(paquete, socket_cpu);
    eliminar_paquete(paquete);
}

void recibir_solicitud_marco(uint32_t *PID, int *pagina)
{
    t_list *valores_paquete = recibir_paquete(socket_cpu);
    *PID = *(uint32_t *)list_get(valores_paquete, 0);
    *pagina = *(int *)list_get(valores_paquete, 1);
    list_destroy_and_destroy_elements(valores_paquete, free);
}

void atender_solicitud_marco()
{
    uint32_t PID;
    int pagina;
    recibir_solicitud_marco(&PID, &pagina);
    int marco = buscar_marco(PID, pagina);
    loggear_acceso_tablas_de_paginas(PID, pagina, marco);
    enviar_marco(marco);
}

void recibir_solicitud_resize(uint32_t *PID, uint32_t *tamanio_proceso)
{
    t_list *valores_paquete = recibir_paquete(socket_cpu);
    *PID = *(uint32_t *)list_get(valores_paquete, 0);
    *tamanio_proceso = *(uint32_t *)list_get(valores_paquete, 1);
    list_destroy_and_destroy_elements(valores_paquete, free);
}

void atender_resize()
{
    uint32_t PID, tamanio_proceso;
    recibir_solicitud_resize(&PID, &tamanio_proceso);
    op_code respuesta = resize(PID, tamanio_proceso);
    enviar_cod_op(respuesta, socket_cpu);
}

op_code resize(uint32_t PID, uint32_t tamanio_proceso)
{
    t_list *tp_de_proceso = obtener_tp_de_proceso(PID);
    int cantidad_pags = list_size(tp_de_proceso);
    int pags_solicitadas = (tamanio_proceso + obtener_tam_pagina() - 1) / obtener_tam_pagina();
    op_code resultado = OK;

    if (pags_solicitadas < cantidad_pags)
    {
        loggear_reduccion_de_proceso(PID, cantidad_pags * obtener_tam_pagina(), tamanio_proceso);

        for (int i = cantidad_pags - 1; i >= pags_solicitadas; i--)
        {
            quitar_ultima_pagina(tp_de_proceso);
        }
    }

    if (pags_solicitadas > cantidad_pags)
    {
        loggear_ampliacion_de_proceso(PID, cantidad_pags * obtener_tam_pagina(), tamanio_proceso);

        int pags_adicionales = pags_solicitadas - cantidad_pags;
        if (cantidad_marcos_libres() >= pags_adicionales)
        {
            for (int i = 0; i < pags_adicionales; i++)
            {
                agregar_pagina(tp_de_proceso);
            }
        }
        else
        {
            resultado = OUT_OF_MEMORY;
        }
    }

    return resultado;
}
