#include "serializacion.h"

// ------------------------ CLIENTE ------------------------ //

void *serializar_paquete(t_paquete *paquete, int bytes)
{
    void *magic = malloc_or_die(bytes, "Error al reservar memoria para serializar paquete");
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento += paquete->buffer->size;

    return magic; // paquete serializado: OpCode + tamaño + stream
}

t_paquete *crear_paquete(int codigo_operacion)
{
    t_paquete *paquete = malloc_or_die(sizeof(t_paquete), "Error al reservar memoria para crear paquete");
    paquete->codigo_operacion = codigo_operacion;
    crear_buffer(paquete);
    return paquete;
}

void crear_buffer(t_paquete *paquete)
{
    paquete->buffer = malloc_or_die(sizeof(t_buffer), "Error al reservar memoria para crear buffer");
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
}

void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio)
{
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int) + tamanio); // Tamaño previo + int que contendrá tamañoDeDatoNuevo + tamanioDatoNuevo

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));        // Agrego tamaño del dato nuevo al paquete
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio); // Agrego dato nuevo al paquete

    paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete *paquete, int socket)
{
    int bytes = paquete->buffer->size + 2 * sizeof(int); // tamaño del stream + int con streamSize + opCode
    void *a_enviar = serializar_paquete(paquete, bytes);

    send(socket, a_enviar, bytes, 0);

    free(a_enviar);
}

int enviar_paquete_interfaz(t_paquete *paquete, int conexion)
{
    int bytes = paquete->buffer->size + 2 * sizeof(int); // tamaño del stream + int con streamSize + opCode
    void *a_enviar = serializar_paquete(paquete, bytes);

    int sigue_la_conexion = send(conexion, a_enviar, bytes, MSG_NOSIGNAL);

    free(a_enviar);
    return sigue_la_conexion;
}

void agregar_parametros_a_paquete(t_paquete *paquete, t_list *parametros)
{
    for (int i = 0; i < list_size(parametros); i++)
    {
        char *param = list_get(parametros, i);
        agregar_a_paquete_string(paquete, param);
    }
}

void enviar_cod_op(op_code codigo_de_operacion, int socket)
{
    void *buffer = malloc_or_die(sizeof(int), "Error al reservar memoria para crear buffer de op_code");
    memcpy(buffer, &(codigo_de_operacion), sizeof(int));
    send(socket, buffer, sizeof(int), 0);
    free(buffer);
}

// Crea buffer y crear paquete previo al envio
void enviar_mensaje(char *mensaje, int socket) // ?
{
    t_paquete *paquete = crear_paquete(MENSAJE);
    agregar_a_paquete_string(paquete, mensaje);
    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);
}

void eliminar_paquete(t_paquete *paquete)
{
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}
// ------------------------ Funciones por tipo de dato ------------------------ //

void agregar_a_paquete_uint32(t_paquete *paquete, uint32_t data)
{
    agregar_a_paquete(paquete, &data, sizeof(uint32_t));
}

void agregar_a_paquete_uint8(t_paquete *paquete, uint8_t data)
{
    agregar_a_paquete(paquete, &data, sizeof(uint8_t));
}
void agregar_a_paquete_string(t_paquete *paquete, char *data)
{
    agregar_a_paquete(paquete, data, strlen(data) + 1);
}

// ------------------------ SERVIDOR ------------------------ //

int recibir_operacion(int socket) // En base a operacion recibida -> defino como deserializar el buffer
{
    int cod_op;
    if (recv(socket, &cod_op, sizeof(int), MSG_WAITALL) > 0)
        return cod_op;
    else
    {
        close(socket);
        return -1;
    }
}

// Paso puntero a entero (size) para guardar tamaño del buffer (usado para deserializar)
void *recibir_buffer(int socket, int *size) //
{
    void *buffer;

    recv(socket, size, sizeof(int), MSG_WAITALL);
    buffer = malloc(*size);
    recv(socket, buffer, *size, MSG_WAITALL);

    return buffer;
}

char *recibir_string(int socket)
{
    t_list *paquete_con_string = recibir_paquete(socket);
    char *string = string_duplicate(list_get(paquete_con_string, 0));
    list_destroy_and_destroy_elements(paquete_con_string, free);
    return string;
}

uint32_t *recibir_uint32_t(int socket)
{
    t_list *paquete_con_uint32_t = recibir_paquete(socket);
    uint32_t *entero = (uint32_t *)list_get(paquete_con_uint32_t, 0);
    list_destroy_and_destroy_elements(paquete_con_uint32_t, free);
    return entero;
}

t_list *recibir_paquete(int socket)
{
    int size;
    int desplazamiento = 0;
    void *buffer;
    t_list *valores = list_create();
    int tamanio;

    buffer = recibir_buffer(socket, &size);
    while (desplazamiento < size)
    {
        memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
        desplazamiento += sizeof(int);
        char *valor = malloc(tamanio);
        memcpy(valor, buffer + desplazamiento, tamanio);
        desplazamiento += tamanio;
        list_add(valores, valor);
    }
    free(buffer);
    return valores;
}
