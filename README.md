# WiFi从模式MCU SDK

## 项目介绍

![image](/docs/images/hardware-solution-slave.png)

从模式开发是用户直接使用自选的MCU作为主控，MCU连接各类传感器和控制器，MCU通过串口与模组进行通讯， MCU发送AT命令和模组进行数据交互来完成与云平台的通讯，模组实现了与平台之间的数据交互。用户将直接在MCU端开发设备软件。

用户无需关注设备与平台之间交互的实现，只需注重自己MCU的软件开发即可，极大的方便了用户开发物联网产品。

## 架构框图

第三方设备SDK采取分层设计，分为硬件HAL层、中间层、应用SDK层。通过分层，SDK可以很方便地移植到不同硬件平台。

![image](/docs/images/software-architecture-slave-mcu-sdk.png)

从模式SDK固件具备如下特点：

* 提供SDK接口，SDK接口实现了设备与平台之间的通讯，数据交互，以及MCU与模组之间的串口通讯等。
* 用户可自选MCU移植此SDK。
* SDK基于Keil-MDK 5.23及其以上版本建立，方便开发和移植。

## 目录和文件组成

```
+-- docs                : 项目文档
+-- examples            : 项目示例
+-- platform            : 硬件HAL层
    +-- inc             : 硬件HAL层头文件
    +-- src             : 硬件HAL层实现
        +-- stm32f103rbx: stm32f103rbx HAL层实现
        +-- template    : HAL层模板实现
+-- sdk                 : SDK代码实现
    +-- cloud           : 云端应用层实现
        +-- datapoint   : 数据点功能
    +-- comm-if         : 通讯协议中间层
    +-- comm            : 通讯层协议
    +-- config          : SDK配置文件
    +-- log             : LOG日志
    +-- sdk-impl        : SDK接口
    +-- system          : 系统接口层
    +-- utils           : 工具层

```

## 设备支持

|  硬件平台     |  HAL层实现                  |
| ------------- | --------------------------- |
| stm32f103rbx  | platform/src/stm32f103rbx   |

## 开发指南

- [SDK项目实例](https://github.com/IntoYun/intoyun-wifi-mcu-demo)
- [固件库API接口](http://docs.intoyun.com/devicedev/software-develop-slave/sdk-api/)

