#include <utils/funcionalidades_basicas.h>

t_log *crear_logger(char *modulo)
{
    char *nombre_logger = string_duplicate(modulo);
    char *log = ".log";
    string_append_with_format(&nombre_logger, "%s", log);

    t_log *nuevo_logger = log_create(nombre_logger, modulo, 1, LOG_LEVEL_INFO);
    free(nombre_logger);

    if (nuevo_logger == NULL)
    {
        printf("No se pudo iniciar un logger\n");
        exit(EXIT_FAILURE);
    }

    return nuevo_logger;
}

t_config *iniciar_config(t_log *logger, char *ruta_archivo)
{
    t_config *nuevo_config = config_create(ruta_archivo);
    if (nuevo_config == NULL)
    {
        log_error(logger, "No se pudo crear la configuracion en %s", ruta_archivo);
        exit(EXIT_FAILURE);
    }
    return nuevo_config;
}

void *malloc_or_die(size_t tamanio, const char *mensaje)
{
    void *direccion_memoria = malloc(tamanio);
    if (!direccion_memoria && tamanio)
    {
        fprintf(stderr, "%s\n", mensaje);
        exit(EXIT_FAILURE);
    }
    return direccion_memoria;
}

void *dictionary_get_with_int_key(t_dictionary *dictionary, int key)
{
    char *char_key = string_itoa(key);
    void *value = dictionary_get(dictionary, char_key);
    free(char_key);
    return value;
}

void dictionary_put_with_int_key(t_dictionary *dictionary, int key, void *element)
{
    char *char_key = string_itoa(key);
    dictionary_put(dictionary, char_key, element);
    free(char_key);
}

void *dictionary_remove_with_int_key(t_dictionary *dictionary, int key)
{
    char *char_key = string_itoa(key);
    void *value = dictionary_remove(dictionary, char_key);
    free(char_key);
    return value;
}