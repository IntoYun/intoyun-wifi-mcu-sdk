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

#include "intoyun_protocol.h"
#include "intoyun_datapoint.h"
#include "intoyun_interface.h"
#include "intoyun_config.h"
#include "intoyun_log.h"
#include "hal_interface.h"


#define PROTOCOL_DEBUG

#define MAX_SIZE        512  //!< max expected messages (used with RX)

//! check for timeout
#define TIMEOUT(t, ms)  ((ms != TIMEOUT_BLOCKING) && ((millis() - t) > ms))


pipe_t pipeRx; //定义串口数据接收缓冲区

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

static bool PipeWriteable(pipe_t *pipe)
{
    return PipeFree(pipe) > 0;
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
    HAL_UartWrite(data);
    return c;
}

static int SerialPipePut(const void* buffer, int length, bool blocking)
{
    int n;
    const char* ptr = (const char*)buffer;

    for(n=0; n<length; n++)
    {
        SerialPipePutc(ptr[n]);
    }
    return length;
}

static void SerialPipeRxIrqBuf(uint8_t c)
{
    if(PipeWriteable(&pipeRx))
    {
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
    if (sta)
    {
        while (*sta)
        {
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
    while (end[x])
    {
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
    if (fmt)
    {
        while (*fmt)
        {
            if (++o > len) return WAIT;
            char ch = PipeNext(pipe);
            if (*fmt == '%')
            {
                fmt++;
                if (*fmt == 'd')
                { // numeric
                    fmt ++;
                    while (ch >= '0' && ch <= '9')
                    {
                        if (++o > len) return WAIT;
                        ch = PipeNext(pipe);
                    }
                }
                else if (*fmt == 'n')
                { // data len
                    fmt ++;
                    num = 0;
                    while (ch >= '0' && ch <= '9')
                    {
                        num = num * 10 + (ch - '0');
                        if (++o > len) return WAIT;
                        ch = PipeNext(pipe);
                    }
                }
                else if (*fmt == 'c')
                { // char buffer (takes last numeric as length)
                    fmt ++;
                    while (--num)
                    {
                        if (++o > len) return WAIT;
                        ch = PipeNext(pipe);
                    }
                    continue;
                }
                else if (*fmt == 's')
                {
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
    while (len > 0)
    {
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

        for (int i = 0; i < (int)(sizeof(lutF)/sizeof(*lutF)); i ++)
        {
            PipeSet(pipe,unkn);
            int ln = ProtocolParserFormated(pipe, len, lutF[i].fmt);
            if (ln == WAIT && fr)
                return WAIT;
            if ((ln != NOT_FOUND) && (unkn > 0))
                return TYPE_UNKNOWN | PipeGet(pipe, buf, unkn, false);
            if (ln > 0)
                return lutF[i].type  | PipeGet(pipe, buf, ln, false);
        }

        for (int i = 0; i < (int)(sizeof(lut)/sizeof(*lut)); i ++)
        {
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
    uint32_t start = millis();
    do {
        int ret = ProtocolParserGetPacket(buf, sizeof(buf));
        if ((ret != WAIT) && (ret != NOT_FOUND))
        {
            int type = TYPE(ret);
            //handle unsolicited commands here
            if (type == TYPE_PLUS)
            {
                const char* cmd = buf+1;
                #ifdef PROTOCOL_DEBUG
                log_v("cmd = %s\r\n",cmd);
                #endif

                int a,b,c,d;
                int modeEventParam;
                int networkEventParam;
                int event;
                wifi_info_t wifi;
                uint16_t platformDataLen;
                uint8_t *platformData;

                //+RECMODE:<event>
                if(sscanf(cmd,"RECMODE:%d\r\n",(int*)&event) == 1){
                    switch(event){
                    case 1:
                        modeEventParam = ep_mode_normal;
                        break;
                    case 2:
                        modeEventParam = ep_mode_imlink_config;
                        break;
                    case 3:
                        modeEventParam = ep_mode_ap_config;
                        break;
                    case 4:
                        modeEventParam = ep_mode_binding;
                        break;
                    default:
                        break;
                    }
                    eventHandler(event_mode_changed,modeEventParam,NULL,0);
                }
                //+RECNET:<event>,[<ssid>,<ip>,<rssi>]
                else if(sscanf(cmd,"RECNET:%d",(int*)&event) == 1)
                {
                    switch(event){
                    case 1:
                        networkEventParam = ep_network_status_disconnected;
                        moduleConnectNetwork = false;
                        break;
                    case 2:
                        networkEventParam = ep_network_status_connected;
                        moduleConnectNetwork = true;
                        break;
                    case 3:
                        networkEventParam = ep_cloud_status_disconnected;
                        cloudConnected = false;
                        break;
                    case 4:
                        networkEventParam = ep_cloud_status_connected;
                        cloudConnected = true;
                        break;
                    default:
                        break;
                    }
                    if(networkEventParam > 1){
                        if(sscanf(cmd,"RECNET:%d,\"%[^\"]\",\""IPSTR"\",%d\r\n",(int*)&networkEventParam,wifi.ssid,(int*)&a,(int*)&b,(int*)&c,(int*)&d,(int*)&wifi.rssi) == 7){
                            wifi.ipAddr = IPADR(a,b,c,d);
                        }
                    }
                    eventHandler(event_network_status,networkEventParam,NULL,0);
                }
                //+RECDATA,<len>:<data>
                else if(sscanf(cmd, "RECDATA,%d", (int*)&platformDataLen) == 1)
                {
                    platformData = (uint8_t *)strchr(buf, ':');
                    //原始数据
                    eventHandler(event_cloud_data,ep_cloud_data_raw,platformData,platformDataLen);
                    uint8_t datapointType = ProtocolParserPlatformData(platformData+1, platformDataLen);
                    if(datapointType == CUSTOMER_DEFINE_DATA){
                        eventHandler(event_cloud_data,ep_cloud_data_custom,platformData+1,platformDataLen);
                    }else{
                        eventHandler(event_cloud_data,ep_cloud_data_datapoint,NULL,0);
                    }
                }
            }
            /*******************************************/
            if (cb)
            {
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
    }while (!TIMEOUT(start, timeout_ms) && !cancelAllOperations);

    return WAIT;
}

//接收串口数据存入缓冲区
void ProtocolPutPipe(uint8_t c)
{
    SerialPipeRxIrqBuf(c);
}

bool ProtocolParserInit(void)
{
    PipeInit(&pipeRx,PIPE_MAX_SIZE,NULL);

    if(!parserInitDone){
        cancelAllOperations = false;
        bool continue_cancel = false;
        bool retried_after_reset = false;
        int i = 10;
        while (i--){
            if (cancelAllOperations){
                continue_cancel = true;
                ProtocolParserResume(); // make sure we can talk to the modem
            }

            ProtocolParserSendFormated("AT\r\n");
            int r = ProtocolParserWaitFinalResp(NULL,NULL,1000);
            if(RESP_OK == r){
                break;
            }else if (i==0 && !retried_after_reset){
                retried_after_reset = true; // only perform reset & retry sequence once
                i = 10;
            }
        }

        if (i < 0){
            continue_cancel = true;
            #ifdef PROTOCOL_DEBUG
            log_v("[ No Reply from Modem ]\r\n");
            #endif
        }

        if (continue_cancel){
            ProtocolParserCancel();
            return false; //串口不通 通讯失败
        }

        ProtocolParserSendFormated("ATE0\r\n"); //关闭回显
        #ifdef PROTOCOL_DEBUG
        log_v("protocol parser init done\r\n");
        #endif
        parserInitDone = true;
        return true;
    }else{
        return true;
    }
}

//将模组重启
bool ProtocolExecuteRestart(void)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+RST\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000))
        {
            return true;
        }
    }
    return false;
}

//将模组恢复出厂
bool ProtocolExecuteRestore(void)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+RESTORE\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000))
        {
            return true;
        }
    }
    return false;
}

int ProtocolQueryInfoCallback(int type, const char* buf, int len, device_info_t *info)
{
    if (info && (type == TYPE_PLUS))
    {
        if (sscanf(buf, "+INFO:\"%[^\"]\",\"%[^\"]\",\"%[^\"]\",%d\r\n", info->module_version,info->module_type,info->device_id,(int*)&info->at_mode) == 4)
        {
        }
    }
    return WAIT;
}

//获取模块信息
bool ProtocolQueryInfo(device_info_t *info)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+INFO?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryInfoCallback, info, 5000))
        {
            return true;
        }
    }
    return false;
}

int ProtocolQueryDeviceCallback(int type, const char* buf, int len, device_info_t *info)
{
    if (info && (type == TYPE_PLUS))
    {
        if (sscanf(buf, "+DEVICE:\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"\r\n", info->product_id,info->hardware_version,info->software_version) == 3)
        {
        }
    }
    return WAIT;

}

//获取设备信息
bool ProtocolQueryDevice(device_info_t *info)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+DEVICE?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryDeviceCallback, info, 5000))
        {
            return true;
        }
    }
    return false;
}

//设置设备信息
bool ProtocolSetupDevice(char *product_id, char *hardware_version, char *software_version)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+DEVICE=\"%s\",\"%s\",\"%s\"\r\n",product_id,hardware_version,software_version);
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL,NULL,5000))
        {
            return true;
        }
    }
    return false;
}

int ProtocolQueryJoinAPCallback(int type, const char* buf, int len, module_status_t *moduleStatus)
{
    if (moduleStatus && (type == TYPE_PLUS))
    {
        int a,b,c,d;
        if(sscanf(buf,"+JOINAP:%d",(int*)&moduleStatus->module_status) == 1){
            if(moduleStatus->module_status != 1){
                if (sscanf(buf, "+JOINAP:%d,\"%[^\"]\",\""IPSTR"\",%d\r\n", (int*)&moduleStatus->module_status,moduleStatus->wifi.ssid,(int*)&a,(int*)&b,(int*)&c,(int*)&d,(int*)&moduleStatus->wifi.rssi) == 7){
                    moduleStatus->wifi.ipAddr = IPADR(a,b,c,d);
                }
            }
        }
    }
    return WAIT;
}

bool ProtocolQueryJoinAP(module_status_t *status)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+JOINAP?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryJoinAPCallback, status, 5000))
        {
            return true;
        }
    }
    return false;
}

//设置模组连接的AP
bool ProtocolSetupJoinAP(char *ssid,char *pwd)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+JOINAP=\"%s\",\"%s\"\r\n",ssid,pwd);
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL,NULL,5000))
        {
            return true;
        }
    }
    return false;
}

int ProtocolQueryModeCallback(int type, const char* buf, int len, int *mode)
{
    if (mode && (type == TYPE_PLUS))
    {
        int workMode;
        if (sscanf(buf, "+MODE:%d\r\n", &workMode) == 1)
        {
            *mode = workMode;
        }
    }
    return WAIT;
}

//查询工作模式
int ProtocolQueryMode(void)
{
    int mode = 0;
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+MODE?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryModeCallback, &mode,5000))
        {
        }
    }
    return mode;
}


int ProtocolSetupMode(uint8_t mode, uint32_t timeout)
{
    int status = -1;
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+MODE=%d,%d\r\n",mode,timeout);
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryModeCallback, &status,5000))
        {
        }
    }
    log_v("mode status=%d\r\n",status);
    return status;
}

//设置连网参数
bool ProtocolSetupJoinParams(char *device_id,char *access_token)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+JPINPARAMS=\"%s\",\"%s\"\r\n",device_id,access_token);
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000))
        {
            return true;
        }
    }
    return false;
}

int ProtocolQueryBasicParamsCallback(int type, const char* buf, int len, basic_params_t *basicParams)
{
    if (basicParams && (type == TYPE_PLUS))
    {
        if (sscanf(buf, "+BASICPARAMS:%d,\"%[^\"]\",%d,\"%[^\"]\",%d,\"%[^\"]\"\r\n", (int*)&basicParams->zone,basicParams->server_domain,(int*)&basicParams->server_port,basicParams->register_domain,(int*)&basicParams->register_port,basicParams->update_domain) == 6)
        {
        }
    }
    return WAIT;
}

bool ProtocolQueryBasicParams(basic_params_t *basicParams)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+BASICPARAMS?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryBasicParamsCallback, basicParams,5000))
        {
            return true;
        }
    }
    return false;
}

//设置基本参数
bool ProtocolSetupBasicParams(int zone, char *server_domain,int server_port,char *register_domain,int register_port, char *update_domain)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+BASICPARAMS=%d,\"%s\",%d,\"%s\",%d,\"%s\"\r\n",zone,server_domain,server_port,register_domain,register_port,update_domain);
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000))
        {
            return true;
        }
    }
    return false;
}

int ProtocolQueryNetTimeCallback(int type, const char* buf, int len, network_time_t *netTime)
{
    if (netTime && (type == TYPE_PLUS))
    {
        if(sscanf(buf,"+NETTIME:%d",(int*)&netTime->status) == 1){
            if(netTime->status == 1){
                if (sscanf(buf, "+NETTIME:%d,\"%[^\"]\",\"%[^\"]\"\r\n", (int*)&netTime->status,netTime->net_time,netTime->timestamp) == 3){
                }
            }
        }
    }
    return WAIT;
}

//查询网络时间
bool ProtocolQueryNetTime(network_time_t *netTime)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+NETTIME?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryNetTimeCallback, netTime,5000))
        {
            return true;
        }
    }
    return false;
}

int ProtocolSetupRegisterCallback(int type, const char* buf, int len, int* errorCode)
{
    if (errorCode && (type == TYPE_PLUS))
    {
        int error;
        if (sscanf(buf, "+REGISTER:%d\r\n", &error) == 1)
        {
            *errorCode = error;
        }
    }
    return WAIT;
}

//设置设备注册信息
int ProtocolSetupRegister(char *product_id, char *timestamp, char *signature)
{
    int errorCode = -1;
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+REGISTER=\"%s\",\"%s\",\"%s\"\r\n",product_id,timestamp,signature);
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolSetupRegisterCallback, &errorCode,5000))
        {
        }
    }
    return errorCode;
}

//设置连接或者断开服务器
bool ProtocolSetupJoin(uint8_t mode)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+JOIN=%d\r\n",mode);
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000))
        {
            return true;
        }
    }
    return false;
}

int ProtocolQueryStatusCallback(int type, const char *buf, int len, module_status_t *moduleStatus)
{
    if (moduleStatus && (type == TYPE_PLUS))
    {
        int a,b,c,d;
        if(sscanf(buf,"+STATUS:%d",(int*)&moduleStatus->module_status) == 1){
            if(moduleStatus->module_status != 1){
                if (sscanf(buf, "+STATUS:%d,\"%[^\"]\",\""IPSTR"\",%d\r\n", (int*)&moduleStatus->module_status,moduleStatus->wifi.ssid,(int*)&a,(int*)&b,(int*)&c,(int*)&d,(int*)&moduleStatus->wifi.rssi) == 7){
                    moduleStatus->wifi.ipAddr = IPADR(a,b,c,d);
                }
            }
        }
    }
    return WAIT;
}

//查询网络状态
bool ProtocolQueryStatus(module_status_t *status)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+STATUS?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryStatusCallback, status,5000))
        {
            return true;
        }
    }
    return false;
}

//模组主动下发的数据
void ProtocolModuleActiveSendHandle(void)
{
    ProtocolParserWaitFinalResp(NULL, NULL, 0);
}

//解析平台数据
uint8_t ProtocolParserPlatformData(const uint8_t *buffer, uint16_t len)
{
    uint8_t customData = 0;
    #ifdef CONFIG_INTOYUN_DATAPOINT
    intoyunParseReceiveDatapoints(buffer,len,&customData);
    #endif
    return customData;
}

//发送数据点数据
bool ProtocolSendPlatformData(const uint8_t *buffer, uint16_t length)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+SENDDATA=%d\r\n",length);
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000))
        {
            if (RESP_PROMPT == ProtocolParserWaitFinalResp(NULL, NULL,5000))
            {
                SerialPipePut(buffer,(int)length,false);
                if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000))
                {
                    return true;
                }
            }
        }
    }
    return false;
}
