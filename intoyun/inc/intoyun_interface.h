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

typedef struct
{
    void (*init)(void);
    void (*setEventCallback)(event_handler_t handler);
    void (*setMode)(mode_type_t mode, uint32_t timeout);
    void (*loop)(void);
    void (*setDatapointControl)(dp_transmit_mode_t mode, uint32_t lapse);
    void (*sendProductInfo)(char *productId, char *hardVer, char *softVer);

    //定义数据点
    void (*defineDatapointBool)(const uint16_t dpID, dp_permission_t permission, const bool value, dp_policy_t policy, const int lapse);
    void (*defineDatapointNumber)(const uint16_t dpID, dp_permission_t permission, const double minValue, const double maxValue, const int resolution, const double value, dp_policy_t policy, const int lapse);
    void (*defineDatapointEnum)(const uint16_t dpID, dp_permission_t permission, const int value, dp_policy_t policy, const int lapse);
    void (*defineDatapointString)(const uint16_t dpID, dp_permission_t permission, const char *value, dp_policy_t policy, const int lapse);
    void (*defineDatapointBinary)(const uint16_t dpID, dp_permission_t permission, const uint8_t *value, const uint16_t len, dp_policy_t policy, const int lapse);

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
    void (*sendDatapointBool)(const uint16_t dpID, bool value);
    void (*sendDatapointNumberInt32)(const uint16_t dpID, int32_t value);
    void (*sendDatapointNumberDouble)(const uint16_t dpID, double value);
    void (*sendDatapointEnum)(const uint16_t dpID, int value);
    void (*sendDatapointString)(const uint16_t dpID, const char *value);
    void (*sendDatapointBinary)(const uint16_t dpID, const uint8_t *value, uint16_t len);
    void (*sendDatapointAll)(void);
    void (*sendCustomData)(const uint8_t *buffer, uint16_t len);
}intoyun_t;

extern const intoyun_t IntoYun;


#endif /*_INTERFACE_H*/
