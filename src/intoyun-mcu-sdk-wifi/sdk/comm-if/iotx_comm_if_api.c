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

static int iotx_conninfo_inited = 0;
static iotx_conn_info_t iotx_conn_info;

iotx_conn_info_pt iotx_conn_info_get(void)
{
    IOT_Comm_Init();
    return &iotx_conn_info;
}

/* get state of conn */
static iotx_conn_state_t iotx_get_conn_state(void)
{
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();
    iotx_conn_state_t state;

    state = pconn_info->conn_state;
    return state;
}

/* set state of conn */
static void iotx_set_conn_state(iotx_conn_state_t newState)
{
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();

    if(pconn_info->conn_state != newState) {
        switch(newState) {
            case IOTX_CONN_STATE_INITIALIZED:    /* initializing state */
            case IOTX_CONN_STATE_DISCONNECTED:   /* disconnected state */
                if(pconn_info->conn_state == IOTX_CONN_STATE_CONNECTED) {
                    IOT_SYSTEM_NotifyEvent(event_network_status, ep_cloud_status_disconnected, NULL, 0);
                }
                break;
            case IOTX_CONN_STATE_CONNECTED:      /* connected state */
                IOT_SYSTEM_NotifyEvent(event_network_status, ep_cloud_status_connected, NULL, 0);
                break;
            default:
                break;
        }
    }

    pconn_info->conn_state = newState;
}

static void cloud_data_receive_callback(uint8_t *data, uint32_t len)
{
    log_d("cloud_data_receive_callback");
#if CONFIG_CLOUD_DATAPOINT_ENABLED == 1
    intoyunParseReceiveDatapoints(data, len);
#endif
    IOT_SYSTEM_NotifyEvent(event_cloud_comm, ep_cloud_comm_data, (uint8_t *)data, len);
}

int IOT_Comm_Init(void)
{
    if (iotx_conninfo_inited) {
        //log_debug("conninfo already created, return!");
        return 0;
    }
    memset(&iotx_conn_info, 0x0, sizeof(iotx_conn_info_t));

    IOT_Protocol_SetRevCallback(cloud_data_receive_callback);

#if CONFIG_CLOUD_DATAPOINT_ENABLED == 1
    // 添加默认数据点
    IOT_DataPoint_DefineBool(DPID_DEFAULT_BOOL_RESET, DP_PERMISSION_UP_DOWN, false);               //reboot
    IOT_DataPoint_DefineBool(DPID_DEFAULT_BOOL_GETALLDATAPOINT, DP_PERMISSION_UP_DOWN, false);     //get all datapoint
#endif

    iotx_conninfo_inited = 1;
    log_i("conn_info created successfully!");
    return 0;
}

int IOT_Comm_Connect(void)
{
    IOT_Protocol_Join(2);
    return 0;
}

bool IOT_Comm_IsConnected(void)
{
#if 0
    if(!IOT_Network_IsConnected()) {
        return false;
    }

    if(IOTX_CONN_STATE_CONNECTED != iotx_get_conn_state()) {
        return false;
    }

    bool rst = iotx_comm_isconnected();
    if(rst) {
        iotx_set_conn_state(IOTX_CONN_STATE_CONNECTED);
    } else {
        iotx_set_conn_state(IOTX_CONN_STATE_DISCONNECTED);
    }
    return rst;
#endif
    return true;
}

int IOT_Comm_Disconnect(void)
{
    IOT_Protocol_Join(1);
    return 0;
}

int IOT_Comm_SendData(const uint8_t *data, uint16_t datalen)
{
    log_d("IOT_Comm_SendData");

    if(!IOT_Comm_IsConnected()) {
        return -1;
    }

    return IOT_Protocol_SendData(data, datalen) ? 0 : -1;
}

int IOT_Comm_Yield(void)
{
    if(IOT_Comm_IsConnected()) {
        intoyunSendDatapointAutomatic();
    }

    IOT_Protocol_loop();
    return 0;
}

