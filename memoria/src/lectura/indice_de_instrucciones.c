#include "lectura.h"

t_dictionary *crear_indice_de_instrucciones()
{
    t_dictionary *diccionario = dictionary_create();
    return diccionario;
}

void destruir_lista_instrucciones(void *tp)
{
    list_destroy_and_destroy_elements(tp, free);
}

void destruir_indice_de_instrucciones(t_dictionary *indice_de_instrucciones)
{
    dictionary_destroy_and_destroy_elements(indice_de_instrucciones, destruir_lista_instrucciones);
}

bool agregar_instrucciones_al_indice(t_dictionary *indice_de_instrucciones, uint32_t PID, char *path)
{
    t_list *instrucciones_de_proceso = subir_instrucciones(path); // sube las instrucciones a la estructura de memoria
    bool agregado_exitoso = false;
    if (!list_is_empty(instrucciones_de_proceso))
    {
        agregado_exitoso = true;
        dictionary_put_with_int_key(indice_de_instrucciones, PID, instrucciones_de_proceso);
    }
    else
    {
        list_destroy(instrucciones_de_proceso);
    }

    return agregado_exitoso;
}

void quitar_instrucciones_al_indice(t_dictionary *indice_de_instrucciones, uint32_t PID)
{
    t_list *instrucciones_de_proceso = dictionary_remove_with_int_key(indice_de_instrucciones, PID);
    if (instrucciones_de_proceso != NULL)
    {
        list_destroy_and_destroy_elements(instrucciones_de_proceso, free);
    }
}
