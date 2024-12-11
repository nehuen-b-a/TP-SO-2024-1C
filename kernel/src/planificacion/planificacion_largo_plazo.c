#include "planificacion.h"

t_list *pcbs_en_EXIT;
t_list *pcbs_en_READY;
t_list *pcbs_en_aux_READY;
t_list *pcbs_en_NEW;
t_list *pcbs_en_memoria;
t_pcb *pcb_en_EXEC;
t_list *pcbs_en_BLOCKED;

sem_t hay_pcbs_NEW;
sem_t hay_pcbs_READY;
sem_t sem_grado_multiprogramacion;
sem_t planificacion_largo_plazo_liberada;
sem_t planificacion_corto_plazo_liberada;
sem_t desalojo_liberado;
sem_t planificacion_pausada;
sem_t transicion_estados_corto_plazo_liberada;

pthread_mutex_t mutex_lista_NEW;
pthread_mutex_t mutex_cola_READY;
pthread_mutex_t mutex_cola_aux_READY;
pthread_mutex_t mutex_lista_BLOCKED;
pthread_mutex_t mutex_pcb_EXEC;
pthread_mutex_t mutex_lista_memoria;
pthread_mutex_t mutex_lista_EXIT;
pthread_mutex_t mutex_lista_PIDS;

int32_t procesos_creados = 1;
char *algoritmo;
int32_t grado_multiprogramacion_auxiliar;
pthread_mutex_t mutex_multiprogramacion_auxiliar;

pthread_t hilo_planificador_largo_plazo;
pthread_t hilo_planificador_corto_plazo;
pthread_t hilo_recursos_liberados;

char *motivos[] = {
    [SUCCESS] = "SUCCESS",
    [INVALID_RESOURCE] = "INVALID_RESOURCE",
    [INVALID_INTERFACE] = "INVALID_INTERFACE",
    [FINALIZACION_OUT_OF_MEMORY] = "OUT_OF_MEMORY",
    [INTERRUPTED_BY_USER] = "INTERRUPTED_BY_USER",
};

void iniciar_planificacion(void)
{
    inicializar_listas_planificacion();
    inicializar_semaforos_planificacion();

    if (pthread_create(&hilo_planificador_largo_plazo, NULL, (void *)planificar_a_largo_plazo, NULL))
        log_error(logger_propio, "Error creando el hilo del planificador de largo plazo.");
    if (pthread_create(&hilo_planificador_corto_plazo, NULL, (void *)planificar_a_corto_plazo_segun_algoritmo, NULL))
        log_error(logger_propio, "Error creando el hilo del planificador de corto plazo.");

    pthread_detach(hilo_planificador_largo_plazo);
    pthread_detach(hilo_planificador_corto_plazo);
}

void planificar_a_largo_plazo(void)
{
    grado_multiprogramacion_auxiliar = obtener_grado_multiprogramacion();

    while (1)
    {
        sem_wait(&hay_pcbs_NEW);
        t_pcb *pcb = obtener_siguiente_pcb_READY();
        if (pcb != NULL)
        {
            sem_wait(&sem_grado_multiprogramacion);
            sem_wait(&planificacion_largo_plazo_liberada);
            if(pcb->estado == NEW) 
            {
                pcb = desencolar_pcb(pcbs_en_NEW);
                estado anterior = pcb->estado;
                pcb->estado = READY;

                // log minimo y obligatorio
                loggear_cambio_de_estado(pcb->PID, anterior, pcb->estado);

                ingresar_pcb_a_READY(pcb);
                
                pthread_mutex_lock(&mutex_multiprogramacion_auxiliar);
                grado_multiprogramacion_auxiliar--;
                pthread_mutex_unlock(&mutex_multiprogramacion_auxiliar);

                sem_post(&planificacion_largo_plazo_liberada);
            }
            else 
            {
                sem_post(&planificacion_largo_plazo_liberada);
                sem_post(&sem_grado_multiprogramacion);    
            }
        }
    }
}

t_pcb *obtener_siguiente_pcb_READY(void)
{
    t_pcb *pcb = NULL;
    pthread_mutex_lock(&mutex_lista_NEW);
    if(list_size(pcbs_en_NEW) > 0)
        pcb = list_get(pcbs_en_NEW, 0);
    pthread_mutex_unlock(&mutex_lista_NEW);
    return pcb;
}

void ingresar_pcb_a_READY(t_pcb *pcb)
{
    // sem_wait(&transicion_estados_corto_plazo_liberada);

    pthread_mutex_lock(&mutex_cola_READY);
    encolar_pcb(pcbs_en_READY, pcb);
    pthread_mutex_unlock(&mutex_cola_READY);

    // sem_wait(&transicion_estados_corto_plazo_liberada);

    // log minimo y obligatorio
    pthread_mutex_lock(&mutex_lista_PIDS);
    lista_PIDS = string_new();
    mostrar_PIDS(pcbs_en_READY);
    loggear_ingreso_a_READY(lista_PIDS, false);
    free(lista_PIDS);
    pthread_mutex_unlock(&mutex_lista_PIDS);

    sem_post(&hay_pcbs_READY);
}

void ingresar_pcb_a_NEW(t_pcb *pcb)
{
    pthread_mutex_lock(&mutex_lista_NEW);
    encolar_pcb(pcbs_en_NEW, pcb);
    pthread_mutex_unlock(&mutex_lista_NEW);

    // log minimo y obligatorio
    loggear_creacion_proceso(pcb->PID);
    sem_post(&hay_pcbs_NEW);
}

void inicializar_listas_planificacion(void)
{
    algoritmo = obtener_algoritmo_planificacion();

    pcbs_en_NEW = list_create();
    pcbs_en_READY = list_create();

    if (strcmp(obtener_algoritmo_planificacion(), "VRR") == 0)
        pcbs_en_aux_READY = list_create();

    pcbs_en_memoria = list_create();
    pcbs_en_BLOCKED = list_create();
    pcbs_en_EXIT = list_create();
}

void destruir_listas_planificacion(void)
{
    list_destroy_and_destroy_elements(pcbs_en_NEW, (void *)destruir_pcb);
    list_destroy_and_destroy_elements(pcbs_en_READY, (void *)destruir_pcb);

    if (strcmp(obtener_algoritmo_planificacion(), "VRR") == 0)
        list_destroy_and_destroy_elements(pcbs_en_aux_READY, (void *)destruir_pcb);

    list_destroy_and_destroy_elements(pcbs_en_memoria, (void *)destruir_pcb);
    destruir_pcb(pcb_en_EXEC);
    list_destroy_and_destroy_elements(pcbs_en_BLOCKED, (void *)destruir_pcb);
    list_destroy_and_destroy_elements(pcbs_en_EXIT, (void *)destruir_pcb);
    destruir_colas_de_recursos();
}

void inicializar_semaforos_planificacion(void)
{
    pthread_mutex_init(&mutex_lista_NEW, NULL);
    pthread_mutex_init(&mutex_cola_READY, NULL);
    pthread_mutex_init(&mutex_cola_aux_READY, NULL);
    pthread_mutex_init(&mutex_lista_BLOCKED, NULL);
    pthread_mutex_init(&mutex_pcb_EXEC, NULL);
    pthread_mutex_init(&mutex_lista_memoria, NULL);
    pthread_mutex_init(&mutex_lista_EXIT, NULL);
    pthread_mutex_init(&mutex_colas_de_recursos, NULL);
    pthread_mutex_init(&mutex_instancias_recursos, NULL);
    pthread_mutex_init(&mutex_lista_PIDS, NULL);
    pthread_mutex_init(&mutex_multiprogramacion_auxiliar, NULL);
    sem_init(&hay_pcbs_NEW, 0, 0);
    sem_init(&hay_pcbs_READY, 0, 0);
    sem_init(&sem_grado_multiprogramacion, 0, obtener_grado_multiprogramacion());
    sem_init(&planificacion_largo_plazo_liberada, 0, 1);
    sem_init(&planificacion_corto_plazo_liberada, 0, 1);
    sem_init(&desalojo_liberado, 0, 1);
    sem_init(&transicion_estados_corto_plazo_liberada, 0, 1);
}

void destruir_semaforos_planificacion(void)
{
    pthread_mutex_destroy(&mutex_lista_NEW);
    pthread_mutex_destroy(&mutex_cola_READY);
    pthread_mutex_destroy(&mutex_cola_aux_READY);
    pthread_mutex_destroy(&mutex_lista_BLOCKED);
    pthread_mutex_destroy(&mutex_pcb_EXEC);
    pthread_mutex_destroy(&mutex_lista_memoria);
    pthread_mutex_destroy(&mutex_lista_EXIT);
    pthread_mutex_destroy(&mutex_colas_de_recursos);
    pthread_mutex_destroy(&mutex_instancias_recursos);
    pthread_mutex_destroy(&mutex_lista_PIDS);
    pthread_mutex_destroy(&mutex_multiprogramacion_auxiliar);
    sem_close(&hay_pcbs_NEW);
    sem_close(&hay_pcbs_READY);
    sem_close(&sem_grado_multiprogramacion);
    sem_close(&planificacion_largo_plazo_liberada);
    sem_close(&planificacion_corto_plazo_liberada);
    sem_close(&desalojo_liberado);
    sem_close(&transicion_estados_corto_plazo_liberada);
}

void enviar_pcb_a_EXIT(t_pcb *pcb, int motivo)
{
    // es debatible que al detener la planificacion nos manden un pcb a EXIT
    sem_wait(&planificacion_largo_plazo_liberada);

    remover_pcb_de_listas_globales(pcb);

    if (pcb->estado != NEW)
    {
        pthread_mutex_lock(&mutex_multiprogramacion_auxiliar);
        bool grado_multiprogramacion_positivo = ++grado_multiprogramacion_auxiliar > 0;
        if (grado_multiprogramacion_positivo)
            sem_post(&sem_grado_multiprogramacion);
        pthread_mutex_unlock(&mutex_multiprogramacion_auxiliar);
    }
    
    pcb->estado = EXIT;

    pthread_mutex_lock(&mutex_lista_EXIT);
    list_add(pcbs_en_EXIT, pcb);
    pthread_mutex_unlock(&mutex_lista_EXIT);

    sem_post(&planificacion_largo_plazo_liberada);

    // log minimo y obligatorio
    loggear_fin_de_proceso(pcb->PID, motivo);

    liberar_recursos(pcb);

    // libero memoria
    t_paquete *paquete = crear_paquete(FINALIZAR_PROCESO_KERNEL);
    agregar_a_paquete_uint32(paquete, pcb->PID);
    enviar_paquete(paquete, conexion_kernel_memoria);
    eliminar_paquete(paquete);

    if (recibir_operacion(conexion_kernel_memoria) == OK)
        log_info(logger_propio, "Finalizacion de proceso exitosa en memoria.");

    else
        log_info(logger_propio, "Finalizacion de proceso fallida en memoria.");
}

void remover_pcb_de_listas_globales(t_pcb *pcb)
{
    int valor_semaforo;
    pthread_mutex_lock(&mutex_lista_memoria);
    list_remove_element(pcbs_en_memoria, pcb);
    pthread_mutex_unlock(&mutex_lista_memoria);

    switch (pcb->estado)
    {
    case NEW:
        pthread_mutex_lock(&mutex_lista_NEW);
        list_remove_element(pcbs_en_NEW, pcb);
        pthread_mutex_unlock(&mutex_lista_NEW);
        
        break;

    case READY:
        pthread_mutex_lock(&mutex_cola_READY);
        list_remove_element(pcbs_en_READY, pcb);
        pthread_mutex_unlock(&mutex_cola_READY);

        if (strcmp("VRR", algoritmo) == 0)
        {
            pthread_mutex_lock(&mutex_cola_aux_READY);
            list_remove_element(pcbs_en_aux_READY, pcb);
            pthread_mutex_unlock(&mutex_cola_aux_READY);
        }

        // para que el wait no se quede trabado si ya es cero
        sem_getvalue(&hay_pcbs_READY, &valor_semaforo);
        if (--valor_semaforo >= 0)
            sem_wait(&hay_pcbs_READY);

        break;

    case EXEC:
        pthread_mutex_lock(&mutex_pcb_EXEC);
        pcb_en_EXEC = NULL;
        pthread_mutex_unlock(&mutex_pcb_EXEC);
        break;

    case BLOCKED:
        pthread_mutex_lock(&mutex_lista_BLOCKED);
        list_remove_element(pcbs_en_BLOCKED, pcb);
        pthread_mutex_unlock(&mutex_lista_BLOCKED);
        break;

    default:
        log_error(logger_propio, "Error al remover un pcb de una lista global.");
        break;
    }
}