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

#ifndef __IOTX_COMM_IF_API_H__
#define __IOTX_COMM_IF_API_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* State of Connect */
typedef enum {
    IOTX_CONN_STATE_INVALID = 0,                    /* in invalid state */
    IOTX_CONN_STATE_INITIALIZED = 1,                /* in initializing state */
    IOTX_CONN_STATE_CONNECTED = 2,                  /* in connected state */
    IOTX_CONN_STATE_DISCONNECTED = 3,               /* in disconnected state */
    IOTX_CONN_STATE_DISCONNECTED_RECONNECTING = 4,  /* in reconnecting state */
} iotx_conn_state_t;

typedef struct {
    iotx_conn_state_t conn_state;
} iotx_conn_info_t, *iotx_conn_info_pt;

iotx_conn_info_pt iotx_conn_info_get(void);

int IOT_Comm_Init(void);
int IOT_Comm_Connect(void);
bool IOT_Comm_IsConnected(void);
int IOT_Comm_Disconnect(void);
int IOT_Comm_SendData(const uint8_t *data, uint16_t dataLen);
int IOT_Comm_Yield(void);

#ifdef __cplusplus
}
#endif

#endif

