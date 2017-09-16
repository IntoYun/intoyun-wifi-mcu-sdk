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
//AT指令协议数据处理
#ifndef _INTOYUN_PROCOTOL_H
#define _INTOYUN_PROCOTOL_H

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

/** 事件枚举*/
typedef enum
{
    EVENT_MODE_NORMAL = 0,    //YunSlave处于正常工作模式
    EVENT_MODE_IMLINK_CONFIG, //YunSlave处于IMLINK配置模式
    EVENT_MODE_AP_CONFIG,     //YunSlave处于AP配置模式
    EVENT_MODE_BINDING,       //YunSlave处于绑定模式
    EVENT_MODE_TEST,          //YunSlave处于测试模式

    EVENT_CON_ROUTER,         //YunSlave已连接路由器
    EVENT_DISCON_ROUTER,      //YunSlave已断开连接路由器
    EVENT_CON_SERVER,         //YunSlave已连服务器
    EVENT_DISCON_SERVER,      //YunSlave已断开连服务器

    EVENT_CON_APP,            //YunSlave已连接APP
    EVENT_DISCON_APP,         //YunSlave已断开APP

    EVENT_DATAPOINT,          //YunSlave接收到数据点
    EVENT_CUSTOM_DATA,        //YunSlave接受到透传数据

    EVENT_TYPE_MAX            //枚举成员数量计算 (用户误删)
} event_type_t;

/** WiFi模组配置参数*/
typedef enum
{
    MODE_NORMAL = 0,   //设置YunSlave为正常工作模式
    MODE_IMLINK_CONFIG,//设置YunSlave为ImLink配置模式
    MODE_AP_CONFIG,    //设置YunSlave为AP配置模式
    MODE_BINDING,      //设置YunSlave为绑定模式
    MODE_TEST          //设置YunSlave为产测模式
}mode_type_t;



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

#define PIPE_MAX_SIZE 256

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


typedef struct{
    char module_version[10];//模组版本号
    char module_type[10];//模组类型
    char device_id[32];//设备ID
    uint8_t at_mode;//注册类型
    char product_id[32];//产品ID
    char product_secret[32];//产品秘钥
    char hardware_version[10];//板子硬件版本
    char software_version[10];//板子软件版本
    char activation_code[32];//设备激活码
    char access_token[32];//设备秘钥
}device_info_t;

typedef struct
{
    char ssid[33];
    uint8_t bssid[6];
    uint8_t channel;
    int rssi;        // when scanning
    MDM_IP  ipAddr;  // byte 0 is MSB, byte 3 is LSB
}wifi_info_t;

typedef struct{
    event_type_t network_event;
    wifi_info_t wifi;
}network_t;

typedef int (*callbackPtr)(int type, const char *buf, int len, void *param);

//数据缓冲区
void PipeInit(pipe_t *pipe, int n, char *b);// 默认值b = NULL
void PipeRelease(pipe_t *pipe);
void PipeDump(pipe_t *pipe);
bool PipeWriteable(pipe_t *pipe);
int PipeFree(pipe_t *pipe);
char PipePutc(pipe_t *pipe, char c);
int PipePut(pipe_t *pipe, const char* p, int n, bool t );//默认值t = false
bool PipeReadable(pipe_t *pipe);
int PipeSize(pipe_t *pipe);
char PipeGetc(pipe_t *pipe);
int PipeGet(pipe_t *pipe, char *p, int n, bool t);//默认值t = false
int PipeSet(pipe_t *pipe, int ix);
char PipeNext(pipe_t *pipe);
void PipeDone(pipe_t *pipe);

//串口接收
int SerialPipeWriteable(void);
int SerialPipePutc(int c);
int SerialPipePut(const void* buffer, int length, bool blocking);
int SerialPipeReadable(pipe_t *pipe);
int SerialPipeGetc(pipe_t *pipe);
int SerialPipeGet(pipe_t *pipe, void* buffer, int length, bool blocking);
void SerialPipeRxIrqBuf(uint8_t c);

//AT指令解析
bool ProtocolParserInit(void);
void ProtocolParserCancel(void);
void ProtocolParserResume(void);
int ProtocolParserSend(const char* buf, int len);
int ProtocolParserSendFormated(const char* format, ...);
int ProtocolParserWaitFinalResp(callbackPtr cb, void* param, uint32_t timeout_ms); //NULL NULL 5000
int ProtocolParserMatch(pipe_t *pipe, int len, const char* sta, const char* end);
int ProtocolParserFormated(pipe_t *pipe, int len, const char* fmt);
int ProtocolParserGetOnePacket(pipe_t *pipe, char* buf, int len);
int ProtocolParserTransmit(const void* buf, int len);
int ProtocolParserGetPacket(char* buffer, int length);

bool ProtocolModuleRestart(void);
bool ProtocolModuleRestore(void);
int ProtocolGetModuleInfoCallback(int type, const char* buf, int len, device_info_t *info);
bool ProtocolGetModuleInfo(device_info_t *info);
int ProtocolGetDeviceInfoCallback(int type, const char* buf, int len, device_info_t *info);
bool ProtocolGetDeviceInfo(device_info_t *info);
bool ProtocolSetDeviceInfo(char *product_id, char *hardware_version, char *software_version);
bool ProtocolSetDeviceRegisterInfo(uint8_t at_mode, char *device_id, char *activation_code,char *access_token);
int ProtocolQueryWorkModeCallback(int type, const char* buf, int len, char *mode);
uint8_t ProtocolQueryWorkMode(void);
bool ProtocolSetWorkMode(uint8_t mode, uint32_t timeout);
int ProtocolQueryNetworkStatusCallback(int type, const char* buf, int len, network_t *network);
bool ProtocolQueryNetworkStatus(network_t *network);

void ProtocolModuleActiveSendHandle(void);
uint8_t ProtocolParserPlatformData(const uint8_t *platformData, uint16_t len);
bool ProtocolSendPlatformData(const uint8_t *buffer, uint16_t length);

#endif