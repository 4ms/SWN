#include "hal_handlers.h"

void HAL_MspInit(void)
{
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);

  HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
  HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
  HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
  HAL_NVIC_SetPriority(SVCall_IRQn, 0, 0);
  HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);
  HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

//#define USE_FULL_ASSERT
#ifdef  USE_FULL_ASSERT
	#define assert_param(expr) ((expr) ? (void)0 : _Error_Handler((char *)__FILE__, __LINE__))

	/**
	  * @brief  Reports the name of the source file and the source line number
	  *   where the assert_param error has occurred.
	  * @param  file: pointer to the source file name
	  * @param  line: assert_param error line source number
	  * @retval None
	  */
#endif

void _Error_Handler(const char* file, uint32_t line)
{ 
	volatile char f[100];
	volatile uint32_t l;

	while (1)
	{
		f[0]=file[0];
		f[1]=file[1];
		f[2]=file[2];
		l=line;
		UNUSED(f);
		UNUSED(l);
	}

}


/* exception handlers - so we know what's failing */
void NMI_Handler(void)
{ 
	while(1){};
}

void HardFault_Handler(void)
{ 
	volatile uint8_t foobar;
	uint32_t hfsr,dfsr,afsr,bfar,mmfar,cfsr;

	volatile uint8_t pause=1;

	foobar=0;
	mmfar=SCB->MMFAR;
	bfar=SCB->BFAR;

	hfsr=SCB->HFSR;
	afsr=SCB->AFSR;
	dfsr=SCB->DFSR;
	cfsr=SCB->CFSR;

	UNUSED(hfsr);
	UNUSED(afsr);
	UNUSED(dfsr);
	UNUSED(cfsr);
	UNUSED(mmfar);
	UNUSED(bfar);
	
	if (foobar){
		return;
	} else {
		while(pause){};
	}
}

void MemManage_Handler(void)
{ 
	while(1){};
}

void BusFault_Handler(void)
{ 
	while(1){};
}

void UsageFault_Handler(void)
{ 
	while(1){};
}

void SVC_Handler(void)
{ 
	while(1){};
}

void DebugMon_Handler(void)
{ 
	while(1){};
}

void PendSV_Handler(void)
{ 
	while(1){};
}
