#include "ciclo_de_instruccion.h"

pthread_mutex_t mutex_interrupt;
char *motivo_interrupcion;
bool continua_ejecucion = true;
bool hay_interrupcion = false;
// bool enviar_interrupcion = false;
t_contexto *contexto;
t_contexto **contexto_original;

void destruir_instruccion(t_instruccion *instruccion)
{
    list_destroy_and_destroy_elements(instruccion->parametros, free);
    list_destroy_and_destroy_elements(instruccion->direcciones_fisicas, free);
    free(instruccion);
}

void ciclo_de_instruccion(t_contexto **contexto_a_ejecutar)
{
    contexto_original = contexto_a_ejecutar;
    contexto = *contexto_a_ejecutar;
    continua_ejecucion = true;
    hay_interrupcion = false;

    while (continua_ejecucion)
    {
        char *instruccion_leida = fetch();
        t_instruccion *instruccion = decode(instruccion_leida);
        free(instruccion_leida);
        execute(instruccion);
        check_interrupt(instruccion);
        destruir_instruccion(instruccion);
    }
}

// -------------------- FETCH -------------------- //

char *fetch()
{
    solicitar_lectura_de_instruccion(contexto->PID, obtener_valor_registro(contexto->registros_cpu, "PC"));
    char *instruccion_string = recibir_instruccion_string();
    loggear_fetch_instrucccion(contexto->PID, obtener_valor_registro(contexto->registros_cpu, "PC"));

    // actualizo PC
    uint32_t valor = obtener_valor_registro(contexto->registros_cpu, "PC");
    valor++;
    set("PC", valor);

    return instruccion_string;
}

char *recibir_instruccion_string()
{
    if (recibir_operacion(conexion_cpu_memoria) == INSTRUCCION)
    {
        char *instruccion_string = recibir_string(conexion_cpu_memoria);

        log_info(logger_propio, "String recibido de memoria: %s", instruccion_string);
        return instruccion_string;
    }
    log_info(logger_propio, "Error en recibir_instruccion_string()");
    return "EXIT";
}

t_instruccion *convertir_string_a_instruccion_beta(char *instruccion_string)
{
    t_instruccion *instruccion = malloc_or_die(sizeof(t_instruccion), "Error al asignar memoria para la instrucción");
    char **instruccion_array = string_split(instruccion_string, " ");

    instruccion->parametros = list_create();
    instruccion->direcciones_fisicas = list_create();

    instruccion->id = string_id_to_enum_id(instruccion_array[0]);

    int i = 1;
    while (instruccion_array[i] != NULL)
    {
        list_add(instruccion->parametros, string_duplicate(instruccion_array[i]));
        i++;
    }

    for (i = 0; instruccion_array[i] != NULL; i++)
        free(instruccion_array[i]);
    free(instruccion_array);

    return instruccion;
}

t_id string_id_to_enum_id(char *id_string)
{
    t_id id_enum;

    if (strcmp(id_string, "SET") == 0)
        id_enum = SET;
    else if (strcmp(id_string, "MOV_IN") == 0)
        id_enum = MOV_IN;
    else if (strcmp(id_string, "MOV_OUT") == 0)
        id_enum = MOV_OUT;
    else if (strcmp(id_string, "SUM") == 0)
        id_enum = SUM;
    else if (strcmp(id_string, "SUB") == 0)
        id_enum = SUB;
    else if (strcmp(id_string, "JNZ") == 0)
        id_enum = JNZ;
    else if (strcmp(id_string, "RESIZE") == 0)
        id_enum = RESIZE;
    else if (strcmp(id_string, "COPY_STRING") == 0)
        id_enum = COPY_STRING;
    else if (strcmp(id_string, "WAIT") == 0)
        id_enum = WAIT;
    else if (strcmp(id_string, "SIGNAL") == 0)
        id_enum = SIGNAL;
    else if (strcmp(id_string, "IO_GEN_SLEEP") == 0)
        id_enum = IO_GEN_SLEEP;
    else if (strcmp(id_string, "IO_STDIN_READ") == 0)
        id_enum = IO_STDIN_READ;
    else if (strcmp(id_string, "IO_STDOUT_WRITE") == 0)
        id_enum = IO_STDOUT_WRITE;
    else if (strcmp(id_string, "IO_FS_CREATE") == 0)
        id_enum = IO_FS_CREATE;
    else if (strcmp(id_string, "IO_FS_DELETE") == 0)
        id_enum = IO_FS_DELETE;
    else if (strcmp(id_string, "IO_FS_TRUNCATE") == 0)
        id_enum = IO_FS_TRUNCATE;
    else if (strcmp(id_string, "IO_FS_WRITE") == 0)
        id_enum = IO_FS_WRITE;
    else if (strcmp(id_string, "IO_FS_READ") == 0)
        id_enum = IO_FS_READ;
    else
        id_enum = EXIT;

    return id_enum;
}

// -------------------- DECODE -------------------- //
t_instruccion *decode(char *instruccion_leida)
{
    t_instruccion *instruccion = convertir_string_a_instruccion_beta(instruccion_leida);

    // traducir direcciones lógicas si corresponde
    uint32_t direccion_logica = 0;
    int tamanio_a_operar = 0;

    switch (instruccion->id)
    {
    case IO_STDIN_READ:
    case IO_STDOUT_WRITE:
        direccion_logica = obtener_valor_registro(contexto->registros_cpu, (char *)list_get(instruccion->parametros, 1));
        tamanio_a_operar = obtener_valor_registro(contexto->registros_cpu, (char *)list_get(instruccion->parametros, 2));
        agregar_direcciones_fisicas(instruccion, direccion_logica, tamanio_a_operar);
        break;

    case MOV_IN:
        direccion_logica = obtener_valor_registro(contexto->registros_cpu, (char *)list_get(instruccion->parametros, 1));
        tamanio_a_operar = tamanio_de_registro((char *)list_get(instruccion->parametros, 0));
        agregar_direcciones_fisicas(instruccion, direccion_logica, tamanio_a_operar);
        break;

    case MOV_OUT:
        direccion_logica = obtener_valor_registro(contexto->registros_cpu, (char *)list_get(instruccion->parametros, 0));
        tamanio_a_operar = tamanio_de_registro((char *)list_get(instruccion->parametros, 1));
        agregar_direcciones_fisicas(instruccion, direccion_logica, tamanio_a_operar);
        break;

    case COPY_STRING:
        direccion_logica = obtener_valor_registro(contexto->registros_cpu, "SI");
        tamanio_a_operar = atoi((char *)list_get(instruccion->parametros, 0));
        agregar_direcciones_fisicas(instruccion, direccion_logica, tamanio_a_operar);
        direccion_logica = obtener_valor_registro(contexto->registros_cpu, "DI");
        agregar_direcciones_fisicas(instruccion, direccion_logica, tamanio_a_operar);
        break;

    case IO_FS_WRITE:
    case IO_FS_READ:
        direccion_logica = obtener_valor_registro(contexto->registros_cpu, list_get(instruccion->parametros, 2));
        tamanio_a_operar = obtener_valor_registro(contexto->registros_cpu, list_get(instruccion->parametros, 3));
        agregar_direcciones_fisicas(instruccion, direccion_logica, tamanio_a_operar);
        break;

    default:
        break;
    }

    return instruccion;
}

void agregar_direcciones_fisicas(t_instruccion *instruccion, uint32_t direccion_logica, int tamanio_a_operar)
{
    if (tamanio_a_operar > 0)
    {
        int pagina, desplazamiento, bytes_disponibles_en_marco, bytes_a_operar;
        uint32_t direccion_fisica = 0;

        pagina = floor(direccion_logica / tamanio_pagina);
        desplazamiento = direccion_logica - pagina * tamanio_pagina;
        direccion_fisica = calcular_direccion_fisica(tlb, contexto->PID, direccion_logica);
        bytes_disponibles_en_marco = tamanio_pagina - desplazamiento;
        bytes_a_operar = tamanio_a_operar > bytes_disponibles_en_marco ? bytes_disponibles_en_marco : tamanio_a_operar;
        list_add(instruccion->direcciones_fisicas, memcpy(malloc(sizeof(uint32_t)), &direccion_fisica, sizeof(uint32_t)));
        list_add(instruccion->direcciones_fisicas, memcpy(malloc(sizeof(int)), &bytes_a_operar, sizeof(int)));

        if (bytes_disponibles_en_marco < tamanio_a_operar)
        {
            direccion_logica += bytes_disponibles_en_marco;
            tamanio_a_operar -= bytes_disponibles_en_marco;
            int cantidad_pags_necesarias = (tamanio_a_operar + tamanio_pagina - 1) / tamanio_pagina;
            int bytes_a_operar;

            for (int i = 0; i < cantidad_pags_necesarias; i++)
            {
                direccion_fisica = calcular_direccion_fisica(tlb, contexto->PID, direccion_logica);
                bytes_a_operar = tamanio_a_operar > tamanio_pagina ? tamanio_pagina : tamanio_a_operar;
                list_add(instruccion->direcciones_fisicas, memcpy(malloc(sizeof(uint32_t)), &direccion_fisica, sizeof(uint32_t)));
                list_add(instruccion->direcciones_fisicas, memcpy(malloc(sizeof(int)), &bytes_a_operar, sizeof(int)));
                direccion_logica += tamanio_pagina;
                tamanio_a_operar -= tamanio_pagina;
            }
        }
    }
}

uint8_t tamanio_de_registro(char *registro)
{
    if (strlen(registro) == 3 || !strcmp(registro, "SI") || !strcmp(registro, "DI") || !strcmp(registro, "PC")) // caso registros de 4 bytes
        return 4;
    else if (strlen(registro) == 2) // caso registros de 1 bytes
        return 1;
    return 0;
}

// -------------------- EXECUTE -------------------- //

void execute(t_instruccion *instruccion)
{
    switch (instruccion->id)
    {
    case SET:
        set(list_get(instruccion->parametros, 0), atoi(list_get(instruccion->parametros, 1)));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: SET - <%s %s>", contexto->PID, (char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1));
        break;

    case SUM:
        sum((char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: SUM - <%s %s>", contexto->PID, (char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1));
        break;

    case SUB:
        sub((char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: SUB - <%s %s>", contexto->PID, (char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1));
        break;

    case JNZ:
        jnz(list_get(instruccion->parametros, 0), atoi(list_get(instruccion->parametros, 1)));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: JNZ - <%s %s>", contexto->PID, (char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1));
        break;

    case IO_GEN_SLEEP:
        io_gen_sleep((char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: IO_GEN_SLEEP - <%s %s> ", contexto->PID, (char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1));
        break;

    case IO_STDIN_READ:
        io_stdin_read((char *)list_get(instruccion->parametros, 0), instruccion->direcciones_fisicas, (char *)list_get(instruccion->parametros, 2));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: IO_STDIN_READ - <%s %s %s> ", contexto->PID, (char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1), (char *)list_get(instruccion->parametros, 2));
        break;

    case IO_STDOUT_WRITE:
        io_stdout_write((char *)list_get(instruccion->parametros, 0), instruccion->direcciones_fisicas, (char *)list_get(instruccion->parametros, 2));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: IO_STDOUT_WRITE - <%s %s %s> ", contexto->PID, (char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1), (char *)list_get(instruccion->parametros, 2));
        break;

    case MOV_IN:
        mov_in((char *)list_get(instruccion->parametros, 0), instruccion->direcciones_fisicas);
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: MOV_IN - <%s %s> ", contexto->PID, (char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1));
        break;

    case MOV_OUT:
        mov_out((char *)list_get(instruccion->parametros, 1), instruccion->direcciones_fisicas);
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: MOV_OUT - <%s %s> ", contexto->PID, (char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1));
        break;

    case RESIZE:
        resize(atoi((char *)list_get(instruccion->parametros, 0)));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: RESIZE - <%s> ", contexto->PID, (char *)list_get(instruccion->parametros, 0));
        break;

    case COPY_STRING:
        copy_string(atoi((char *)list_get(instruccion->parametros, 0)), instruccion->direcciones_fisicas);
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: COPY_STRING - <%s> ", contexto->PID, (char *)list_get(instruccion->parametros, 0));
        break;

    case WAIT:
        wait((char *)list_get(instruccion->parametros, 0));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: WAIT - <%s> ", contexto->PID, (char *)list_get(instruccion->parametros, 0));
        break;

    case SIGNAL:
        signal((char *)list_get(instruccion->parametros, 0));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: SIGNAL - <%s> ", contexto->PID, (char *)list_get(instruccion->parametros, 0));
        break;

    case IO_FS_CREATE:
        io_fs_create((char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: IO_FS_CREATE - <%s %s>", contexto->PID, (char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1));
        break;

    case IO_FS_DELETE:
        io_fs_delete(list_get(instruccion->parametros, 0), list_get(instruccion->parametros, 1));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: IO_FS_DELETE - <%s %s>", contexto->PID, (char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1));
        break;

    case IO_FS_TRUNCATE:
        io_fs_truncate(list_get(instruccion->parametros, 0), list_get(instruccion->parametros, 1), list_get(instruccion->parametros, 2));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: IO_FS_TRUNCATE - <%s %s %s>", contexto->PID, (char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1), (char *)list_get(instruccion->parametros, 2));
        break;

    case IO_FS_WRITE:
        io_fs_write(list_get(instruccion->parametros, 0), list_get(instruccion->parametros, 1), instruccion->direcciones_fisicas, list_get(instruccion->parametros, 3), list_get(instruccion->parametros, 4));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: IO_FS_WRITE - <%s %s %s %s %s>", contexto->PID, (char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1), (char *)list_get(instruccion->parametros, 2), (char *)list_get(instruccion->parametros, 3), (char *)list_get(instruccion->parametros, 4));
        break;

    case IO_FS_READ:
        io_fs_read(list_get(instruccion->parametros, 0), list_get(instruccion->parametros, 1), instruccion->direcciones_fisicas, list_get(instruccion->parametros, 3), list_get(instruccion->parametros, 4));
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: IO_FS_READ - <%s %s %s %s %s>", contexto->PID, (char *)list_get(instruccion->parametros, 0), (char *)list_get(instruccion->parametros, 1), (char *)list_get(instruccion->parametros, 2), (char *)list_get(instruccion->parametros, 3), (char *)list_get(instruccion->parametros, 4));
        break;

    case EXIT:
        exit_inst();
        log_info(logger_obligatorio, "PID: <%d> - Ejecutando: EXIT", contexto->PID);
        break;

    default:
        break;
    }
}

bool instruccion_bloqueante(t_id id_instruccion)
{
    switch (id_instruccion)
    {
    case IO_GEN_SLEEP:
    case IO_STDIN_READ:
    case IO_STDOUT_WRITE:
    case IO_FS_CREATE:
    case IO_FS_DELETE:
    case IO_FS_TRUNCATE:
    case IO_FS_WRITE:
    case IO_FS_READ:
    case EXIT:
        return true;
        break;
    default:
        return false;
        break;
    }
}

// -------------------- CHECK INTERRUPT -------------------- //

void check_interrupt(t_instruccion *instruccion)
{
    if (instruccion->id == WAIT || instruccion->id == SIGNAL)
    {
        if (recibir_operacion(conexion_cpu_kernel_dispatch) != CONTEXTO_EJECUCION)
        {
            continua_ejecucion = false;
            return;
        }

        uint32_t pid_viejo = contexto->PID;
        destruir_contexto(contexto);
        *contexto_original = recibir_contexto(conexion_cpu_kernel_dispatch);
        contexto = *contexto_original;
        uint32_t pid_nuevo = contexto->PID;

        if (pid_nuevo != pid_viejo)
        {
            pthread_mutex_lock(&mutex_interrupt);
            hay_interrupcion = false;
            pthread_mutex_unlock(&mutex_interrupt);
        }
    }

    if (instruccion_bloqueante(instruccion->id))
    {
        continua_ejecucion = false;

        pthread_mutex_lock(&mutex_interrupt);
        hay_interrupcion = false;
        pthread_mutex_unlock(&mutex_interrupt);
    }

    if (!instruccion_bloqueante(instruccion->id))
    {
        pthread_mutex_lock(&mutex_interrupt);
        if (hay_interrupcion && continua_ejecucion)
        {
            motivo_desalojo motivo = string_interrupcion_to_enum_motivo(motivo_interrupcion);
            free(motivo_interrupcion);
            motivo_interrupcion = NULL;
            hay_interrupcion = false;
            pthread_mutex_unlock(&mutex_interrupt);
            devolver_contexto(motivo, NULL);
            continua_ejecucion = false;
        }
        else
        {
            pthread_mutex_unlock(&mutex_interrupt);
        }
    }

    pthread_mutex_lock(&mutex_interrupt);
    if (motivo_interrupcion != NULL)
    {
        free(motivo_interrupcion);
        motivo_interrupcion = NULL;
    }
    pthread_mutex_unlock(&mutex_interrupt);
}

// -------------------- INSTRUCCIONES -------------------- //

void set(char *nombre_registro, uint32_t valor)
{
    if (strlen(nombre_registro) == 3 || !strcmp(nombre_registro, "SI") || !strcmp(nombre_registro, "DI") || !strcmp(nombre_registro, "PC")) // caso registros de 4 bytes
    {
        uint32_t *registro = dictionary_get(contexto->registros_cpu, nombre_registro);
        *registro = valor;
    }
    else if (strlen(nombre_registro) == 2) // caso registros de 1 bytes
    {
        uint8_t *registro = dictionary_get(contexto->registros_cpu, nombre_registro);
        *registro = (uint8_t)valor;
    }
}

void sum(char *nombre_destino, char *nombre_origen)
{
    uint32_t val_destino = obtener_valor_registro(contexto->registros_cpu, nombre_destino);
    uint32_t val_origen = obtener_valor_registro(contexto->registros_cpu, nombre_origen);
    val_destino += val_origen;
    set(nombre_destino, val_destino);
}

void sub(char *nombre_destino, char *nombre_origen)
{
    uint32_t val_destino = obtener_valor_registro(contexto->registros_cpu, nombre_destino);
    uint32_t val_origen = obtener_valor_registro(contexto->registros_cpu, nombre_origen);
    val_destino -= val_origen;
    set(nombre_destino, val_destino);
}

void jnz(char *nombre_registro, uint32_t nro_instruccion)
{
    uint32_t val_registro = obtener_valor_registro(contexto->registros_cpu, nombre_registro);
    if (val_registro != 0)
        set("PC", nro_instruccion);
}

void wait(char *recurso)
{
    t_list *parametros = list_create();
    list_add(parametros, string_duplicate(recurso));
    devolver_contexto(DESALOJO_WAIT, parametros);
}

void signal(char *recurso)
{
    t_list *parametros = list_create();
    list_add(parametros, string_duplicate(recurso));
    devolver_contexto(DESALOJO_SIGNAL, parametros);
}

void io_gen_sleep(char *nombre, char *unidades)
{
    t_list *param = list_create();
    list_add(param, string_duplicate(nombre));
    list_add(param, string_itoa(IO_GEN_SLEEP));
    list_add(param, string_duplicate(unidades));
    devolver_contexto(DESALOJO_IO, param);
}

void io_stdin_read(char *nombre, t_list *direcciones_fisicas, char *registro_tamanio)
{
    t_list *param = list_create();
    list_add(param, string_duplicate(nombre));
    list_add(param, string_itoa(IO_STDIN_READ));
    list_add(param, string_itoa(obtener_valor_registro(contexto->registros_cpu, registro_tamanio)));
    for (int i = 0; i < list_size(direcciones_fisicas); i++)
        list_add(param, string_itoa(*(int *)list_get(direcciones_fisicas, i)));
    devolver_contexto(DESALOJO_IO, param);
}

void io_stdout_write(char *nombre, t_list *direcciones_fisicas, char *registro_tamanio)
{
    t_list *param = list_create();
    list_add(param, string_duplicate(nombre));
    list_add(param, string_itoa(IO_STDOUT_WRITE));
    list_add(param, string_itoa(obtener_valor_registro(contexto->registros_cpu, registro_tamanio)));
    for (int i = 0; i < list_size(direcciones_fisicas); i++)
        list_add(param, string_itoa(*(int *)list_get(direcciones_fisicas, i)));
    devolver_contexto(DESALOJO_IO, param);
}

void mov_in(char *registro_datos_destino, t_list *direcciones_fisicas)
{
    uint32_t valor_parcial_leido, *direccion_fisica, *bytes_a_leer;
    void *valor_total = malloc(tamanio_de_registro(registro_datos_destino));
    uint8_t desplazamiento = 0;

    for (int i = 0; i < list_size(direcciones_fisicas); i += 2)
    {
        direccion_fisica = (uint32_t *)list_get(direcciones_fisicas, i);
        bytes_a_leer = (uint32_t *)list_get(direcciones_fisicas, i + 1);
        enviar_lectura_espacio_usuario(contexto->PID, direccion_fisica, bytes_a_leer);

        if (recibir_operacion(conexion_cpu_memoria) == OK)
        {
            valor_parcial_leido = 0;
            t_list *paquete_valor = recibir_paquete(conexion_cpu_memoria);
            memcpy(valor_total + desplazamiento, list_get(paquete_valor, 0), *bytes_a_leer);
            memcpy(&valor_parcial_leido, list_get(paquete_valor, 0), *bytes_a_leer);
            loggear_lectura_memoria_num(contexto->PID, *direccion_fisica, valor_parcial_leido);
            list_destroy_and_destroy_elements(paquete_valor, free);
        }
        else
            log_info(logger_propio, "Conflicto en mov_in"); // TODO E:

        desplazamiento += *bytes_a_leer;
    }

    if (tamanio_de_registro(registro_datos_destino) == 1)
        set(registro_datos_destino, *(uint8_t *)valor_total);
    else
        set(registro_datos_destino, *(uint32_t *)valor_total);
    free(valor_total);
}

void mov_out(char *registro_datos, t_list *direcciones_fisicas)
{
    void *valor = dictionary_get(contexto->registros_cpu, registro_datos);
    uint32_t valor_reconstruido, *direccion, *bytes_a_escribir;
    int desplazamiento = 0;
    void *valor_a_enviar;

    for (int i = 0; i < list_size(direcciones_fisicas); i += 2)
    {
        direccion = list_get(direcciones_fisicas, i);
        bytes_a_escribir = (uint32_t *)list_get(direcciones_fisicas, i + 1);
        valor_a_enviar = malloc(*bytes_a_escribir);
        memcpy(valor_a_enviar, valor + desplazamiento, *bytes_a_escribir);
        enviar_escritura_espacio_usuario(contexto->PID, direccion, valor_a_enviar, bytes_a_escribir);

        if (recibir_operacion(conexion_cpu_memoria) == OK)
        {
            valor_reconstruido = 0;
            memcpy(&valor_reconstruido, valor_a_enviar, *bytes_a_escribir);
            loggear_escritura_memoria_num(contexto->PID, *direccion, valor_reconstruido);
        }
        else
            log_info(logger_propio, "Conflicto en mov_out"); // TODO E:

        desplazamiento += *bytes_a_escribir;

        free(valor_a_enviar);
    }
}

void resize(uint32_t tamanio)
{
    // solicita resize a memoria
    t_paquete *paquete = crear_paquete(RESIZE_PROCESO);
    agregar_a_paquete_uint32(paquete, contexto->PID);
    agregar_a_paquete_uint32(paquete, tamanio);
    enviar_paquete(paquete, conexion_cpu_memoria);
    eliminar_paquete(paquete);

    // lee respuesta de la memoria
    int respuesta = recibir_operacion(conexion_cpu_memoria);
    if (respuesta == OUT_OF_MEMORY)
    {
        devolver_contexto(DESALOJO_OUT_OF_MEMORY, NULL);
        continua_ejecucion = false;
    }
}

void copy_string(int tamanio_a_operar, t_list *direcciones_fisicas)
{
    int tamanio_a_op = tamanio_a_operar;
    void *direccion;
    uint32_t *tamanio;
    int pos_lectura = 0;
    char *valores_leidos = string_new();
    char *a_enviar = NULL;

    for (int i = 0; i < list_size(direcciones_fisicas) && tamanio_a_op > 0; i += 2)
    {

        direccion = list_get(direcciones_fisicas, i);
        tamanio = (uint32_t *)list_get(direcciones_fisicas, i + 1);
        tamanio_a_op -= *tamanio;
        enviar_lectura_espacio_usuario(contexto->PID, (uint32_t *)direccion, tamanio);
        if (recibir_operacion(conexion_cpu_memoria) == OK)
        {
            t_list *string_leido = recibir_paquete(conexion_cpu_memoria);
            char *valor_leido = malloc(sizeof(char) * (*tamanio) + 1); // capaz se podría mejorar para hacer esto una sola vez
            valor_leido[sizeof(char) * (*tamanio)] = '\0';
            memcpy(valor_leido, list_get(string_leido, 0), sizeof(char) * (*tamanio));
            string_append(&valores_leidos, valor_leido);
            loggear_lectura_memoria_char(contexto->PID, *(int32_t *)direccion, valor_leido);
            list_destroy_and_destroy_elements(string_leido, free);
            free(valor_leido);
        }
        else
            log_info(logger_propio, "Algo ocurrio no se pudo leer en memoria");
        pos_lectura = i;
    }
    // escribimos en memoria
    for (int o = pos_lectura + 2; o < list_size(direcciones_fisicas); o += 2)
    {

        direccion = list_get(direcciones_fisicas, o);
        tamanio = (uint32_t *)list_get(direcciones_fisicas, o + 1);
        a_enviar = string_substring(valores_leidos, 0, *tamanio); // liberar a_enviar

        char *valores_sin_escribir = string_substring_from(valores_leidos, string_length(a_enviar));
        free(valores_leidos); // talves podria ser mejor
        valores_leidos = valores_sin_escribir;

        enviar_escritura_espacio_usuario(contexto->PID, (uint32_t *)direccion, (void *)a_enviar, tamanio);

        if (recibir_operacion(conexion_cpu_memoria) == OK)
            loggear_escritura_memoria_char(contexto->PID, *(uint32_t *)direccion, a_enviar);
        else
            log_info(logger_propio, "Algo ocurrio no se pudo escribir en memoria");
        free(a_enviar);
    }

    free(valores_leidos);
}

void io_fs_create(char *interfaz, char *nombre_archivo)
{
    t_list *parametros = list_create();
    list_add(parametros, string_duplicate(interfaz));
    list_add(parametros, string_itoa(IO_FS_CREATE));
    list_add(parametros, string_duplicate(nombre_archivo));
    devolver_contexto(DESALOJO_IO, parametros);
}

void io_fs_delete(char *interfaz, char *nombre_archivo)
{
    t_list *param = list_create();
    list_add(param, string_duplicate(interfaz));
    list_add(param, string_itoa(IO_FS_DELETE));
    list_add(param, string_duplicate(nombre_archivo));
    devolver_contexto(DESALOJO_IO, param);
}

void io_fs_truncate(char *interfaz, char *nombre_archivo, char *registro_tamanio)
{
    t_list *param = list_create();
    list_add(param, string_duplicate(interfaz));
    list_add(param, string_itoa(IO_FS_TRUNCATE));
    list_add(param, string_duplicate(nombre_archivo));
    list_add(param, string_itoa(obtener_valor_registro(contexto->registros_cpu, registro_tamanio)));
    devolver_contexto(DESALOJO_IO, param);
}

void io_fs_write(char *interfaz, char *nombre_archivo, t_list *direcciones_fisicas, char *registro_tamanio, char *registro_puntero_arch)
{
    t_list *param = list_create();
    list_add(param, string_duplicate(interfaz));
    list_add(param, string_itoa(IO_FS_WRITE));
    list_add(param, string_duplicate(nombre_archivo));
    list_add(param, string_itoa(obtener_valor_registro(contexto->registros_cpu, registro_tamanio)));
    list_add(param, string_itoa(obtener_valor_registro(contexto->registros_cpu, registro_puntero_arch)));
    for (int i = 0; i < list_size(direcciones_fisicas); i++)
        list_add(param, string_itoa(*(int *)list_get(direcciones_fisicas, i)));
    devolver_contexto(DESALOJO_IO, param);
}

void io_fs_read(char *interfaz, char *nombre_archivo, t_list *direcciones_fisicas, char *registro_tamanio, char *registro_puntero_arch)
{
    t_list *param = list_create();
    list_add(param, string_duplicate(interfaz));
    list_add(param, string_itoa(IO_FS_READ));
    list_add(param, string_duplicate(nombre_archivo));
    list_add(param, string_itoa(obtener_valor_registro(contexto->registros_cpu, registro_tamanio)));
    list_add(param, string_itoa(obtener_valor_registro(contexto->registros_cpu, registro_puntero_arch)));
    for (int i = 0; i < list_size(direcciones_fisicas); i++)
        list_add(param, string_itoa(*(int *)list_get(direcciones_fisicas, i)));
    devolver_contexto(DESALOJO_IO, param);
}

void exit_inst()
{
    devolver_contexto(DESALOJO_EXIT_SUCCESS, NULL);
}

void devolver_contexto(motivo_desalojo motivo_desalojo, t_list *param)
{
    t_paquete *paquete = crear_paquete(motivo_desalojo);
    agregar_a_paquete_contexto(paquete, contexto);

    if (param != NULL)
    {
        for (int i = 0; i < list_size(param); i++)
            agregar_a_paquete_string(paquete, (char *)list_get(param, i));
    }

    enviar_paquete(paquete, conexion_cpu_kernel_dispatch);
    eliminar_paquete(paquete);

    if (param != NULL)
        list_destroy_and_destroy_elements(param, free);
}

void enviar_lectura_espacio_usuario(uint32_t PID, uint32_t *direccion, uint32_t *bytes_a_leer)
{
    t_paquete *paquete_direccion = crear_paquete(ACCESO_ESPACIO_USUARIO_LECTURA);
    agregar_a_paquete_uint32(paquete_direccion, PID);
    agregar_a_paquete_uint32(paquete_direccion, *direccion);
    agregar_a_paquete_uint32(paquete_direccion, *bytes_a_leer);
    enviar_paquete(paquete_direccion, conexion_cpu_memoria);
    eliminar_paquete(paquete_direccion);
}

void enviar_escritura_espacio_usuario(uint32_t PID, uint32_t *direccion, void *valor_a_escribir, uint32_t *bytes_a_escribir)
{
    t_paquete *paquete_direccion = crear_paquete(ACCESO_ESPACIO_USUARIO_ESCRITURA);
    agregar_a_paquete_uint32(paquete_direccion, PID);
    agregar_a_paquete_uint32(paquete_direccion, *direccion);
    agregar_a_paquete(paquete_direccion, valor_a_escribir, *bytes_a_escribir);
    agregar_a_paquete_uint32(paquete_direccion, *bytes_a_escribir);
    enviar_paquete(paquete_direccion, conexion_cpu_memoria);
    eliminar_paquete(paquete_direccion);
}