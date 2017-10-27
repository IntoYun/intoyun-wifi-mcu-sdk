#ifndef _HAL_INTERFACE_H
#define _HAL_INTERFACE_H

void HAL_SystemInit(void);
uint32_t HAL_Millis(void);
void HAL_UartWrite(uint8_t c);
void HAL_Print(char *data, uint16_t len);

#endif
