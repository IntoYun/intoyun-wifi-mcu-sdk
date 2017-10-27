#include "intoyun_interface.h"
#include "stm32f1xx_hal.h"

#define PRODUCT_ID                       "QRGro2Xk9P4c42eb"//产品ID
#define PRODUCT_SECRET                   "606b833b5879b55498f89f03d95f6e29"//产品秘钥
#define HARDWARE_VERSION                 "V1.0.0"          //硬件版本号
#define SOFTWARE_VERSION                 "V1.0.0"          //软件版本号


#define LED_PIN             GPIO_PIN_0
#define LED_GPIO_PORT       GPIOB
#define LED_ON	 	          HAL_GPIO_WritePin(LED_GPIO_PORT,LED_PIN, GPIO_PIN_RESET)
#define LED_OFF		          HAL_GPIO_WritePin(LED_GPIO_PORT,LED_PIN, GPIO_PIN_SET)
#define LED_TOG		          HAL_GPIO_TogglePin(LED_GPIO_PORT,LED_PIN)


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
    LED_Init();
    //添加数据点定义
    Cloud.defineDatapointEnum(DPID_ENUM_LIGHT_MODE, DP_PERMISSION_UP_DOWN, 2);                         //颜色模式
    Cloud.defineDatapointNumber(DPID_NUMBER_TEMPERATURE, DP_PERMISSION_UP_ONLY, -100, 100, 2, 22.34);  //温度
    Cloud.defineDatapointBool(DPID_BOOL_SWITCH, DP_PERMISSION_UP_DOWN, false);                         //灯泡开关
    Cloud.defineDatapointBool(DPID_BOOL_LIGHT_STATUS, DP_PERMISSION_UP_ONLY, false);                   //灯泡状态
    Cloud.defineDatapointNumber(DPID_NUMBER_SPEED, DP_PERMISSION_UP_DOWN, 0, 1000, 0, 55);         //速度
    /* Cloud.defineDatapointString(DPID_STRING_LCD_DISPLAY, DP_PERMISSION_UP_DOWN, "hello world!!!");     //字符显示 */
    Cloud.defineDatapointString(DPID_STRING_LCD_DISPLAY, DP_PERMISSION_UP_DOWN, dpStringLcdDisplay);     //字符显示
    Cloud.defineDatapointBinary(DPID_BINARY, DP_PERMISSION_UP_DOWN, dpBinaryVal,9);                    //二进制数据
}

void userHandle(void)
{
    //处理需要上送到云平台的数据
    Cloud.writeDatapointNumberInt32(DPID_NUMBER_SPEED, dpNumberSpeed);
    Cloud.writeDatapointNumberDouble(DPID_NUMBER_TEMPERATURE, dpNumberTemperature);
    Cloud.writeDatapointEnum(DPID_ENUM_LIGHT_MODE,dpEnumLightMode);
    Cloud.writeDatapointString(DPID_STRING_LCD_DISPLAY,dpStringLcdDisplay);
    Cloud.writeDatapointBinary(DPID_BINARY,dpBinaryVal,9);
}


void eventProcess(uint8_t eventType, uint8_t event, uint8_t *data, uint32_t len)
{
    if(eventType == event_cloud_data){
        switch(event){
        case ep_datapoint_data: //处理平台数据
            //灯泡控制
            if (RESULT_DATAPOINT_NEW == Cloud.readDatapointBool(DPID_BOOL_SWITCH, &dpBoolLightSwitch)){
                log_v("switch value = %d\r\n",dpBoolLightSwitch);
                if(dpBoolLightSwitch){
                    /* 开灯 */
                    LED_ON;
                    Cloud.writeDatapointBool(DPID_BOOL_LIGHT_STATUS, true);
                    Cloud.writeDatapointBool(DPID_BOOL_SWITCH, true);
                }else{
                    //关灯
                    LED_OFF;
                    Cloud.writeDatapointBool(DPID_BOOL_LIGHT_STATUS, false);
                    Cloud.writeDatapointBool(DPID_BOOL_SWITCH, false);
                }
            }
            //速度控制
            if (RESULT_DATAPOINT_NEW == Cloud.readDatapointNumberInt32(DPID_NUMBER_SPEED, &dpNumberSpeed)){
                log_v("speed = %d\r\n",dpNumberSpeed);
            }

            if(RESULT_DATAPOINT_NEW == Cloud.readDatapointNumberDouble(DPID_NUMBER_TEMPERATURE,&dpNumberTemperature)){
                log_v("tempature = %f\r\n",dpNumberTemperature);
            }
            //颜色模式
            if (RESULT_DATAPOINT_NEW == Cloud.readDatapointEnum(DPID_ENUM_LIGHT_MODE, &dpEnumLightMode)){
                log_v("corlor mode = %d\r\n",dpEnumLightMode);
            }
            //字符串显示
            if (RESULT_DATAPOINT_NEW == Cloud.readDatapointString(DPID_STRING_LCD_DISPLAY, dpStringLcdDisplay)){
                log_v("string = %s\r\n",dpStringLcdDisplay);
            }
            //二进制数据
            if(RESULT_DATAPOINT_NEW == Cloud.readDatapointBinary(DPID_BINARY,dpBinaryVal,9)){
                log_v("dpBinaryVal\r\n");
                log_v_dump(dpBinaryVal,9);
            }

            break;
        case ep_custom_data: //接受到透传数据
            break;
        default:
            break;
        }
    }else if(eventType == event_network_status){
            switch(event){
            case ep_network_disconnect_router:  //模组已断开路由器
                log_v("event network disconnect router\r\n");
                break;
            case ep_network_connect_router:     //模组已连接路由器
                log_v("event network connect router\r\n");
                break;
            case ep_network_disconnect_server:  //模组已断开平台
                log_v("event network disconnect server\r\n");
                break;
            case ep_network_connect_server:     //模组已连接平台
                log_v("event network connect server\r\n");
                break;
            default:
                break;
            }
    }else if(eventType == event_mode_changed){
            switch(event){
            case ep_mode_normal:          //模组已处于正常工作模式
                log_v("event mode normal\r\n");
                break;
            case ep_mode_imlink_config:   //模组已处于imlink配置模式
                log_v("event mode imlink config\r\n");
                break;
            case ep_mode_ap_config:       //模组已处于ap配置模式
                log_v("event mode ap config\r\n");
                break;
            case ep_mode_binding:         //模组已处于绑定模式
                log_v("event mode binding\r\n");
                break;
            default:
                break;
            }
    }
}

int main(void)
{
    System.init();
    log_v("wifi mcu slave\r\n");
    userInit();
    delay(200);
    System.setEventCallback(eventProcess);
    System.setDeviceInfo(PRODUCT_ID,PRODUCT_SECRET, HARDWARE_VERSION,SOFTWARE_VERSION);
    Cloud.connect();

    while(1)
    {
        System.loop();
        if(Cloud.connected()){
            userHandle();
        }
    }
}
