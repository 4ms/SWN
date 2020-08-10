
#include "bl_utils.h"
#include "hal_handlers.h"
#include "flash.h"

extern const uint32_t FLASH_SECTOR_ADDRESSES[];

void *memcpy(void *dest, const void *src, size_t n)
{
    char *dp = (char *)dest;
    const char *sp = (const char *)src;
    while (n--)
        *dp++ = *sp++;
    return dest;
}

void copy_flash_page(uint32_t src_addr, uint32_t dst_addr, uint32_t bytes_to_copy)
{
	uint32_t i;

	flash_begin_open_program();

	for (i=0; i<bytes_to_copy; i+=4) {

		//Erase sectors each time we start a new one
		//If dst_addr is not a sector start, then nothing happens
		flash_open_erase_sector(dst_addr);


		//Program the word
		flash_open_program_word(*(uint32_t*)src_addr, dst_addr);

		src_addr += 4;
		dst_addr += 4;
	}

	flash_end_open_program();
}


void write_flash_page(const uint8_t* data, uint32_t dst_addr, uint32_t bytes_to_write)
{

	flash_begin_open_program();

	//Erase sector if dst_addr is a sector start
	flash_open_erase_sector(dst_addr);


	flash_open_program_block_words((uint32_t *)data, dst_addr, bytes_to_write/4);

	flash_end_open_program();

}



void SetVectorTable(uint32_t reset_address)
{ 
	SCB->VTOR = reset_address & (uint32_t)0x1FFFFF80;
}

__attribute__((naked)) void branch_to_bootloader(uint32_t r0, uint32_t addr) {
	__asm volatile (
		"ldr r2, [r1, #0]\n"    // get address of stack pointer
		"msr msp, r2\n"         // get stack pointer
		"ldr r2, [r1, #4]\n"    // get address of destination
		"bx r2\n"               // branch to bootloader
	);
}

void JumpTo(uint32_t address) {
	branch_to_bootloader(0, address);
}

// typedef void (*EntryPoint)(void);
// void JumpTo(uint32_t address) 
// {
// 	uint32_t application_address = *(__IO uint32_t*)(address + 4);
// 	EntryPoint application = (EntryPoint)(application_address);
// 	__set_MSP(*(__IO uint32_t*)address);
// 	application();
// }


void SystemClock_Config(void)
{

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

	//Configure the main internal regulator output voltage 
	__HAL_RCC_PWR_CLK_ENABLE();

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	//Initializes the CPU, AHB and APB busses clocks 
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 432;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		_Error_Handler(__FILE__, __LINE__);

	//Activate the OverDrive to reach the 216 MHz Frequency 
	if (HAL_PWREx_EnableOverDrive() != HAL_OK)
		_Error_Handler(__FILE__, __LINE__);

	//Initializes the CPU, AHB and APB busses clocks 
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
		_Error_Handler(__FILE__, __LINE__);


	//Note: Do not start the SAI clock (I2S) at this time. 
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C2 | RCC_PERIPHCLK_I2C1;;

	PeriphClkInitStruct.PLLI2S.PLLI2SP 	= RCC_PLLP_DIV2;
	PeriphClkInitStruct.PLLI2S.PLLI2SR	= 2;

	PeriphClkInitStruct.I2c2ClockSelection 		= RCC_I2C2CLKSOURCE_PCLK1;
	PeriphClkInitStruct.I2c1ClockSelection 		= RCC_I2C1CLKSOURCE_PCLK1; //54MHz

	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	//Enables the Clock Security System 
	HAL_RCC_EnableCSS();

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	//Configure the Systick 
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	// SysTick_IRQn interrupt configuration 
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


