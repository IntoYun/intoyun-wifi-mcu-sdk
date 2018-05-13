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

#ifndef __SDK_CONFIG_H__
#define __SDK_CONFIG_H__

#define SDK_VERSION                        "1.2.0"

#define CONFIG_CLOUD_DATAPOINT_ENABLED     1   //是否使能数据点通讯接口


// 数据点相关配置
#define CONFIG_PROPERTIES_MAX              50  //支持的数据点的最大个数
#define CONFIG_AUTOMATIC_INTERVAL          600 //数据点自动发送默认时间间隔

#define CONFIG_PIPE_MAX_SIZE               256 //串口接收缓冲区大小
#define CONFIG_SYSTEM_KEY_ENABLE           1   //是否按键接口功能
#define CONFIG_SYSTEM_TIMER_ENABLE         1   //是否定时器接口功能

//MOLMC_LOG_NONE,       /*!< No log output */
//MOLMC_LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
//MOLMC_LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
//MOLMC_LOG_INFO,       /*!< Information messages which describe normal flow of events */
//MOLMC_LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
//MOLMC_LOG_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
#define CONFIG_LOG_DEFAULT_LEVEL           MOLMC_LOG_VERBOSE

#endif
