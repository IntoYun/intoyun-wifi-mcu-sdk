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

#ifndef INTOYUN_TIMER_H__
#define INTOYUN_TIMER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

typedef void (*cbTimerFunc)(void);

typedef struct timer_param_s{
    uint8_t     timerNum;    //定时编号
    uint32_t    period;      //定时周期
    bool        oneShot;     //true只执行一次
    bool        start;       //开始启动
    uint32_t    timerTick;   //定时计数
    cbTimerFunc timerCbFunc; //定时回调
    struct timer_param_s *next;
}timer_t;

void intoyunTimerRegister(uint8_t num, uint32_t period, bool oneShot, cbTimerFunc cbFunc);
void intoyunTimerChangePeriod(uint8_t num, uint32_t period);
void intoyunTimerStart(uint8_t num);
void intoyunTimerStop(uint8_t num);
void intoyunTimerReset(uint8_t num);
void intoyunTimerLoop(void);


#ifdef __cplusplus
}
#endif

#endif /* INTOYUN_TIMER_H__ */

