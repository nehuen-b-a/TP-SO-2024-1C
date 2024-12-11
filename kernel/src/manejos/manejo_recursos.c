#include "manejo_recursos.h"

int *instancias_recursos;
t_list *colas_de_recursos;
char **nombres_recursos;
pthread_mutex_t mutex_colas_de_recursos;
pthread_mutex_t mutex_instancias_recursos;
pthread_mutex_t *mutex_cola_recurso;

void crear_colas_de_bloqueo(void)
{
    instancias_recursos = NULL;
    colas_de_recursos = list_create();
    nombres_recursos = obtener_recursos();
    char **instancias_aux = obtener_instancias_recursos();

    for (int i = 0; i < string_array_size(instancias_aux); i++)
    {
        int instancia_en_entero = atoi(instancias_aux[i]);
        instancias_recursos = realloc(instancias_recursos, (i + 1) * sizeof(int));
        instancias_recursos[i] = instancia_en_entero;

        t_list *cola_bloqueo = list_create();
        pthread_mutex_lock(&mutex_colas_de_recursos);
        list_add(colas_de_recursos, (void *)cola_bloqueo);
        pthread_mutex_unlock(&mutex_colas_de_recursos);
    }
    crear_mutex_por_cola_de_recurso();

    destruir_lista_string(instancias_aux);
}

void liberar_recursos(t_pcb *pcb)
{
    eliminar_pcb_de_colas_de_recursos(pcb);

    // elimino los recursos dentro del pcb
    for (int i = 0; i < list_size(pcb->recursos_asignados); i++)
    {
        int *pos_recurso = (int *)list_get(pcb->recursos_asignados, i);
        if (pos_recurso == NULL)
        {
            log_info(logger_propio, "No se pudo obtener el recurso asignado del pcb.");
        }
        else
        {
            // aumento la cantidad de instancias de ese recurso
            pthread_mutex_lock(&mutex_instancias_recursos);
            instancias_recursos[*pos_recurso]++;
            pthread_mutex_unlock(&mutex_instancias_recursos);

            desbloquear_pcb_si_corresponde(*pos_recurso);
        }
    }
}

void desbloquear_pcb_si_corresponde(int pos_recurso)
{
    pthread_mutex_lock(&mutex_colas_de_recursos);
    t_list *cola_bloqueo_recurso = list_get(colas_de_recursos, pos_recurso);
    if (list_size(cola_bloqueo_recurso) > 0)
    {
        t_pcb *pcb_a_desbloquear = desencolar_pcb(cola_bloqueo_recurso);
        pthread_mutex_unlock(&mutex_colas_de_recursos);

        // sem_wait(&transicion_estados_corto_plazo_liberada);

        pthread_mutex_lock(&mutex_lista_BLOCKED);
        list_remove_element(pcbs_en_BLOCKED, pcb_a_desbloquear);
        pthread_mutex_unlock(&mutex_lista_BLOCKED);

        // sem_post(&transicion_estados_corto_plazo_liberada);

        int *pos = malloc(sizeof(int));
        *pos = pos_recurso;
        list_add(pcb_a_desbloquear->recursos_asignados, pos);

        // pthread_mutex_lock(&mutex_instancias_recursos);
        // instancias_recursos[pos_recurso]--;
        // pthread_mutex_unlock(&mutex_instancias_recursos);

        // no considero mandarlo a Ready con prioridad ya que no se bloqueo por una IO
        ingresar_pcb_a_READY(pcb_a_desbloquear);
    }
    else
    {
        pthread_mutex_unlock(&mutex_colas_de_recursos);
    }
}

bool wait_recurso(char *recurso, t_pcb *pcb)
{
    bool recurso_asignado = false;

    if (existe_recurso(recurso))
    {
        int pos_recurso = posicion_recurso(recurso);
        pthread_mutex_lock(&mutex_instancias_recursos);
        instancias_recursos[pos_recurso]--;
        pthread_mutex_unlock(&mutex_instancias_recursos);
        int cantidad_instancias_recurso = instancias_recursos[pos_recurso];

        if (cantidad_instancias_recurso < 0)
        {
            // sem_wait(&transicion_estados_corto_plazo_liberada);
            pcb->estado = BLOCKED;
            pcb_en_EXEC = NULL;

            pthread_mutex_lock(&mutex_lista_BLOCKED);
            list_add(pcbs_en_BLOCKED, pcb);
            pthread_mutex_unlock(&mutex_lista_BLOCKED);

            // logs minimos y obligatorios
            loggear_cambio_de_estado(pcb->PID, EXEC, BLOCKED);
            loggear_motivo_de_bloqueo(pcb->PID, recurso);

            // sem_post(&transicion_estados_corto_plazo_liberada);

            // se queda bloqueado en la lista correspondiente al recurso
            pthread_mutex_lock(&mutex_colas_de_recursos);
            t_list *cola_bloqueo_recurso = list_get(colas_de_recursos, pos_recurso);
            list_add(cola_bloqueo_recurso, pcb);
            pthread_mutex_unlock(&mutex_colas_de_recursos);
        }
        else
        {
            int *pos = malloc(sizeof(int));
            *pos = pos_recurso;
            list_add(pcb->recursos_asignados, pos);

            recurso_asignado = true;
            // devolvemos la ejecucion al pcb sin resetear quantum en caso de ser RR o VRR
            // pthread_t hilo_quantum;
            // if(pcb->quantum > 0){
            //     procesar_pcb_segun_algoritmo(pcb, &hilo_quantum);
            //     esperar_contexto_y_manejar_desalojo(pcb, &hilo_quantum);
            // }
        }
    }
    else
    {
        enviar_pcb_a_EXIT(pcb, INVALID_RESOURCE);
    }

    return recurso_asignado;
}

bool signal_recurso(char *recurso, t_pcb *pcb)
{
    bool recurso_liberado = false;

    if (existe_recurso(recurso))
    {
        // aumento las instancias de recurso
        int pos_recurso = posicion_recurso(recurso);
        pthread_mutex_lock(&mutex_instancias_recursos);
        ++instancias_recursos[pos_recurso];
        pthread_mutex_unlock(&mutex_instancias_recursos);

        // le quito el recurso al pcb
        bool pcb_tiene_recurso(void *data)
        {
            return pos_recurso == *(int *)data;
        }
        free(list_remove_by_condition(pcb->recursos_asignados, pcb_tiene_recurso));

        pthread_mutex_lock(&mutex_colas_de_recursos);
        t_list *cola_bloqueo_recurso = list_get(colas_de_recursos, posicion_recurso(recurso));
        if (list_size(cola_bloqueo_recurso) > 0)
        {
            // desbloqueamos al primer proceso de la cola de bloqueados de ese recurso
            t_pcb *pcb_a_desbloquear = desencolar_pcb(cola_bloqueo_recurso);
            pthread_mutex_unlock(&mutex_colas_de_recursos);

            // sem_wait(&transicion_estados_corto_plazo_liberada);

            pthread_mutex_lock(&mutex_lista_BLOCKED);
            list_remove_element(pcbs_en_BLOCKED, pcb_a_desbloquear);
            pthread_mutex_unlock(&mutex_lista_BLOCKED);

            // no considero mandarlo a Ready con prioridad ya que no se bloqueo por una IO
            ingresar_pcb_a_READY(pcb_a_desbloquear);

            // sem_post(&transicion_estados_corto_plazo_liberada);

            int *pos = malloc(sizeof(int));
            *pos = pos_recurso;
            list_add(pcb_a_desbloquear->recursos_asignados, pos);

            // pthread_mutex_lock(&mutex_instancias_recursos);
            // instancias_recursos[pos_recurso]--;
            // pthread_mutex_unlock(&mutex_instancias_recursos);
        }
        else
        {
            pthread_mutex_unlock(&mutex_colas_de_recursos);
        }

        // devolvemos la ejecucion al pcb sin resetear quantum en caso de ser RR o VRR
        // pthread_t hilo_quantum;
        // procesar_pcb_segun_algoritmo(pcb, &hilo_quantum);
        // esperar_contexto_y_manejar_desalojo(pcb, NULL);

        recurso_liberado = true;
    }
    else
    {
        enviar_pcb_a_EXIT(pcb, INVALID_RESOURCE);
    }

    return recurso_liberado;
}

void eliminar_pcb_de_colas_de_recursos(t_pcb *pcb)
{
    for (int i = 0; i < list_size(colas_de_recursos); i++)
    {
        t_list *cola = list_get(colas_de_recursos, i);
        pthread_mutex_lock(&mutex_cola_recurso[i]);
        bool fue_quitada = list_remove_element(cola, pcb);
        pthread_mutex_unlock(&mutex_cola_recurso[i]);
        if (fue_quitada)
        {
            pthread_mutex_lock(&mutex_instancias_recursos);
            ++instancias_recursos[i];
            pthread_mutex_unlock(&mutex_instancias_recursos);
        }
    }
}

void destruir_lista_string(char **lista_string)
{
    for (int i = 0; i < string_array_size(lista_string); i++)
        free(lista_string[i]);
    free(lista_string);
}

bool existe_recurso(char *recurso)
{
    for (int i = 0; i < cantidad_recursos(); i++)
    {
        if (strcmp(nombres_recursos[i], recurso) == 0)
            return true;
    }
    return false;
}

int posicion_recurso(char *recurso)
{
    for (int i = 0; i < cantidad_recursos(); i++)
    {
        if (strcmp(nombres_recursos[i], recurso) == 0)
            return i;
    }
    return 0;
}

int cantidad_recursos(void)
{
    int cantidad = 0;
    while (nombres_recursos[cantidad] != NULL)
        cantidad++;

    return cantidad;
}

void destruir_colas_de_recursos(void)
{
    list_destroy_and_destroy_elements(colas_de_recursos, (void *)list_destroy);
}

void crear_mutex_por_cola_de_recurso(void)
{
    mutex_cola_recurso = malloc(sizeof(pthread_mutex_t) * cantidad_recursos());
    for (int i = 0; i < cantidad_recursos(); i++)
    {
        pthread_mutex_init(&mutex_cola_recurso[i], NULL);
    }
}

void destruir_mutex_por_colas_de_recurso(void)
{
    for (int i = 0; i < cantidad_recursos(); i++)
    {
        pthread_mutex_destroy(&mutex_cola_recurso[i]);
    }
    free(mutex_cola_recurso);
}