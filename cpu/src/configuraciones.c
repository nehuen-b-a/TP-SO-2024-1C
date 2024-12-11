#include <configuraciones.h>

void loggear_fetch_instrucccion(uint32_t PID, uint32_t PC)
{
    log_info(logger_obligatorio, "Fetch Instrucción: PID: <%d> - FETCH - Program Counter: <%d>", PID, PC);
}
/*
//TODO: Varia cantidad de parametros. (Me la complica) Mas fácil log info para cada instruccion.
void loggear_instrucion_ejecutada(uint32_t PID, t_instruccion instruccion)
{
    log_info(logger_obligatorio, "Instrucción Ejecutada: PID: <%d> - Ejecutando: <%s> - <PARAMETROS>", PID, instruccion->id); /
}
*/
void loggear_tlb_hit(uint32_t PID, int numero_pagina)
{
    log_info(logger_obligatorio, "PID: <%d> - TLB HIT - Pagina: <%d>", PID, numero_pagina);
}

void loggear_tlb_miss(uint32_t PID, int numero_pagina)
{
    log_info(logger_obligatorio, "PID: <%d> - TLB MISS - Pagina: <%d>", PID, numero_pagina);
}

void loggear_obtener_marco(uint32_t PID, int numero_pagina, int numero_marco)
{
    log_info(logger_obligatorio, "PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>", PID, numero_pagina, numero_marco);
}

void loggear_lectura_memoria_num(uint32_t PID, uint32_t direccion_fisica, uint32_t valor)
{
    log_info(logger_obligatorio, "PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%d>", PID, direccion_fisica, valor);
}

void loggear_lectura_memoria_char(uint32_t PID, uint32_t direccion_fisica, char *valor)
{
    log_info(logger_obligatorio, "PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%s>", PID, direccion_fisica, valor);
}

void loggear_escritura_memoria_num(uint32_t PID, uint32_t direccion_fisica, uint32_t valor)
{
    log_info(logger_obligatorio, "PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%d>", PID, direccion_fisica, valor);
}

void loggear_escritura_memoria_char(uint32_t PID, uint32_t direccion_fisica, char *valor)
{
    log_info(logger_obligatorio, "PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%s>", PID, direccion_fisica, valor);
}