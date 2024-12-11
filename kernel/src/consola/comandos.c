#include "consola.h"

bool planificacion_detenida = false;

void ejecutar_script(char *path_relativo)
{
    FILE *archivo;

    if (path_relativo != NULL)
    {
        char *path_kernel = obtener_path_kernel();
        size_t tam = strlen(path_kernel) + strlen(path_relativo) + 1;
        char *path_absoluto = malloc_or_die(tam, "No se pudo reservar espacio para la ruta absoluta");
        strcpy(path_absoluto, path_kernel);
        strcat(path_absoluto, path_relativo);

        archivo = fopen(path_absoluto, "r");
        if (archivo == NULL)
        {
            free(path_absoluto);
            log_error(logger_propio, "Error al abrir el archivo.");
            return;
        }

        char comando[256];
        while (!feof(archivo))
        {
            fgets(comando, sizeof(comando), archivo);

            // Eliminar el carácter de nueva línea si está presente
            size_t longitud = strlen(comando);
            if (longitud > 0 && comando[longitud - 1] == '\n')
                comando[longitud - 1] = '\0';

            char *token = strtok(comando, " ");
            buscar_y_ejecutar_comando(token);
        }

        free(path_absoluto);
        fclose(archivo);
    }
    else
    {
        log_error(logger_propio, "No se ingreso un path para iniciar el proceso.");
    }
}

void iniciar_proceso(char *path)
{
    if (path != NULL)
    {
        t_pcb *pcb = crear_pcb();

        t_paquete *paquete = crear_paquete(CREAR_PROCESO_KERNEL);
        agregar_a_paquete_uint32(paquete, pcb->PID);
        agregar_a_paquete_string(paquete, path);
        enviar_paquete(paquete, conexion_kernel_memoria);
        eliminar_paquete(paquete);

        if (recibir_operacion(conexion_kernel_memoria) == OK)
        {
            log_info(logger_propio, "Creacion de proceso exitosa en memoria.");
            ingresar_pcb_a_NEW(pcb);
            
            pthread_mutex_lock(&mutex_lista_memoria);
            list_add(pcbs_en_memoria, pcb);
            pthread_mutex_unlock(&mutex_lista_memoria);
        }
        else
        {
            destruir_pcb(pcb);
            procesos_creados--;
            log_info(logger_propio, "Creacion de proceso fallida en memoria.");
        }
    }
    else
    {
        log_error(logger_propio, "No se ingreso un path para iniciar el proceso.");
    }
}

void finalizar_proceso(char *PID)
{
    t_pcb *pcb = NULL;

    if (PID != NULL)
    {
        pcb = buscar_pcb_por_PID(pcbs_en_memoria, (uint32_t)atoi(PID));

        if (pcb == NULL)
            log_error(logger_propio, "No existe un PCB con ese PID.");
        else if (pcb->estado == EXEC)
            enviar_interrupcion("EXIT");
        else
            enviar_pcb_a_EXIT(pcb, INTERRUPTED_BY_USER);
    }
    else
    {
        log_error(logger_propio, "No se ingreso un PID para finalizar el proceso.");
    }
}

void detener_planificacion(void)
{
    if (!planificacion_detenida)
    {
        planificacion_detenida = true;
        cambiar_valor_de_semaforo(&planificacion_largo_plazo_liberada, 0);
        cambiar_valor_de_semaforo(&planificacion_corto_plazo_liberada, 0);
        sem_wait(&desalojo_liberado);
        sem_wait(&transicion_estados_corto_plazo_liberada);
        sem_post(&planificacion_pausada);
    }
    else
        log_error(logger_propio, "La planificacion ya esta detenida.");
}

void reanudar_planificacion(void)
{
    if (planificacion_detenida)
    {
        sem_post(&planificacion_largo_plazo_liberada);
        sem_post(&planificacion_corto_plazo_liberada);
        sem_post(&desalojo_liberado);
        sem_post(&transicion_estados_corto_plazo_liberada);
        planificacion_detenida = false;
    }
    else
        log_error(logger_propio, "La planificacion no esta detenida.");
}

void cambiar_grado_multiprogramacion(char *valor_deseado)
{
    if (valor_deseado == NULL)
        log_error(logger_propio, "No se indico el numero de grado de multiprogramacion");
    else 
    {
        int valor_deseado_entero = atoi(valor_deseado);
        int valor_config = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");

        if (valor_deseado_entero < 0)
            log_error(logger_propio, "Se indico un grado de multiprogramacion negativo");
        else if (valor_deseado_entero == valor_config)
            log_info(logger_propio, "Ya se tiene un grado de multiprogramacion de %d", valor_config);
        else
        {
            int valor_resultante, valor_actual;
            sem_getvalue(&sem_grado_multiprogramacion, &valor_actual);
            
            valor_resultante = valor_deseado_entero - valor_config + valor_actual;

            pthread_mutex_lock(&mutex_multiprogramacion_auxiliar);
            int valor_auxiliar_anterior = grado_multiprogramacion_auxiliar;
            grado_multiprogramacion_auxiliar = valor_auxiliar_anterior < 0 ? valor_resultante + valor_auxiliar_anterior : valor_resultante;
            valor_resultante = grado_multiprogramacion_auxiliar >= 0 ? grado_multiprogramacion_auxiliar : 0;
            pthread_mutex_unlock(&mutex_multiprogramacion_auxiliar);
            
            // y si es negativo el valor resultante y el auxiliar??
            //valor_resultante = valor_auxiliar_anterior < 0 ? valor_deseado_entero + valor_auxiliar_anterior : valor_resultante;
            //valor_resultante = valor_resultante < 0 ? 0 : valor_resultante;
            // valor_resultante = valor_auxiliar_anterior < 0 && valor_resultante < 0 ? valor_deseado_entero + valor_auxiliar_anterior : valor_resultante;

            log_info(logger_propio, "Valor resultante: %d", valor_resultante);

            cambiar_valor_de_semaforo(&sem_grado_multiprogramacion, valor_resultante);
            config_set_value(config, "GRADO_MULTIPROGRAMACION", valor_deseado);
            config_save(config);
        }
    }
    
}

void listar_procesos_por_cada_estado(void)
{
    listar_procesos_por_estado("NEW", pcbs_en_NEW);
    listar_procesos_por_estado("READY", pcbs_en_READY);
    strcmp(obtener_algoritmo_planificacion(), "VRR") == 0 ? listar_procesos_por_estado("READY Prioridad", pcbs_en_aux_READY) : 1;
    char *pid_pcb_exec = pcb_en_EXEC != NULL ? string_itoa(pcb_en_EXEC->PID) : " ";
    printf("EXEC: %s\n", pid_pcb_exec);
    listar_procesos_por_estado("BLOCKED", pcbs_en_BLOCKED);
    listar_procesos_por_estado("EXIT", pcbs_en_EXIT);

    pcb_en_EXEC != NULL ? free(pid_pcb_exec) : 1;
}

void listar_recursos(void)
{
    for (int i = 0; i < cantidad_recursos(); i++)
    {
        printf("Cantidad de instancias de %s: %d \n", nombres_recursos[i], instancias_recursos[i]);
    }
}

void multiprogramacion_actual(void)
{
    int valor;
    sem_getvalue(&sem_grado_multiprogramacion, &valor);
    log_info(logger_propio, "Grado multiprogramacion actual real: %d", valor);
    pthread_mutex_lock(&mutex_multiprogramacion_auxiliar);
    log_info(logger_propio, "Grado multiprogramacion actual auxiliar: %d", grado_multiprogramacion_auxiliar);
    pthread_mutex_unlock(&mutex_multiprogramacion_auxiliar);
}