#include "consola.h"

t_comando comandos[] = {
    {"EJECUTAR_SCRIPT", ejecutar_script},
    {"INICIAR_PROCESO", iniciar_proceso},
    {"FINALIZAR_PROCESO", finalizar_proceso},
    {"DETENER_PLANIFICACION", detener_planificacion},
    {"INICIAR_PLANIFICACION", reanudar_planificacion},
    {"MULTIPROGRAMACION", cambiar_grado_multiprogramacion},
    {"PROCESO_ESTADO", listar_procesos_por_cada_estado},
    {"RECURSOS", listar_recursos},
    {"GRADO_MULTIPROGRAMACION", multiprogramacion_actual},
};

void consola_interactiva(void)
{
    char *leido = readline("> ");
    char *token = strtok(leido, " ");
    add_history(leido);

    while (1)
    {
        buscar_y_ejecutar_comando(token);
        free(leido);

        leido = readline("> ");
        add_history(leido);
        token = strtok(leido, " ");
    }

    free(leido);
}

void buscar_y_ejecutar_comando(char *token)
{
    if (token != NULL)
    {
        int i;
        for (i = 0; i < sizeof(comandos) / sizeof(t_comando); i++)
        {
            if (strcmp(token, comandos[i].nombre) == 0)
            {
                token = strtok(NULL, " ");
                comandos[i].funcion_de_comando(token);
                break;
            }
        }

        if (i == sizeof(comandos) / sizeof(t_comando))
        {
            log_error(logger_propio, "Comando invalido.");
        }
    }
}

void listar_procesos_por_estado(char *estado, t_list *lista)
{
    printf("%s: ", estado);
    for (int i = 0; i < list_size(lista); i++)
    {
        t_pcb *pcb = (t_pcb *)list_get(lista, i);
        printf(" %d ", pcb->PID);
    }
    printf("\n");
}

void cambiar_valor_de_semaforo(sem_t *sem, int valor_resultante)
{
    if (valor_resultante < 0)
        log_error(logger_propio, "Grado de multiprogramacion no valido");
    else
    {
        int valor_actual;
        // Obtener el valor actual del semáforo
        sem_getvalue(sem, &valor_actual);

        // Realizar operaciones para ajustar el semáforo al valor deseado
        while (valor_actual > valor_resultante)
        {
            sem_wait(sem);
            valor_actual--;
        }
        while (valor_actual < valor_resultante)
        {
            sem_post(sem);
            valor_actual++;
        }
    }
}