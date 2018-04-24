#include "intoyun_interface.h"
#include "user_interface.h"

static bool ledFlag = false;
static bool onceSet = false;

void LedBlink(void)
{
}

void LedControl(bool level)
{
}

void LedPinInit(void)
{
}

void KeyGpioInit(void)
{
}

uint16_t KeyGetValue(void)
{
    return 0;
}

//短按键执行
void KeyClickHandle(void)
{
    log_v("key is click\r\n");
}

//长按键执行 可根据按键时间来执行不同的操作
void KeyLongPressHandle(uint32_t ms)
{
    if(ms >= 9000) //按键时间大于9s
    {
        log_v("key is long press 9s\r\n");
    }
    else if(ms >= 6000)
    {
        log_v("key is long press 6s\r\n");
    }
    else if(ms >= 3000)
    {
        if(!onceSet)
        {
            onceSet = true;
            log_v("key is long press 3s\r\n");
            //模组进入配置模式
            Timer.start(LED_TIMER_NUM);
            System.setModuleMode(MODE_IMLINK_CONFIG, 0);
        }
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

