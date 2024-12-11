#include "instrucciones.h"

// PROVISORIO solicitud_de_instruccion

t_solicitud_de_instruccion *crear_solicitud_de_instruccion()
{

    t_solicitud_de_instruccion *solicitud_de_instruccion = malloc(sizeof(int32_t));
    if (solicitud_de_instruccion != NULL)
    {
        solicitud_de_instruccion->desplazamiento = 0;
    }
    return solicitud_de_instruccion;
}

void destruir_solicitud_de_instruccion(t_solicitud_de_instruccion *solicitud_de_instruccion)
{
    free(solicitud_de_instruccion);
}

void serializar_solicitud_de_instruccion(t_solicitud_de_instruccion *solicitud, t_paquete *paquete)
{
    int tamanio_solicitud = tamanio_solicitud_de_instruccion(solicitud);

    agregar_a_paquete(paquete, &(solicitud->desplazamiento), tamanio_solicitud);
}

void generar_solicitud_de_instruccion(t_solicitud_de_instruccion *solicitud, t_list *valores)
{
    // Obtener el desplazamiento del paquete
    solicitud->desplazamiento = *(int32_t *)(valores->head->data);
}

int tamanio_solicitud_de_instruccion(t_solicitud_de_instruccion *solicitud)
{
    return sizeof(int32_t); // por ahora
}

// PROVISORIO INSTRUCCIONES

t_instruccion_cadena *crear_instruccion()
{
    t_instruccion_cadena *instruccion = malloc(sizeof(char *));
    if (instruccion != NULL)
    {
        instruccion->instruccion = NULL;
    }
    instruccion->instruccion = NULL; // Inicialmente no hay instrucción
    return instruccion;
}

void destruir_instruccion_cadena(t_instruccion_cadena *instruccion)
{
    if (instruccion->instruccion != NULL)
    {
        free(instruccion->instruccion);
    }
    free(instruccion);
}

void serializar_instruccion(t_instruccion_cadena *instruccion, t_paquete *paquete)
{
    if (instruccion->instruccion == NULL)
    {
        // Manejo del error
        fprintf(stderr, "Error: La instrucción no está inicializada.\n");
        return;
    }

    uint32_t longitud_instruccion = strlen(instruccion->instruccion) + 1; // +1 para incluir el null terminator
    agregar_a_paquete(paquete, instruccion->instruccion, longitud_instruccion);
}

// REVISAR!!
void generar_instruccion(t_instruccion_cadena *instruccion, t_list *valores)
{
    // Calcular el tamaño total necesario para la instrucción
    int longitud_total = 0;
    for (int i = 0; i < list_size(valores); i++)
    {
        char *valor = list_get(valores, i);
        longitud_total += strlen(valor) + 1; // +1 para el espacio o el terminador nulo
    }

    // Reservar memoria para la instrucción
    instruccion->instruccion = string_new();

    // Concatenar los valores en una sola cadena
    for (int i = 0; i < list_size(valores); i++)
    {
        char *valor = list_get(valores, i);
        strcat(instruccion->instruccion, valor);
        if (i < list_size(valores) - 1)
        {
            strcat(instruccion->instruccion, " "); // Agregar espacio entre valores
        }
    }
}

int tamanio_instruccion(t_instruccion_cadena *instruccion)
{
    return sizeof(uint32_t) + strlen(instruccion->instruccion) + 1; // +1 para incluir el null terminator
}