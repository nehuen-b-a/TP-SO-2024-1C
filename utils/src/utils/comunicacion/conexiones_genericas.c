#include <utils/comunicacion/conexiones_genericas.h>

int crear_conexion(t_log *logger, char *ip, char *puerto)
{
    struct addrinfo hints, *server_info;
    int s;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(ip, puerto, &hints, &server_info);
    if (s)
    {
        log_error(logger, "No se pudo obtener información para conectar al puerto %s en la IP %s: %s", puerto, ip, gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    int socket_cliente = socket(server_info->ai_family,
                                server_info->ai_socktype,
                                server_info->ai_protocol);
    if (socket_cliente == -1)
    {
        log_error(logger, "No se pudo crear el socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        close(socket_cliente);
        log_error(logger, "No se pudo conectar el socket al puerto %s en la IP %s: %s", puerto, ip, strerror(errno));
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(server_info);

    return socket_cliente;
}

int iniciar_servidor(t_log *logger, char *puerto)
{
    struct addrinfo hints, *servinfo;
    int s;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(NULL, puerto, &hints, &servinfo);
    if (s)
    {
        log_error(logger, "No se pudo obtener información para conectar al puerto %s: %s", puerto, gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // Creamos el socket de escucha del servidor
    int socket_servidor = socket(servinfo->ai_family,
                                 servinfo->ai_socktype,
                                 servinfo->ai_protocol);
    if (socket_servidor == -1)
    {
        log_error(logger, "No se pudo crear el socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
        log_error(logger, "No se pudo hacer reusable el socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Asociamos el socket a un puerto
    if (bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen))
    {
        close(socket_servidor);
        log_error(logger, "No se pudo bindear el socket al puerto %s: %s", puerto, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Escuchamos las conexiones entrantes
    if (listen(socket_servidor, SOMAXCONN))
    {
        close(socket_servidor);
        log_error(logger, "No se pudo poner el socket en escucha: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(servinfo);

    return socket_servidor;
}

// Retorna socket que representa conexion bidireccional
int esperar_cliente(t_log *logger, int socket_servidor)
{
    // Aceptamos un nuevo cliente
    int socket_cliente = accept(socket_servidor, NULL, NULL);
    if (socket_cliente == -1)
    {
        close(socket_servidor);
        log_error(logger, "No se pudo aceptar socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return socket_cliente;
}

void terminar_programa(int conexion, t_log *logger, t_config *config)
{
    close(conexion);
    log_destroy(logger);
    config_destroy(config);
}

int handshake_cliente(t_log *logger, int conexion, int32_t handshake)
{
    int32_t result;

    send(conexion, &handshake, sizeof(int32_t), 0);
    recv(conexion, &result, sizeof(int32_t), MSG_WAITALL);

    if (result == 0)
    {
        log_info(logger, "Handshake realizado correctamente");
        return 1;
    }
    else
    {
        log_error(logger, "Ocurrio un error en el Handshake");
        return 0;
    }
}

int handshake_servidor(t_log *logger, int conexion, int32_t handshake_esperado)
{
    size_t bytes;

    int32_t handshake;
    int32_t resultOk = 0;
    int32_t resultError = -1;

    bytes = recv(conexion, &handshake, sizeof(int32_t), MSG_WAITALL);
    if (handshake == handshake_esperado)
    {
        bytes = send(conexion, &resultOk, sizeof(int32_t), 0);
        return 1;
    }
    else
    {
        bytes = send(conexion, &resultError, sizeof(int32_t), 0);
        return bytes;
    }
}

int conectar_a(char *servidor, t_log *logger, int tiempo_de_reintento_seg)
{
    // tomo el puerto y el ip de un servidor de forma generica
    char *claves[] = {
        string_from_format("PUERTO_%s", servidor),
        string_from_format("IP_%s", servidor),
    };
    char *puerto = obtener_configuracion(claves[0]);
    free(claves[0]);
    char *ip = obtener_configuracion(claves[1]);
    free(claves[1]);
    int conexion = -1;

    // mientras la conexion es fallida
    while (conexion == -1)
    {
        conexion = crear_conexion(logger, ip, puerto);
        if (conexion == -1)
        {
            log_warning(logger, "No se pudo conectar a %s, en %d segundos se reintentara conectar.", servidor, tiempo_de_reintento_seg);
            sleep(tiempo_de_reintento_seg);
        }
    }
    log_info(logger, "Conectado a %s, en socket %d", servidor, conexion);

    return conexion;
}

void esperar_a(char *tipo, int *cliente, int server)
{
    char *aviso = string_new();
    string_append(&aviso, "Esperando ");
    string_append(&aviso, tipo);
    string_append(&aviso, "...");

    log_info(logger_propio, "%s", aviso);
    *cliente = esperar_cliente(logger_propio, server);

    int32_t handshake_esperado = 1;
    handshake_servidor(logger_propio, *cliente, handshake_esperado);
}