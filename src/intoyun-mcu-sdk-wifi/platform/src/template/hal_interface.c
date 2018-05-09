#define "hal_interface.h"

void *HAL_MutexCreate(void)
{
    return NULL;
}

void HAL_MutexDestroy(void *mutex)
{
}

void HAL_MutexLock(void *mutex)
{
}

void HAL_MutexUnlock(void *mutex)
{
}

void *HAL_Malloc(uint32_t size)
{
    return NULL;
}

void HAL_Free(void *ptr)
{
}

void HAL_SystemInit(void)
{
}

void HAL_SystemReboot(void)
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

void HAL_Print(const char * output)
{
}

