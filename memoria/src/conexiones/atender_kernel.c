#include "atender_kernel.h"

int socket_kernel;

void atender_kernel(int *socket_cliente)
{
    socket_kernel = *socket_cliente;
    int op_code = 0;
    while (op_code != -1)
    {
        op_code = recibir_operacion(socket_kernel);
        retardo_de_peticion();
        switch (op_code)
        {
        case CREAR_PROCESO_KERNEL:
            atender_crear_proceso();
            break;

        case FINALIZAR_PROCESO_KERNEL:
            atender_finalizar_proceso();
            break;

        case -1:
            log_info(logger_propio, "Conexión con kernel cerrada.");

        default:
            log_info(logger_propio, "Codigo de operacion incorrecto en atender_kernel");
            break;
        }
    }
    free(socket_cliente);
}

void atender_crear_proceso()
{
    uint32_t PID;
    char *path;
    recibir_creacion_proceso(&PID, &path);
    op_code respuesta = OK;
    if (!crear_estructuras_administrativas(PID, path))
    {
        respuesta = ERROR;
    }
    free(path);
    enviar_cod_op(respuesta, socket_kernel);
}

void recibir_creacion_proceso(uint32_t *PID, char **ptr_path)
{
    t_list *paquete_crear_proceso = recibir_paquete(socket_kernel);
    *PID = *(uint32_t *)list_get(paquete_crear_proceso, 0);
    *ptr_path = string_duplicate((char *)list_get(paquete_crear_proceso, 1));
    list_destroy_and_destroy_elements(paquete_crear_proceso, free);
}

bool crear_estructuras_administrativas(uint32_t PID, char *path)
{
    bool agregado_exitoso = agregar_instrucciones_al_indice(indice_instrucciones, PID, path);
    if (agregado_exitoso)
    {
        agregar_proceso_al_indice(PID);
    }

    return agregado_exitoso;
}

void atender_finalizar_proceso(void)
{
    uint32_t PID;
    recibir_pid(&PID);
    liberar_marcos_proceso(PID);
    liberar_estructuras_administrativas(PID);
    enviar_cod_op(OK, socket_kernel);
}

void recibir_pid(uint32_t *PID)
{
    t_list *paquete = recibir_paquete(socket_kernel);
    *PID = *(uint32_t *)list_get(paquete, 0);
    list_destroy_and_destroy_elements(paquete, free);
}

void liberar_estructuras_administrativas(uint32_t PID)
{
    quitar_instrucciones_al_indice(indice_instrucciones, PID);
    quitar_proceso_del_indice(PID);
}
