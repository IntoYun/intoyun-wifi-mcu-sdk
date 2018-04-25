#ifndef USER_INTERFACE_H_
#define USER_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MODE_KEY_NUM     1
#define LED_TIMER_NUM    1

#define LED_PIN             GPIO_PIN_0
#define LED_GPIO_PORT       GPIOB
#define LED_ON                HAL_GPIO_WritePin(LED_GPIO_PORT,LED_PIN, GPIO_PIN_RESET)
#define LED_OFF               HAL_GPIO_WritePin(LED_GPIO_PORT,LED_PIN, GPIO_PIN_SET)

#define KEY_PIN             GPIO_PIN_7
#define KEY_GPIO_PORT       GPIOA


void userInterfaceInit(void);
void userInterfaceLoop(void);
void LedControl(bool level);

#ifdef __cplusplus
}
#endif

#endif

