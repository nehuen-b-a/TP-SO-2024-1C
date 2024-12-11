#include "interface_cpu.h"
#include <utils/estructuras_compartidas/instrucciones.h>

void solicitar_lectura_de_instruccion(uint32_t PID, uint32_t PC)
{
    t_paquete *paquete = crear_paquete(SOLICITUD_INSTRUCCION);
    agregar_a_paquete_uint32(paquete, PID);
    agregar_a_paquete_uint32(paquete, PC);
    enviar_paquete(paquete, conexion_cpu_memoria);
    eliminar_paquete(paquete);
}

void solicitar_marco_memoria(uint32_t PID, int pagina)
{
    t_paquete *paquete = crear_paquete(SOLICITUD_MARCO);

    agregar_a_paquete_uint32(paquete, PID);
    agregar_a_paquete(paquete, &pagina, sizeof(int));

    enviar_paquete(paquete, conexion_cpu_memoria);
    eliminar_paquete(paquete);
}

int recibir_marco_memoria()
{
    int codigo_operacion = recibir_operacion(conexion_cpu_memoria);
    t_list *valores = recibir_paquete(conexion_cpu_memoria);

    if (codigo_operacion == SOLICITUD_MARCO)
    {
        int marco = *(int *)list_get(valores, 0);
        list_destroy_and_destroy_elements(valores, free);
        return marco;
    }
    else
    {
        list_destroy(valores);
        return -1;
    }
}
