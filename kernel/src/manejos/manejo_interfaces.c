#include "manejo_interfaces.h"

t_list *interfaces;
pthread_mutex_t mutex_interfaces;
int operaciones_stdout[1] = {IO_STDOUT_WRITE};
int operaciones_stdin[1] = {IO_STDIN_READ};
int operaciones_generic[1] = {IO_GEN_SLEEP};
int operaciones_fs[5] = {IO_FS_CREATE, IO_FS_DELETE, IO_FS_TRUNCATE, IO_FS_WRITE, IO_FS_READ};

void ejecutar_espera_interfaces(void)
{
    inicializar_interfaces();
    while (1)
    {
        int fd_cliente = esperar_cliente(logger_propio, servidor_kernel_fd);
        int operacion = recibir_operacion(fd_cliente);

        switch (operacion)
        {
        case CONEXION_INTERFAZ_KERNEL:
            t_list *interfaz = recibir_paquete(fd_cliente);
            char *nombre = (char *)list_get(interfaz, 0);
            char *tipo = (char *)list_get(interfaz, 1);
            agregar_a_lista_io_global(nombre, tipo, fd_cliente);
            log_info(logger_propio, "Se ha conectado la interfaz %s del tipo %s", nombre, tipo);
            list_destroy_and_destroy_elements(interfaz, free);
            break;
        default:
            log_info(logger_propio, "Se recibio una operacion no valida de la interfaz.");
            break;
        }
    }
}

void inicializar_interfaces()
{
    interfaces = list_create();
    pthread_mutex_init(&mutex_interfaces, NULL);
}

void agregar_a_lista_io_global(char *nombre, char *tipo, int fd)
{
    t_io *interfaz = crear_interfaz(nombre, tipo, fd);

    pthread_mutex_lock(&mutex_interfaces);
    list_add(interfaces, (void *)interfaz);
    pthread_mutex_unlock(&mutex_interfaces);

    // creo hilo para atender cada interfaz se conecta
    pthread_t hilo_interfaz;
    if (pthread_create(&hilo_interfaz, NULL, (void *)atender_interfaz, (void *)interfaz) != 0)
        log_error(logger_propio, "Error creando el hilo para atender una interfaz");

    interfaz->hilo_interfaz = hilo_interfaz;

    pthread_detach(hilo_interfaz);
}

void manejador_interfaz(t_pcb *pcb, t_list *parametros)
{
    char *nombre_interfaz = (char *)list_remove(parametros, 0);
    char *tipo_de_operacion = (char *)list_get(parametros, 0);

    t_io *io = buscar_interfaz(nombre_interfaz); // verifico que se conecto y que sigue conectada

    if (io != NULL)
    {
        if (puede_realizar_operacion(io, tipo_de_operacion)) // verifico que puede hacer el tipo de operación
        {
            // agrego pcb a bloqueados
            // sem_wait(&transicion_estados_corto_plazo_liberada);
            pthread_mutex_lock(&mutex_lista_BLOCKED);
            list_add(pcbs_en_BLOCKED, (void *)pcb);
            pthread_mutex_unlock(&mutex_lista_BLOCKED);

            // logs minimos y obligatorios
            pcb->estado = BLOCKED;
            // sem_post(&transicion_estados_corto_plazo_liberada);
            loggear_cambio_de_estado(pcb->PID, EXEC, BLOCKED);
            loggear_motivo_de_bloqueo(pcb->PID, nombre_interfaz);

            // creo la estructura para guardar el pcb y los parametros
            t_proceso_bloqueado *proceso_bloqueado = crear_proceso_bloqueado(pcb, parametros);

            // agrego a la lista de bloqueados de la io
            pthread_mutex_lock(&io->cola_bloqueados);
            list_add(io->procesos_bloqueados, (void *)proceso_bloqueado);
            pthread_mutex_unlock(&io->cola_bloqueados);

            sem_post(&io->procesos_en_cola);
        }
        else
        {
            list_destroy_and_destroy_elements(parametros, free);
            log_info(logger_propio, "La interfaz %s no puede realizar la operacion %s.", nombre_interfaz, tipo_de_operacion);
            enviar_pcb_a_EXIT(pcb, INVALID_INTERFACE);
        }
    }
    else
    {
        list_destroy_and_destroy_elements(parametros, free);
        log_info(logger_propio, "La interfaz %s no existe.", nombre_interfaz);
        enviar_pcb_a_EXIT(pcb, INVALID_INTERFACE);
    }

    free(nombre_interfaz);
}

bool puede_realizar_operacion(t_io *io, char *operacion)
{
    if (strcmp(io->tipo, "STDOUT") == 0)
    {
        return operaciones_stdout[0] == atoi(operacion);
    }
    else if (strcmp(io->tipo, "STDIN") == 0)
    {
        return operaciones_stdin[0] == atoi(operacion);
    }
    else if (strcmp(io->tipo, "GENERICA") == 0)
    {
        return operaciones_generic[0] == atoi(operacion);
    }
    else if (strcmp(io->tipo, "DIALFS") == 0)
    {
        for (int i = 0; i < 5; i++)
        {
            if (operaciones_fs[i] == atoi(operacion))
            {
                return true;
            }
        }
        return false;
    }
    else
    {
        return false;
    }
}

void atender_interfaz(void *interfaz)
{
    t_io *io = (t_io *)interfaz;

    while (1)
    {
        sem_wait(&io->procesos_en_cola);

        if(io->desconectada){break;}

        pthread_mutex_lock(&io->cola_bloqueados);
        t_proceso_bloqueado *proceso = (t_proceso_bloqueado *)list_remove(io->procesos_bloqueados, 0);
        pthread_mutex_unlock(&io->cola_bloqueados);

        // creamos el paquete y lo mandamos a la interfaz
        char *op_a_realizar = (char *)list_remove(proceso->parametros, 0);
        int op_interfaz = atoi(op_a_realizar);
        t_paquete *p_interfaz = crear_paquete(op_interfaz);
        uint32_t *pid = &proceso->pcb->PID;
        agregar_a_paquete(p_interfaz, (void *)pid, sizeof(uint32_t));
        agregar_parametros_a_paquete(p_interfaz, proceso->parametros);
        enviar_paquete(p_interfaz, io->fd);

        if (recibir_operacion(io->fd) == OK)
        {
            if (proceso->pcb->estado == BLOCKED)
            {
                sem_wait(&transicion_estados_corto_plazo_liberada);
                pthread_mutex_lock(&mutex_lista_BLOCKED);
                list_remove_element(pcbs_en_BLOCKED, proceso->pcb);
                pthread_mutex_unlock(&mutex_lista_BLOCKED);

                encolar_pcb_ready_segun_algoritmo(proceso->pcb);
                sem_post(&transicion_estados_corto_plazo_liberada);
            }
        }
        else
        {
            log_error(logger_propio, "La interfaz de entrada salida %s no respondio correctamente.", io->nombre);
        }

        free(op_a_realizar);
        eliminar_proceso_bloqueado(proceso);
        eliminar_paquete(p_interfaz);
    }

    liberar_interfaz(io);
}

// funciones de manejo de t_proceso_bloqueado

void eliminar_proceso_bloqueado(t_proceso_bloqueado *proceso)
{
    list_destroy_and_destroy_elements(proceso->parametros, free);
    free(proceso);
}

t_proceso_bloqueado *crear_proceso_bloqueado(t_pcb *pcb, t_list *parametros)
{
    t_proceso_bloqueado *proceso_bloqueado = malloc_or_die(sizeof(t_proceso_bloqueado), "No se pudo asignar memoria a proceso_bloqueado");
    proceso_bloqueado->parametros = parametros;
    proceso_bloqueado->pcb = pcb;

    return proceso_bloqueado;
}

// funciones de manejo de t_io

t_io *crear_interfaz(char *nombre, char *tipo, int fd)
{
    t_io *interfaz = malloc_or_die(sizeof(t_io), "No se pudo reservar memoria para interfaz");
    interfaz->fd = fd;
    interfaz->nombre = malloc(strlen(nombre) + 1);
    interfaz->tipo = malloc(strlen(tipo) + 1);
    interfaz->desconectada = false;
    strcpy(interfaz->nombre, nombre);
    strcpy(interfaz->tipo, tipo);
    interfaz->procesos_bloqueados = list_create();
    pthread_mutex_init(&(interfaz->cola_bloqueados), NULL);
    sem_init(&(interfaz->procesos_en_cola), 0, 0);

    return interfaz;
}

t_io *buscar_interfaz(char *nombre_io)
{
    for (int i = 0; i < list_size(interfaces); i++)
    {
        t_io *io = list_get(interfaces, i);
        if (strcmp(io->nombre, nombre_io) == 0)
        {
            if (socket_desconectado(io->fd))
            {
                io->desconectada = true;
                sem_post(&(io->procesos_en_cola));
                return NULL;
            }
            else
                return io;
        }
    }
    return NULL;
}

void liberar_interfaz(t_io *io)
{
    pthread_mutex_lock(&mutex_interfaces);
    list_remove_element(interfaces, io);
    pthread_mutex_unlock(&mutex_interfaces);
    free(io->nombre);
    free(io->tipo);
    liberar_procesos_io(io->procesos_bloqueados);
    free(io->procesos_bloqueados);
    pthread_mutex_destroy(&(io->cola_bloqueados));
    sem_destroy(&(io->procesos_en_cola));
    free(io);
}

void liberar_procesos_io(t_list *procesos_io)
{
    int size = list_size(procesos_io);
    for (int i = 0; i < size; i++)
    {
        t_proceso_bloqueado *proceso = list_remove(procesos_io, i);
        eliminar_proceso_bloqueado(proceso);
    }
}

bool socket_desconectado(int socket)
{
    fd_set read_fds;
    struct timeval timeout;
    int result;

    // Configurar el conjunto de descriptores de archivo
    FD_ZERO(&read_fds);
    FD_SET(socket, &read_fds);

    // Configurar el tiempo de espera a cero para que no espere
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    // Usar select para ver si hay datos disponibles en el socket
    result = select(socket + 1, &read_fds, NULL, NULL, &timeout);

    if (result == -1)
    {
        perror("select");
        return true; // Error en select, considerar el socket desconectado
    }
    else if (result == 0)
    {
        // No hay datos disponibles, el socket sigue activo
        return false;
    }
    else
    {
        // El socket está listo para lectura, verificar si está desconectado
        char buffer[1];
        ssize_t bytes_recibidos = recv(socket, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT);
        if (bytes_recibidos == 0)
        {
            // Conexión cerrada ordenadamente
            return true;
        }
        else if (bytes_recibidos < 0)
        {
            // Error en recv
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                // No hay datos disponibles, el socket sigue activo
                return false;
            }
            else
            {
                perror("recv");
                return true; // Otro error
            }
        }
        else
        {
            // Datos disponibles, el socket sigue activo
            return false;
        }
    }
}