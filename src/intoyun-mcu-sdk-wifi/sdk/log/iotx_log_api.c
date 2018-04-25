/*
 * Copyright (c) 2013-2018 Molmc Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "iot_import.h"
#include "sdk_config.h"
#include "iotx_log_api.h"


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

