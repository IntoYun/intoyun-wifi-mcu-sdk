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

#include <string.h>
#include "iotx_system_api.h"
#include "iotx_protocol_api.h"

static event_handler_t eventHandler = NULL;

void IOT_SYSTEM_SetDeviceInfo(char *productID, char *productSecret, char *hardwareVersion, char *softwareVersion)
{
    ProtocolSetupDevice(productID, hardwareVersion, softwareVersion);
}

void IOT_SYSTEM_GetModuleInfo(char *moduleVersion, char *moduleType, char *deviceId, uint8_t *at_mode)
{
    module_info_t info;
    if(!ProtocolQueryInfo(&info)) {
        return;
    }

    log_v("moduleVer = %s\r\n",info.module_version);
    log_v("moduleType = %s\r\n",info.module_type);
    log_v("deviceId = %s\r\n",info.device_id);
    log_v("atmode = %d\r\n",info.at_mode);

    strncpy(moduleVersion, info.module_version, sizeof(info.module_version));
    strncpy(moduleType, info.module_type, sizeof(info.module_type));
    strncpy(deviceId, info.device_id, sizeof(info.device_id));
    *at_mode = info.at_mode;
}

void IOT_SYSTEM_Init(void)
{
    IOT_Comm_Init();
}

void IOT_SYSTEM_Loop(void)
{
    IOT_Comm_Yield();
}

void IOT_SYSTEM_SetEventCallback(event_handler_t handler)
{
    if(handler != NULL) {
        eventHandler = handler;
    }
}

void IOT_SYSTEM_NotifyEvent(iotx_system_event_t event, iotx_system_events_param_t param, uint8_t *data, uint32_t len)
{
    if(eventHandler != NULL) {
        eventHandler(event, param, data, len);
    }
}

bool IOT_SYSTEM_SetMode(mode_type_t mode, uint32_t timeout)
{
    return ProtocolSetupMode((uint8_t)mode,timeout);
}

mode_type_t IOT_SYSTEM_GetMode(void)
{
    return (mode_type_t)ProtocolQueryMode();
}

bool IOT_SYSTEM_Restart(void)
{
    return ProtocolExecuteRestart();
}

bool IOT_SYSTEM_Restore(void)
{
    return ProtocolExecuteRestore();
}

void IOT_SYSTEM_PutPipe(uint8_t value)
{
    ProtocolPutPipe(value);//将接收到的数据放入缓冲区
}

bool IOT_SYSTEM_GetNetTime(char *net_time, char *timestamp)
{
    network_time_t netTime;

    if(!ProtocolQueryNetTime(&netTime)) {
        return false;
    }
    if(netTime.status == 1) {
        strncpy(net_time,netTime.net_time,sizeof(netTime.net_time));
        strncpy(timestamp,netTime.timestamp,sizeof(netTime.timestamp));
        return true;
    }
    return false;
}

uint8_t IOT_SYSTEM_GetStatus(char *ssid, uint32_t *ipAddr, int *rssi)
{
    module_status_t moduleStatus;

    if(!ProtocolQueryStatus(&moduleStatus)) {
        return 0;
    }

    if(moduleStatus.module_status != 1) {
        log_v("ssid=%s\r\n",moduleStatus.wifi.ssid);
        log_v("ipAddr=%d\r\n",moduleStatus.wifi.ipAddr);
        log_v("rssi=%d\r\n",moduleStatus.wifi.rssi);
        strncpy(ssid,moduleStatus.wifi.ssid,sizeof(moduleStatus.wifi.ssid));
        *ipAddr = moduleStatus.wifi.ipAddr;
        *rssi = moduleStatus.wifi.rssi;
    }
    return moduleStatus.module_status;
}

