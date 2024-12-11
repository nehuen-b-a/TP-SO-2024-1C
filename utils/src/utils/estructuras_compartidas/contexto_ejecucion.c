#include "contexto_ejecucion.h"

t_contexto *iniciar_contexto(void)
{
    t_contexto *contexto = malloc(sizeof(t_contexto));
    contexto->PID = 0;
    contexto->registros_cpu = crear_registros_cpu();
    return contexto;
}

void destruir_contexto(t_contexto *contexto)
{
    if(contexto != NULL){
        if(!dictionary_is_empty(contexto->registros_cpu)){
            dictionary_destroy_and_destroy_elements(contexto->registros_cpu, free);
        }
        free(contexto);
        contexto = NULL;
    }
}

// AX, EAX, BX, EBX, CX, ECX, DX, EDX, PC, SI, DI
void agregar_a_paquete_registros_cpu(t_paquete *paquete, t_dictionary *registros_cpu)
{
    char name[3] = "AX", long_name[4] = "EAX";

    for (int i = 0; i < 4; i++)
    {
        char *registroAX = (char *)dictionary_get(registros_cpu, name);
        char *registroEAX = (char *)dictionary_get(registros_cpu, long_name);

        agregar_a_paquete(paquete, (void *)registroAX, sizeof(uint8_t));
        agregar_a_paquete(paquete, (void *)registroEAX, sizeof(uint32_t));
    
        name[0]++;
        long_name[1]++;
    }

    agregar_a_paquete(paquete, (void *)dictionary_get(registros_cpu, "PC"), sizeof(uint32_t));
    agregar_a_paquete(paquete, (void *)dictionary_get(registros_cpu, "SI"), sizeof(uint32_t));
    agregar_a_paquete(paquete, (void *)dictionary_get(registros_cpu, "DI"), sizeof(uint32_t));
}

void agregar_a_paquete_contexto(t_paquete *paquete, t_contexto *contexto)
{
    agregar_a_paquete_uint32(paquete, *(uint32_t *)&contexto->PID);
    agregar_a_paquete_registros_cpu(paquete, contexto->registros_cpu);
}

void enviar_contexto(int socket, t_contexto *contexto)
{
    t_paquete *paquete = crear_paquete(CONTEXTO_EJECUCION);
    agregar_a_paquete_contexto(paquete, contexto);
    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);
    destruir_contexto(contexto);
}

motivo_desalojo string_interrupcion_to_enum_motivo(char *interrupcion)
{
    motivo_desalojo motivo;

    if (strcmp(interrupcion, "EXIT") == 0)
        motivo = DESALOJO_EXIT_INTERRUPTED;
    else if (strcmp(interrupcion, "FIN_QUANTUM") == 0)
        motivo = DESALOJO_FIN_QUANTUM;
    else
        log_info(logger_propio, "Motivo de desalojo inexistente");
    return motivo;
}

t_contexto *recibir_contexto(int socket) // SIN opCode
{
    t_contexto *contexto = malloc(sizeof(t_contexto));
    contexto->registros_cpu = dictionary_create();

    // Orden en lista: AX, EAX, BX, EBX, CX, ECX, DX, EDX, PC, SI, DI
    t_list *registros_contexto = recibir_paquete(socket);

    memcpy(&(contexto->PID), (uint32_t *)list_get(registros_contexto, 0), sizeof(uint32_t));
    dictionary_put(contexto->registros_cpu, "AX",
                   memcpy(malloc(sizeof(uint8_t)), list_get(registros_contexto, 1), sizeof(uint8_t)));
    dictionary_put(contexto->registros_cpu, "EAX",
                   memcpy(malloc(sizeof(uint32_t)), list_get(registros_contexto, 2), sizeof(uint32_t)));
    dictionary_put(contexto->registros_cpu, "BX",
                   memcpy(malloc(sizeof(uint8_t)), list_get(registros_contexto, 3), sizeof(uint8_t)));
    dictionary_put(contexto->registros_cpu, "EBX",
                   memcpy(malloc(sizeof(uint32_t)), list_get(registros_contexto, 4), sizeof(uint32_t)));
    dictionary_put(contexto->registros_cpu, "CX",
                   memcpy(malloc(sizeof(uint8_t)), list_get(registros_contexto, 5), sizeof(uint8_t)));
    dictionary_put(contexto->registros_cpu, "ECX",
                   memcpy(malloc(sizeof(uint32_t)), list_get(registros_contexto, 6), sizeof(uint32_t)));
    dictionary_put(contexto->registros_cpu, "DX",
                   memcpy(malloc(sizeof(uint8_t)), list_get(registros_contexto, 7), sizeof(uint8_t)));
    dictionary_put(contexto->registros_cpu, "EDX",
                   memcpy(malloc(sizeof(uint32_t)), list_get(registros_contexto, 8), sizeof(uint32_t)));
    dictionary_put(contexto->registros_cpu, "PC",
                   memcpy(malloc(sizeof(uint32_t)), list_get(registros_contexto, 9), sizeof(uint32_t)));
    dictionary_put(contexto->registros_cpu, "SI",
                   memcpy(malloc(sizeof(uint32_t)), list_get(registros_contexto, 10), sizeof(uint32_t)));
    dictionary_put(contexto->registros_cpu, "DI",
                   memcpy(malloc(sizeof(uint32_t)), list_get(registros_contexto, 11), sizeof(uint32_t)));

    list_destroy_and_destroy_elements(registros_contexto, free);

    return contexto;
}