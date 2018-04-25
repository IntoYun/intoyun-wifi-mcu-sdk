#define "hal_interface.h"

void HAL_SystemInit(void)
{
}

void *HAL_Malloc(uint32_t size)
{
}

void HAL_Free(void *ptr)
{
}

uint32_t HAL_UptimeMs(void)
{
    return 0;
}

//串口发送数据到模组
void HAL_CommWrite(uint8_t c)
{
}

void HAL_Print(char *data, uint16_t len)
{
}

