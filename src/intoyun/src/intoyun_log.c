#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "intoyun_log.h"
#include "intoyun_interface.h"
#include "hal_interface.h"

#define MAX_DEBUG_MESSAGE_LENGTH        512

#if !defined(arraySize)
#   define arraySize(a)            (sizeof((a))/sizeof((a[0])))
#endif

const char * pathToFileName(const char * path)
{
    size_t i = 0;
    size_t pos = 0;
    char * p = (char *)path;
    while(*p){
        i++;
        if(*p == '/' || *p == '\\') {
            pos = i;
        }
        p++;
    }
    return path+pos;
}

void log_int(void)
{

}

void _log_print(const char *fmt, ...)
{
    char _buffer[MAX_DEBUG_MESSAGE_LENGTH];
    va_list args;

    va_start(args, fmt);
    int trunc = snprintf(_buffer, arraySize(_buffer), "[%010u] ", millis());
    HAL_Print(_buffer,trunc);

    trunc = vsnprintf(_buffer, arraySize(_buffer), fmt, args);
    HAL_Print(_buffer,trunc);

    if (trunc > arraySize(_buffer)) {
        HAL_Print("...",3);
    }
    va_end(args);
}

void _log_print_dump(uint8_t *buf, uint16_t len)
{
    int i = 0;
    char buffer[8];

    if(len>0){
        for(i = 0; i < len-1; i++) {
            memset(buffer,0,sizeof(buffer));
            sprintf(buffer, "%02x:", buf[i]);
            HAL_Print(buffer,sizeof(buffer));
        }
        sprintf(buffer, "%02x\r\n", buf[i]);
        HAL_Print(buffer,sizeof(buffer));
    }
}

void _log_failed(const char *file, uint16_t line) {
    log_e("Program failed in file: %s, line: %d\r\n", file, line);
    while(1) {
    }
}

