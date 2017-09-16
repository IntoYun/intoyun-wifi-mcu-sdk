#ifndef _TIME_H
#define _TIME_H

#include "stm32f1xx_hal.h"

//延时　单位ms
void delay(uint32_t ms);

//获取滴答时钟的计数值
uint32_t millis(void);

#endif
