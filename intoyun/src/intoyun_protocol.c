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
#include "usart.h"
#include "time.h"

/* #define PROTOCOL_DEBUG */

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

void PipeInit(pipe_t *pipe, int n, char *b)//默认b = NULL
{
    pipe->_a = b ? NULL : n ? malloc(n) : NULL;
    pipe->_r = 0;
    pipe->_w = 0;
    pipe->_b = b ? b : pipe->_a;
    pipe->_s = n;
}

void PipeRelease(pipe_t *pipe)
{
    if(pipe->_a)
    {
        free(pipe->_a);
    }
}

void PipeDump(pipe_t *pipe)
{
    int o = pipe->_r;
    int MAX = PipeSize(pipe);
    char temp1[MAX*3];
    char temp2[4];
    sprintf(temp1,"pipe: %d/%d ", PipeSize(pipe), pipe->_s);
    while (o != pipe->_w)
    {
        char t = pipe->_b[o];
        sprintf(temp2, "%0*X", sizeof(char)*2, t);
        strcat(temp1, temp2);
        o = PipeInc(pipe, o, 1);
    }
    strcat(temp1,"\n");
}

bool PipeWriteable(pipe_t *pipe)
{
    return PipeFree(pipe) > 0;
}

/** Return the number of free elements in the buffer
    \return the number of free elements
*/
int PipeFree(pipe_t *pipe)
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
char PipePutc(pipe_t *pipe, char c)
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


/* Add a buffer of elements to the buffer.
   \param p the elements to add
   \param n the number elements to add from p
   \param t set to true if blocking, false otherwise
   \return number elements added
*/
int PipePut(pipe_t *pipe, const char* p, int n, bool t )//默认t= false
{
    int c = n;
    while (c)
    {
        int f;
        for (;;) // wait for space
        {
            f = PipeFree(pipe);
            if (f > 0) break;     // data avail
            if (!t) return n - c; // no more space and not blocking
            /* nothing / just wait */;
        }
        // check free space
        if (c < f) f = c;
        int w = pipe->_w;
        int m = pipe->_s - w;
        // check wrap
        if (f > m) f = m;
        memcpy(&pipe->_b[w], p, f);
        pipe->_w = PipeInc(pipe, w, f);
        c -= f;
        p += f;
    }
    return n - c;
}

/** Check if there are any emelemnt available (readble / not empty)
    \return true if readable/not empty
*/
bool PipeReadable(pipe_t *pipe)
{
    return (pipe->_r != pipe->_w);
}

/** Get the number of values available in the buffer
    return the number of element available
*/
int PipeSize(pipe_t *pipe)
{
    int s = pipe->_w - pipe->_r;
    if (s < 0)
        s += pipe->_s;
    return s;
}

/** get a single value from buffered pipe (this function will block if no values available)
    \return the element extracted
*/
char PipeGetc(pipe_t *pipe)
{
    int r = pipe->_r;
    while (r == pipe->_w) // = !readable()
        /* nothing / just wait */;
    char t = pipe->_b[r];
    pipe->_r = PipeInc(pipe, r ,1);
    return t;
}

/*! get elements from the buffered pipe
    \param p the elements extracted
    \param n the maximum number elements to extract
    \param t set to true if blocking, false otherwise
    \return number elements extracted
*/
int PipeGet(pipe_t *pipe, char *p, int n, bool t )//默认t= false
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
int PipeSet(pipe_t *pipe, int ix)
{
    int sz = PipeSize(pipe);
    ix = (ix > sz) ? sz : ix;
    pipe->_o = PipeInc(pipe, pipe->_r, ix);
    return sz - ix;
}

/** get the next element from parsing position and increment parsing index
    \return the extracted element.
*/
char PipeNext(pipe_t *pipe)
{
    int o = pipe->_o;
    char t = pipe->_b[o];
    pipe->_o = PipeInc(pipe, o, 1);
    return t;
}

/** commit the index, mark the current parsing index as consumed data.
 */
void PipeDone(pipe_t *pipe)
{
    pipe->_r = pipe->_o;
}

// tx channel
int SerialPipeWriteable(void)
{
    return 1;
}

int SerialPipePutc(int c)
{
    uint8_t data = c;
    UART_Transmit(data);
    return c;
}

int SerialPipePut(const void* buffer, int length, bool blocking)
{
    int n;
    const char* ptr = (const char*)buffer;

    for(n=0; n<length; n++)
    {
        SerialPipePutc(ptr[n]);
    }
    return length;
}

// rx channel
int SerialPipeReadable(pipe_t *pipe)
{
    return PipeReadable(pipe);
}

int SerialPipeGetc(pipe_t *pipe)
{
    if(!PipeReadable(pipe))
    {
        return EOF;
    }
    return PipeGetc(pipe);
}

int SerialPipeGet(pipe_t *pipe, void* buffer, int length, bool blocking)
{
    return PipeGet(pipe,(char*)buffer,length,blocking);
}

void SerialPipeRxIrqBuf(uint8_t c)
{
    if(PipeWriteable(&pipeRx))
    {
        PipePutc(&pipeRx,c);
    }
}

bool ProtocolParserInit(void)
{
    PipeInit(&pipeRx,256,NULL);

    if(!parserInitDone)
    {
        cancelAllOperations = false;

        bool continue_cancel = false;
        bool retried_after_reset = false;

        int i = 10;
        while (i--)
        {
            if (cancelAllOperations)
            {
                continue_cancel = true;
                ProtocolParserResume(); // make sure we can talk to the modem
            }

            ProtocolParserSendFormated("AT\r\n");
            int r = ProtocolParserWaitFinalResp(NULL,NULL,1000);
            if(RESP_OK == r)
            {
                break;
            }
            else if (i==0 && !retried_after_reset)
            {
                retried_after_reset = true; // only perform reset & retry sequence once
                i = 10;
            }
        }

        if (i < 0)
        {
            continue_cancel = true;
            #ifdef PROTOCOL_DEBUG
            printf("[ No Reply from Modem ]\r\n");
            #endif
        }

        if (continue_cancel)
        {
            ProtocolParserCancel();
            return false; //串口不通 通讯失败
        }

        ProtocolParserSendFormated("ATE0\r\n"); //关闭回显
        #ifdef PROTOCOL_DEBUG
        printf("protocol parser init done\r\n");
        #endif
        parserInitDone = true;

        return true;
    }
    else
    {
        return true;
    }
}

//取消协议解析
void ProtocolParserCancel(void)
{
    cancelAllOperations = true;
}

//恢复协议解析
void ProtocolParserResume(void)
{
    cancelAllOperations = false;
}

//发送指令
int ProtocolParserSend(const char* buf, int len)
{
    return ProtocolParserTransmit(buf, len);
}

int ProtocolParserSendFormated(const char* format, ...)
{
    if (cancelAllOperations) return 0;

    char buf[MAX_SIZE];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buf,sizeof(buf), format, args);
    va_end(args);

    return ProtocolParserSend(buf, len);
}

//等待响应
int ProtocolParserWaitFinalResp(callbackPtr cb, void* param, uint32_t timeout_ms) //NULL NULL 5000
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
                printf("cmd = %s\r\n",cmd);
                #endif

                int a,b,c,d;
                network_t network;
                event_type_t networkEvent;

                uint16_t platformDataLen;
                uint8_t *platformData;

                //+RECMODE:<event>
                if(sscanf(cmd,"RECMODE:%d\r\n",(int*)&networkEvent) == 1)
                {
                    #ifdef PROTOCOL_DEBUG
                    printf("recmode = %d\r\n",networkEvent);
                    #endif
                    eventHandler(networkEvent,NULL,0);
                }
                //+RECNET:<event>,[<ssid>,<ip>,<rssi>]
                else if(sscanf(cmd,"RECNET:%d",(int*)&network.network_event) == 1)
                {
                    if(network.network_event > 1)
                    {
                        if(sscanf(cmd,"RECNET:%d,\"%[^\"]\",\""IPSTR"\",%d\r\n",(int*)&network.network_event,network.wifi.ssid,(int*)&a,(int*)&b,(int*)&c,(int*)&d,(int*)&network.wifi.rssi) == 7)
                        {
                            network.wifi.ipAddr = IPADR(a,b,c,d);
                            #ifdef PROTOCOL_DEBUG
                            printf("network event = %d\r\n",network.network_event);
                            printf("network wifi ssid = %s\r\n",network.wifi.ssid);
                            printf("network wifi ip = 0x%x\r\n",network.wifi.ipAddr);
                            printf("network widi rssi = %d\r\n",network.wifi.rssi);
                            #endif
                        }
                    }
                    eventHandler(network.network_event,NULL,0);
                }
                //+RECDATA,<len>:<data>
                else if(sscanf(cmd, "RECDATA,%d", (int*)&platformDataLen) == 1)
                {
                    platformData = (uint8_t *)strchr(buf, ':');

                    uint8_t datapointType = ProtocolParserPlatformData(platformData+1, platformDataLen);
                    if(datapointType == CUSTOMER_DEFINE_DATA)
                    {
                        eventHandler(EVENT_CUSTOM_DATA,platformData+1,platformDataLen); //数据的第一个字节为0x32　用户自定义数据
                    }
                    else
                    {
                        eventHandler(EVENT_DATAPOINT,NULL,0);
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

//匹配查询响应的数据
int ProtocolParserMatch(pipe_t *pipe, int len, const char* sta, const char* end)
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
int ProtocolParserFormated(pipe_t *pipe, int len, const char* fmt)
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


int ProtocolParserGetOnePacket(pipe_t *pipe, char* buf, int len)
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
            { "> ",                 NULL,               TYPE_PROMPT  }, //模组接收数据
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

//通过串口发送数据
int ProtocolParserTransmit(const void* buf, int len)
{
    return SerialPipePut((const char*)buf, len, true);
}

//抓取一个包解析
int ProtocolParserGetPacket(char* buffer, int length)
{
    return ProtocolParserGetOnePacket(&pipeRx, buffer, length);
}

//将模组重启
bool ProtocolModuleRestart(void)
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
bool ProtocolModuleRestore(void)
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

//获取模块信息回调
int ProtocolGetModuleInfoCallback(int type, const char* buf, int len, device_info_t *info)
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
bool ProtocolGetModuleInfo(device_info_t *info)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+INFO?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolGetModuleInfoCallback, info, 5000))
        {
            return true;
        }
    }
    return false;
}

int ProtocolGetDeviceInfoCallback(int type, const char* buf, int len, device_info_t *info)
{
    if (info && (type == TYPE_PLUS))
    {
        if (sscanf(buf, "+INFO:\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"\r\n", info->product_id,info->hardware_version,info->software_version) == 3)
        {
        }
    }
    return WAIT;

}

//获取设备信息
bool ProtocolGetDeviceInfo(device_info_t *info)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+DEVICE?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolGetDeviceInfoCallback, info, 5000))
        {
            return true;
        }
    }
    return false;
}


//设置设备信息
bool ProtocolSetDeviceInfo(char *product_id, char *hardware_version, char *software_version)
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

//设置设备注册信息
bool ProtocolSetDeviceRegisterInfo(uint8_t at_mode, char *device_id, char *activation_code,char *access_token)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+SECURITY=%d,\"%s\",\"%s\",\"%s\"\r\n",at_mode,device_id,activation_code,access_token);
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000))
        {
            return true;
        }
    }
    return false;
}


int ProtocolQueryWorkModeCallback(int type, const char* buf, int len, char *mode)
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
uint8_t ProtocolQueryWorkMode(void)
{
    char mode;
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+MODE?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryWorkModeCallback, &mode,5000))
        {
        }
    }
    return (uint8_t)mode;
}


bool ProtocolSetWorkMode(uint8_t mode, uint32_t timeout)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+MODE=%d,%d\r\n",mode,timeout);
        if (RESP_OK == ProtocolParserWaitFinalResp(NULL, NULL,5000))
        {
            return true;
        }
    }
    return false;
}

int ProtocolQueryNetworkStatusCallback(int type, const char *buf, int len, network_t *network)
{
    if (network && (type == TYPE_PLUS))
    {
        int a,b,c,d;
        if (sscanf(buf, "+NETSTATUS:%d,\"%[^\"]\",\""IPSTR"\",%d\r\n", (int*)&network->network_event,network->wifi.ssid,(int*)&a,(int*)&b,(int*)&c,(int*)&d,(int*)&network->wifi.rssi) == 7)
            network->wifi.ipAddr = IPADR(a,b,c,d);
    }
    return WAIT;
}

//查询网络状态
bool ProtocolQueryNetworkStatus(network_t *network)
{
    if (parserInitDone)
    {
        ProtocolParserSendFormated("AT+NETSTATUS?\r\n");
        if (RESP_OK == ProtocolParserWaitFinalResp((callbackPtr)ProtocolQueryNetworkStatusCallback, network,5000))
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
    intoyunParseReceiveDatapoints(buffer,len,&customData);
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
