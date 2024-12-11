#include "conexiones.h"

int conexion_memoria;
int conexion_kernel;

void conexion_con_kernel(void)
{
    conexion_kernel = crear_conexion(logger_propio, obtener_ip_kernel(), obtener_puerto_kernel());

    t_paquete *paquete = crear_paquete(CONEXION_INTERFAZ_KERNEL);
    agregar_a_paquete_string(paquete, nombre);
    agregar_a_paquete_string(paquete, obtener_tipo_interfaz());
    enviar_paquete(paquete, conexion_kernel);
    eliminar_paquete(paquete);

    log_info(logger_propio, "Se ha conectado correctamente con el kernel");
}

void conexion_con_memoria(void)
{
    if (strcmp(obtener_tipo_interfaz(), "GENERICA") != 0)
    {
        conexion_memoria = crear_conexion(logger_propio, obtener_ip_memoria(), obtener_puerto_memoria());
        enviar_cod_op(CONEXION_IO, conexion_memoria);
    }
}

void recibir_peticiones_del_kernel(void)
{
    uint8_t respuesta;
    int cod_op;
    t_list *parametros;

    while ((cod_op = recibir_operacion(conexion_kernel)) != -1)
    {
        parametros = recibir_paquete(conexion_kernel);
        respuesta = atender(cod_op, parametros);
        enviar_cod_op(respuesta, conexion_kernel);
        list_destroy_and_destroy_elements(parametros, free);
    }
    log_info(logger_propio, "Conexión cerrada con kernel");
}