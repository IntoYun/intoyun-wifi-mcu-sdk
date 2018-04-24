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

#ifndef __IOT_EXPORT_CLOUD_H__
#define __IOT_EXPORT_CLOUD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "intoyun_datapoint.h"
#include "intoyun_protocol.h"
#include "intoyun_key.h"
#include "intoyun_timer.h"
#include "intoyun_config.h"
#include "intoyun_log.h"

#define UINT_MAX    0xFFFFFFFF

typedef struct
{
    void (*init)(void);
    void (*loop)(void);

    void (*setEventCallback)(event_handler_t handler);
    bool (*setModuleMode)(mode_type_t mode, uint32_t timeout);
    mode_type_t (*getModuleMode)(void);
    void (*setDatapointControl)(dp_transmit_mode_t mode, uint32_t lapse);
    void (*setDeviceInfo)(char *productId, char *productSecret, char *hardVer, char *softVer);  //设置设备信息
    void (*getDeviceInfo)(char *productId, char *hardVer, char *softVer);  //获取设备信息
    void (*getModuleInfo)(char *moduleVersion, char *moduleType, char *deviceId, uint8_t *atMode); //获取模块信息
    bool (*resetModule)(void);
    bool (*restoreModule)(void);
    void (*putPipe)(uint8_t value);
    bool (*getNetTime)(char *net_time, char *timestamp);
    uint8_t (*getStatus)(char *ssid, uint32_t *ipAddr, int *rssi);
}system_t;

typedef struct
{
    void (*connect)(void);
    bool (*connected)(void);
    void (*disconnect)(void);
    bool (*disconnected)(void);

    #ifdef CONFIG_INTOYUN_DATAPOINT
    //定义数据点
    void (*defineDatapointBool)(const uint16_t dpID, dp_permission_t permission, const bool value);
    void (*defineDatapointNumber)(const uint16_t dpID, dp_permission_t permission, const double minValue, const double maxValue, const int resolution, const double value);
    void (*defineDatapointEnum)(const uint16_t dpID, dp_permission_t permission, const int value);
    void (*defineDatapointString)(const uint16_t dpID, dp_permission_t permission, const char *value);
    void (*defineDatapointBinary)(const uint16_t dpID, dp_permission_t permission, const uint8_t *value, const uint16_t len);

    //读取数据点
    read_datapoint_result_t (*readDatapointBool)(const uint16_t dpID, bool *value);
    read_datapoint_result_t (*readDatapointNumberInt32)(const uint16_t dpID, int32_t *value);
    read_datapoint_result_t (*readDatapointNumberDouble)(const uint16_t dpID, double *value);
    read_datapoint_result_t (*readDatapointEnum)(const uint16_t dpID, int *value);
    read_datapoint_result_t (*readDatapointString)(const uint16_t dpID, char *value);
    read_datapoint_result_t (*readDatapointBinary)(const uint16_t dpID, uint8_t *value, uint16_t *len);

    //写数据点
    void (*writeDatapointBool)(const uint16_t dpID, bool value);
    void (*writeDatapointNumberInt32)(const uint16_t dpID, int32_t value);
    void (*writeDatapointNumberDouble)(const uint16_t dpID, double value);
    void (*writeDatapointEnum)(const uint16_t dpID, int value);
    void (*writeDatapointString)(const uint16_t dpID, const char *value);
    void (*writeDatapointBinary)(const uint16_t dpID, const uint8_t *value, uint16_t len);

    //发送数据点值
    void (*sendDatapointBool)(const uint16_t dpID, bool value);
    void (*sendDatapointNumberInt32)(const uint16_t dpID, int32_t value);
    void (*sendDatapointNumberDouble)(const uint16_t dpID, double value);
    void (*sendDatapointEnum)(const uint16_t dpID, int value);
    void (*sendDatapointString)(const uint16_t dpID, const char *value);
    void (*sendDatapointBinary)(const uint16_t dpID, const uint8_t *value, uint16_t len);
    void (*sendDatapointAll)(void);
    #endif
    void (*sendCustomData)(const uint8_t *buffer, uint16_t len);
}cloud_t;

#ifdef CONFIG_INTOYUN_KEY

typedef struct {
    void (*init)(void);
    void (*setParams)(bool invert, uint32_t debounceTime, uint32_t clickTime, uint32_t pressTime);
    void (*keyRegister)(uint8_t num, cbInitFunc initFunc, cbGetValueFunc getValFunc);
    void (*attachClick)(uint8_t num, cbClickFunc cbFunc);           //注册单击处理函数
    void (*attachDoubleClick)(uint8_t num, cbClickFunc cbFunc);     //注册双击处理函数
    void (*attachLongPressStart)(uint8_t num, cbPressFunc cbFunc);  //注册按下按键处理函数
    void (*attachLongPressStop)(uint8_t num, cbPressFunc cbFunc);   //注册释放按键处理函数
    void (*attachDuringLongPress)(uint8_t num, cbPressFunc cbFunc); //注册按键按下回调函数
    void (*loop)(void);
}keys_t;

extern const keys_t Key;

#endif

#ifdef CONFIG_INTOYUN_TIMER

typedef struct {
    void (*timerRegister)(uint8_t num, uint32_t period, bool oneShot, cbTimerFunc cbFunc);
    void (*changePeriod)(uint8_t num, uint32_t period);
    void (*start)(uint8_t num);
    void (*stop)(uint8_t num);
    void (*reset)(uint8_t num);
    void (*loop)(void);
}timers_t;

extern const timers_t Timer;

#endif

void delay(uint32_t ms);
uint32_t millis(void);
uint32_t timerGetId(void);
bool timerIsEnd(uint32_t timerID, uint32_t time);

extern const cloud_t Cloud;
extern const system_t System;





#include "iotx_datapoint_api.h"

typedef struct
{
    int (*connect)(void);
    bool (*connected)(void);
    int (*disconnect)(void);

    //发送自定义数据
    int (*sendCustomData)(const uint8_t *buffer, uint16_t length);

    //数据点格式编程API接口
    //定义数据点
    void (*defineDatapointBool)(const uint16_t dpID, dp_permission_t permission, const bool value);
    void (*defineDatapointNumber)(const uint16_t dpID, dp_permission_t permission, const double minValue, const double maxValue, const int resolution, const double value);
    void (*defineDatapointEnum)(const uint16_t dpID, dp_permission_t permission, const int value);
    void (*defineDatapointString)(const uint16_t dpID, dp_permission_t permission, const char *value);
    void (*defineDatapointBinary)(const uint16_t dpID, dp_permission_t permission, const uint8_t *value, const uint16_t len);
    //读取数据点
    read_datapoint_result_t (*readDatapointBool)(const uint16_t dpID, bool *value);
    read_datapoint_result_t (*readDatapointNumberInt32)(const uint16_t dpID, int32_t *value);
    read_datapoint_result_t (*readDatapointNumberDouble)(const uint16_t dpID, double *value);
    read_datapoint_result_t (*readDatapointEnum)(const uint16_t dpID, int *value);
    read_datapoint_result_t (*readDatapointString)(const uint16_t dpID, char *value);
    read_datapoint_result_t (*readDatapointBinary)(const uint16_t dpID, uint8_t *value, uint16_t len);
    //写数据点
    void (*writeDatapointBool)(const uint16_t dpID, bool value);
    void (*writeDatapointNumberInt32)(const uint16_t dpID, int32_t value);
    void (*writeDatapointNumberDouble)(const uint16_t dpID, double value);
    void (*writeDatapointEnum)(const uint16_t dpID, int value);
    void (*writeDatapointString)(const uint16_t dpID, const char *value);
    void (*writeDatapointBinary)(const uint16_t dpID, const uint8_t *value, uint16_t len);
    //发送数据点值
    int (*sendDatapointBool)(const uint16_t dpID, bool value);
    int (*sendDatapointNumberInt32)(const uint16_t dpID, int32_t value);
    int (*sendDatapointNumberDouble)(const uint16_t dpID, double value);
    int (*sendDatapointEnum)(const uint16_t dpID, int value);
    int (*sendDatapointString)(const uint16_t dpID, const char *value);
    int (*sendDatapointBinary)(const uint16_t dpID, const uint8_t *value, uint16_t len);
    int (*sendDatapointAll)(void);
} iot_cloud_if_t;

extern const iot_cloud_if_t Cloud;

#ifdef __cplusplus
}
#endif

#endif

