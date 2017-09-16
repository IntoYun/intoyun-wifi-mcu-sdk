
#include "usart.h"
#include "intoyun_protocol.h"

UART_HandleTypeDef huart1; //USART1用于与模组通讯
UART_HandleTypeDef huart2; //USART2 用于调试


void USART1_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE; //无奇偶校验位
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);
}

void USART2_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE; //无奇偶校验位
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart2);
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if(huart->Instance == USART1)
    {
        /* Peripheral clock enable */
        __USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART1 GPIO Configuration
           PA9     ------> USART1_TX
           PA10     ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* Peripheral interrupt init*/
        HAL_NVIC_SetPriority(USART1_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
        __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
    }
    else if(huart->Instance == USART2)
    {
        /* Peripheral clock enable */
        __USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART2 GPIO Configuration
           PA2     ------> USART2_TX
           PA3     ------> USART2_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
    if(huart->Instance == USART1)
    {
        /* Peripheral clock disable */
        __USART1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);
        /* Peripheral interrupt Deinit*/
        HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
    else if(huart->Instance == USART2)
    {
        __USART2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);
        /* Peripheral interrupt Deinit*/
        /* HAL_NVIC_DisableIRQ(USART2_IRQn); */
    }
}

//printf 重定向
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else

#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

#endif
PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, 100);
    return ch;
}

//USART1 发送数据
void UART_Transmit(uint8_t c)
{
    uint8_t data = c;
    HAL_UART_Transmit(&huart1, &data, 1, 100);
}

//USART1 接收数据,在中断里面调用
static void UART_Receive(void)
{
    uint8_t c;
    HAL_UART_Receive(&huart1, &c, 1, 100);
    SerialPipeRxIrqBuf(c);
}

void USART1_IRQHandler(void)
{
    UART_Receive();
}

