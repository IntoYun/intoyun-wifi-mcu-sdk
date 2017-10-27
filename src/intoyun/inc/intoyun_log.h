/**
 ******************************************************************************
  Copyright (c) 2013-2014 IntoRobot Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
  ******************************************************************************
*/

#ifndef INTOYUN_LOG_H__
#define INTOYUN_LOG_H__

#include "hal_interface.h"
#include "intoyun_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define INTOYUN_LOG_LEVEL_NONE       (0)
#define INTOYUN_LOG_LEVEL_ERROR      (1)
#define INTOYUN_LOG_LEVEL_WARN       (2)
#define INTOYUN_LOG_LEVEL_INFO       (3)
#define INTOYUN_LOG_LEVEL_DEBUG      (4)
#define INTOYUN_LOG_LEVEL_VERBOSE    (5)

#ifndef CONFIG_INTOYUN_LOG_DEFAULT_LEVEL
#define CONFIG_INTOYUN_LOG_DEFAULT_LEVEL INTOYUN_LOG_LEVEL_NONE
#endif

#ifndef INTOYUN_DEBUG_LEVEL
#define INTOYUN_LOG_LEVEL CONFIG_INTOYUN_LOG_DEFAULT_LEVEL
#else
#define INTOYUN_LOG_LEVEL INTOYUN_DEBUG_LEVEL
#endif

#ifndef CONFIG_INTOYUN_LOG_COLORS
#define CONFIG_INTOYUN_LOG_COLORS 0
#endif

#if CONFIG_INTOYUN_LOG_COLORS
#define INTOYUN_LOG_COLOR_BLACK   "30"
#define INTOYUN_LOG_COLOR_RED     "31" //ERROR
#define INTOYUN_LOG_COLOR_GREEN   "32" //INFO
#define INTOYUN_LOG_COLOR_YELLOW  "33" //WARNING
#define INTOYUN_LOG_COLOR_BLUE    "34"
#define INTOYUN_LOG_COLOR_MAGENTA "35"
#define INTOYUN_LOG_COLOR_CYAN    "36" //DEBUG
#define INTOYUN_LOG_COLOR_GRAY    "37" //VERBOSE
#define INTOYUN_LOG_COLOR_WHITE   "38"

#define INTOYUN_LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define INTOYUN_LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define INTOYUN_LOG_RESET_COLOR   "\033[0m"

#define INTOYUN_LOG_COLOR_E       INTOYUN_LOG_COLOR(LORAWAN_LOG_COLOR_RED)
#define INTOYUN_LOG_COLOR_W       INTOYUN_LOG_COLOR(LORAWAN_LOG_COLOR_YELLOW)
#define INTOYUN_LOG_COLOR_I       INTOYUN_LOG_COLOR(LORAWAN_LOG_COLOR_GREEN)
#define INTOYUN_LOG_COLOR_D       INTOYUN_LOG_COLOR(LORAWAN_LOG_COLOR_CYAN)
#define INTOYUN_LOG_COLOR_V       INTOYUN_LOG_COLOR(LORAWAN_LOG_COLOR_GRAY)
#else
#define INTOYUN_LOG_COLOR_E
#define INTOYUN_LOG_COLOR_W
#define INTOYUN_LOG_COLOR_I
#define INTOYUN_LOG_COLOR_D
#define INTOYUN_LOG_COLOR_V
#define INTOYUN_LOG_RESET_COLOR
#endif

const char * pathToFileName(const char * path);
void _log_print(const char *fmt, ...);
void _log_print_dump(uint8_t *buf, uint16_t len);

#define INTOYUN_SHORT_LOG_FORMAT(letter, format)  INTOYUN_LOG_COLOR_ ## letter format INTOYUN_LOG_RESET_COLOR ""
#define INTOYUN_LOG_FORMAT(letter, format)  INTOYUN_LOG_COLOR_ ## letter "[" #letter "][%20s:%u] %15s() --> " format INTOYUN_LOG_RESET_COLOR "", pathToFileName(__FILE__), __LINE__, __FUNCTION__

#if INTOYUN_LOG_LEVEL >= INTOYUN_LOG_LEVEL_VERBOSE
#define log_v(format, ...) _log_print(INTOYUN_SHORT_LOG_FORMAT(V, format), ##__VA_ARGS__)
#define log_v_dump _log_print_dump
#else
#define log_v(format, ...)
#define log_v_dump(a, b)
#endif

#if INTOYUN_LOG_LEVEL >= INTOYUN_LOG_LEVEL_DEBUG
#define log_d(format, ...) _log_print(INTOYUN_SHORT_LOG_FORMAT(D, format), ##__VA_ARGS__)
#define log_d_dump _log_print_dump
#else
#define log_d(format, ...)
#define log_d_dump(a, b)
#endif

#if INTOYUN_LOG_LEVEL >= INTOYUN_LOG_LEVEL_INFO
#define log_i(format, ...) _log_print(INTOYUN_SHORT_LOG_FORMAT(I, format), ##__VA_ARGS__)
#define log_i_dump _log_print_dump
#else
#define log_i(format, ...)
#define log_i_dump(a, b)
#endif

#if INTOYUN_LOG_LEVEL >= INTOYUN_LOG_LEVEL_WARN
#define log_w(format, ...) _log_print(INTOYUN_SHORT_LOG_FORMAT(W, format), ##__VA_ARGS__)
#define log_w_dump _log_print_dump
#else
#define log_w(format, ...)
#define log_w_dump(a, b)
#endif

#if INTOYUN_LOG_LEVEL >= INTOYUN_LOG_LEVEL_ERROR
#define log_e(format, ...) _log_print(INTOYUN_SHORT_LOG_FORMAT(E, format), ##__VA_ARGS__)
#define log_e_dump _log_print_dump
#else
#define log_e(format, ...)
#define log_e_dump(a, b)
#endif

#if !defined(CONFIG_NOASSERT)
#define ASSERT(cond) if(!(cond)) _log_failed(__FILE__, __LINE__)
#else
#define ASSERT(cond) /**/
#endif

void log_int(void);

#ifdef __cplusplus
}
#endif

#endif /* INTOYUN_LOG_H__ */

