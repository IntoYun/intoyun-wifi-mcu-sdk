/**
******************************************************************************
Copyright (c) 2013-2014 Intoyun Team.  All right reserved.

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

#include "intoyun_datapoint.h"
#include "time.h"

/* #define INTOYUN_DEBUG */


property_conf *properties[PROPERTIES_MAX];

int properties_count = 0;
event_handler_t eventHandler = NULL;

//初始化设置参数 自动发送 发送间隔时间  发送运行时间
volatile datapoint_control_t g_datapoint_control = {DP_TRANSMIT_MODE_AUTOMATIC, DATAPOINT_TRANSMIT_AUTOMATIC_INTERVAL, 0};


void intoyunSendProductInfo(char *productId, char *hardVer, char *softVer)
{
    ProtocolSetDeviceInfo(productId,hardVer,softVer);
}

void intoyunInit(void)
{
    ProtocolParserInit();
}

void intoyunSetEventCallback(event_handler_t handler)
{
    if(handler != NULL)
    {
        eventHandler = handler;
    }
}

void intoyunSetMode(mode_type_t mode, uint32_t timeout)
{
    ProtocolSetWorkMode(mode,timeout);
}

void intoyunLoop(void)
{
    ProtocolModuleActiveSendHandle();
    intoyunSendDatapointAutomatic();
}

void intoyunSetDataAutoSend(uint32_t time)
{
    g_datapoint_control.datapoint_transmit_mode = DP_TRANSMIT_MODE_AUTOMATIC;
    if(time < DATAPOINT_TRANSMIT_AUTOMATIC_INTERVAL)
    {
        g_datapoint_control.datapoint_transmit_lapse = DATAPOINT_TRANSMIT_AUTOMATIC_INTERVAL;
        return;
    }
    g_datapoint_control.datapoint_transmit_lapse = time;
}

static int intoyunDiscoverProperty(const uint16_t dpID)
{
    for (int index = 0; index < properties_count; index++)
    {
        if (properties[index]->dpID == dpID)
        {
            return index;
        }
    }
    return -1;
}

static bool intoyunPropertyChanged(void)
{
    for (int i = 0; i < properties_count; i++)
    {
        if (properties[i]->change)
        {
            return true;
        }
    }
    return false;
}

static uint8_t intoyunGetPropertyCount(void)
{
    return properties_count;
}

static void intoyunPropertyChangeClear(void)
{
    for (int i = 0; i < properties_count; i++)
    {
        if (properties[i]->change)
        {
            properties[i]->change = false;
        }
    }
}


dp_transmit_mode_t intoyunGetDatapointTransmitMode(void)
{
    return g_datapoint_control.datapoint_transmit_mode;
}

void intoyunDatapointControl(dp_transmit_mode_t mode, uint32_t lapse)
{
    g_datapoint_control.datapoint_transmit_mode = mode;
    /* if(DP_TRANSMIT_MODE_AUTOMATIC == g_datapoint_control.datapoint_transmit_mode) */
    {
        if(lapse < DATAPOINT_TRANSMIT_AUTOMATIC_INTERVAL)
        {
            g_datapoint_control.datapoint_transmit_lapse = DATAPOINT_TRANSMIT_AUTOMATIC_INTERVAL;
        }
        else
        {
            g_datapoint_control.datapoint_transmit_lapse = lapse;
        }
    }
}

static void intoyunDatapointValueInit(uint16_t count,uint16_t dpID,data_type_t dataType,dp_permission_t permission, dp_policy_t policy,int lapse)
{
    properties[count]=(property_conf*)malloc(sizeof(property_conf));

    properties[count]->dpID       = dpID;
    properties[count]->dataType   = dataType;
    properties[count]->permission = permission;
    properties[count]->policy     = policy;
    properties[count]->lapse      = lapse;
    properties[count]->runtime    = 0;
    properties[count]->readFlag   = RESULT_DATAPOINT_OLD;
}

void intoyunDefineDatapointBool(const uint16_t dpID, dp_permission_t permission, const bool value, dp_policy_t policy, const int lapse)
{
    int lapseTemp = lapse;

    if (-1 == intoyunDiscoverProperty(dpID))
    {
        if(DP_POLICY_NONE == policy)
        {
            lapseTemp = 0;
        }

        intoyunDatapointValueInit(properties_count,dpID,DATA_TYPE_BOOL,permission,policy,lapseTemp*1000);
        properties[properties_count]->boolValue=value;

        #ifdef INTOYUN_DEBUG
        printf("bool type datapoint\r\n");
        printf("%d\r\n",properties_count);
        printf("%d\r\n",properties[properties_count]->dpID);
        printf("%d\r\n",properties[properties_count]->dataType);
        printf("%d\r\n",properties[properties_count]->permission);
        printf("%d\r\n",properties[properties_count]->policy);
        #endif
        properties_count++; // count the number of properties
    }
}

void intoyunDefineDatapointNumber(const uint16_t dpID, dp_permission_t permission, const double minValue, const double maxValue, const int resolution, const double value, dp_policy_t policy, const int lapse)
{
    int lapseTemp = lapse;

    if (-1 == intoyunDiscoverProperty(dpID))
    {
        if(DP_POLICY_NONE == policy)
        {
            lapseTemp = 0;
        }

        intoyunDatapointValueInit(properties_count,dpID,DATA_TYPE_NUM,permission,policy,lapseTemp*1000);

        double defaultValue = value;
        if(defaultValue < minValue)
        {
            defaultValue = minValue;
        }
        else if(defaultValue > maxValue)
        {
            defaultValue = maxValue;
        }

        if(resolution == 0)
        {
            properties[properties_count]->numberIntValue=value;
        }
        else
        {
            properties[properties_count]->numberDoubleValue=value;
        }
        properties[properties_count]->numberProperty.minValue = minValue;
        properties[properties_count]->numberProperty.maxValue = maxValue;
        properties[properties_count]->numberProperty.resolution = resolution;

        #ifdef INTOYUN_DEBUG
        printf("number type datapoint\r\n");
        printf("%d\r\n",properties_count);
        printf("%d\r\n",properties[properties_count]->dpID);
        printf("%d\r\n",properties[properties_count]->dataType);
        printf("%d\r\n",properties[properties_count]->permission);
        printf("%d\r\n",properties[properties_count]->policy);
        printf("%f\r\n",properties[properties_count]->numberProperty.minValue);
        printf("%f\r\n",properties[properties_count]->numberProperty.maxValue);
        printf("%d\r\n",properties[properties_count]->numberProperty.resolution);

        if(resolution == 0)
            printf("%d\r\n",properties[properties_count]->numberIntValue);
        else
            printf("%f\r\n",properties[properties_count]->numberDoubleValue);

        #endif
        properties_count++; // count the number of properties
    }
}

void intoyunDefineDatapointEnum(const uint16_t dpID, dp_permission_t permission, const int value, dp_policy_t policy, const int lapse)
{
    int lapseTemp;

    if (-1 == intoyunDiscoverProperty(dpID))
    {
        if(DP_POLICY_NONE == policy)
        {
            lapseTemp = 0;
        }

        int defaultValue = value;
        if(defaultValue < 0)
        {
            defaultValue = 0;
        }

        intoyunDatapointValueInit(properties_count,dpID,DATA_TYPE_ENUM,permission,policy,lapseTemp*1000);
        properties[properties_count]->enumValue = defaultValue;
        #ifdef INTOYUN_DEBUG
        printf("enum type datapoint\r\n");
        printf("%d\r\n",properties_count);
        printf("%d\r\n",properties[properties_count]->dpID);
        printf("%d\r\n",properties[properties_count]->dataType);
        printf("%d\r\n",properties[properties_count]->permission);
        printf("%d\r\n",properties[properties_count]->policy);
        printf("%d\r\n",properties[properties_count]->enumValue);
        #endif
        properties_count++; // count the number of properties
    }
}

void intoyunDefineDatapointString(const uint16_t dpID, dp_permission_t permission, const char *value, dp_policy_t policy, const int lapse)
{
    int lapseTemp;

    if (-1 == intoyunDiscoverProperty(dpID))
    {
        if(DP_POLICY_NONE == policy)
        {
            lapseTemp = 0;
        }

        intoyunDatapointValueInit(properties_count,dpID,DATA_TYPE_STRING,permission,policy,lapseTemp*1000);
        properties[properties_count]->stringValue = (char *)malloc(strlen(value)+1);
        strncpy(properties[properties_count]->stringValue,value,strlen(value)+1);

        #ifdef INTOYUN_DEBUG
        printf("string type datapoint\r\n");
        printf("%d\r\n",properties_count);
        printf("%d\r\n",properties[properties_count]->dpID);
        printf("%d\r\n",properties[properties_count]->dataType);
        printf("%d\r\n",properties[properties_count]->permission);
        printf("%d\r\n",properties[properties_count]->policy);
        printf("%s\r\n",properties[properties_count]->stringValue);
        #endif
        properties_count++; // count the number of properties
    }
}

void intoyunDefineDatapointBinary(const uint16_t dpID, dp_permission_t permission, const uint8_t *value, const uint16_t len, dp_policy_t policy, const int lapse)
{
    int lapseTemp;

    if (-1 == intoyunDiscoverProperty(dpID))
    {
        if(DP_POLICY_NONE == policy)
        {
            lapseTemp = 0;
        }

        intoyunDatapointValueInit(properties_count,dpID,DATA_TYPE_BINARY,permission,policy,lapseTemp*1000);
        properties[properties_count]->binaryValue.value = (uint8_t *)malloc(len);
        for(uint8_t i=0;i<len;i++)
        {
            properties[properties_count]->binaryValue.value[i] = value[i];
        }
        properties[properties_count]->binaryValue.len = (uint16_t)len;

        #ifdef INTOYUN_DEBUG
        printf("binary type datapoint\r\n");
        printf("%d\r\n",properties_count);
        printf("%d\r\n",properties[properties_count]->dpID);
        printf("%d\r\n",properties[properties_count]->dataType);
        printf("%d\r\n",properties[properties_count]->permission);
        printf("%d\r\n",properties[properties_count]->policy);
        printf("%d\r\n",properties[properties_count]->binaryValue.len);

        for(uint8_t i=0;i<len;i++)
        {
            printf("data=0x%x\r\n",properties[properties_count]->binaryValue.value[i]);
        }
        #endif
        properties_count++; // count the number of properties
    }
}

read_datapoint_result_t intoyunReadDatapointBool(const uint16_t dpID, bool *value)
{
    int index = intoyunDiscoverProperty(dpID);
    if (index == -1)
    {
        return RESULT_DATAPOINT_NONE;
    }

    (*value) = properties[index]->boolValue;
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t intoyunReadDatapointNumberInt32(const uint16_t dpID, int32_t *value)
{
    int index = intoyunDiscoverProperty(dpID);
    if (index == -1)
    {
        return RESULT_DATAPOINT_NONE;
    }

    (*value) = properties[index]->numberIntValue;
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t intoyunReadDatapointNumberDouble(const uint16_t dpID, double *value)
{
    int index = intoyunDiscoverProperty(dpID);
    if (index == -1)
    {
        return RESULT_DATAPOINT_NONE;
    }

    (*value) = properties[index]->numberDoubleValue;
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t intoyunReadDatapointEnum(const uint16_t dpID, int *value)
{
    int index = intoyunDiscoverProperty(dpID);
    if (index == -1)
    {
        return RESULT_DATAPOINT_NONE;
    }

    (*value) = properties[index]->enumValue;
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}


read_datapoint_result_t intoyunReadDatapointString(const uint16_t dpID, char *value)
{
    int index = intoyunDiscoverProperty(dpID);
    if (index == -1)
    {
        return RESULT_DATAPOINT_NONE;
    }

    for(uint16_t i=0;i<strlen(properties[index]->stringValue);i++)
    {
        value[i] = properties[index]->stringValue[i];
    }
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t intoyunReadDatapointBinary(const uint16_t dpID, uint8_t *value, uint16_t len)
{
    int index = intoyunDiscoverProperty(dpID);
    if (index == -1)
    {
        return RESULT_DATAPOINT_NONE;
    }

    for(uint16_t i=0;i<len;i++)
    {
        value[i] = properties[index]->binaryValue.value[i];
    }
    len = properties[index]->binaryValue.len;
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;

    return readResult;
}

// dpCtrlType   0: 平台控制写数据   1：用户写数据

void intoyunPlatformWriteDatapointBool(const uint16_t dpID, bool value, bool dpCtrlType)
{
    int index = intoyunDiscoverProperty(dpID);
    if(index == -1)
    {
        return;
    }

    if(properties[index]->dataType != DATA_TYPE_BOOL)
    {
        return;
    }
    else
    {
        if(properties[index]->boolValue != value)
        {
            properties[index]->change = true;
            if(dpCtrlType)
            { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            }
            else
            {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
            properties[index]->boolValue = value;
        }
        else
        {
            properties[index]->change = false;
            if(dpCtrlType)
            { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            }
            else
            {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
        }

    }
}
void intoyunWriteDatapointBool(const uint16_t dpID, bool value)
{
    intoyunPlatformWriteDatapointBool(dpID,value,true);
}

void intoyunPlatformWriteDatapointNumberInt32(const uint16_t dpID, int32_t value, bool dpCtrlType)
{
    int index = intoyunDiscoverProperty(dpID);
    if(index == -1)
    {
        return;
    }

    if(properties[index]->dataType != DATA_TYPE_NUM || properties[index]->numberProperty.resolution != 0)
    {
        return;
    }
    else
    {
        int32_t tmp = value;
        if(tmp < properties[index]->numberProperty.minValue)
        {
            tmp = properties[index]->numberProperty.minValue;
        }
        else if(tmp > properties[index]->numberProperty.maxValue)
        {
            tmp = properties[index]->numberProperty.maxValue;
        }

        if(properties[index]->numberIntValue != value)
        {
            properties[index]->change = true;
            if(dpCtrlType)
            { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            }
            else
            {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
            properties[index]->numberIntValue = tmp;
        }
        else
        {
            properties[index]->change = false;
            if(dpCtrlType)
            { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            }
            else
            {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
        }
    }
}

void intoyunWriteDatapointNumberInt32(const uint16_t dpID, int32_t value)
{
    intoyunPlatformWriteDatapointNumberInt32(dpID,value,true);
}

void intoyunPlatformWriteDatapointNumberDouble(const uint16_t dpID, double value, bool dpCtrlType)
{
    int index = intoyunDiscoverProperty(dpID);
    if(index == -1)
    {
        return;
    }

    if(properties[index]->dataType != DATA_TYPE_NUM || properties[index]->numberProperty.resolution == 0)
    {
        return;
    }
    else
    {
        int32_t tmp;
        double d = value;
        if(d < properties[index]->numberProperty.minValue)
        {
            d = properties[index]->numberProperty.minValue;
        }
        else if(d > properties[index]->numberProperty.maxValue)
        {
            d = properties[index]->numberProperty.maxValue;
        }

        //保证小数点位数
        tmp = (int32_t)(d * pow(10, properties[index]->numberProperty.resolution));

        if(properties[index]->numberDoubleValue != d)
        {
            properties[index]->change = true;
            if(dpCtrlType)
            { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            }
            else
            {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }

            properties[index]->numberDoubleValue = tmp/(double)pow(10, properties[index]->numberProperty.resolution);
        }
        else
        {
            properties[index]->change = false;
            if(dpCtrlType)
            { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            }
            else
            {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
        }
    }
}

void intoyunWriteDatapointNumberDouble(const uint16_t dpID, double value)
{
    intoyunPlatformWriteDatapointNumberDouble(dpID,value,true);
}

void intoyunPlatformWriteDatapointEnum(const uint16_t dpID, int value, bool dpCtrlType)
{
    int index = intoyunDiscoverProperty(dpID);
    if(index == -1)
    {
        return;
    }

    if(properties[index]->dataType != DATA_TYPE_ENUM)
    {
        return;
    }
    else
    {
        if(properties[index]->enumValue != value)
        {
            properties[index]->change = true;
            if(dpCtrlType)
            { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            }
            else
            {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
            properties[index]->enumValue = value;
        }
        else
        {
            properties[index]->change = false;
            if(dpCtrlType)
            { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            }
            else
            {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
        }
    }
}

void intoyunWriteDatapointEnum(const uint16_t dpID, int value)
{
    intoyunPlatformWriteDatapointEnum(dpID,value,true);
}

void intoyunPlatformWriteDatapointString(const uint16_t dpID, const char *value, bool dpCtrlType)
{
    int index = intoyunDiscoverProperty(dpID);
    if(index == -1)
    {
        return;
    }

    if(properties[index]->dataType != DATA_TYPE_STRING)
    {
        return;
    }
    else
    {
        if(strcmp(properties[index]->stringValue,value) != 0)
        {
            properties[index]->change = true;
            if(dpCtrlType)
            { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            }
            else
            {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
            properties[index]->stringValue = (char *)malloc(strlen(value)+1);
            strncpy(properties[index]->stringValue,value,strlen(value)+1);
        }
        else
        {
            properties[index]->change = false;
            if(dpCtrlType)
            { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            }
            else
            {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
        }
    }
}

void intoyunWriteDatapointString(const uint16_t dpID, const char *value)
{
    intoyunPlatformWriteDatapointString(dpID,value,true);
}

void intoyunPlatformWriteDatapointBinary(const uint16_t dpID, const uint8_t *value, uint16_t len, bool dpCtrlType)
{
    int index = intoyunDiscoverProperty(dpID);
    if(index == -1)
    {
        return;
    }

    if(properties[index]->dataType != DATA_TYPE_BINARY)
    {
        return;
    }
    else
    {
        if(memcmp(properties[index]->binaryValue.value,value,len) != 0)
        {
            properties[index]->change = true;
            if(dpCtrlType)
            { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            }
            else
            {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }

            properties[index]->binaryValue.value = (uint8_t *)malloc(len);
            for(uint16_t i=0;i<len;i++)
            {
                properties[index]->binaryValue.value[i] = value[i];
            }
            properties[index]->binaryValue.len = (uint16_t)len;

        }
        else
        {
            properties[index]->change = false;
            if(dpCtrlType)
            { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            }
            else
            {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
        }
    }
}

void intoyunWriteDatapointBinary(const uint16_t dpID, const uint8_t *value, uint16_t len)
{
    intoyunPlatformWriteDatapointBinary(dpID,value,len,true);
}

void intoyunSendDatapointBool(const uint16_t dpID, bool value)
{
    int index = intoyunDiscoverProperty(dpID);

    if (index == -1)
    {
        return;
    }

    intoyunPlatformWriteDatapointBool(dpID, value, true);

    if(DP_TRANSMIT_MODE_AUTOMATIC == intoyunGetDatapointTransmitMode()) {
        return;
    }

    //只允许下发
    if ( properties[index]->permission == DP_PERMISSION_DOWN_ONLY)
    {
        return;
    }

    //数值未发生变化
    if (!properties[index]->change && properties[index]->policy == DP_POLICY_NONE)
    {
        return;
    }

    intoyunSendSingleDatapoint(index);
}

void intoyunSendDatapointNumberInt32(const uint16_t dpID, int32_t value)
{
    int index = intoyunDiscoverProperty(dpID);

    if (index == -1)
    {
        return;
    }

    intoyunPlatformWriteDatapointNumberInt32(dpID, value, true);

    if(DP_TRANSMIT_MODE_AUTOMATIC == intoyunGetDatapointTransmitMode()) {
        return;
    }

    //只允许下发
    if ( properties[index]->permission == DP_PERMISSION_DOWN_ONLY)
    {
        return;
    }

    //数值未发生变化
    if (!properties[index]->change && properties[index]->policy == DP_POLICY_NONE)
    {
        return;
    }

    intoyunSendSingleDatapoint(index);
}

void intoyunSendDatapointNumberDouble(const uint16_t dpID, double value)
{
    int index = intoyunDiscoverProperty(dpID);

    if (index == -1)
    {
        // not found, nothing to do
        return;
    }

    intoyunPlatformWriteDatapointNumberDouble(dpID, value, true);

    if(DP_TRANSMIT_MODE_AUTOMATIC == intoyunGetDatapointTransmitMode()) {
        return;
    }

    //只允许下发
    if ( properties[index]->permission == DP_PERMISSION_DOWN_ONLY)
    {
        return;
    }

    //数值未发生变化
    if (!properties[index]->change && properties[index]->policy == DP_POLICY_NONE)
    {
        return;
    }

    intoyunSendSingleDatapoint(index);
}

void intoyunSendDatapointEnum(const uint16_t dpID, int value)
{
    int index = intoyunDiscoverProperty(dpID);

    if (index == -1)
    {
        // not found, nothing to do
        return;
    }

    intoyunPlatformWriteDatapointEnum(dpID, value, true);

    if(DP_TRANSMIT_MODE_AUTOMATIC == intoyunGetDatapointTransmitMode()) {
        return;
    }


    //只允许下发
    if ( properties[index]->permission == DP_PERMISSION_DOWN_ONLY)
    {
        return;
    }

    //数值未发生变化
    if (!properties[index]->change && properties[index]->policy == DP_POLICY_NONE)
    {
        return;
    }

    intoyunSendSingleDatapoint(index);
}

void intoyunSendDatapointString(const uint16_t dpID, const char *value)
{
    int index = intoyunDiscoverProperty(dpID);

    if (index == -1)
    {
        // not found, nothing to do
        return;
    }

    intoyunPlatformWriteDatapointString(dpID, value, true);

    if(DP_TRANSMIT_MODE_AUTOMATIC == intoyunGetDatapointTransmitMode()) {
        return;
    }


    //只允许下发
    if (properties[index]->permission == DP_PERMISSION_DOWN_ONLY)
    {
        return;
    }

    //数值未发生变化
    if (!properties[index]->change && properties[index]->policy == DP_POLICY_NONE)
    {
        return;
    }

    intoyunSendSingleDatapoint(index);
}

void intoyunSendDatapointBinary(const uint16_t dpID, const uint8_t *value, uint16_t len)
{
    int index = intoyunDiscoverProperty(dpID);

    if (index == -1)
    {
        // not found, nothing to do
        return;
    }

    intoyunPlatformWriteDatapointBinary(dpID, value, len, true);

    if(DP_TRANSMIT_MODE_AUTOMATIC == intoyunGetDatapointTransmitMode()) {
        return;
    }


    //只允许下发
    if ( properties[index]->permission == DP_PERMISSION_DOWN_ONLY)
    {
        return;
    }

    //数值未发生变化
    if (!properties[index]->change && properties[index]->policy == DP_POLICY_NONE)
    {
        return;
    }

    intoyunSendSingleDatapoint(index);
}

void intoyunParseReceiveDatapoints(const uint8_t *payload, uint32_t len, uint8_t *customData)
{
    #ifdef INTOYUN_DEBUG
    printf("receive data length = %d\r\n",len);
    printf("receive data:\r\n");
    for(uint16_t i = 0; i < len; i++)
    {
        printf("0x%x",payload[i]);
        printf(" ");
    }
    printf("\r\n");
    #endif

    //dpid(1-2 bytes)+data type(1 byte)+data len(1-2 bytes)+data(n bytes)
    //大端表示，如果最高位是1，则表示两个字节，否则是一个字节
    int32_t index = 0;
    uint16_t dpID = 0;
    uint8_t dataType;
    uint16_t dataLength=0;

    if(payload[0] == CUSTOMER_DEFINE_DATA) //用户透传数据
    {
        (*customData) = CUSTOMER_DEFINE_DATA;
        return;
    }
    else
    {
        (*customData) = INTOYUN_DATAPOINT_DATA;
    }

    index++;

    while(index < len)
    {
        dpID = payload[index++] & 0xff;
        if(dpID >= 0x80) //数据点有2个字节
        {
            dpID = dpID & 0x7f; //去掉最高位
            dpID = (dpID << 8) | payload[index++];
        }

        dataType = payload[index++];

        switch(dataType)
        {
            case DATA_TYPE_BOOL:
            {
                dataLength = payload[index++] & 0xff;
                bool value = payload[index++];
                if(intoyunDiscoverProperty(dpID) != -1)//数据点存在
                {
                    intoyunPlatformWriteDatapointBool(dpID,value,false);
                }
            }
            break;

            case DATA_TYPE_NUM:
            {
                dataLength = payload[index++] & 0xff;
                int32_t value = payload[index++] & 0xff;
                if(dataLength == 2)
                {
                    value = (value << 8) | payload[index++];
                }
                else if(dataLength == 3)
                {
                    value = (value << 8) | payload[index++];
                    value = (value << 8) | payload[index++];
                }
                else if(dataLength == 4)
                {
                    value = (value << 8) | payload[index++];
                    value = (value << 8) | payload[index++];
                    value = (value << 8) | payload[index++];
                }

                uint8_t id = intoyunDiscoverProperty(dpID);
                if(properties[id]->numberProperty.resolution == 0) //此数据点为int
                {
                    value = value + properties[id]->numberProperty.minValue;

                    if(intoyunDiscoverProperty(dpID) != -1)//数据点存在
                    {
                        intoyunPlatformWriteDatapointNumberInt32(dpID,value,false);
                    }
                }
                else
                {
                    double dValue = (value/(double)pow(10, properties[id]->numberProperty.resolution)) + properties[id]->numberProperty.minValue;

                    if(intoyunDiscoverProperty(dpID) != -1)//数据点存在
                    {
                        intoyunPlatformWriteDatapointNumberDouble(dpID,dValue,false);
                    }
                }

            }

            break;

            case DATA_TYPE_ENUM:
            {
                dataLength = payload[index++] & 0xff;
                int value = payload[index++] & 0xff;
                if(dataLength == 2)
                {
                    value = (value << 8) | payload[index++];
                }
                else if(dataLength == 3)
                {
                    value = (value << 8) | payload[index++];
                    value = (value << 8) | payload[index++];
                }
                else if(dataLength == 4)
                {
                    value = (value << 8) | payload[index++];
                    value = (value << 8) | payload[index++];
                    value = (value << 8) | payload[index++];
                }

                if(intoyunDiscoverProperty(dpID) != -1)//数据点存在
                {
                    intoyunPlatformWriteDatapointEnum(dpID,value,false);
                }
            }
            break;

            case DATA_TYPE_STRING:
            {
                dataLength = payload[index++] & 0xff;
                if(dataLength >= 0x80) //数据长度有2个字节
                {
                    dataLength = dataLength & 0x7f;
                    dataLength = (dataLength) << 8 | payload[index++];
                }
                char *str = (char *)malloc(dataLength+1);
                if(NULL != str)
                {
                    memset(str, 0, dataLength+1);
                    memcpy(str, &payload[index], dataLength);
                }

                index += dataLength;
                if(intoyunDiscoverProperty(dpID) != -1)//数据点存在
                {
                    intoyunPlatformWriteDatapointString(dpID,str,false);
                }
                free(str);
            }
            break;

            case DATA_TYPE_BINARY:
            {
                dataLength = payload[index++] & 0xff;
                if(dataLength >= 0x80) //数据长度有2个字节
                {
                    dataLength = dataLength & 0x7f;
                    dataLength = (dataLength) << 8 | payload[index++];
                }

                if(intoyunDiscoverProperty(dpID) != -1)//数据点存在
                {
                    intoyunPlatformWriteDatapointBinary(dpID, &payload[index], dataLength,false);
                }

                index += dataLength;
            }
            break;

            default:
                break;
        }
    }
}

//组织数据点数据
static uint16_t intoyunFormDataPointBinary(int property_index, uint8_t* buffer)
{
    int32_t index = 0;

    if(properties[property_index]->dpID < 0x80)
    {
        buffer[index++] = properties[property_index]->dpID & 0xFF;
    }
    else
    {
        buffer[index++] = (properties[property_index]->dpID >> 8) | 0x80;
        buffer[index++] = properties[property_index]->dpID & 0xFF;
    }

    switch(properties[property_index]->dataType)
    {
        case DATA_TYPE_BOOL:       //bool型
            buffer[index++] = 0x00;  //类型
            buffer[index++] = 0x01;  //长度
            buffer[index++] = (bool)(properties[property_index]->boolValue);
            break;

        case DATA_TYPE_NUM:        //数值型 int型
            {
                buffer[index++] = 0x01;
                int32_t value;
                if(properties[property_index]->numberProperty.resolution == 0)
                {
                    value = (int32_t)properties[property_index]->numberIntValue;
                }
                else
                {
                    value = (properties[property_index]->numberDoubleValue - properties[property_index]->numberProperty.minValue) \
                        * pow(10, properties[property_index]->numberProperty.resolution);
                }

                if(value & 0xFFFF0000) {
                    buffer[index++] = 0x04;
                    buffer[index++] = (value >> 24) & 0xFF;
                    buffer[index++] = (value >> 16) & 0xFF;
                    buffer[index++] = (value >> 8) & 0xFF;
                    buffer[index++] = value & 0xFF;
                } else if(value & 0xFFFFFF00) {
                    buffer[index++] = 0x02;
                    buffer[index++] = (value >> 8) & 0xFF;
                    buffer[index++] = value & 0xFF;
                } else {
                    buffer[index++] = 0x01;
                    buffer[index++] = value & 0xFF;
                }
            }
            break;

        case DATA_TYPE_ENUM:       //枚举型
            buffer[index++] = 0x02;
            buffer[index++] = 0x01;
            buffer[index++] = (uint8_t)properties[property_index]->enumValue & 0xFF;
            break;

        case DATA_TYPE_STRING:     //字符串型
            {
                uint16_t strLength = strlen(properties[property_index]->stringValue);

                buffer[index++] = 0x03;
                if(strLength < 0x80)
                {
                    buffer[index++] = strLength & 0xFF;
                }
                else
                {
                    buffer[index++] = (strLength >> 8) | 0x80;
                    buffer[index++] = strLength & 0xFF;
                }
                memcpy(&buffer[index], properties[property_index]->stringValue, strLength);
                index+=strLength;
                break;
            }

        case DATA_TYPE_BINARY:     //二进制型
            {
                uint16_t len = properties[property_index]->binaryValue.len;
                buffer[index++] = DATA_TYPE_BINARY;
                if(len < 0x80) {
                    buffer[index++] = len & 0xFF;
                } else {
                    buffer[index++] = (len >> 8) | 0x80;
                    buffer[index++] = len & 0xFF;
                }
                memcpy(&buffer[index], properties[property_index]->binaryValue.value, len);
                index+=len;
                break;
            }

        default:
            break;
    }
    return index;
}

//组织单个数据点的数据
static uint16_t intoyunFormSingleDatapoint(int property_index, uint8_t *buffer, uint16_t len)
{
    int32_t index = 0;

    buffer[index++] = INTOYUN_DATAPOINT_DATA;
    index += intoyunFormDataPointBinary(property_index, buffer+index);
    return index;
}

// dpForm   false: 组织改变的数据点   true：组织全部的数据点
//组织所有数据点的数据
static uint16_t intoyunFormAllDatapoint(uint8_t *buffer, uint16_t len, bool dpForm)
{
    int32_t index = 0;

    buffer[index++] = INTOYUN_DATAPOINT_DATA;
    for (int i = 0; i < properties_count; i++)
    {
        //只允许下发  不上传
        if (properties[i]->permission == DP_PERMISSION_DOWN_ONLY)
        {
            continue;
        }

        //系统默认dpID  不上传
        if (properties[i]->dpID > 0xFF00)
        {
            continue;
        }

        if( dpForm || ((!dpForm) && properties[i]->change) )
        {
            index += intoyunFormDataPointBinary(i, (uint8_t *)buffer+index);
        }
    }
    return index;
}

//发送单个数据点的数据
void intoyunSendSingleDatapoint(const uint16_t dpID)
{
    //发送时间间隔到
    uint32_t current_millis = millis();
    int32_t elapsed_millis = current_millis - properties[dpID]->runtime;
    if (elapsed_millis < 0)
    {
        elapsed_millis =  0xFFFFFFFF - properties[dpID]->runtime + current_millis;
    }

    if (elapsed_millis >= properties[dpID]->lapse)
    {
        uint8_t buffer[256];
        uint16_t len;

        len = intoyunFormSingleDatapoint(dpID, buffer, sizeof(buffer));
        intoyunTransmitData(buffer,len);
        properties[dpID]->runtime = current_millis;
    }
}

//发送所有数据点的数据
void intoyunSendDatapointAll(bool dpForm)
{
    uint8_t buffer[512];
    uint16_t len;

    len = intoyunFormAllDatapoint(buffer, sizeof(buffer), dpForm);

    #ifdef INTOYUN_DEBUG
    printf("send data length = %d\r\n",len);
    printf("send data:\r\n");
    for(uint16_t i=0;i<len;i++)
    {
        printf("0x%x",buffer[i]);
        printf(" ");
    }
    printf("\r\n");
    #endif

    intoyunTransmitData(buffer,len);
}

void intoyunSendCustomData(const uint8_t *buffer, uint16_t len)
{
    uint8_t buf[256];
    uint16_t index = len+1;
    if(index > 256)
    {
        index = 256;
    }

    buf[0] = CUSTOMER_DEFINE_DATA;
    memcpy(&buf[1],buffer,index-1);
    intoyunTransmitData(buf,index);
}

//发送数据
void intoyunTransmitData(const uint8_t *buffer, uint16_t len)
{
    ProtocolSendPlatformData(buffer, len);
}

void intoyunSendAllDatapointManual(void)
{
    if(0 == intoyunGetPropertyCount()) {
        return;
    }

    if(DP_TRANSMIT_MODE_AUTOMATIC == intoyunGetDatapointTransmitMode()) {
        return;
    }

    //发送时间间隔到
    uint32_t current_millis = millis();
    int32_t elapsed_millis = current_millis - g_datapoint_control.runtime;
    if (elapsed_millis < 0)
    {
        elapsed_millis =  0xFFFFFFFF - g_datapoint_control.runtime + current_millis;
    }

    //发送时间时间到
    if ( elapsed_millis >= g_datapoint_control.datapoint_transmit_lapse*1000 )
    {
        intoyunSendDatapointAll(true);

        g_datapoint_control.runtime = millis();
        intoyunPropertyChangeClear();
    }

}

void intoyunSendDatapointAutomatic(void)
{
    bool sendFlag = false;

    if(0 == intoyunGetPropertyCount()) {
        return;
    }

    if(DP_TRANSMIT_MODE_MANUAL == intoyunGetDatapointTransmitMode()) {
        return;
    }

    //当数值发生变化
    if(intoyunPropertyChanged())
    {
        sendFlag = true;
        intoyunSendDatapointAll(false);
    }
    else
    {
        //发送时间间隔到
        uint32_t current_millis = millis();
        int32_t elapsed_millis = current_millis - g_datapoint_control.runtime;
        if (elapsed_millis < 0)
        {
            elapsed_millis =  0xFFFFFFFF - g_datapoint_control.runtime + current_millis;
        }

        //发送时间时间到
        if ( elapsed_millis >= g_datapoint_control.datapoint_transmit_lapse*1000 )
        {
            sendFlag = true;
            intoyunSendDatapointAll(true);
        }
    }

    if(sendFlag)
    {
        g_datapoint_control.runtime = millis();
        intoyunPropertyChangeClear();
    }
}
