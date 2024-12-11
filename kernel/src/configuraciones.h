#ifndef KERNEL_CONFIGURACIONES_H
#define KERNEL_CONFIGURACIONES_H

#include <commons/log.h>
#include <commons/config.h>

#define obtener_puerto_escucha() config_get_string_value(config, "PUERTO_ESCUCHA")
#define obtener_ip_memoria() config_get_string_value(config, "IP_MEMORIA")
#define obtener_puerto_memoria() config_get_string_value(config, "PUERTO_MEMORIA")
#define obtener_ip_cpu() config_get_string_value(config, "IP_CPU")
#define obtener_puerto_cpu_dispatch() config_get_string_value(config, "PUERTO_CPU_DISPATCH")
#define obtener_puerto_cpu_interrupt() config_get_string_value(config, "PUERTO_CPU_INTERRUPT")
#define obtener_algoritmo_planificacion() config_get_string_value(config, "ALGORITMO_PLANIFICACION")
#define obtener_quantum() config_get_int_value(config, "QUANTUM")
#define obtener_recursos() config_get_array_value(config, "RECURSOS")
#define obtener_instancias_recursos() config_get_array_value(config, "INSTANCIAS_RECURSOS")
#define obtener_grado_multiprogramacion() config_get_int_value(config, "GRADO_MULTIPROGRAMACION")
#define obtener_path_kernel() config_get_string_value(config, "PATH_KERNEL")

extern t_log *logger_obligatorio;
extern t_log *logger_propio;
extern t_config *config;
extern char *estados[5];
extern char *motivos[];

// Logs minimos y obligatorios
void loggear_creacion_proceso(int pcbPID);
void loggear_fin_de_proceso(int PID, int motivo);
void loggear_cambio_de_estado(int PID, int anterior, int actual);
void loggear_motivo_de_bloqueo(int PID, char *interfaz_o_recurso);
void loggear_fin_de_quantum(int PID);
void loggear_ingreso_a_READY(char *lista_PIDS, bool cola_prioritaria);

#endif