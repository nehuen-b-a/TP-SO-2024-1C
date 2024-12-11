#ifndef MEMORIA_CONEXIONES_H
#define MEMORIA_CONEXIONES_H

#include <utils/funcionalidades_basicas.h>
#include <utils/comunicacion/comunicacion.h>
#include "atender_kernel.h"
#include "atender_cpu.h"
#include "atender_io.h"

extern t_log *logger_propio;
extern int server_fd;

void iniciar_semaforo_para_peticiones(void);

void esperar_a(char *tipo, int *socket, int server);

void iniciar_servidor_memoria(void);
void iniciar_conexiones();

#endif
