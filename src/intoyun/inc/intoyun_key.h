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

#ifndef INTOYUN_KEY_H__
#define INTOYUN_KEY_H__

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

typedef void (*cbInitFunc)(void);          //按键初始化回调函数
typedef uint16_t (*cbGetValueFunc)(void);  //获取按键值回调函数
typedef void (*cbClickFunc)(void);         //单击双击回调函数
typedef void (*cbPressFunc)(uint32_t ms);  //长按回调函数 ms为长按键时间

typedef struct key_cb_s {
    uint8_t    keyNum; //按键编号
    cbInitFunc  cbKeyInitFunc; //初始化回调
    cbGetValueFunc cbKeyGetValueFunc; //获取按键电平回调
    cbClickFunc cbKeyClickFunc;
    cbClickFunc cbKeyDoubleClickFunc;
    cbPressFunc cbKeyPressStartFunc; //按键按下回调
    cbPressFunc cbKeyPressStopFunc; //按键松开回调
    cbPressFunc cbKeyPressDuringFunc; //按键长按间隔回调
    struct key_cb_s *next;
}key_t;

typedef struct key_param_s{
    uint8_t    _state;
    uint32_t   _debounceTime; //抖动时间
    uint32_t   _clickTime;  //连击时间
    uint32_t   _pressTime; //长按间隔回调时间 比如1000表示每隔1000ms回调长按键函数
    uint8_t    _buttonReleased;
    uint8_t    _buttonPressed;
    bool       _isLongPressed;
    uint32_t   _startTime;
    uint32_t   _stopTime;
}key_param_t;

enum keyCbFuncType{
    KEY_CLICK_CB        = 1,
    KEY_DOUBLE_CLICK_CB = 2,
    KEY_PRESS_SATRT_CB  = 3,
    KEY_PRESS_STOP_CB   = 4,
    KEY_PRESS_DURING_CB = 5,
};


void intoyunKeyInit(void);
void intoyunKeySetParams(bool invert, uint32_t debounceTime, uint32_t clickTime, uint32_t pressTime);
void intoyunKeyRegister(uint8_t num, cbInitFunc initFunc, cbGetValueFunc getValFunc);
void intoyunKeyClickCb(uint8_t num, cbClickFunc cbFunc);
void intoyunKeyDoubleClickCb(uint8_t num, cbClickFunc cbFunc);
void intoyunKeyPressStartCb(uint8_t num, cbPressFunc cbFunc);
void intoyunKeyPressStopCb(uint8_t num, cbPressFunc cbFunc);
void intoyunKeyPressDuringCb(uint8_t num, cbPressFunc cbFunc);
void intoyunKeyLoop(void);


#ifdef __cplusplus
}
#endif

#endif /* INTOYUN_LOG_H__ */

