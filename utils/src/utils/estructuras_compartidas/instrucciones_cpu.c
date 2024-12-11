#include "instrucciones_cpu.h"

char *nombres_de_instrucciones[] = {
    [SET] = "SET",
    [MOV_IN] = "MOV_IN",
    [MOV_OUT] = "MOV_OUT",
    [SUM] = "SUM",
    [SUB] = "SUB",
    [JNZ] = "JNZ",
    [RESIZE] = "RESIZE",
    [COPY_STRING] = "COPY_STRING",
    [WAIT] = "WAIT",
    [SIGNAL] = "SIGNAL",
    [IO_GEN_SLEEP] = "IO_GEN_SLEEP",
    [IO_STDIN_READ] = "IO_STDIN_READ",
    [IO_STDOUT_WRITE] = "IO_STDOUT_WRITE",
    [IO_FS_CREATE] = "IO_FS_CREATE",
    [IO_FS_DELETE] = "IO_FS_DELETE",
    [IO_FS_TRUNCATE] = "IO_FS_TRUNCATE",
    [IO_FS_WRITE] = "IO_FS_WRITE",
    [IO_FS_READ] = "IO_FS_READ",
    [EXIT] = "EXIT"};