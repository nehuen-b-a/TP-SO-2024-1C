#ifndef ENTRADASALIDA_SISTEMA_ARCHIVOS_H
#define ENTRADASALIDA_SISTEMA_ARCHIVOS_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <utils/funcionalidades_basicas.h>
#include <utils/comunicacion/comunicacion.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <dirent.h>
#include "configuraciones.h"

extern t_config *metadata;
extern t_bitarray *bitmap;
extern t_list *fcbs;

typedef struct
{
    char *nombre;
    int bloque_inicial;
    int tamanio_en_bytes;
} t_fcb;

void crear_archivo(uint32_t *PID, char *nombre);
void eliminar_archivo(uint32_t *PID, char *nombre);
void truncar_archivo(uint32_t *PID, char *nombre, int tam);
bool validar_compactacion(int tam_nuevo_en_bloques, t_fcb *fcb);
void *leer_archivo(uint32_t *PID, char *nombre, int tam, int puntero);
void escribir_archivo(uint32_t *PID, char *nombre, int tam, int puntero, char *dato_a_escribir);
void iniciar_bitmap(void);
void leer_bloques(void);
void leer_fcbs(void);
void cargar_fcb(t_fcb *fcb);
bool ordenar_fcb_por_bloque_inicial(void *fcb1, void *fcb2);
int obtener_bloque_libre(void);

int bloque_inicial(char *archivo);
int tamanio_en_bloques(char *archivo);
t_fcb *metadata_de_archivo(char *archivo);
void liberar_archivo(char *archivo);
void eliminar_metadata(char *archivo);
void destruir_fcb(void *data);
void actualizar_metadata(t_fcb *fcb);
// mueve el fcb al nuevo inicio, pisando lo que haya allí
void mover_fcb(t_fcb *fcb, int nuevo_inicio);
void mover_contenido_fcb(t_fcb *fcb, int nuevo_inicio, void *src_contenido, bool hacer_clean, int tamanio_a_truncar_en_bytes);
void compactar(uint32_t *PID, t_fcb *archivo_a_truncar, int tamanio_execedente_en_bloques);
int bytes_a_bloques(int bytes);
char *leer_bitmap(const t_bitarray *bitmap, size_t indice_inico, size_t cantidad_de_bits);
void assignBlock(int blockIndex);
void unassignBlock(int blockIndex);

#endif