#define "hal_interface.h"

void HAL_SystemInit(void)
{
}

//获取系统滴答定时器计数值 单位ms
uint32_t HAL_Millis(void)
{
    return 0;
}

//串口发送数据到模组
void HAL_UartWrite(uint8_t c)
{
}

void HAL_Print(char *data, uint16_t len)
{
}

