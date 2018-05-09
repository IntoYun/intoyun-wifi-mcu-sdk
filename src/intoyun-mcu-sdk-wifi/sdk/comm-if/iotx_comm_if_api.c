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

#include "iot_import.h"
#include "iotx_comm_if_api.h"
#include "iotx_system_api.h"
#include "iotx_datapoint_api.h"

const static char *TAG = "sdk:comm-if";

static int iotx_conninfo_inited = 0;
static iotx_conn_state_t iotx_conn_state = IOTX_CONN_STATE_INITIALIZED;

/* get state of conn */
iotx_conn_state_t iotx_get_conn_state(void)
{
    return iotx_conn_state;
}

/* set state of conn */
void iotx_set_conn_state(iotx_conn_state_t newState)
{
    iotx_conn_state_t oldState = iotx_get_conn_state();

    if(oldState != newState) {
        switch(newState) {
            case IOTX_CONN_STATE_INITIALIZED:    /* initializing state */
            case IOTX_CONN_STATE_NETWORK_DISCONNECTED:   /* disconnected state */
                if(oldState > IOTX_CONN_STATE_NETWORK_DISCONNECTED) {
                    IOT_SYSTEM_NotifyEvent(event_network_status, ep_network_status_disconnected, NULL, 0);
                }
                break;
            case IOTX_CONN_STATE_NETWORK_CONNECTED:      /* connected state */
                IOT_SYSTEM_NotifyEvent(event_network_status, ep_network_status_connected, NULL, 0);
                break;
            case IOTX_CONN_STATE_CLOUD_DISCONNECTED:      /* connected state */
                IOT_SYSTEM_NotifyEvent(event_cloud_status, ep_cloud_status_connected, NULL, 0);
                break;
            case IOTX_CONN_STATE_CLOUD_CONNECTED:      /* connected state */
                IOT_SYSTEM_NotifyEvent(event_cloud_status, ep_cloud_status_connected, NULL, 0);
                break;
            default:
                break;
        }
    }
    iotx_conn_state = newState;
}

static void cloud_data_receive_callback(uint8_t *data, uint32_t len)
{
    MOLMC_LOGD(TAG, "cloud_data_receive_callback");
#if CONFIG_CLOUD_DATAPOINT_ENABLED == 1
    IOT_DataPoint_ParseReceiveDatapoints(data, len);
#endif
    IOT_SYSTEM_NotifyEvent(event_cloud_comm, ep_cloud_comm_data, (uint8_t *)data, len);
}

int IOT_Comm_Init(void)
{
    if (iotx_conninfo_inited) {
        return 0;
    }

    IOT_Protocol_SetRevCallback(cloud_data_receive_callback);
#if CONFIG_CLOUD_DATAPOINT_ENABLED == 1
    // 添加默认数据点
    IOT_DataPoint_DefineBool(DPID_DEFAULT_BOOL_RESET, DP_PERMISSION_UP_DOWN, false);               //reboot
    IOT_DataPoint_DefineBool(DPID_DEFAULT_BOOL_GETALLDATAPOINT, DP_PERMISSION_UP_DOWN, false);     //get all datapoint
#endif

    iotx_conninfo_inited = 1;
    return 0;
}

void IOT_Comm_Connect(void)
{
    IOT_Protocol_Join(2);
}

bool IOT_Comm_IsConnected(void)
{
    if(IOTX_CONN_STATE_CLOUD_CONNECTED == iotx_get_conn_state()) {
        return true;
    }
    return false;
}

void IOT_Comm_Disconnect(void)
{
    IOT_Protocol_Join(1);
}

int IOT_Comm_SendData(const uint8_t *data, uint16_t datalen)
{
    MOLMC_LOGD(TAG, "IOT_Comm_SendData");

    if(!IOT_Comm_IsConnected()) {
        return -1;
    }

    return IOT_Protocol_SendData(data, datalen) ? 0 : -1;
}

int IOT_Comm_Yield(void)
{
#if CONFIG_CLOUD_DATAPOINT_ENABLED == 1
    if(IOT_Comm_IsConnected()) {
        IOT_DataPoint_SendDatapointAutomatic();
    }
#endif

    IOT_Protocol_loop();
    return 0;
}

