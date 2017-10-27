# WiFi从模式MCU SDK
说明：在应用此SDK前，请先阅读[IntoYun文档中心](http://docs.intoyun.com/guide/getting-started/start/)。

## 目录说明
```
|-- docs
|-- keil
|-- README.md
 -- src
```
- 1.docs为文档类
- 2.keil为示例工程(Keil-MDK平台)
- 3.src为源码

### keil目录说明
```
`-- stm32f103rbx
    `-- smartLight
```
stm32f103rbx为支持的MCU型号，smartLight为示例工程名。

### src目录说明
```
|-- apps
|-- hal
|-- intoyun
`-- mcu
```
- apps为示例应用代码
- hal为硬件外设调用接口
- intoyun为平台功能实现和AT指令解析等
- mcu为驱动库

### intoyun文件说明
```
|-- inc
|   |-- intoyun_config.h
|   |-- intoyun_datapoint.h
|   |-- intoyun_interface.h
|   |-- intoyun_log.h
|   |-- intoyun_md5.h
|   `-- intoyun_protocol.h
`-- src
    |-- intoyun_datapoint.c
    |-- intoyun_interface.c
    |-- intoyun_log.c
    |-- intoyun_md5.c
    `-- intoyun_protocol.c
```
#### intoyun_config
配置文件，里面有以下几个设置：
- SDK_VERSION sdk版本号。
- INTOYUN_DEBUG_LEVEL 调试信息输出等级：
 0：关闭调试信息输出
 1：错误信息
 2：警告信息
 3：通告信息
 4：调试信息
 5：详细信息

- PROPERTIES_MAX 数据点的个数大小设置，默认为50，也就是说可以添加的数据点最大个数为50。
- PIPE_MAX_SIZE 串口数据接收缓冲区大小设置，单位为byte，默认为256，可根据实际MCU内存大小或者需要修改。
- DATAPOINT_TRANSMIT_AUTOMATIC_INTERVAL 数据点上送到平台的时间间隔，单位秒，默认是20，也就是说如果数据点的数据为自动上送模式则2次相邻的上送间隔至少要20秒。

#### intoyun_interface
用户函数接口

#### intoyun_datapoint
数据点处理

#### intoyun_protocol
AT指令解析处理

#### intoyun_md5
md5码生成，用户无需使用。


## 开发使用

### 产品信息
写入产品ID和秘钥，有关产品的信息请点击[这里](http://docs.intoyun.com/guide/getting-started/start/)。<br>
在intoyun_config.h的中在填入以下信息：<br>
```cpp
#define PRODUCT_ID                       "y4NFFyDE9uq6H202"//产品ID
#define PRODUCT_SECRET                   "ab697b0dc1716d24cfc49b071668e766"//产品秘钥
#define HARDWARE_VERSION                 "V1.0.0"          //硬件版本号
#define SOFTWARE_VERSION                 "V1.0.0"          //软件版本号
```

### 流程说明
在userInit()里面定义好数据点，如果是透传模式则无需定义数据点。

调用System.setDeviceInfo()来发送产品信息

调用System.setEventCallback()来定义事件回调处理，当MCU收到模组主动下发的信息或者数据时会通过此回调来及时运行。

查看[函数接口说明](http://docs.intoyun.com/devicedev/software-develop/slave-sdk-api/w67/)。

### 事件处理
在eventProcess()里面事件状态和数据点数据。

具体实际应用自行查看示例应用。

## 移植说明
对于将此SDK移植到其他MCU平台只需移植intoyun文件夹下的文件和hal文件即可。

### hal接口实现
MCU与模组是通过串口进行通讯的，其波特率是115200，8位数据位，1位停止位。故在hal里面需实现以下几个函数接口：

- HAL__SystemInit() 此函数接口为MCU的系统时钟、串口初始化等。
- HAL_Millis() 此函数接口为系统时钟计数器，在stm32中就是系统滴答定时器，计数器变化一个数值，代表1ms。
- HAL_UartWrite() 此函数接口为串口发送数据
- HAL_Print() 此函数接口为串口打印调试信息，如果MCU只有一个串口或者不需要串口打印则无需实现，但是函数不能在文件中删除。

以上4个接口用户移植时需实现，且不能修改其名字。

MCU接收模组串口发来的数据时需调用System.putPipe()来将数据放入缓冲区，以便intoyun协议层来解析收到的数据。

### intoyun移植
此文件夹下的文件可直接移植，无需修改内容。



