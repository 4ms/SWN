#include "bl_hal_handlers.h"



void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

/*

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


void _Error_Handler(const char* file, uint32_t line)
{ 
	volatile char f[100];
	volatile uint32_t l=0;
	UNUSED(l);
	UNUSED(f);

	while (1)
	{
		f[0]=file[0];
		f[1]=file[1];
		f[2]=file[2];
		l=line;
	}
}

void NMI_Handler(void)
{ 
	while(1){};
}

void HardFault_Handler(void)
{ 
	volatile uint8_t foobar;
	// uint32_t hfsr,dfsr,afsr,bfar,mmfar,cfsr;

	volatile uint8_t pause=1;

	foobar=0;
	// mmfar=SCB->MMFAR;
	// bfar=SCB->BFAR;

	// hfsr=SCB->HFSR;
	// afsr=SCB->AFSR;
	// dfsr=SCB->DFSR;
	// cfsr=SCB->CFSR;


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
*/
