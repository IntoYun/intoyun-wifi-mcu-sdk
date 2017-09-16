#ifndef _MAIN_H
#define _MAIN_H

#include "stm32f1xx_hal.h"
#include "init.h"
#include "usart.h"
#include "time.h"

#define PRODUCT_ID                       "fayGrbMBXoasr1e5"//产品ID
#define PRODUCT_SECRET                   "121922ec2a224563b4a587109a0dc3f2"//产品秘钥
#define HARDWARE_VERSION                 "V1.2.3"          //硬件版本号
#define SOFTWARE_VERSION                 "V1.1.1"          //软件版本号

#define LED_PIN           GPIO_PIN_0
#define LED_GPIO_PORT     GPIOB
#define LED_ON	 	        HAL_GPIO_WritePin(LED_GPIO_PORT,LED_PIN, GPIO_PIN_RESET)
#define LED_OFF		        HAL_GPIO_WritePin(LED_GPIO_PORT,LED_PIN, GPIO_PIN_SET)
#define LED_TOG		        HAL_GPIO_TogglePin(LED_GPIO_PORT,LED_PIN)


#endif
