#pragma once
#include <stm32f7xx.h>

UART_HandleTypeDef* UART_Init(uint32_t baud_rate);
void UART_Start(uint8_t *pData, uint16_t Size);

#define UARTx UART5
#define UARTx_IRQn UART5_IRQn
#define UART_RX_GPIO GPIOD
#define UART_RX_PIN GPIO_PIN_2
#define UART_RX_PIN_AF GPIO_AF8_UART5
#define UART_RX_NVIC_PRI 3
#define UART_RX_NVIC_SUBPRI 1
