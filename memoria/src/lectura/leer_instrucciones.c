#include "lectura.h"

t_list *subir_instrucciones(char *path)
{
    char *path_instrucciones = obtener_path_instrucciones();

    size_t tam = strlen(path_instrucciones) + strlen(path) + 1;
    char *path_absoluto = malloc(tam);
    strcpy(path_absoluto, path_instrucciones);
    strcat(path_absoluto, path);

    FILE *archivo = fopen(path_absoluto, "r");
    if (archivo == NULL)
    {
        free(path_absoluto);
        log_error(logger_propio, "No se pudo encontrar el archivo de instrucciones.");
        return list_create();
    }

    t_list *instrucciones = list_create();
    char *instruccion;

    while ((instruccion = leer_instruccion(archivo)) != NULL)
    {
        list_add(instrucciones, instruccion);
    }

    free(path_absoluto);
    fclose(archivo);
    return instrucciones;
}

char *leer_instruccion(FILE *archivo)
{
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), archivo) != NULL)
    {
        size_t tamanio = strlen(buffer);
        if (tamanio > 0 && buffer[tamanio - 1] == '\n')
        {
            buffer[tamanio - 1] = '\0'; // Elimina el carácter de nueva línea
        }
        return strdup(buffer); // Duplica la cadena y la devuelve
    }
    return NULL;
}