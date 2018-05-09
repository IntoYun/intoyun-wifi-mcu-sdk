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

#include "iot_export.h"
#include "project_config.h"

const static char *TAG = "user:project";
uint32_t timerID;

void eventProcess(int event, int param, uint8_t *data, uint32_t datalen)
{
    if(event == event_cloud_comm) {
        switch(param) {
            case ep_cloud_comm_data:
                //处理用户控制数据
                MOLMC_LOGD(TAG, "Recv the cloud data: ");
                MOLMC_LOG_BUFFER_HEX(TAG, data, datalen);
                break;
            default:
                break;
        }
    } else if(event == event_network_status) {
        switch(param){
            case ep_network_status_disconnected:  //模组已断开路由器
                MOLMC_LOGI(TAG, "event network disconnect router");
                break;
            case ep_network_status_connected:     //模组已连接路由器
                MOLMC_LOGI(TAG, "event network connect router");
                break;
            default:
                break;
        }
    } else if(event == event_cloud_status) {
        switch(param){
            case ep_cloud_status_disconnected:    //模组已断开平台
                MOLMC_LOGI(TAG, "event cloud disconnect server");
                break;
            case ep_cloud_status_connected:       //模组已连接平台
                MOLMC_LOGI(TAG, "event cloud connect server");
            default:
                break;
        }
    } else if(event == event_mode_changed) {
        switch(param) {
            case ep_mode_normal:          //模组已处于正常工作模式
                MOLMC_LOGI(TAG, "event mode normal\r\n");
                break;
            case ep_mode_imlink_config:   //模组已处于imlink配置模式
                MOLMC_LOGI(TAG, "event mode imlink config\r\n");
                break;
            case ep_mode_ap_config:       //模组已处于ap配置模式
                MOLMC_LOGI(TAG, "event mode ap config\r\n");
                break;
            case ep_mode_binding:         //模组已处于绑定模式
                MOLMC_LOGI(TAG, "event mode binding\r\n");
                break;
            default:
                break;
        }
    }
}

void userInit(void)
{
    //初始设备信息
    System.init();

    System.setDeviceInfo(PRODUCT_ID_DEF,PRODUCT_SECRET_DEF, HARDWARE_VERSION_DEF,SOFTWARE_VERSION_DEF);
    System.setEventCallback(eventProcess);
    /*************此处修改和添加用户初始化代码**************/
    Cloud.connect();

    timerID = timerGetId();
    /*******************************************************/
}

void userHandle(void)
{
    if(Cloud.connected()) {
        if(timerIsEnd(timerID, 10000)) {
            timerID = timerGetId();
            char *upstring = "hello! molmc/intorobot/intoyun!!!";

            //处理需要上送到云平台的数据
            Cloud.sendCustomData(upstring, strlen(upstring));
        }
    }
}

int userMain(void)
{
    userInit();
    while(1) {
        userHandle();
        System.loop();
    }
    return 0;
}

