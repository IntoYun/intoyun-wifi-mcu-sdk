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

const static char *TAG = "sdk:system";

static event_handler_t eventHandler = NULL;
static iotx_work_mode_t iotx_work_mode = IOTX_WORK_MODE_NORMAL;

/* get type of mode */
iotx_work_mode_t iotx_get_work_mode(void)
{
    return iotx_work_mode;
}

void iotx_set_work_mode(iotx_work_mode_t newMode)
{
    if(iotx_get_work_mode() != newMode) {
        switch(newMode) {
            case IOTX_WORK_MODE_NORMAL:
                IOT_SYSTEM_NotifyEvent(event_mode_changed, ep_mode_normal, NULL, 0);
                break;
            case IOTX_WORK_MODE_IMLINK_CONFIG:
                IOT_SYSTEM_NotifyEvent(event_mode_changed, ep_mode_imlink_config, NULL, 0);
                break;
            case IOTX_WORK_MODE_AP_CONFIG:
                IOT_SYSTEM_NotifyEvent(event_mode_changed, ep_mode_ap_config, NULL, 0);
                break;
            case IOTX_WORK_MODE_BINDING:
                IOT_SYSTEM_NotifyEvent(event_mode_changed, ep_mode_binding, NULL, 0);
                break;
            default:
                break;
        }
    }
    iotx_work_mode = newMode;
}

void IOT_SYSTEM_SetDeviceInfo(char *productID, char *productSecret, char *hardwareVersion, char *softwareVersion)
{
    IOT_Protocol_SetDeviceInfo(productID, hardwareVersion, softwareVersion);
}

void IOT_SYSTEM_GetModuleInfo(char *moduleVersion, char *moduleType, char *deviceId, uint8_t *at_mode)
{
    module_info_t info;
    if(!IOT_Protocol_QueryInfo(&info)) {
        return;
    }

    MOLMC_LOGV(TAG, "moduleVer = %s\r\n",info.module_version);
    MOLMC_LOGV(TAG, "moduleType = %s\r\n",info.module_type);
    MOLMC_LOGV(TAG, "deviceId = %s\r\n",info.device_id);
    MOLMC_LOGV(TAG, "atmode = %d\r\n",info.at_mode);

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

bool IOT_SYSTEM_SetMode(iotx_work_mode_t mode, uint32_t timeout)
{
    return IOT_Protocol_SetMode((uint8_t)mode,timeout);
}

iotx_work_mode_t IOT_SYSTEM_GetMode(void)
{
    return (iotx_work_mode_t)IOT_Protocol_QueryMode();
}

bool IOT_SYSTEM_Restart(void)
{
    return IOT_Protocol_Reboot();
}

bool IOT_SYSTEM_Restore(void)
{
    return IOT_Protocol_Restore();
}

void IOT_SYSTEM_PutPipe(uint8_t value)
{
    IOT_Protocol_PutPipe(value);//将接收到的数据放入缓冲区
}

bool IOT_SYSTEM_GetNetTime(char *net_time, char *timestamp)
{
    network_time_t netTime;

    if(!IOT_Protocol_QueryNetTime(&netTime)) {
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

    if(!IOT_Protocol_QueryStatus(&moduleStatus)) {
        return 0;
    }

    if(moduleStatus.module_status != 1) {
        MOLMC_LOGV(TAG, "ssid=%s\r\n",moduleStatus.wifi.ssid);
        MOLMC_LOGV(TAG, "ipAddr=%d\r\n",moduleStatus.wifi.ipAddr);
        MOLMC_LOGV(TAG, "rssi=%d\r\n",moduleStatus.wifi.rssi);
        strncpy(ssid,moduleStatus.wifi.ssid,sizeof(moduleStatus.wifi.ssid));
        *ipAddr = moduleStatus.wifi.ipAddr;
        *rssi = moduleStatus.wifi.rssi;
    }
    return moduleStatus.module_status;
}

