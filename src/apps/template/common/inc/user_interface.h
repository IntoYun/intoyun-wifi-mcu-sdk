#ifndef USER_INTERFACE_H_
#define USER_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MODE_KEY_NUM     1
#define LED_TIMER_NUM    1

void userInterfaceInit(void);
void userInterfaceLoop(void);
void LedControl(bool level);

#ifdef __cplusplus
}
#endif

#endif

