#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "main.h"
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
char dpStringLcdDisplay[50];  //字符显示
double dpNumberTemperature;   //温度
uint8_t dpBinaryVal[9] = {0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9}; //二进制数据


void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    __GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_RESET);
}

void userInit(void)
{
    //添加数据点定义
    IntoYun.defineDatapointEnum(DPID_ENUM_LIGHT_MODE, DP_PERMISSION_DOWN_ONLY, 2, DP_POLICY_NONE, 0);                         //颜色模式
    IntoYun.defineDatapointNumber(DPID_NUMBER_TEMPERATURE, DP_PERMISSION_UP_ONLY, -100, 100, 2, 22.34, DP_POLICY_NONE, 0);  //温度
    IntoYun.defineDatapointBool(DPID_BOOL_SWITCH, DP_PERMISSION_UP_DOWN, false, DP_POLICY_NONE, 0);                         //灯泡开关
    IntoYun.defineDatapointBool(DPID_BOOL_LIGHT_STATUS, DP_PERMISSION_UP_ONLY, false, DP_POLICY_NONE, 0);                   //灯泡开关
    IntoYun.defineDatapointNumber(DPID_NUMBER_SPEED, DP_PERMISSION_UP_DOWN, -1000, 1000, 0, 55, DP_POLICY_NONE, 0);             //速度
    IntoYun.defineDatapointString(DPID_STRING_LCD_DISPLAY, DP_PERMISSION_UP_DOWN, "hello world!!!", DP_POLICY_NONE, 0);     //字符显示
    IntoYun.defineDatapointBinary(DPID_BINARY, DP_PERMISSION_UP_DOWN, dpBinaryVal,9, DP_POLICY_NONE, 0);               //二进制数据
}

void userHandle(void)
{
    //处理需要上送到云平台的数据
    IntoYun.writeDatapointNumberInt32(DPID_NUMBER_SPEED, dpNumberSpeed);
    IntoYun.writeDatapointNumberDouble(DPID_NUMBER_TEMPERATURE, dpNumberTemperature);
    IntoYun.writeDatapointEnum(DPID_ENUM_LIGHT_MODE,dpEnumLightMode);
    IntoYun.writeDatapointString(DPID_STRING_LCD_DISPLAY,dpStringLcdDisplay);
    IntoYun.writeDatapointBinary(DPID_BINARY,dpBinaryVal,9);
}


void eventProcess(event_type_t event, uint8_t *data, uint32_t len)
{
    switch(event)
    {
        case EVENT_DATAPOINT: //处理平台数据
            //灯泡控制
            if (RESULT_DATAPOINT_NEW == IntoYun.readDatapointBool(DPID_BOOL_SWITCH, &dpBoolLightSwitch))
            {
                printf("switch value = %d\r\n",dpBoolLightSwitch);
                if(dpBoolLightSwitch)
                {
                    /* 开灯 */
                    LED_ON;
                    IntoYun.writeDatapointBool(DPID_BOOL_LIGHT_STATUS, true);
                    IntoYun.writeDatapointBool(DPID_BOOL_SWITCH, true);
                }
                else
                {
                    //关灯
                    LED_OFF;
                    IntoYun.writeDatapointBool(DPID_BOOL_LIGHT_STATUS, false);
                    IntoYun.writeDatapointBool(DPID_BOOL_SWITCH, false);
                }
            }
            //速度控制
            if (RESULT_DATAPOINT_NEW == IntoYun.readDatapointNumberInt32(DPID_NUMBER_SPEED, &dpNumberSpeed))
            {
                printf("speed = %d\r\n",dpNumberSpeed);
            }

            if(RESULT_DATAPOINT_NEW == IntoYun.readDatapointNumberDouble(DPID_NUMBER_TEMPERATURE,&dpNumberTemperature))
            {
                printf("tempature = %f\r\n",dpNumberTemperature);
            }
            //颜色模式
            if (RESULT_DATAPOINT_NEW == IntoYun.readDatapointEnum(DPID_ENUM_LIGHT_MODE, &dpEnumLightMode))
            {
                printf("corlor mode = %d\r\n",dpEnumLightMode);
            }
            //字符串显示
            if (RESULT_DATAPOINT_NEW == IntoYun.readDatapointString(DPID_STRING_LCD_DISPLAY, dpStringLcdDisplay))
            {
                printf("string = %s\r\n",dpStringLcdDisplay);
            }
            //二进制数据
            if(RESULT_DATAPOINT_NEW == IntoYun.readDatapointBinary(DPID_BINARY,dpBinaryVal,9))
            {
                for(uint8_t i = 0; i < 9; i++)
                {
                    printf("binary val = 0x%x\r\n",dpBinaryVal[i]);
                }
            }
            break;

        case EVENT_CUSTOM_DATA: //接受到透传数据
            break;

        case EVENT_MODE_NORMAL: //模组处于正常工作模式
            break;

        case EVENT_MODE_IMLINK_CONFIG: //模组处于IMLINK配置模式
            break;

        case EVENT_CON_ROUTER:  //模组已连接路由器
            break;

        case EVENT_DISCON_ROUTER://模组已断开路由器
            break;

        case EVENT_CON_SERVER://模组已连服务器
            break;

        case EVENT_DISCON_SERVER://模组已断开服务器
            break;

        case EVENT_CON_APP://模组已连接APP
            break;

        case EVENT_DISCON_APP: //模组已断开APP
            break;

        default:
            break;
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    LED_Init();
    USART2_Init();
    USART1_Init();

    userInit();
    IntoYun.init();
    IntoYun.sendProductInfo(PRODUCT_ID,HARDWARE_VERSION,SOFTWARE_VERSION);
    delay(50);
    IntoYun.setEventCallback(eventProcess);

    while(1)
    {
        userHandle();
        IntoYun.loop();
    }
}
