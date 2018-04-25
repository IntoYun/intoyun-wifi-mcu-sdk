#include "stm32f1xx_hal.h"
#include "iot_export.h"

extern int userMain(void);

int main(void)
{
    userMain();
    return 0;
}
