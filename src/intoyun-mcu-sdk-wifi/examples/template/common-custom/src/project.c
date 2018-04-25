#include "iot_export.h"
#include "project_config.h"

void eventProcess(int event, int param, uint8_t *data, uint32_t len)
{
    if(event == event_cloud_comm) {
        switch(param) {
            case ep_cloud_comm_data: //处理平台数据
                break;
            default:
                break;
        }
    } else if(event == event_network_status) {
        switch(param) {
            case ep_network_status_disconnected:  //模组已断开路由器
                log_v("event network disconnect router\r\n");
                break;
            case ep_network_status_connected:     //模组已连接路由器
                log_v("event network connect router\r\n");
                break;
            default:
                break;
        }
    } else if(event == event_cloud_status) {
        switch(param){
            case ep_cloud_status_disconnected:    //模组已断开平台
                log_info("event cloud disconnect server\r\n");
                break;
            case ep_cloud_status_connected:       //模组已连接平台
                log_info("event cloud connect server\r\n");
                break;
            default:
                break;
        }
    } else if(event == event_mode_changed) {
        switch(param) {
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

void userInit(void)
{
    System.init();
    System.setEventCallback(eventProcess);
    System.setDeviceInfo(PRODUCT_ID_DEF,PRODUCT_SECRET_DEF, HARDWARE_VERSION_DEF,SOFTWARE_VERSION_DEF);
    Cloud.connect();
}

void userHandle(void)
{
    if(Cloud.connected()) {
        //处理需要上送到云平台的数据
        Cloud.sendCustomData(NULL, 0);
    }
}

int userMain(void)
{
    userInit();
    while(1) {
        userHandle();
        System.loop();
    }
}

