#include "atender_io.h"

void atender_io(int *socket_cliente)
{
    int op_code = 0;
    while (op_code != -1)
    {
        op_code = recibir_operacion(*socket_cliente);
        retardo_de_peticion();
        switch (op_code)
        {
        case ACCESO_ESPACIO_USUARIO_ESCRITURA:
            atender_escritura_espacio_usuario(*socket_cliente);
            break;
        case ACCESO_ESPACIO_USUARIO_LECTURA:
            atender_lectura_espacio_usuario(*socket_cliente);
            break;
        case -1:
            log_info(logger_propio, "Conexión con IO cerrada.");
            break;
        default:
            log_info(logger_propio, "Codigo de operacion incorrecto en atender_io");
            break;
        }
    }

    free(socket_cliente);
}
