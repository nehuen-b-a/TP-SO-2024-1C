#include "sistema_archivos.h"

t_bitarray *bitmap;
t_list *fcbs;
void *bloques;
void *espacio_bitmap;
size_t tamanio_bitmap;
char *bitmap_string;

void iniciar_bitmap(void)
{
    char *path = string_new();
    string_append_with_format(&path, "%s/%s", obtener_path_base_dialfs(), "bitmap.dat");
    FILE *fbitmap = fopen(path, "ab+");
    free(path);
    if (fbitmap == NULL)
    {
        log_error(logger_propio, "No se pudo crear o abrir el archivo bitmap.dat: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    tamanio_bitmap = (obtener_block_count() + 8 - 1) / 8;
    ftruncate(fileno(fbitmap), tamanio_bitmap);

    espacio_bitmap = mmap(NULL, tamanio_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(fbitmap), 0);
    fclose(fbitmap);
    if (espacio_bitmap == MAP_FAILED)
    {
        log_error(logger_propio, "No se pudo mapear a memoria el archivo bitmap.dat: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    bitmap = bitarray_create_with_mode(espacio_bitmap, tamanio_bitmap, MSB_FIRST);

    // bitmap_string = leer_bitmap(bitmap, 0, 32);
    // log_info(logger_propio, "Bitmap string: %s", bitmap_string);
    // log_info(logger_propio, "Bitmap size: %ld", bitmap->size);

    // for (int i = 20; i < 33; i++)
    //     assignBlock(i);
    // bitmap_string = leer_bitmap(bitmap, 0, 32);
    // for (int i = 0; i < 5; i++)
    //     unassignBlock(i);
    // bitmap_string = leer_bitmap(bitmap, 0, 32);
    // log_info(logger_propio, "Bitmap string: %s", bitmap_string);
}

void leer_bloques(void)
{
    char *path = string_new();
    string_append_with_format(&path, "%s/%s", obtener_path_base_dialfs(), "bloques.dat");
    FILE *fbloques = fopen(path, "ab+");
    free(path);
    if (fbloques == NULL)
    {
        log_error(logger_propio, "No se pudo crear o abrir el archivo bloques.dat: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    size_t tamanio = obtener_block_count() * obtener_block_size();
    ftruncate(fileno(fbloques), tamanio);

    bloques = mmap(NULL, tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(fbloques), 0);
    fclose(fbloques);
    if (bloques == MAP_FAILED)
    {
        log_error(logger_propio, "No se pudo mapear a memoria el archivo bloques.dat: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void leer_fcbs()
{
    // creo directorio para metadata si no existe
    char *path = string_new();
    string_append_with_format(&path, "%s/metadata", obtener_path_base_dialfs());
    if (mkdir(path, 0777) == -1 && errno != EEXIST)
    {
        free(path);
        log_error(logger_propio, "No se pudo crear el directorio de metadata: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // abro el directorio
    DIR *drmetadata = opendir(path);
    if (drmetadata == NULL)
    {
        free(path);
        log_error(logger_propio, "No se pudo abrir el directorio de metadata: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // leo la metadata
    struct dirent *de;

    t_config *config;
    char *config_ruta;
    t_fcb *fcb;
    while ((de = readdir(drmetadata)) != NULL)
    {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
        {
            // abro config
            config_ruta = string_new();
            string_append_with_format(&config_ruta, "%s/%s", path, de->d_name);
            config = iniciar_config(logger_propio, config_ruta);

            if (config == NULL)
            {
                log_error(logger_propio, "No se encontró el archivo en %s", config_ruta);
                closedir(drmetadata);
                free(config_ruta);
                free(path);
                exit(EXIT_FAILURE);
            }
            else
                free(config_ruta);

            // cargo fcb
            fcb = malloc_or_die(sizeof(t_fcb), "Fallo en malloc para fcb");
            if (fcb == NULL)
            {
                closedir(drmetadata);
                config_destroy(config);
                free(path);
                exit(EXIT_FAILURE);
            }

            fcb->nombre = string_duplicate(de->d_name);
            fcb->bloque_inicial = config_get_int_value(config, "BLOQUE_INICIAL");
            fcb->tamanio_en_bytes = config_get_int_value(config, "TAMANIO_ARCHIVO");

            config_destroy(config);
            cargar_fcb(fcb);
        }
    }
    closedir(drmetadata);
    free(path);
}

void cargar_fcb(t_fcb *fcb)
{
    list_add_sorted(fcbs, fcb, ordenar_fcb_por_bloque_inicial);
}

bool ordenar_fcb_por_bloque_inicial(void *fcb1, void *fcb2)
{
    return ((t_fcb *)fcb1)->bloque_inicial <= ((t_fcb *)fcb2)->bloque_inicial;
}

int obtener_bloque_libre(void)
{
    int bloque = -1;
    for (int i = 0; i < obtener_block_count() && bloque < 0; i++)
    {
        if (!bitarray_test_bit(bitmap, i))
            bloque = i;
    }
    return bloque;
}

void crear_archivo(uint32_t *PID, char *nombre)
{
    char *path_absoluto = string_new();
    string_append_with_format(&path_absoluto, "%s/metadata/%s", obtener_path_base_dialfs(), nombre);

    FILE *metadataArchivo = fopen(path_absoluto, "w");
    fclose(metadataArchivo);
    free(path_absoluto);

    int bloque_libre = obtener_bloque_libre();

    t_fcb *fcb = malloc_or_die(sizeof(t_fcb), "No se pudo crear el fcb.");
    fcb->nombre = string_duplicate(nombre);
    fcb->bloque_inicial = bloque_libre;
    fcb->tamanio_en_bytes = 0;

    bool buscar_por_nombre(void *fcb) { return strcmp(((t_fcb *)fcb)->nombre, nombre) == 0; }
    t_fcb *fcb_encontrado = (t_fcb *)list_find(fcbs, buscar_por_nombre);
    if (fcb_encontrado != NULL)
    {
        log_error(logger_propio, "Ya existe el archivo con el nombre: %s", fcb_encontrado->nombre);
    }
    else
    {
        bitarray_set_bit(bitmap, bloque_libre);
        msync(espacio_bitmap, tamanio_bitmap, MS_SYNC);

        actualizar_metadata(fcb);

        cargar_fcb(fcb);

        // log minimo y obligatorio
        loggear_dialfs_crear_archivo(*PID, nombre);
    }
}

void eliminar_archivo(uint32_t *PID, char *nombre)
{
    // log minimo y obligatorio
    loggear_dialfs_eliminar_archivo(*PID, nombre);

    liberar_archivo(nombre);
    eliminar_metadata(nombre);
}

void liberar_archivo(char *archivo)
{
    for (int i = 0; i < tamanio_en_bloques(archivo); i++)
        bitarray_clean_bit(bitmap, bloque_inicial(archivo) + i);
    msync(espacio_bitmap, tamanio_bitmap, MS_SYNC);
}

void destruir_fcb(void *data)
{
    t_fcb *fcb = (t_fcb *)data;
    free(fcb->nombre);
    free(fcb);
}

void eliminar_metadata(char *archivo)
{
    bool buscar_por_nombre(void *fcb) { return strcmp(((t_fcb *)fcb)->nombre, archivo) == 0; }
    list_remove_and_destroy_by_condition(fcbs, buscar_por_nombre, destruir_fcb);

    char *path = string_new();
    string_append_with_format(&path, "%s/metadata/%s", obtener_path_base_dialfs(), archivo);
    remove(path);
    free(path);
}

int tamanio_en_bloques(char *archivo)
{
    t_fcb *fcb = metadata_de_archivo(archivo);
    return (fcb->tamanio_en_bytes + obtener_block_size() - 1) / obtener_block_size();
}

void truncar_archivo(uint32_t *PID, char *nombre, int tam)
{
    t_fcb *fcb = metadata_de_archivo(nombre);

    // Obtengo el bloque inicial del archivo y su tamaño actual
    int bloque_inicial = fcb->bloque_inicial;
    int tamanio_anterior = fcb->tamanio_en_bytes;

    // Calcular el nuevo tamaño en bloques,  REVISAR
    int tam_nuevo_en_bloques = bytes_a_bloques(tam);
    int tam_anterior_en_bloques = bytes_a_bloques(tamanio_anterior);

    // Si el nuevo tamaño es menor, se liberan los bloques adicionales
    if (tam_nuevo_en_bloques < tam_anterior_en_bloques)
    {
        for (int i = tam_nuevo_en_bloques; i < tam_anterior_en_bloques; i++)
        {
            int bloque_a_liberar = bloque_inicial + i;
            bitarray_clean_bit(bitmap, bloque_a_liberar);
        }
    }
    else
    {
        // Si es necesario compactar, llamar a la función compactar
        if (validar_compactacion(tam_nuevo_en_bloques, fcb))
        {
            compactar(PID, fcb, tam);
        }
        else
        { // asignar bloques extra que necesita
            for (int i = 0; i < tam_nuevo_en_bloques - tam_anterior_en_bloques; i++)
            {
                bitarray_set_bit(bitmap, fcb->bloque_inicial + tam_anterior_en_bloques + i);
            }
        }
    }

    // Actualizar metadata
    fcb->tamanio_en_bytes = tam;
    actualizar_metadata(fcb);

    msync(espacio_bitmap, tamanio_bitmap, MS_SYNC);

    // log minimo y obligatorio
    loggear_dialfs_truncar_archivo(*PID, nombre, tam);
}

bool validar_compactacion(int tam_nuevo_en_bloques, t_fcb *fcb)
{
    t_list_iterator *fcb_iterando = list_iterator_create(fcbs);
    bool hay_que_compactar = false;
    int bloque_inicial_siguiente = 0;
    t_fcb *fcb_siguiente;

    // busco el fcb del archivo que le sigue a mi archivo
    while (list_iterator_has_next(fcb_iterando) && bloque_inicial_siguiente <= fcb->bloque_inicial)
    {
        fcb_siguiente = list_iterator_next(fcb_iterando);
        bloque_inicial_siguiente = fcb_siguiente->bloque_inicial;
    }

    if (bloque_inicial_siguiente > fcb->bloque_inicial)
    {
        hay_que_compactar = bloque_inicial_siguiente - fcb->bloque_inicial < tam_nuevo_en_bloques;
    }

    list_iterator_destroy(fcb_iterando);

    return hay_que_compactar;
}

void compactar(uint32_t *PID, t_fcb *archivo_a_truncar, int tamanio_a_truncar_en_bytes)
{
    // log minimo y obligatorio
    loggear_dialfs_inicio_compactacion(*PID);

    usleep(obtener_retraso_compactacion() * 1000);

    // Ordenar la lista de FCBs por bloque inicial
    // ordenar_fcb_por_bloque_inicial(fcbs); // rocio dice que no hace falta

    // Copiar el contenido del archivo a truncar en un auxiliar y eliminarlo de la lista
    void *archivo_auxiliar = malloc_or_die(bytes_a_bloques(archivo_a_truncar->tamanio_en_bytes) * obtener_block_size(), "Error al reservar memoria para archivo_auxiliar en compactar()");
    void *src = bloques + archivo_a_truncar->bloque_inicial * obtener_block_size();
    memcpy(archivo_auxiliar, src, bytes_a_bloques(archivo_a_truncar->tamanio_en_bytes) * obtener_block_size());
    list_remove_element(fcbs, archivo_a_truncar);

    for (int i = 0; i < bytes_a_bloques(archivo_a_truncar->tamanio_en_bytes); i++)
        bitarray_clean_bit(bitmap, archivo_a_truncar->bloque_inicial + i);

    if (((t_fcb *)list_get(fcbs, 0))->bloque_inicial != 0)
    {
        t_fcb *primer_fcb = (t_fcb *)list_get(fcbs, 0);
        mover_fcb(primer_fcb, 0);
        actualizar_metadata(primer_fcb);
    }

    int i;
    for (i = 0; i < list_size(fcbs) - 1; i++)
    {
        t_fcb *fcb_actual = list_get(fcbs, i);
        int bloque_inicial_actual = fcb_actual->bloque_inicial;
        int ultimo_bloque_actual = bytes_a_bloques(fcb_actual->tamanio_en_bytes) + bloque_inicial_actual - 1;

        t_fcb *fcb_siguiente = list_get(fcbs, i + 1);
        int bloque_inicial_siguiente = fcb_siguiente->bloque_inicial;
        // int ultimo_bloque_siguiente = bytes_a_bloques(fcb_siguiente->tamanio_en_bytes) + bloque_inicial_siguiente;

        if (ultimo_bloque_actual + 1 != bloque_inicial_siguiente)
        {
            mover_fcb(fcb_siguiente, ultimo_bloque_actual + 1);
            fcb_siguiente->bloque_inicial = ultimo_bloque_actual + 1;
            actualizar_metadata(fcb_siguiente);
        }
    }

    // Pegar el contenido de archivo_auxiliar desde bloque_final
    mover_contenido_fcb(archivo_a_truncar, obtener_bloque_libre(), archivo_auxiliar, false, tamanio_a_truncar_en_bytes);

    cargar_fcb(archivo_a_truncar);

    free(archivo_auxiliar);

    msync(espacio_bitmap, tamanio_bitmap, MS_SYNC);
    msync(bloques, obtener_block_count() * obtener_block_size(), MS_SYNC);

    // log minimo y obligatorio
    loggear_dialfs_fin_compactacion(*PID);
}

void leer_metadata(char *archivo)
{
    // abro config
    char *config_ruta = string_new();
    string_append_with_format(&config_ruta, "%s/metadata/%s", obtener_path_base_dialfs(), archivo);
    t_config *config = iniciar_config(logger_propio, config_ruta);

    // seteo valores
    int bloque_inicial = config_get_int_value(config, "BLOQUE_INICIAL");
    int tamanio_en_bytes = config_get_int_value(config, "TAMANIO_ARCHIVO");
    log_info(logger_propio, "================================================");
    log_info(logger_propio, "archivo %s", archivo);
    log_info(logger_propio, "bloque inicial: %d", bloque_inicial);
    log_info(logger_propio, "tam en bytes: %d", tamanio_en_bytes);
    log_info(logger_propio, "================================================");

    // cierro config y libero valores
    free(config_ruta);
    config_destroy(config);
}

void *leer_archivo(uint32_t *PID, char *nombre, int tam, int puntero)
{
    int pos_inicial = bloque_inicial(nombre) * obtener_block_size() + puntero;
    void *lectura = malloc(tam);
    memcpy(lectura, bloques + pos_inicial, tam);

    // log minimo y obligatorio
    loggear_dialfs_leer_archivo(*PID, nombre, tam, puntero);

    return lectura;
}

int bloque_inicial(char *archivo)
{
    t_fcb *fcb = metadata_de_archivo(archivo);
    return fcb->bloque_inicial;
}

t_fcb *metadata_de_archivo(char *archivo)
{
    bool buscar_por_nombre(void *fcb)
    {
        return strcmp(((t_fcb *)fcb)->nombre, archivo) == 0;
    }

    t_fcb *fcb = (t_fcb *)list_find(fcbs, buscar_por_nombre);
    if (fcb == NULL)
    {
        log_error(logger_propio, "No se encontro ningun archivo que tenga el nombre: %s", archivo);
        exit(EXIT_FAILURE);
    }

    return fcb;
}

void escribir_archivo(uint32_t *PID, char *nombre, int tam, int puntero, char *dato_a_escribir)
{
    int pos_inicial = bloque_inicial(nombre) * obtener_block_size() + puntero;
    log_info(logger_propio, "Archivo: %s, Dato a escribir: %s, Tam: %d", nombre, dato_a_escribir, tam);
    memcpy(bloques + pos_inicial, dato_a_escribir, tam);
    msync(bloques, obtener_block_count() * obtener_block_size(), MS_SYNC);
    // log minimo y obligatorio
    loggear_dialfs_escribir_archivo(*PID, nombre, tam, puntero);
}

void actualizar_metadata(t_fcb *fcb)
{
    // abro config
    char *config_ruta = string_new();
    string_append_with_format(&config_ruta, "%s/metadata/%s", obtener_path_base_dialfs(), fcb->nombre);
    t_config *config = iniciar_config(logger_propio, config_ruta);

    // seteo valores
    char *bloque_inicial = string_itoa(fcb->bloque_inicial);
    char *tamanio_en_bytes = string_itoa(fcb->tamanio_en_bytes);
    config_set_value(config, "BLOQUE_INICIAL", bloque_inicial);
    config_set_value(config, "TAMANIO_ARCHIVO", tamanio_en_bytes);

    // grabo config
    config_save(config);

    // cierro config y libero valores
    free(bloque_inicial);
    free(tamanio_en_bytes);
    free(config_ruta);
    config_destroy(config);

    // leer_metadata(fcb->nombre); // TODO: eliminar, está para hacer pruebas nomás la función
}

void mover_fcb(t_fcb *fcb, int nuevo_inicio)
{
    int viejo_inicio = fcb->bloque_inicial;
    void *src = bloques + viejo_inicio * obtener_block_size();
    mover_contenido_fcb(fcb, nuevo_inicio, src, true, fcb->tamanio_en_bytes);
}

void mover_contenido_fcb(t_fcb *fcb, int nuevo_inicio, void *src_contenido, bool hacer_clean, int tamanio_a_truncar)
{
    int viejo_inicio = fcb->bloque_inicial;
    int tam_en_bloques = bytes_a_bloques(fcb->tamanio_en_bytes);
    void *dst = bloques + nuevo_inicio * obtener_block_size();
    memcpy(dst, src_contenido, tam_en_bloques * obtener_block_size());
    fcb->tamanio_en_bytes = tamanio_a_truncar;
    tam_en_bloques = bytes_a_bloques(fcb->tamanio_en_bytes);

    // Actualizar el bitmap
    for (int i = 0; i < tam_en_bloques; i++)
    {
        hacer_clean ? bitarray_clean_bit(bitmap, viejo_inicio + i) : 1;
        bitarray_set_bit(bitmap, nuevo_inicio + i);
    }

    // Actualizar el FCB del siguiente con su nuevo bloque inicial
    fcb->bloque_inicial = nuevo_inicio;
}

int bytes_a_bloques(int bytes)
{
    return bytes == 0 ? 1 : (bytes + obtener_block_size() - 1) / obtener_block_size();
}

char *leer_bitmap(const t_bitarray *bitmap, size_t indice_inico, size_t cantidad_de_bits)
{
    char *result = (char *)malloc((cantidad_de_bits + 1) * sizeof(char));
    if (!result)
        return NULL; // Error de asignación

    size_t byte_index, bit_index;
    for (size_t i = 0; i < cantidad_de_bits; i++)
    {
        byte_index = (indice_inico + i) / 8;
        bit_index = (indice_inico + i) % 8;

        if (bitmap->bitarray[byte_index] & (1 << (7 - bit_index)))
            result[i] = '1';
        else
            result[i] = '0';
    }

    result[cantidad_de_bits] = '\0'; // Añadir el terminador nulo
    return result;
}

void assignBlock(int blockIndex)
{
    if (bitmap == NULL || bitmap->bitarray == NULL)
    {
        log_info(logger_propio, "El bitmap no está inicializado correctamente\n");
        return;
    }

    bitarray_set_bit(bitmap, blockIndex);

    if (msync(bitmap->bitarray, bitmap->size, MS_SYNC) == -1)
        log_info(logger_propio, "Error en la sincronización con msync()\n");
}

void unassignBlock(int blockIndex)
{
    if (bitmap == NULL || bitmap->bitarray == NULL)
    {
        log_info(logger_propio, "El bitmap no está inicializado correctamente\n");
        return;
    }

    bitarray_clean_bit(bitmap, blockIndex);

    if (msync(bitmap->bitarray, bitmap->size, MS_SYNC) == -1)
        log_info(logger_propio, "Error en la sincronización con msync()\n");
}