#include "gestor_interfaces.h"

op_code (*atender)(int cod_op, t_list *parametros) = NULL;

void atender_segun_tipo_interfaz(void)
{
    char *tipo_interfaz = obtener_tipo_interfaz();
    if (strcmp(tipo_interfaz, "GENERICA") == 0)
    {
        atender = atender_gen;
    }
    else if (strcmp(tipo_interfaz, "STDIN") == 0)
    {
        atender = atender_stdin;
    }
    else if (strcmp(tipo_interfaz, "STDOUT") == 0)
    {
        atender = atender_stdout;
    }
    else if (strcmp(tipo_interfaz, "DIALFS") == 0)
    {
        fcbs = list_create();
        if (mkdir(obtener_path_base_dialfs(), 0777) && errno != EEXIST)
        {
            log_error(logger_propio, "No se pudo crear el directorio base del sistema de archivos.");
            exit(EXIT_FAILURE);
        }
        atender = atender_dialfs;

        leer_bloques();
        iniciar_bitmap();
        leer_fcbs();
    }
}

uint32_t calcular_tiempo_de_espera(int unidades_de_trabajo)
{
    return unidades_de_trabajo * obtener_tiempo_unidad_trabajo();
}

op_code atender_gen(int cod_op, t_list *parametros)
{
    op_code respuesta = OK;

    if (cod_op == IO_GEN_SLEEP)
    {
        loggear_operacion(*(uint32_t *)list_get(parametros, 0), nombres_de_instrucciones[cod_op]);

        int unidades_de_trabajo = atoi((char *)list_get(parametros, 1));
        uint32_t tiempo_de_espera_ms = calcular_tiempo_de_espera(unidades_de_trabajo) * 1000;
        usleep((__useconds_t)tiempo_de_espera_ms);
    }
    else
        respuesta = OPERACION_INVALIDA;

    return respuesta;
}

op_code atender_stdin(int cod_op, t_list *parametros)
{
    op_code respuesta = OK;

    if (cod_op == IO_STDIN_READ)
    {
        uint32_t *PID = (uint32_t *)list_get(parametros, 0);
        loggear_operacion(*PID, nombres_de_instrucciones[cod_op]);

        int tam = atoi(list_get(parametros, 1));

        char *lectura = NULL;
        while (!lectura)
        {
            lectura = readline("Ingresar texto para STDIN:");
            if (strlen(lectura) != tam)
            {
                log_info(logger_propio, "Se esperaba una string de %d caracteres", tam);
                free(lectura);
                lectura = NULL;
            }
        }

        int desplazamiento = 0;
        char *texto_a_enviar = NULL;
        for (int i = 2; i < list_size(parametros); i += 2)
        {
            uint32_t direccion_fisica = atoi(list_get(parametros, i));
            uint32_t bytes_a_operar = atoi(list_get(parametros, i + 1));
            texto_a_enviar = string_substring(lectura, desplazamiento, bytes_a_operar);

            t_paquete *paquete = crear_paquete(ACCESO_ESPACIO_USUARIO_ESCRITURA);
            agregar_a_paquete_uint32(paquete, *PID);
            agregar_a_paquete_uint32(paquete, direccion_fisica);
            agregar_a_paquete_string(paquete, texto_a_enviar);
            agregar_a_paquete_uint32(paquete, bytes_a_operar);
            enviar_paquete(paquete, conexion_memoria);

            desplazamiento += bytes_a_operar;
            respuesta = recibir_operacion(conexion_memoria);
            if (respuesta != OK)
            {
                log_info(logger_propio, "Se produjo un error al intentar escribir %s en la memoria", texto_a_enviar);
                respuesta = OPERACION_INVALIDA;
                eliminar_paquete(paquete);
                free(texto_a_enviar);
                break;
            }

            eliminar_paquete(paquete);
            free(texto_a_enviar);
        }

        free(lectura);
    }
    else
        respuesta = OPERACION_INVALIDA;

    return respuesta;
}

op_code atender_stdout(int cod_op, t_list *parametros)
{
    op_code respuesta;

    if (cod_op == IO_STDOUT_WRITE)
    {
        char *valor_leido_completo = NULL;
        uint32_t *PID = (uint32_t *)list_get(parametros, 0);
        loggear_operacion(*PID, nombres_de_instrucciones[cod_op]);
        t_list *direcciones_fisicas = list_slice(parametros, 2, list_size(parametros) - 2);

        respuesta = leer_de_memoria(*PID, atoi((char *)list_get(parametros, 1)), direcciones_fisicas, &valor_leido_completo);
        if (respuesta == OK && valor_leido_completo != NULL)
            log_info(logger_obligatorio, "PID: <%d> - Interfaz: <STDOUT> - Valor: %s", *PID, valor_leido_completo);
        else
            log_error(logger_propio, "No se pudo leer el valor de la memoria.");

        list_destroy(direcciones_fisicas);
        free(valor_leido_completo);
        return respuesta;
    }
    else
        return OPERACION_INVALIDA;
}

op_code atender_dialfs(int cod_op, t_list *parametros)
{
    op_code respuesta = OK;
    uint32_t *PID = list_get(parametros, 0);
    t_list *direcciones_fisicas;

    usleep(obtener_tiempo_unidad_trabajo() * 1000);

    switch (cod_op)
    {
    case IO_FS_CREATE:
        crear_archivo(PID, (char *)list_get(parametros, 1));
        break;
    case IO_FS_DELETE:
        eliminar_archivo(PID, (char *)list_get(parametros, 1));
        break;
    case IO_FS_TRUNCATE:
        truncar_archivo(PID, (char *)list_get(parametros, 1), atoi((char *)list_get(parametros, 2)));
        break;
    case IO_FS_READ:
        void *lectura = leer_archivo(PID, (char *)list_get(parametros, 1), atoi((char *)list_get(parametros, 2)), atoi((char *)list_get(parametros, 3)));
        direcciones_fisicas = list_slice(parametros, 4, list_size(parametros) - 4);
        if (!escribir_en_memoria(*PID, direcciones_fisicas, lectura))
            respuesta = OPERACION_INVALIDA;
        list_destroy(direcciones_fisicas);
        free(lectura);
        break;
    case IO_FS_WRITE:
        char *valor_leido_completo = NULL;
        direcciones_fisicas = list_slice(parametros, 4, list_size(parametros) - 4);
        respuesta = leer_de_memoria(*PID, atoi((char *)list_get(parametros, 2)), direcciones_fisicas, &valor_leido_completo);
        if (valor_leido_completo == NULL)
        {
            log_error(logger_propio, "No se pudo leer el valor de la memoria.");
            exit(EXIT_FAILURE);
        }
        escribir_archivo(PID, (char *)list_get(parametros, 1), atoi((char *)list_get(parametros, 2)), atoi((char *)list_get(parametros, 3)), valor_leido_completo);
        list_destroy(direcciones_fisicas);
        free(valor_leido_completo);
        break;
    default:
        respuesta = OPERACION_INVALIDA;
        break;
    }
    return respuesta;
}

op_code leer_de_memoria(uint32_t PID, uint32_t cantidad_caracteres, t_list *direcciones_fisicas, char **valor_leido_completo)
{
    uint32_t tamanio = sizeof(char) * cantidad_caracteres + 1;
    *valor_leido_completo = malloc_or_die(tamanio, "Error al reservar memoria dinamica para valor_leido_completo de memoria");
    (*valor_leido_completo)[tamanio - 1] = '\0';
    int desplazamiento = 0;

    for (int i = 0; i < list_size(direcciones_fisicas); i += 2)
    {
        uint32_t direccion_fisica = atoi(list_get(direcciones_fisicas, i));
        uint32_t bytes_a_operar = atoi(list_get(direcciones_fisicas, i + 1));

        enviar_lectura_espacio_usuario(PID, direccion_fisica, bytes_a_operar);

        if (recibir_operacion(conexion_memoria) == OK)
        {
            recibir_lectura_parcial_memoria(*valor_leido_completo, desplazamiento, bytes_a_operar);
            desplazamiento += bytes_a_operar;

            char *valor_parcial = malloc(desplazamiento + 1);
            memcpy(valor_parcial, *valor_leido_completo, desplazamiento);
            valor_parcial[desplazamiento] = '\0';
            free(valor_parcial);
        }
        else
        {
            log_error(logger_propio, "Error al intentar leer %d bytes en la direccion fisica %d", bytes_a_operar, direccion_fisica);
            return OPERACION_INVALIDA;
        }
    }

    return OK;
}

void enviar_lectura_espacio_usuario(uint32_t PID, uint32_t direccion, uint32_t bytes_a_leer)
{
    t_paquete *paquete_direccion = crear_paquete(ACCESO_ESPACIO_USUARIO_LECTURA);
    agregar_a_paquete_uint32(paquete_direccion, PID);
    agregar_a_paquete_uint32(paquete_direccion, direccion);
    agregar_a_paquete_uint32(paquete_direccion, bytes_a_leer);
    enviar_paquete(paquete_direccion, conexion_memoria);
    eliminar_paquete(paquete_direccion);
}

void recibir_lectura_parcial_memoria(char *valor_leido_completo, int desplazamiento, uint32_t bytes_a_operar)
{
    t_list *paquete_recibido = recibir_paquete(conexion_memoria);
    void *valor_leido = list_get(paquete_recibido, 0);
    memcpy(valor_leido_completo + desplazamiento, valor_leido, sizeof(char) * bytes_a_operar);
    list_destroy_and_destroy_elements(paquete_recibido, free);
}

bool escribir_en_memoria(uint32_t PID, t_list *direcciones_fisicas, void *escritura)
{
    bool operacion_exitosa = true;
    uint32_t direccion_fisica;
    uint32_t bytes_a_operar;
    int desplazamiento = 0;
    void *escritura_a_enviar = NULL;

    for (int i = 0; i < list_size(direcciones_fisicas); i += 2)
    {
        // obtengo trozo a escribir
        direccion_fisica = atoi(list_get(direcciones_fisicas, i));
        bytes_a_operar = atoi(list_get(direcciones_fisicas, i + 1));
        escritura_a_enviar = malloc(bytes_a_operar);
        memcpy(escritura_a_enviar, escritura + desplazamiento, bytes_a_operar);

        // solicito escritura en la memoria
        t_paquete *paquete = crear_paquete(ACCESO_ESPACIO_USUARIO_ESCRITURA);
        agregar_a_paquete_uint32(paquete, PID);
        agregar_a_paquete_uint32(paquete, direccion_fisica);
        agregar_a_paquete(paquete, escritura_a_enviar, bytes_a_operar);
        agregar_a_paquete_uint32(paquete, bytes_a_operar);
        enviar_paquete(paquete, conexion_memoria);

        if (recibir_operacion(conexion_memoria) != OK)
        {
            log_error(logger_propio, "Error al intentar escribir %s en la memoria", (char *)escritura_a_enviar);
            operacion_exitosa = false;
            break;
        }

        desplazamiento += bytes_a_operar;
        eliminar_paquete(paquete);
        free(escritura_a_enviar);
    }

    return operacion_exitosa;
}

void setear_config(char *archivo_config_io)
{
    char *config_ruta = malloc(strlen("configuraciones/") + strlen(archivo_config_io));
    strcpy(config_ruta, "configuraciones/");
    string_append(&config_ruta, archivo_config_io);
    config = iniciar_config(logger_propio, config_ruta);
    free(config_ruta);
}