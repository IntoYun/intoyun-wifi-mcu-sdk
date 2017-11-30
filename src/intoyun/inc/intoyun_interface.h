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
#ifndef _INTOYUN_INTERFACE_H
#define _INTOYUN_INTERFACE_H

#include "intoyun_datapoint.h"
#include "intoyun_protocol.h"
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
    void (*sendCustomData)(const uint8_t *buffer, uint16_t len);
}cloud_t;

void delay(uint32_t ms);
uint32_t millis(void);
uint32_t timerGetId(void);
bool timerIsEnd(uint32_t timerID, uint32_t time);

extern const cloud_t Cloud;
extern const system_t System;


#endif /*_INTERFACE_H*/
