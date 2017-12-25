/**
 ******************************************************************************
 Copyright (c) IntoRobot Team.  All right reserved.

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

#ifndef PROJECT_CONFIG_H_
#define PROJECT_CONFIG_H_

//产品ID
//产品ID是一个16个字节的字符串，为一个产品的标识符。设备通过上送产品ID可以把设备归属到该类产品中。
//产品ID属于绝密信息，请防止泄露。
#define PRODUCT_ID_DEF                          "qu4HnEqfVbXAa22b"

//产品密钥
//产品密钥是一个32个字节的字符串，通过产品密钥可以完成设备的自动注册。产品密钥在任何时候不传输。
//产品密钥属于绝密信息，请防止泄露。
#define PRODUCT_SECRET_DEF                      "826565d54bee6ef21cc09e02fcc709f0"

//硬件版本号
//硬件版本号，为设备的硬件版本号。该版本号将上送至服务器。
#define HARDWARE_VERSION_DEF                    "V1.0.0"

//软件版本号
//软件版本号，为设备当前软件的版本号。该版本号将上送至服务器。
#define SOFTWARE_VERSION_DEF                    "V1.0.0"

//设备数据上报策略是否为系统自动上报
//设备数据上报支持两种方式：
//true  : 系统自动处理上报，当数据点数值发生变化 或者 上报间隔超过600秒时，上报设备数据。
//false : 用户手动处理上报，数据点上报由用户调用相关函数处理。
#define DEV_REPORT_POLICY_AUTO                  true

#endif

