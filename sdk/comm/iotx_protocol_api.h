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

//AT指令协议数据处理
#ifndef __IOTX_PROTOCOL_API_H__
#define __IOTX_PROTOCOL_API_H__

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    char module_version[20];//模组版本号
    char module_type[10];//模组类型
    char device_id[32];//设备ID
    uint8_t at_mode;//注册类型
} module_info_t;

/** WiFi 模组状态*/
typedef enum
{
    MODULE_DISCONNECT_ROUTER = 1,  //未连接路由器
    MODULE_CONNECT_ROUTER,         //已连接路由器,未连接服务器
    MODULE_CONNECT_SERVER,         //已连接路由并连接服务器
} module_status_type_t;

typedef enum {
    WIFIMODE_STATION            = 1,
    WIFIMODE_SOFTAP             = 2,
    WIFIMODE_STATION_AND_SOFTAP = 3
} wifi_mode_t;

typedef enum {
    SMARTCONFIGTYPE_ESPTOUCH         = 1,
    SMARTCONFIGTYPE_AIRKISS          = 2,
    SMARTCONFIGTYPE_ESPTOUCH_AIRKISS = 3
} smart_config_t;

typedef enum {
    IPSTATUS_ATERROR        = 1,
    IPSTATUS_GETAP          = 2 ,
    IPSTATUS_CONNECTED      = 3,
    IPSTATUS_DISCONNECTED   = 4,
    IPSTATUS_NOTCONNECTWIFI = 5
} ip_status_t;

typedef enum {
    JOINAP_SUCCESS    = 0,
    JOINAP_TIMEOUT    = 1,
    JOINAP_PSWERROR   = 2,
    JOINAP_NOFOUNDAP  = 3,
    JOINAP_CONNETFAIL = 4,
} wifi_join_ap_t;

typedef enum {
    DEALSTATUS_SUCCESS = 0,
    DEALSTATUS_FAIL = 1,
    DEALSTATUS_DOING = 2,
    DEALSTATUS_IDLE = 3,
} deal_status_t;

//! Type of IP protocol
typedef enum
{
    MDM_IPPROTO_TCP = 0,
    MDM_IPPROTO_UDP = 1
} IpProtocol;

enum {
    // waitFinalResp Responses
    NOT_FOUND     =  0,
    WAIT          = -1, // TIMEOUT
    RESP_OK       = -2,
    RESP_ERROR    = -3,
    RESP_FAIL     = -4,
    RESP_PROMPT   = -5,
    RESP_ABORTED  = -6,

    // getLine Responses
    #define LENGTH(x)  (x & 0x00FFFF) //!< extract/mask the length
    #define TYPE(x)    (x & 0xFF0000) //!< extract/mask the type

    TYPE_UNKNOWN            = 0x000000,
    TYPE_OK                 = 0x110000,
    TYPE_ERROR              = 0x120000,
    TYPE_FAIL               = 0x130000,
    TYPE_CONNECT            = 0x210000,
    TYPE_UNLINK             = 0x220000,
    TYPE_CONNECTCLOSTED     = 0x230000,
    TYPE_DHCP               = 0x240000,
    TYPE_DISCONNECT         = 0x250000,
    TYPE_BUSY               = 0x260000,
    TYPE_SMARTCONFIG        = 0x270000,
    TYPE_PROMPT             = 0x300000,
    TYPE_PLUS               = 0x400000,
    TYPE_TEXT               = 0x500000,
    TYPE_ABORTED            = 0x600000,

    // special timout constant
    TIMEOUT_BLOCKING = 0x7FFFFFFF
};


#define AT_ERROR  -1
#define CUSTOMER_DEFINE_DATA     0x32
#define INTOYUN_DATAPOINT_DATA   0x31

typedef struct {
    char *_b;        //!< buffer
    char *_a;        //!< allocated buffer
    int _s;          //!< size of buffer (s - 1) elements can be stored
    volatile int _w; //!< write index
    volatile int _r; //!< read index
    int _o;          //!< offest index used by parsing functions

}pipe_t;

//! An IP v4 address
typedef uint32_t MDM_IP;
#define NOIP ((MDM_IP)0) //!< No IP address
// IP number formating and conversion
#define IPSTR           "%d.%d.%d.%d"
#define IPNUM(ip)       ((ip)>>24)&0xff, \
                        ((ip)>>16)&0xff, \
                        ((ip)>> 8)&0xff, \
                        ((ip)>> 0)&0xff
#define IPADR(a,b,c,d) ((((uint32_t)(a))<<24) | \
                        (((uint32_t)(b))<<16) | \
                        (((uint32_t)(c))<< 8) | \
                        (((uint32_t)(d))<< 0))

#define MACSTR           "%x:%x:%x:%x:%x:%x"

typedef struct
{
    char ssid[33];
    uint8_t bssid[6];
    uint8_t channel;
    int rssi;        // when scanning
    MDM_IP  ipAddr;  // byte 0 is MSB, byte 3 is LSB
} wifi_info_t;

typedef struct{
    module_status_type_t module_status;
    wifi_info_t wifi;
} module_status_t;

typedef struct{
    uint8_t status;
    char net_time[32];
    char timestamp[16];
} network_time_t;

typedef struct{
    int zone;
    char server_domain[32];
    int server_port;
    char register_domain[32];
    int register_port;
    char update_domain[32];
} basic_params_t;

typedef int (*callbackPtr)(int type, const char *buf, int len, void *param);
typedef void (*recCallback_t)(uint8_t *data, uint32_t len);

//AT指令解析
bool IOT_Protocol_ParserInit(void);
void IOT_Protocol_PutPipe(uint8_t c);
bool IOT_Protocol_Reboot(void);
bool IOT_Protocol_Restore(void);
bool IOT_Protocol_QueryInfo(module_info_t *info);
bool IOT_Protocol_QueryDevice(module_info_t *info);
bool IOT_Protocol_SetDeviceInfo(char *product_id, char *hardware_version, char *software_version);
bool IOT_Protocol_SetDevice(char *product_id, char *hardware_version, char *software_version);
bool IOT_Protocol_QueryJoinAP(module_status_t *status);
bool IOT_Protocol_JoinAP(char *ssid,char *pwd);
int  IOT_Protocol_QueryMode(void);
int  IOT_Protocol_SetMode(uint8_t mode, uint32_t timeout);
bool IOT_Protocol_SetJoinParams(char *device_id,char *access_token);
bool IOT_Protocol_QueryBasicParams(basic_params_t *basicParams);
bool IOT_Protocol_SetBasicParams(int zone, char *server_domain,int server_port,char *register_domain,int register_port, char *update_domain);
bool IOT_Protocol_QueryNetTime(network_time_t *netTime);
bool IOT_Protocol_Join(uint8_t mode);
bool IOT_Protocol_QueryStatus(module_status_t *status);
bool IOT_Protocol_SendData(const uint8_t *buffer, uint16_t length);

bool IOT_Protocol_loop(void);
void IOT_Protocol_SetRevCallback(recCallback_t handler);

#ifdef __cplusplus
}
#endif

#endif
