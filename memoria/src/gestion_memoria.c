#include <gestion_memoria.h>

void *espacio_usuario;
t_dictionary *indice_tablas;
t_dictionary *indice_instrucciones;
t_bitarray *marcos_libres;

pthread_mutex_t mutex_memoria;

void inicializar_memoria(void)
{
    espacio_usuario = malloc(obtener_tam_memoria());
    memset(espacio_usuario, 0, obtener_tam_memoria());
    indice_tablas = crear_indice_de_tablas();
    marcos_libres = crear_marcos_libres();
    indice_instrucciones = crear_indice_de_instrucciones();
    pthread_mutex_init(&mutex_memoria, NULL);
}

void retardo_de_peticion()
{
    usleep(obtener_retardo_respuesta() * 1000);
}

void atender_escritura_espacio_usuario(int sockete)
{
    uint32_t PID, direccion_fisica, tamanio;
    void *valor_a_escribir = NULL;
    recibir_escritura_espacio_usuario(sockete, &PID, &direccion_fisica, &valor_a_escribir, &tamanio);
    escribir_espacio_usuario(direccion_fisica, valor_a_escribir, tamanio);
    enviar_cod_op(OK, sockete);
    loggear_escritura_espacio_de_usuario(PID, direccion_fisica, tamanio);
    free(valor_a_escribir);
}

void recibir_escritura_espacio_usuario(int sockete, uint32_t *PID, uint32_t *direccion_fisica,
                                       void **valor_a_escribir, uint32_t *tamanio)
{
    t_list *paquete_escritura_espacio = recibir_paquete(sockete);
    *PID = *(uint32_t *)list_get(paquete_escritura_espacio, 0);
    *direccion_fisica = *(uint32_t *)list_get(paquete_escritura_espacio, 1);
    void *valor_temp = list_get(paquete_escritura_espacio, 2);
    *tamanio = *(uint32_t *)list_get(paquete_escritura_espacio, 3);
    *valor_a_escribir = malloc_or_die(*tamanio, "No se pudo reservar tamaño para el valor a escribir");
    memcpy(*valor_a_escribir, valor_temp, *tamanio);
    list_destroy_and_destroy_elements(paquete_escritura_espacio, free);
}

void escribir_espacio_usuario(uint32_t direccion, void *valor, uint32_t tamanio)
{
    pthread_mutex_lock(&mutex_memoria);
    memcpy(espacio_usuario + direccion, valor, tamanio);
    pthread_mutex_unlock(&mutex_memoria);
}

void atender_lectura_espacio_usuario(int sockete)
{
    uint32_t PID, direccion_fisica, tamanio_a_leer;
    recibir_lectura_espacio_usuario(sockete, &PID, &direccion_fisica, &tamanio_a_leer);
    void *valor_leido = leer_espacio_usuario(direccion_fisica, tamanio_a_leer);
    enviar_valor_leido(sockete, valor_leido, tamanio_a_leer);
    free(valor_leido);
    loggear_lectura_espacio_de_usuario(PID, direccion_fisica, tamanio_a_leer);
}

void recibir_lectura_espacio_usuario(int sockete, uint32_t *PID, uint32_t *direccion_fisica, uint32_t *tamanio_a_leer)
{
    t_list *paquete_lectura_espacio = recibir_paquete(sockete);
    *PID = *(uint32_t *)list_get(paquete_lectura_espacio, 0);
    *direccion_fisica = *(uint32_t *)list_get(paquete_lectura_espacio, 1);
    *tamanio_a_leer = *(uint32_t *)list_get(paquete_lectura_espacio, 2);
    list_destroy_and_destroy_elements(paquete_lectura_espacio, free);
}

void *leer_espacio_usuario(uint32_t direccion, uint32_t tamanio_a_leer)
{
    void *valor = malloc(tamanio_a_leer);
    pthread_mutex_lock(&mutex_memoria);
    memcpy(valor, espacio_usuario + direccion, tamanio_a_leer);
    pthread_mutex_unlock(&mutex_memoria);
    return valor;
}

void enviar_valor_leido(int sockete, void *valor_leido, uint32_t tamanio_valor)
{
    t_paquete *paquete_valor_leido = crear_paquete(OK);
    agregar_a_paquete(paquete_valor_leido, valor_leido, tamanio_valor);
    enviar_paquete(paquete_valor_leido, sockete);
    eliminar_paquete(paquete_valor_leido);
}
