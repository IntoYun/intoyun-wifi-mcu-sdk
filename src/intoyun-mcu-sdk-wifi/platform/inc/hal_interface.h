#ifndef __HAL_INTERFACE_H__
#define __HAL_INTERFACE_H__

/**
 * @brief Create a mutex.
 *
 * @return NULL, initialize mutex failed; not NULL, the mutex handle.
 * @see None.
 * @note None.
 */
void *HAL_MutexCreate(void);

/**
 * @brief Destroy the specified mutex object, it will release related resource.
 *
 * @param [in] mutex @n The specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexDestroy(void *mutex);

/**
 * @brief Waits until the specified mutex is in the signaled state.
 *
 * @param [in] mutex @n the specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexLock(void *mutex);

/**
 * @brief Releases ownership of the specified mutex object..
 *
 * @param [in] mutex @n the specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexUnlock(void *mutex);

/**
 * @brief 申请指定大小的动态内存，返回动态内存的起始地址
 *
 * @param [in] size: @n 动态内存大小，单位字节
 * @return 动态内存的起始地址
 * @see 无
 * @note 动态内存大小可指定
 */
void *HAL_Malloc(uint32_t size);

/**
 * @brief 释放动态内存
 *
 * @param[in] ptr: @n 通过HAL_Malloc()申请的动态内存的起始地址
 * @return 无
 * @see 无
 * @note 无
 */
void HAL_Free(void *ptr);

/**
 * @brief 系统初始化，主要用于HAL涉及功能初始化
 *
 * @return 无
 * @see 无
 * @note 无
 */
void HAL_SystemInit(void);

/**
 * @brief 设备重启
 *
 * @return 无
 * @see 无
 * @note 无
 */
void HAL_SystemReboot(void);

/**
 * @brief 设备上电运行时间，单位：msJ
 *
 * @param 无
 * @return 上电运行时间
 * @see 无
 * @note 无
 */
uint32_t HAL_UptimeMs(void);

/**
 * @brief 往从通讯模块发送数据
 *
 * @param[in] c: @n 待发送的字节数据
 * @see 无
 * @note 无
 */
void HAL_CommWrite(uint8_t c);

/**
 * @brief 往调试口发送调试信息
 *
 * @param[in] output: @n 待发送字符串的首地址
 * @see 无
 * @note 无
 */
void HAL_Print(const char * output);

#endif

