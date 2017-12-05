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

#ifndef _INTOYUN_DATAPOINT_H
#define _INTOYUN_DATAPOINT_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "intoyun_protocol.h"

#define DPID_DEFAULT_BOOL_GETALLDATAPOINT         0x7F81        //默认数据点  获取所有数据点

// transmit
typedef enum {
    DP_TRANSMIT_MODE_MANUAL = 0,       // 用户控制发送
    DP_TRANSMIT_MODE_AUTOMATIC,        // 系统自动发送
}dp_transmit_mode_t;

// Permission
typedef enum {
    DP_PERMISSION_UP_ONLY = 0,   //只上送
    DP_PERMISSION_DOWN_ONLY,     //只下发
    DP_PERMISSION_UP_DOWN        //可上送下发
}dp_permission_t;

// Policy
typedef enum {
    DP_POLICY_NONE = 0,         //立即发送
    DP_POLICY_TIMED,            //间隔发送
    DP_POLICY_ON_CHANGE         //改变发送
}dp_policy_t;

typedef enum{
    DATA_TYPE_BOOL = 0,   //bool型
    DATA_TYPE_NUM,        //数值型
    DATA_TYPE_ENUM,       //枚举型
    DATA_TYPE_STRING,     //字符串型
    DATA_TYPE_BINARY      //透传型
}data_type_t;

typedef enum{
    RESULT_DATAPOINT_OLD  = 0,   // 旧数据
    RESULT_DATAPOINT_NEW  = 1,   // 新收取数据
    RESULT_DATAPOINT_NONE = 2,   // 没有该数据点
}read_datapoint_result_t;

//float型属性
typedef struct {
    double minValue;
    double maxValue;
    int resolution;
}number_property_t;

//透传型属性
typedef struct {
    uint8_t *value;
    uint16_t len;
}binary_property_t;

// Property configuration
typedef struct {
    uint16_t dpID;
    data_type_t dataType;
    dp_permission_t permission;
    bool change; //数据是否改变 true 数据有变化
    read_datapoint_result_t readFlag;
    number_property_t numberProperty;
    bool boolValue;
    int32_t numberIntValue;
    double numberDoubleValue;
    int enumValue;
    char *stringValue;
    binary_property_t binaryValue;
}property_conf;

//datapoint control
typedef struct {
    dp_transmit_mode_t datapoint_transmit_mode;  // 数据点发送类型
    uint32_t datapoint_transmit_lapse;           // 数据点自动发送 时间间隔
    long runtime;                                // 数据点间隔发送的运行时间
}datapoint_control_t;


// typedef void (*event_handler_t)(uint8_t eventType, uint8_t param, uint8_t *data, uint32_t len);
typedef void (*event_handler_t)(system_event_t event, int param, uint8_t *data, uint16_t len);


//System API
void intoyunInit(void);
void intoyunLoop(void);
void intoyunSetEventCallback(event_handler_t handler);
bool intoyunSetMode(mode_type_t mode, uint32_t timeout);
mode_type_t intoyunGetMode(void);
void intoyunDatapointControl(dp_transmit_mode_t mode, uint32_t lapse);
void intoyunSetDevice(char *productId, char *productSecret, char *hardVer, char *softVer);
void intoyunGetDevice(char *productId, char *hardVer, char *softVer);
void intoyunGetInfo(char *moduleVersion, char *moduleType, char *deviceId, uint8_t *at_mode);
bool intoyunExecuteRestart(void);
bool intoyunExecuteRestore(void);
void intoyunPutPipe(uint8_t value);
bool intoyunGetNetTime(char *net_time, char *timestamp);
uint8_t intoyunGetStatus(char *ssid, uint32_t *ipAddr, int *rssi);

void intoyunConnect(void);
bool intoyunConnected(void);
void intoyunDisconnect(void);
bool intoyunDisconnected(void);

//datapoint API
void intoyunDefineDatapointBool(const uint16_t dpID, dp_permission_t permission, const bool value);
void intoyunDefineDatapointNumber(const uint16_t dpID, dp_permission_t permission, const double minValue, const double maxValue, const int resolution, const double value);
void intoyunDefineDatapointEnum(const uint16_t dpID, dp_permission_t permission, const int value);
void intoyunDefineDatapointString(const uint16_t dpID, dp_permission_t permission, const char *value);
void intoyunDefineDatapointBinary(const uint16_t dpID, dp_permission_t permission, const uint8_t *value, const uint16_t len);

read_datapoint_result_t intoyunReadDatapointBool(const uint16_t dpID, bool *value);
read_datapoint_result_t intoyunReadDatapointNumberInt32(const uint16_t dpID, int32_t *value);
read_datapoint_result_t intoyunReadDatapointNumberDouble(const uint16_t dpID, double *value);
read_datapoint_result_t intoyunReadDatapointEnum(const uint16_t dpID, int *value);
read_datapoint_result_t intoyunReadDatapointString(const uint16_t dpID, char *value);
read_datapoint_result_t intoyunReadDatapointBinary(const uint16_t dpID, uint8_t *value, uint16_t *len);

void intoyunWriteDatapointBool(const uint16_t dpID, bool value);
void intoyunWriteDatapointNumberInt32(const uint16_t dpID, int32_t value);
void intoyunWriteDatapointNumberDouble(const uint16_t dpID, double value);
void intoyunWriteDatapointEnum(const uint16_t dpID, int value);
void intoyunWriteDatapointString(const uint16_t dpID, const char *value);
void intoyunWriteDatapointBinary(const uint16_t dpID, const uint8_t *value, uint16_t len);

void intoyunSendDatapointBool(const uint16_t dpID, bool value);
void intoyunSendDatapointNumberInt32(const uint16_t dpID, int32_t value);
void intoyunSendDatapointNumberDouble(const uint16_t dpID, double value);
void intoyunSendDatapointEnum(const uint16_t dpID, int value);
void intoyunSendDatapointString(const uint16_t dpID, const char *value);
void intoyunSendDatapointBinary(const uint16_t dpID, const uint8_t *value, uint16_t len);

void intoyunParseReceiveDatapoints(const uint8_t *payload, uint32_t len, uint8_t *customData);
static void intoyunSendDatapointAll(bool dpForm);
static void intoyunSendSingleDatapoint(const uint16_t dpID);
void intoyunSendCustomData(const uint8_t *buffer, uint16_t len);
void intoyunSendAllDatapointManual(void);
void intoyunSendDatapointAutomatic(void);

extern event_handler_t eventHandler;
extern bool cloudConnected;
extern bool moduleConnectNetwork;

#endif /*_DATAPOINT_H*/
