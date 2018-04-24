#ifndef _HAL_INTERFACE_H
#define _HAL_INTERFACE_H

/** @defgroup group_platform_memory_manage memory
 *  @{
 */

/**
 * @brief Allocates a block of size bytes of memory, returning a pointer to the beginning of the block.
 *
 * @param [in] size @n specify block size in bytes.
 * @return A pointer to the beginning of the block.
 * @see None.
 * @note Block value is indeterminate.
 */
void *HAL_Malloc(uint32_t size);

/**
 * @brief Deallocate memory block
 *
 * @param[in] ptr @n Pointer to a memory block previously allocated with platform_malloc.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_Free(void *ptr);

/**
 * @brief Retrieves the number of milliseconds that have elapsed since the system was boot.
 *
 * @param None.
 * @return the number of milliseconds.
 * @see None.
 * @note None.
 */
uint32_t HAL_UptimeMs(void);


uint32_t HAL_Millis(void);
void HAL_UartWrite(uint8_t c);
void HAL_Print(char *data, uint16_t len);

#endif

