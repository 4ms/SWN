/* 
 * UART Driver: Simple RX-only UART with interrupt
 *
 */
 
#include "drivers/uart_driver.h"

static UART_HandleTypeDef huart;

void UART_Start(uint8_t *pData, uint16_t Size)
{
	HAL_NVIC_EnableIRQ(UARTx_IRQn);
	HAL_UART_Receive_IT(&huart, pData, Size);
}

UART_HandleTypeDef* UART_Init(uint32_t baud_rate)
{
	HAL_NVIC_SetPriority(UARTx_IRQn, UART_RX_NVIC_PRI, UART_RX_NVIC_SUBPRI);

	GPIO_InitTypeDef gpio = {0};

	gpio.Pin = UART_RX_PIN;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpio.Alternate = UART_RX_PIN_AF; 
	HAL_GPIO_Init(UART_RX_GPIO, &gpio);

	__HAL_RCC_UART5_CLK_ENABLE();

	huart.Instance = UARTx;
	huart.Init.BaudRate = baud_rate;
	huart.Init.WordLength = UART_WORDLENGTH_8B;
	huart.Init.StopBits = UART_STOPBITS_1;
	huart.Init.Parity = UART_PARITY_NONE;
	huart.Init.Mode = UART_MODE_RX;
	huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart.Init.OverSampling = UART_OVERSAMPLING_16;
	huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

	HAL_StatusTypeDef err = HAL_UART_Init(&huart);
	if (err == HAL_OK)
		return &huart;
	else
		return (UART_HandleTypeDef *)0;
}

