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

#ifndef __IOTX_SYSTEM_API_H__
#define __IOTX_SYSTEM_API_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "iot_import.h"
#include "iotx_protocol_api.h"

typedef enum {
    event_mode_changed   = 1,
    event_network_status = 2,
    event_cloud_status   = 3,
    event_cloud_comm     = 4,
} iotx_system_event_t;

typedef enum {
    //mode change
    ep_mode_normal                     = 1, //正常工作模式
    ep_mode_imlink_config              = 2, //imlink配置模式
    ep_mode_ap_config                  = 3, //ap配置模式
    ep_mode_binding                    = 4,

    //network status
    ep_network_status_disconnected     = 1, //已断开路由器
    ep_network_status_connected        = 2, //已连接路由器
    //Cloud connection status
    ep_cloud_status_disconnected       = 3, //已断开连服务器
    ep_cloud_status_connected          = 4, //已连服务器

    //cloud
    ep_cloud_comm_data                 = 1, //接收到云端数据
    ep_cloud_comm_ota                  = 2, //接收到云端OTA升级事件
} iotx_system_events_param_t;

typedef void (*event_handler_t)(int event, int param, uint8_t *data, uint32_t len);

typedef enum
{
    IOTX_WORK_MODE_NORMAL = 1,        //正常工作模式
    IOTX_WORK_MODE_IMLINK_CONFIG,     //ImLink配置模式
    IOTX_WORK_MODE_AP_CONFIG,         //为AP配置模式
    IOTX_WORK_MODE_BINDING,           //绑定模式
} iotx_work_mode_t;

iotx_work_mode_t iotx_get_work_mode(void);
void iotx_set_work_mode(iotx_work_mode_t newMode);

void IOT_SYSTEM_SetDeviceInfo(char *productID, char *productSecret, char *hardwareVersion, char *softwareVersion);
void IOT_SYSTEM_GetModuleInfo(char *moduleVersion, char *moduleType, char *deviceId, uint8_t *at_mode);
void IOT_SYSTEM_Init(void);
void IOT_SYSTEM_Loop(void);
void IOT_SYSTEM_SetEventCallback(event_handler_t handler);
void IOT_SYSTEM_NotifyEvent(iotx_system_event_t event, iotx_system_events_param_t param, uint8_t *data, uint32_t len);
bool IOT_SYSTEM_SetMode(iotx_work_mode_t mode, uint32_t timeout);
iotx_work_mode_t IOT_SYSTEM_GetMode(void);
bool IOT_SYSTEM_Restart(void);
bool IOT_SYSTEM_Restore(void);
void IOT_SYSTEM_PutPipe(uint8_t value);
bool IOT_SYSTEM_GetNetTime(char *net_time, char *timestamp);
uint8_t IOT_SYSTEM_GetStatus(char *ssid, uint32_t *ipAddr, int *rssi);

#ifdef __cplusplus
}
#endif

#endif  /* #ifndef _IOTX_SYSTEM_API_H_ */

