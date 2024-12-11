#ifndef UTILS_INSTRUCCIONES_H_
#define UTILS_INSTRUCCIONES_H_

#include "registros_cpu.h"

// PROVISORIO solicitud_de_instruccion
typedef struct
{
    uint32_t desplazamiento;
} t_solicitud_de_instruccion;

t_solicitud_de_instruccion *crear_solicitud_de_instruccion();
void destruir_solicitud_de_instruccion(t_solicitud_de_instruccion *t_solicitud_de_instruccion);
void serializar_solicitud_de_instruccion(t_solicitud_de_instruccion *solicitud, t_paquete *paquete);
void generar_solicitud_de_instruccion(t_solicitud_de_instruccion *solicitud, t_list *valores);
int tamanio_solicitud_de_instruccion(t_solicitud_de_instruccion *solicitud);

typedef struct
{
    char *instruccion;
} t_instruccion_cadena;

t_instruccion_cadena *crear_instruccion();
void destruir_instruccion_cadena(t_instruccion_cadena *instruccion);
void serializar_instruccion(t_instruccion_cadena *instruccion, t_paquete *paquete);
void generar_instruccion(t_instruccion_cadena *instruccion, t_list *valores);
int tamanio_instruccion(t_instruccion_cadena *instruccion);

#endif