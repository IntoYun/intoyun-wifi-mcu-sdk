/*
*  产品名称：
*  产品描述：
*  说    明： 该代码为平台自动根据产品内容生成的代码模板框架，
*             您可以在此框架下开发。此框架包括数据点的定义和读写。
*  模板版本： v1.4
*/

#include "project_config.h"
#include "intoyun_interface.h"

#define DPID_ENUM_LIGHT_MODE             1    //颜色模式
#define DPID_NUMBER_TEMPERATURE          2    //温度
#define DPID_BOOL_SWITCH                 3    //灯泡开关
#define DPID_BOOL_LIGHT_STATUS           4    //灯泡状态
#define DPID_NUMBER_SPEED                5    //速度
#define DPID_STRING_LCD_DISPLAY          6    //字符显示
#define DPID_BINARY                      7    //二进制

//定义接收云平台数据变量
bool dpBoolLightSwitch;       //开关命令
int32_t dpNumberSpeed;        //速度
int dpEnumLightMode;          //颜色模式
char dpStringLcdDisplay[20] = "www.intoyun.com";  //字符显示
double dpNumberTemperature = 11.39;   //温度
uint8_t dpBinaryVal[9] = {0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9}; //二进制数据
uint16_t binaryLen;

void system_event_callback(system_event_t event, int param, uint8_t *data, uint16_t len)
{
    if(event == event_cloud_data){
        switch(param){
        case ep_cloud_data_datapoint: //处理平台数据
            if (RESULT_DATAPOINT_NEW == Cloud.readDatapointBool(DPID_BOOL_SWITCH, &dpBoolLightSwitch)){
            }
            //速度控制
            if (RESULT_DATAPOINT_NEW == Cloud.readDatapointNumberInt32(DPID_NUMBER_SPEED, &dpNumberSpeed)){
            }

            if(RESULT_DATAPOINT_NEW == Cloud.readDatapointNumberDouble(DPID_NUMBER_TEMPERATURE,&dpNumberTemperature)){
            }
            //颜色模式
            if (RESULT_DATAPOINT_NEW == Cloud.readDatapointEnum(DPID_ENUM_LIGHT_MODE, &dpEnumLightMode)){
            }
            //字符串显示
            if (RESULT_DATAPOINT_NEW == Cloud.readDatapointString(DPID_STRING_LCD_DISPLAY, dpStringLcdDisplay)){
            }
            //二进制数据
            if(RESULT_DATAPOINT_NEW == Cloud.readDatapointBinary(DPID_BINARY,dpBinaryVal,&binaryLen)){
            }

            break;
        case ep_cloud_data_custom: //接受到透传数据
            break;
        default:
            break;
        }
    }else if(event == event_network_status){
            switch(param){
            case ep_network_status_disconnected:  //模组已断开路由器
                break;
            case ep_network_status_connected:     //模组已连接路由器
                break;
            case ep_cloud_status_disconnected:  //模组已断开平台
                break;
            case ep_cloud_status_connected:     //模组已连接平台
                break;
            default:
                break;
            }
    }else if(event == event_mode_changed){
            switch(param){
            case ep_mode_normal:          //模组已处于正常工作模式
                break;
            case ep_mode_imlink_config:   //模组已处于imlink配置模式
                break;
            case ep_mode_ap_config:       //模组已处于ap配置模式
                break;
            case ep_mode_binding:         //模组已处于绑定模式
                break;
            default:
                break;
            }
    }
}


void userInit(void)
{
    Cloud.defineDatapointEnum(DPID_ENUM_LIGHT_MODE, DP_PERMISSION_UP_DOWN, 2);                         //颜色模式
    Cloud.defineDatapointNumber(DPID_NUMBER_TEMPERATURE, DP_PERMISSION_UP_ONLY, -100, 100, 2, 22.34);  //温度
    Cloud.defineDatapointBool(DPID_BOOL_SWITCH, DP_PERMISSION_UP_DOWN, false);                         //灯泡开关
    Cloud.defineDatapointBool(DPID_BOOL_LIGHT_STATUS, DP_PERMISSION_UP_ONLY, false);                   //灯泡状态
    Cloud.defineDatapointNumber(DPID_NUMBER_SPEED, DP_PERMISSION_UP_DOWN, 0, 1000, 0, 55);         //速度
    Cloud.defineDatapointString(DPID_STRING_LCD_DISPLAY, DP_PERMISSION_UP_DOWN, dpStringLcdDisplay);     //字符显示
    Cloud.defineDatapointBinary(DPID_BINARY, DP_PERMISSION_UP_DOWN, dpBinaryVal,9);                    //二进制数据
    delay(200);
    System.setEventCallback(system_event_callback);
    System.setDeviceInfo(PRODUCT_ID_DEF,PRODUCT_SECRET_DEF, HARDWARE_VERSION_DEF,SOFTWARE_VERSION_DEF);
    Cloud.connect();
}

void userHandle(void)
{
    if(Cloud.connected()){
        Cloud.writeDatapointNumberInt32(DPID_NUMBER_SPEED, dpNumberSpeed);
        Cloud.writeDatapointNumberDouble(DPID_NUMBER_TEMPERATURE, dpNumberTemperature);
        Cloud.writeDatapointEnum(DPID_ENUM_LIGHT_MODE,dpEnumLightMode);
        Cloud.writeDatapointString(DPID_STRING_LCD_DISPLAY,dpStringLcdDisplay);
        Cloud.writeDatapointBinary(DPID_BINARY,dpBinaryVal,9);
    }
}

int main(void)
{
    System.init();
    userInit();
    while(1)
    {
        System.loop();
        userHandle();
    }
}
