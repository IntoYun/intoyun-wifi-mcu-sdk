#include "iot_export.h"
#include "user_interface.h"
#include "stm32f1xx_hal.h"

static bool ledFlag = false;

void LedBlink(void)
{
    ledFlag = !ledFlag;
    if(ledFlag) {
        LED_ON;
    } else {
        LED_OFF;
    }
}

void LedControl(bool level)
{
    if(level) {
        LED_ON;
    } else {
        LED_OFF;
    }
}

void LedPinInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    __GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    LED_OFF;
}

void KeyGpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    __GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = KEY_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(KEY_GPIO_PORT, &GPIO_InitStruct);
}

uint16_t KeyGetValue(void)
{
    return HAL_GPIO_ReadPin(KEY_GPIO_PORT, KEY_PIN);
}

//短按键执行
void KeyClickHandle(void)
{
    MOLMC_LOGV(TAG, "key is click\r\n");
}

//长按键执行 可根据按键时间来执行不同的操作
void KeyLongPressHandle(uint32_t ms)
{
    if(ms >= 9000) //按键时间大于9s
    {
        MOLMC_LOGV(TAG, "key is long press 9s\r\n");
    }
    else if(ms >= 6000)
    {
        MOLMC_LOGV(TAG, "key is long press 6s\r\n");
    }
    else if(ms >= 3000)
    {
        MOLMC_LOGV(TAG, "key is long press 3s\r\n");
        //模组进入配置模式
        System.setModuleMode(MODE_IMLINK_CONFIG, 0);
    }
}

void userInterfaceInit(void)
{
    LedPinInit();
    //按键注册
    Key.keyRegister(MODE_KEY_NUM, KeyGpioInit, KeyGetValue);
    Key.attachClick(MODE_KEY_NUM, KeyClickHandle);
    Key.attachDuringLongPress(MODE_KEY_NUM, KeyLongPressHandle);

    //启用一个定时器 周期500ms
    Timer.timerRegister(LED_TIMER_NUM, 500, false, LedBlink);
}

void userInterfaceLoop(void)
{
    //该函数处理时间尽量短, 不允许阻塞
    Key.loop();
    Timer.loop();
}

