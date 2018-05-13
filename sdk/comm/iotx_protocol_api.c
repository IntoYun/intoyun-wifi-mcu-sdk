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
#include "iotx_protocol_api.h"
#include "iotx_datapoint_api.h"
#include "iotx_system_api.h"
#include "iotx_comm_if_api.h"

const static char *TAG = "sdk:protocol";

#define MAX_SIZE        512  //!< max expected messages (used with RX)

//! check for timeout
#define TIMEOUT(t, ms)  ((ms != TIMEOUT_BLOCKING) && ((HAL_UptimeMs() - t) > ms))


pipe_t pipeRx; //定义串口数据接收缓冲区
static recCallback_t recCallbackHandler = NULL;

static bool parserInitDone = false;
static bool cancelAllOperations = false;

static __inline int PipeInc(pipe_t *pipe, int i, int n)//默认n = 1
{
    i += n;
    if (i >= pipe->_s)
        i -= pipe->_s;
    return i;
}

static void PipeInit(pipe_t *pipe, int n, char *b)//默认b = NULL
{
    pipe->_a = b ? NULL : n ? malloc(n) : NULL;
    pipe->_r = 0;
    pipe->_w = 0;
    pipe->_b = b ? b : pipe->_a;
    pipe->_s = n;
}

/** Return the number of free elements in the buffer
  \return the number of free elements
  */
static int PipeFree(pipe_t *pipe)
{
    int s = pipe->_r - pipe->_w;
    if (s <= 0)
        s += pipe->_s;
    return s - 1;
}

static bool PipeWriteable(pipe_t *pipe)
{
    return PipeFree(pipe) > 0;
}

/* Add a single element to the buffer. (blocking)
   \param c the element to add.
   \return c
   */
static char PipePutc(pipe_t *pipe, char c)
{
    int i = pipe->_w;
    int j = i;
    i = PipeInc(pipe, i, 1);
    while (i == pipe->_r) // = !writeable()
        /* nothing / just wait */;
    pipe->_b[j] = c;
    pipe->_w = i;
    return c;
}

/** Get the number of values available in the buffer
  return the number of element available
  */
static int PipeSize(pipe_t *pipe)
{
    int s = pipe->_w - pipe->_r;
    if (s < 0)
        s += pipe->_s;
    return s;
}

/*! get elements from the buffered pipe
  \param p the elements extracted
  \param n the maximum number elements to extract
  \param t set to true if blocking, false otherwise
  \return number elements extracted
  */
static int PipeGet(pipe_t *pipe, char *p, int n, bool t )//默认t= false
{
    int c = n;
    while (c)
    {
        int f;
        for (;;) // wait for data
        {
            /* f = size(); */
            f = PipeSize(pipe);
            if (f)  break;        // free space
            if (!t) return n - c; // no space and not blocking
            /* nothing / just wait */;
        }
        // check available data
        if (c < f) f = c;
        int r = pipe->_r;
        int m = pipe->_s - r;
        // check wrap
        if (f > m) f = m;
        memcpy(p, &pipe->_b[r], f);
        pipe->_r = PipeInc(pipe, r, f);
        c -= f;
        p += f;
    }
    return n - c;
}

/** set the parsing index and return the number of available
  elments starting this position.
  \param ix the index to set.
  \return the number of elements starting at this position
  */
static int PipeSet(pipe_t *pipe, int ix)
{
    int sz = PipeSize(pipe);
    ix = (ix > sz) ? sz : ix;
    pipe->_o = PipeInc(pipe, pipe->_r, ix);
    return sz - ix;
}

/** get the next element from parsing position and increment parsing index
  \return the extracted element.
  */
static char PipeNext(pipe_t *pipe)
{
    int o = pipe->_o;
    char t = pipe->_b[o];
    pipe->_o = PipeInc(pipe, o, 1);
    return t;
}

static int SerialPipePutc(int c)
{
    uint8_t data = c;
    HAL_CommWrite(data);
    return c;
}

static int SerialPipePut(const void* buffer, int length, bool blocking)
{
    int n;
    const char* ptr = (const char*)buffer;

    for(n=0; n<length; n++) {
        SerialPipePutc(ptr[n]);
    }
    return length;
}

static void SerialPipeRxIrqBuf(uint8_t c)
{
    if(PipeWriteable(&pipeRx)) {
        PipePutc(&pipeRx,c);
    }
}

//取消协议解析
static void ProtocolParserCancel(void)
{
    cancelAllOperations = true;
}

//恢复协议解析
static void ProtocolParserResume(void)
{
    cancelAllOperations = false;
}

//通过串口发送数据
static int ProtocolParserTransmit(const void* buf, int len)
{
    return SerialPipePut((const char*)buf, len, true);
}

//发送指令
static int ProtocolParserSend(const char* buf, int len)
{
    return ProtocolParserTransmit(buf, len);
}

static int ProtocolParserSendFormated(const char* format, ...)
{
    if (cancelAllOperations) return 0;

    char buf[MAX_SIZE];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buf,sizeof(buf), format, args);
    va_end(args);

    return ProtocolParserSend(buf, len);
}

//匹配查询响应的数据
static int ProtocolParserMatch(pipe_t *pipe, int len, const char* sta, const char* end)
{
    int o = 0;
    if (sta) {
        while (*sta) {
            if (++o > len) return WAIT;
            char ch = PipeNext(pipe);
            if (*sta++ != ch) return NOT_FOUND;
        }
    }
    if (!end) return o; // no termination
    // at least any char
    if (++o > len) return WAIT;
    PipeNext(pipe);
    // check the end
    int x = 0;
    while (end[x]) {
        if (++o > len) return WAIT;
        char ch = PipeNext(pipe);
        x = (end[x] == ch) ? x + 1 :
            (end[0] == ch) ? 1 :
            0;
    }
    return o;
}


//查询对方主动发过来的数据
static int ProtocolParserFormated(pipe_t *pipe, int len, const char* fmt)
{
    int o = 0;
    int num = 0;
    if (fmt) {
        while (*fmt) {
            if (++o > len) return WAIT;
            char ch = PipeNext(pipe);
            if (*fmt == '%') {
                fmt++;
                if (*fmt == 'd') { // numeric
                    fmt ++;
                    while (ch >= '0' && ch <= '9') {
                        if (++o > len) return WAIT;
                        ch = PipeNext(pipe);
                    }
                } else if (*fmt == 'n') { // data len
                    fmt ++;
                    num = 0;
                    while (ch >= '0' && ch <= '9') {
                        num = num * 10 + (ch - '0');
                        if (++o > len) return WAIT;
                        ch = PipeNext(pipe);
                    }
                } else if (*fmt == 'c') { // char buffer (takes last numeric as length)
                    fmt ++;
                    while (--num) {
                        if (++o > len) return WAIT;
                        ch = PipeNext(pipe);
                    }
                    continue;
                } else if (*fmt == 's') {
                    fmt ++;
                    if (ch != '\"') return NOT_FOUND;

                    do {
                        if (++o > len) return WAIT;
                        ch = PipeNext(pipe);
                    } while (ch != '\"');

                    if (++o > len) return WAIT;
                    ch = PipeNext(pipe);
                }
            }
            if (*fmt++ != ch) return NOT_FOUND;
        }
    }
    return o;
}

static int ProtocolParserGetOnePacket(pipe_t *pipe, char* buf, int len)
{
    int unkn = 0;
    int sz = PipeSize(pipe);
    int fr = PipeFree(pipe);
    if (len > sz)
        len = sz;
    while (len > 0) {
        static struct {
            const char* fmt;              int type;
        } lutF[] = {
            // %d:表示正常数值   %n:表示数据长度   %c:表示数据
            { "+RECDATA,%n:%c",           TYPE_PLUS  }, //平台下发的数据
        };

        static struct {
            const char* sta;       const char* end;    int type;
        } lut[] = {
            { "\r\nOK\r\n",        NULL,               TYPE_OK      },
            { "\r\nERROR\r\n",     NULL,               TYPE_ERROR   },
            { "\r\nFAIL\r\n",      NULL,               TYPE_FAIL    },
            { "+",                 "\r\n",             TYPE_PLUS    },
            { "> ",                 NULL,              TYPE_PROMPT  }, //模组接收数据
            { "\r\nSEND OK\r\n",   NULL,               TYPE_OK      }, //数据发送成功
        };

        for (int i = 0; i < (int)(sizeof(lutF)/sizeof(*lutF)); i ++) {
            PipeSet(pipe,unkn);
            int ln = ProtocolParserFormated(pipe, len, lutF[i].fmt);
            if (ln == WAIT && fr)
                return WAIT;
            if ((ln != NOT_FOUND) && (unkn > 0))
                return TYPE_UNKNOWN | PipeGet(pipe, buf, unkn, false);
            if (ln > 0)
                return lutF[i].type  | PipeGet(pipe, buf, ln, false);
        }

        for (int i = 0; i < (int)(sizeof(lut)/sizeof(*lut)); i ++) {
            PipeSet(pipe,unkn);
            int ln = ProtocolParserMatch(pipe, len, lut[i].sta, lut[i].end);
            if (ln == WAIT && fr)
                return WAIT;
            if ((ln != NOT_FOUND) && (unkn > 0))
                return TYPE_UNKNOWN | PipeGet(pipe, buf, unkn, false);
            if (ln > 0)
                return lut[i].type | PipeGet(pipe, buf, ln, false);
        }
        // UNKNOWN
        unkn ++;
        len--;
    }
    return TYPE_UNKNOWN | PipeGet(pipe, buf, unkn, false); //应该返回TYPE_UNKNOWN 并且应该从缓存里面清掉。否则会一直接受到相同的数据 并且应该从缓存里面清掉。
}

//抓取一个包解析
static int ProtocolParserGetPacket(char* buffer, int length)
{
    return ProtocolParserGetOnePacket(&pipeRx, buffer, length);
}

//等待响应
static int ProtocolParserWaitFinalResp(callbackPtr cb, void* param, uint32_t timeout_ms) //NULL NULL 5000
{
    if (cancelAllOperations) return WAIT;

    char buf[MAX_SIZE] = {0};
    uint32_t start = HAL_UptimeMs();
    do {
        int ret = ProtocolParserGetPacket(buf, sizeof(buf));
        if ((ret != WAIT) && (ret != NOT_FOUND)) {
            int type = TYPE(ret);
            //handle unsolicited commands here
            if (type == TYPE_PLUS) {
                const char* cmd = buf+1;
                MOLMC_LOGV(TAG, "cmd = %s\r\n",cmd);
                int a,b,c,d;
                int event;
                iotx_conn_state_t conn_state = IOTX_CONN_STATE_INITIALIZED;
                wifi_info_t wifi;
                uint16_t platformDataLen;
                uint8_t *platformData;

                //+RECMODE:<event>
                if(sscanf(cmd, "RECMODE:%d\r\n", (int*)&event) == 1) {
                    iotx_set_work_mode((iotx_work_mode_t)event);
                }
                //+RECNET:<event>,[<ssid>,<ip>,<rssi>]
                else if(sscanf(cmd, "RECNET:%d", (int*)&event) == 1) {
                    switch(event) {
                        case 1: //断开路由器
                            conn_state = IOTX_CONN_STATE_NETWORK_DISCONNECTED;
                            break;
                        case 2: //连接路由器
                            conn_state = IOTX_CONN_STATE_NETWORK_CONNECTED;
                            break;
                        case 3: //断开服务器
                            conn_state = IOTX_CONN_STATE_CLOUD_DISCONNECTED;
                            break;
                        case 4: //连接服务器
                            conn_state = IOTX_CONN_STATE_CLOUD_CONNECTED;
                            break;
                        default:
                            break;
                    }
                    if(event > 1) {
                        if(sscanf(cmd,"RECNET:%d,\"%[^\"]\",\""IPSTR"\",%d\r\n", (int*)&event, \
                                wifi.ssid, (int*)&a, (int*)&b, (int*)&c, (int*)&d, (int*)&wifi.rssi) == 7) {
                            wifi.ipAddr = IPADR(a,b,c,d);
                        }
                    }
                    iotx_set_conn_state(conn_state);
                }
                //+RECDATA,<len>:<data>
                else if(sscanf(cmd, "RECDATA,%d", (int*)&platformDataLen) == 1) {
                    platformData = (uint8_t *)strchr(buf, ':');
                    if(recCallbackHandler != NULL) {
                        recCallbackHandler(platformData, platformDataLen);
                    }
                }
            }
            /*******************************************/
            if (cb) {
                int len = LENGTH(ret);
                int ret = cb(type, buf, len, param);
                if (WAIT != ret)
                    return ret;
            }

            if (type == TYPE_OK)
                return RESP_OK;
            if (type == TYPE_FAIL)
                return RESP_FAIL;
            if (type == TYPE_ERROR)
                return RESP_ERROR;
            if (type == TYPE_PROMPT)
                return RESP_PROMPT;
            if (type == TYPE_ABORTED)
                return RESP_ABORTED; // This means the current command was ABORTED, so retry your command if critical.
        }
        // relax a bit
    } while (!TIMEOUT(start, timeout_ms) && !cancelAllOperations);

    return WAIT;
}

//接收串口数据存入缓冲区
void IOT_Protocol_PutPipe(uint8_t c)
{
    SerialPipeRxIrqBuf(c);
}

bool IOT_Protocol_ParserInit(void)
{
    PipeInit(&pipeRx, CONFIG_PIPE_MAX_SIZE, NULL);

    if(!parserInitDone) {
        cancelAllOperations = false;
        bool continue_cancel = false;
        bool retried_after_reset = false;
        int i = 10;
        while (i--) {
            if (cancelAllOperations) {
                continue_cancel = true;
                ProtocolParserResume(); // make sure we can talk to the modem
            }

            ProtocolParserSendFormated("AT\r\n");
            int r = ProtocolParserWaitFinalResp(NULL,NULL,1000);
            if(RESP_OK == r) {
                break;
            } else if (i==0 && !retried_after_reset){
                retried_after_reset = true; // only perform reset & retry sequence once
                i = 10;
            }
        }

        if (i < 0) {
            continue_cancel = true;
            MOLMC_LOGV(TAG, "[ No Reply from Modem ]\r\n");
        }

        if (continue_cancel) {
            ProtocolParserCancel();
            return false; //串口不通 通讯失败
        }

        ProtocolParserSendFormated("ATE0\r\n"); //关闭回显
        MOLMC_LOGV(TAG, "protocol parser init done\r\n");
        parserInitDone = true;
        return true;
    } else {
        return true;
    }
}

//将模组重启
bool IOT_Protocol_Reboot(void)
{
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+RST\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000)) {
            return true;
        }
    }
    return false;
}

//将模组恢复出厂
bool IOT_Protocol_Restore(void)
{
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+RESTORE\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000)) {
            return true;
        }
    }
    return false;
}

static int ProtocolQueryInfoCallback(int type, const char* buf, int len, module_info_t *info)
{
    if (info && (type == TYPE_PLUS)) {
        if (sscanf(buf, "+INFO:\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",%d\r\n", info->module_version,info->module_type,info->device_id,(int*)&info->at_mode) == 4) {
        }
    }
    return WAIT;
}

//获取模块信息
bool IOT_Protocol_QueryInfo(module_info_t *info)
{
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+INFO?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryInfoCallback, info, 5000)) {
            return true;
        }
    }
    return false;
}

//设置设备信息
bool IOT_Protocol_SetDeviceInfo(char *product_id, char *hardware_version, char *software_version)
{
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+DEVICE=\"%s\",\"%s\",\"%s\"\r\n",product_id,hardware_version,software_version);
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL,NULL,5000)) {
            return true;
        }
    }
    return false;
}

static int ProtocolQueryJoinAPCallback(int type, const char* buf, int len, module_status_t *moduleStatus)
{
    if (moduleStatus && (type == TYPE_PLUS)) {
        int a,b,c,d;
        if(sscanf(buf,"+JOINAP:%d",(int*)&moduleStatus->module_status) == 1) {
            if(moduleStatus->module_status != 1) {
                if (sscanf(buf, "+JOINAP:%d,\"%[^\"]\",\""IPSTR"\",%d\r\n", (int*)&moduleStatus->module_status,moduleStatus->wifi.ssid,(int*)&a,(int*)&b,(int*)&c,(int*)&d,(int*)&moduleStatus->wifi.rssi) == 7) {
                    moduleStatus->wifi.ipAddr = IPADR(a,b,c,d);
                }
            }
        }
    }
    return WAIT;
}

bool IOT_Protocol_QueryJoinAP(module_status_t *status)
{
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+JOINAP?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryJoinAPCallback, status, 5000)) {
            return true;
        }
    }
    return false;
}

//设置模组连接的AP
bool IOT_Protocol_JoinAP(char *ssid,char *pwd)
{
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+JOINAP=\"%s\",\"%s\"\r\n",ssid,pwd);
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL,NULL,5000)) {
            return true;
        }
    }
    return false;
}

static int ProtocolQueryModeCallback(int type, const char* buf, int len, int *mode)
{
    if (mode && (type == TYPE_PLUS)) {
        int workMode;
        if (sscanf(buf, "+MODE:%d\r\n", &workMode) == 1) {
            *mode = workMode;
        }
    }
    return WAIT;
}

//查询工作模式
int IOT_Protocol_QueryMode(void)
{
    int mode = 0;
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+MODE?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryModeCallback, &mode,5000)) {
        }
    }
    return mode;
}

int IOT_Protocol_SetMode(uint8_t mode, uint32_t timeout)
{
    int status = -1;
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+MODE=%d,%d\r\n",mode,timeout);
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryModeCallback, &status,5000)) {
        }
    }
    MOLMC_LOGV(TAG, "mode status=%d\r\n",status);
    return status;
}

//设置连网参数
bool IOT_Protocol_SetJoinParams(char *device_id,char *access_token)
{
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+JPINPARAMS=\"%s\",\"%s\"\r\n",device_id,access_token);
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000)) {
            return true;
        }
    }
    return false;
}

static int ProtocolQueryBasicParamsCallback(int type, const char* buf, int len, basic_params_t *basicParams)
{
    if (basicParams && (type == TYPE_PLUS)) {
        if (sscanf(buf, "+BASICPARAMS:%d,\"%[^\"]\",%d,\"%[^\"]\",%d,\"%[^\"]\"\r\n", (int*)&basicParams->zone,basicParams->server_domain,(int*)&basicParams->server_port,basicParams->register_domain,(int*)&basicParams->register_port,basicParams->update_domain) == 6) {
        }
    }
    return WAIT;
}

bool IOT_Protocol_QueryBasicParams(basic_params_t *basicParams)
{
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+BASICPARAMS?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryBasicParamsCallback, basicParams,5000)) {
            return true;
        }
    }
    return false;
}

//设置基本参数
bool IOT_Protocol_SetBasicParams(int zone, char *server_domain,int server_port,char *register_domain,int register_port, char *update_domain)
{
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+BASICPARAMS=%d,\"%s\",%d,\"%s\",%d,\"%s\"\r\n",zone,server_domain,server_port,register_domain,register_port,update_domain);
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000)) {
            return true;
        }
    }
    return false;
}

static int ProtocolQueryNetTimeCallback(int type, const char* buf, int len, network_time_t *netTime)
{
    if (netTime && (type == TYPE_PLUS)) {
        if(sscanf(buf,"+NETTIME:%d",(int*)&netTime->status) == 1) {
            if(netTime->status == 1) {
                if (sscanf(buf, "+NETTIME:%d,\"%[^\"]\",\"%[^\"]\"\r\n", (int*)&netTime->status,netTime->net_time,netTime->timestamp) == 3) {
                }
            }
        }
    }
    return WAIT;
}

//查询网络时间
bool IOT_Protocol_QueryNetTime(network_time_t *netTime)
{
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+NETTIME?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryNetTimeCallback, netTime,5000)) {
            return true;
        }
    }
    return false;
}

//设置连接或者断开服务器
bool IOT_Protocol_Join(uint8_t mode)
{
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+JOIN=%d\r\n",mode);
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000)) {
            return true;
        }
    }
    return false;
}

static int ProtocolQueryStatusCallback(int type, const char *buf, int len, module_status_t *moduleStatus)
{
    if (moduleStatus && (type == TYPE_PLUS)) {
        int a,b,c,d;
        if(sscanf(buf,"+STATUS:%d",(int*)&moduleStatus->module_status) == 1) {
            if(moduleStatus->module_status != 1) {
                if (sscanf(buf, "+STATUS:%d,\"%[^\"]\",\""IPSTR"\",%d\r\n", (int*)&moduleStatus->module_status,moduleStatus->wifi.ssid,(int*)&a,(int*)&b,(int*)&c,(int*)&d,(int*)&moduleStatus->wifi.rssi) == 7) {
                    moduleStatus->wifi.ipAddr = IPADR(a,b,c,d);
                }
            }
        }
    }
    return WAIT;
}

//查询网络状态
bool IOT_Protocol_QueryStatus(module_status_t *status)
{
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+STATUS?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryStatusCallback, status,5000)) {
            return true;
        }
    }
    return false;
}

//发送数据点数据
bool IOT_Protocol_SendData(const uint8_t *buffer, uint16_t length)
{
    if (parserInitDone) {
        ProtocolParserSendFormated("AT+SENDDATA=%d\r\n",length);
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000)) {
            if (RESP_PROMPT == ProtocolParserWaitFinalResp(NULL, NULL,5000)) {
                SerialPipePut(buffer,(int)length,false);
                if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool IOT_Protocol_loop(void)
{
    ProtocolParserWaitFinalResp(NULL, NULL, 0);
    return true;
}

void IOT_Protocol_SetRevCallback(recCallback_t handler)
{
    if(handler != NULL) {
        recCallbackHandler = handler;
    }
}

