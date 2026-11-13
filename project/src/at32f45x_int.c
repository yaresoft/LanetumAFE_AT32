/* add user code begin Header */
/**
  **************************************************************************
  * @file     at32f45x_int.c
  * @brief    main interrupt service routines.
  **************************************************************************
  *
  * Copyright (c) 2025, Artery Technology, All rights reserved.
  * SPDX-License-Identifier: BSD-3-Clause
  *
  **************************************************************************
  */
/* add user code end Header */

/* includes ------------------------------------------------------------------*/
#include "at32f45x_int.h"
#include "wk_system.h"
#include "freertos_app.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */

/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */

/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */

/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */

/* add user code end 0 */

/* external variables ---------------------------------------------------------*/
/* add user code begin external variables */

/* add user code end external variables */

/**
  * @brief  this function handles nmi exception.
  * @param  none
  * @retval none
  */
void NMI_Handler(void)
{
  /* add user code begin NonMaskableInt_IRQ 0 */

  /* add user code end NonMaskableInt_IRQ 0 */

  /* add user code begin NonMaskableInt_IRQ 1 */

  /* add user code end NonMaskableInt_IRQ 1 */
}

/**
  * @brief  this function handles hard fault exception.
  * @param  none
  * @retval none
  */
void HardFault_Handler(void)
{
  /* add user code begin HardFault_IRQ 0 */

  /* add user code end HardFault_IRQ 0 */
  /* go to infinite loop when hard fault exception occurs */
  while (1)
  {
    /* add user code begin W1_HardFault_IRQ 0 */

    /* add user code end W1_HardFault_IRQ 0 */
  }
}


/**
  * @brief  this function handles memory manage exception.
  * @param  none
  * @retval none
  */
void MemManage_Handler(void)
{
  /* add user code begin MemoryManagement_IRQ 0 */

  /* add user code end MemoryManagement_IRQ 0 */
  /* go to infinite loop when memory manage exception occurs */
  while (1)
  {
    /* add user code begin W1_MemoryManagement_IRQ 0 */

    /* add user code end W1_MemoryManagement_IRQ 0 */
  }
}

/**
  * @brief  this function handles bus fault exception.
  * @param  none
  * @retval none
  */
void BusFault_Handler(void)
{
  /* add user code begin BusFault_IRQ 0 */

  /* add user code end BusFault_IRQ 0 */
  /* go to infinite loop when bus fault exception occurs */
  while (1)
  {
    /* add user code begin W1_BusFault_IRQ 0 */

    /* add user code end W1_BusFault_IRQ 0 */
  }
}

/**
  * @brief  this function handles usage fault exception.
  * @param  none
  * @retval none
  */
void UsageFault_Handler(void)
{
  /* add user code begin UsageFault_IRQ 0 */

  /* add user code end UsageFault_IRQ 0 */
  /* go to infinite loop when usage fault exception occurs */
  while (1)
  {
    /* add user code begin W1_UsageFault_IRQ 0 */

    /* add user code end W1_UsageFault_IRQ 0 */
  }
}

/**
  * @brief  this function handles debug monitor exception.
  * @param  none
  * @retval none
  */
void DebugMon_Handler(void)
{
  /* add user code begin DebugMonitor_IRQ 0 */

  /* add user code end DebugMonitor_IRQ 0 */
  /* add user code begin DebugMonitor_IRQ 1 */

  /* add user code end DebugMonitor_IRQ 1 */
}

extern void xPortSysTickHandler(void);

/**
  * @brief  this function handles systick handler.
  * @param  none
  * @retval none
  */
void SysTick_Handler(void)
{
  /* add user code begin SysTick_IRQ 0 */

  /* add user code end SysTick_IRQ 0 */

#if (INCLUDE_xTaskGetSchedulerState == 1 )
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
#endif /* INCLUDE_xTaskGetSchedulerState */
  xPortSysTickHandler();
#if (INCLUDE_xTaskGetSchedulerState == 1 )
  }
#endif /* INCLUDE_xTaskGetSchedulerState */

  /* add user code begin SysTick_IRQ 1 */

  /* add user code end SysTick_IRQ 1 */
}

/**
  * @brief  this function handles EXINT Line 2 handler.
  * @param  none
  * @retval none
  */
void EXINT2_IRQHandler(void)
{
  /* add user code begin EXINT2_IRQ 0 */

  /* add user code end EXINT2_IRQ 0 */

  if(exint_interrupt_flag_get(EXINT_LINE_2) != RESET)
  {
    /* add user code begin EXINT_LINE_2 */
	  SubFrameIndex=0;
    /* clear flag */
    exint_flag_clear(EXINT_LINE_2);
    /* add user code end EXINT_LINE_2 */ 
  }

  /* add user code begin EXINT2_IRQ 1 */

  /* add user code end EXINT2_IRQ 1 */
}

/**
  * @brief  this function handles EXINT Line 4 handler.
  * @param  none
  * @retval none
  */
void EXINT4_IRQHandler(void)
{
  /* add user code begin EXINT4_IRQ 0 */

  /* add user code end EXINT4_IRQ 0 */

  if(exint_interrupt_flag_get(EXINT_LINE_4) != RESET)
  {
    /* add user code begin EXINT_LINE_4 */
    /* clear flag */
    exint_flag_clear(EXINT_LINE_4);
    /* add user code end EXINT_LINE_4 */ 
  }

  /* add user code begin EXINT4_IRQ 1 */

  /* add user code end EXINT4_IRQ 1 */
}

/**
  * @brief  this function handles DMA1 Channel 1 handler.
  * @param  none
  * @retval none
  */
void DMA1_Channel1_IRQHandler(void)
{
  /* add user code begin DMA1_Channel1_IRQ 0 */

  /* add user code end DMA1_Channel1_IRQ 0 */

  if(dma_interrupt_flag_get(DMA1_FDT1_FLAG) != RESET)
  {   
    /* add user code begin DMA1_FDT1_FLAG */
    /* handle full data transfer and clear flag */
    dma_flag_clear(DMA1_FDT1_FLAG);
    /* add user code end DMA1_FDT1_FLAG */ 
  }

  if(dma_interrupt_flag_get(DMA1_HDT1_FLAG) != RESET)
  {   
    /* add user code begin DMA1_HDT1_FLAG */
    /* handle half data transfer and clear flag */
    dma_flag_clear(DMA1_HDT1_FLAG);
    /* add user code end DMA1_HDT1_FLAG */ 
  }

  if(dma_interrupt_flag_get(DMA1_DTERR1_FLAG) != RESET)
  {   
    /* add user code begin DMA1_DTERR1_FLAG */
    /* handle error transfer and clear flag */
    dma_flag_clear(DMA1_DTERR1_FLAG);
    /* add user code end DMA1_DTERR1_FLAG */ 
  }

  /* add user code begin DMA1_Channel1_IRQ 1 */

  /* add user code end DMA1_Channel1_IRQ 1 */
}

/**
  * @brief  this function handles DMA1 Channel 2 handler.
  * @param  none
  * @retval none
  */
void DMA1_Channel2_IRQHandler(void)
{
  /* add user code begin DMA1_Channel2_IRQ 0 */

  /* add user code end DMA1_Channel2_IRQ 0 */

  if(dma_interrupt_flag_get(DMA1_FDT2_FLAG) != RESET)
  {   
    /* add user code begin DMA1_FDT2_FLAG */
    /* handle full data transfer and clear flag */
    dma_flag_clear(DMA1_FDT2_FLAG);
    /* add user code end DMA1_FDT2_FLAG */ 
  }

  if(dma_interrupt_flag_get(DMA1_HDT2_FLAG) != RESET)
  {   
    /* add user code begin DMA1_HDT2_FLAG */
    /* handle half data transfer and clear flag */
    dma_flag_clear(DMA1_HDT2_FLAG);
    /* add user code end DMA1_HDT2_FLAG */ 
  }

  if(dma_interrupt_flag_get(DMA1_DTERR2_FLAG) != RESET)
  {   
    /* add user code begin DMA1_DTERR2_FLAG */
    /* handle error transfer and clear flag */
    dma_flag_clear(DMA1_DTERR2_FLAG);
    /* add user code end DMA1_DTERR2_FLAG */ 
  }

  /* add user code begin DMA1_Channel2_IRQ 1 */

  /* add user code end DMA1_Channel2_IRQ 1 */
}

/**
  * @brief  this function handles DMA1 Channel 3 handler.
  * @param  none
  * @retval none
  */
void DMA1_Channel3_IRQHandler(void)
{
  /* add user code begin DMA1_Channel3_IRQ 0 */

  /* add user code end DMA1_Channel3_IRQ 0 */

  if(dma_interrupt_flag_get(DMA1_FDT3_FLAG) != RESET)
  {   
    /* add user code begin DMA1_FDT3_FLAG */
    /* handle full data transfer and clear flag */
    dma_flag_clear(DMA1_FDT3_FLAG);
    if(CommEvent)
    {
    	BaseType_t xHigherPriorityTaskWoken, xResult;

    		/* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
    	xHigherPriorityTaskWoken = pdFALSE;

    	xResult = xEventGroupSetBitsFromISR(CommEvent, EV_SPI_TX_DONE, &xHigherPriorityTaskWoken );

    	/* Was the message posted successfully? */
    	if( xResult != pdFAIL )
    	{
    		/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
    		   switch should be requested. The macro used is port specific and will
    		   be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
    		   the documentation page for the port being used. */
    		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    	}
    }

    /* add user code end DMA1_FDT3_FLAG */ 
  }

  if(dma_interrupt_flag_get(DMA1_DTERR3_FLAG) != RESET)
  {   
    /* add user code begin DMA1_DTERR3_FLAG */
    /* handle error transfer and clear flag */
    dma_flag_clear(DMA1_DTERR3_FLAG);
    /* add user code end DMA1_DTERR3_FLAG */ 
  }

  /* add user code begin DMA1_Channel3_IRQ 1 */

  /* add user code end DMA1_Channel3_IRQ 1 */
}

/**
  * @brief  this function handles DMA1 Channel 4 handler.
  * @param  none
  * @retval none
  */
void DMA1_Channel4_IRQHandler(void)
{
  /* add user code begin DMA1_Channel4_IRQ 0 */

  /* add user code end DMA1_Channel4_IRQ 0 */

  if(dma_interrupt_flag_get(DMA1_FDT4_FLAG) != RESET)
  {   
    /* add user code begin DMA1_FDT4_FLAG */
    /* handle full data transfer and clear flag */
    dma_flag_clear(DMA1_FDT4_FLAG);
    if(CommEvent)
    {
		BaseType_t xHigherPriorityTaskWoken, xResult;

			/* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
		xHigherPriorityTaskWoken = pdFALSE;

		xResult = xEventGroupSetBitsFromISR(CommEvent, EV_SPI_RX_DONE, &xHigherPriorityTaskWoken );

		/* Was the message posted successfully? */
		if( xResult != pdFAIL )
		{
			/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
			   switch should be requested. The macro used is port specific and will
			   be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
			   the documentation page for the port being used. */
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		}
    }
    /* add user code end DMA1_FDT4_FLAG */ 
  }

  if(dma_interrupt_flag_get(DMA1_DTERR4_FLAG) != RESET)
  {   
    /* add user code begin DMA1_DTERR4_FLAG */
    /* handle error transfer and clear flag */
    dma_flag_clear(DMA1_DTERR4_FLAG);
    /* add user code end DMA1_DTERR4_FLAG */ 
  }

  /* add user code begin DMA1_Channel4_IRQ 1 */

  /* add user code end DMA1_Channel4_IRQ 1 */
}

/**
  * @brief  this function handles DMA1 Channel 5 handler.
  * @param  none
  * @retval none
  */
void DMA1_Channel5_IRQHandler(void)
{
  /* add user code begin DMA1_Channel5_IRQ 0 */

  /* add user code end DMA1_Channel5_IRQ 0 */

  if(dma_interrupt_flag_get(DMA1_FDT5_FLAG) != RESET)
  {   
    /* add user code begin DMA1_FDT5_FLAG */
    /* handle full data transfer and clear flag */
    dma_flag_clear(DMA1_FDT5_FLAG);
    /* add user code end DMA1_FDT5_FLAG */ 
  }

  if(dma_interrupt_flag_get(DMA1_HDT5_FLAG) != RESET)
  {   
    /* add user code begin DMA1_HDT5_FLAG */
    /* handle half data transfer and clear flag */
    dma_flag_clear(DMA1_HDT5_FLAG);
    /* add user code end DMA1_HDT5_FLAG */ 
  }

  if(dma_interrupt_flag_get(DMA1_DTERR5_FLAG) != RESET)
  {   
    /* add user code begin DMA1_DTERR5_FLAG */
    /* handle error transfer and clear flag */
    dma_flag_clear(DMA1_DTERR5_FLAG);
    /* add user code end DMA1_DTERR5_FLAG */ 
  }

  /* add user code begin DMA1_Channel5_IRQ 1 */

  /* add user code end DMA1_Channel5_IRQ 1 */
}

/**
  * @brief  this function handles DMA1 Channel 6 handler.
  * @param  none
  * @retval none
  */
void DMA1_Channel6_IRQHandler(void)
{
  /* add user code begin DMA1_Channel6_IRQ 0 */

  /* add user code end DMA1_Channel6_IRQ 0 */

  if(dma_interrupt_flag_get(DMA1_FDT6_FLAG) != RESET)
  {   
    /* add user code begin DMA1_FDT6_FLAG */
    /* handle full data transfer and clear flag */
    dma_flag_clear(DMA1_FDT6_FLAG);
    /* add user code end DMA1_FDT6_FLAG */ 
  }

  if(dma_interrupt_flag_get(DMA1_HDT6_FLAG) != RESET)
  {   
    /* add user code begin DMA1_HDT6_FLAG */
    /* handle half data transfer and clear flag */
    dma_flag_clear(DMA1_HDT6_FLAG);
    /* add user code end DMA1_HDT6_FLAG */ 
  }

  if(dma_interrupt_flag_get(DMA1_DTERR6_FLAG) != RESET)
  {   
    /* add user code begin DMA1_DTERR6_FLAG */
    /* handle error transfer and clear flag */
    dma_flag_clear(DMA1_DTERR6_FLAG);
    /* add user code end DMA1_DTERR6_FLAG */ 
  }

  /* add user code begin DMA1_Channel6_IRQ 1 */

  /* add user code end DMA1_Channel6_IRQ 1 */
}

/**
  * @brief  this function handles ADC1 2 handler.
  * @param  none
  * @retval none
  */
void ADC1_2_IRQHandler(void)
{
  /* add user code begin ADC1_2_IRQ 0 */

  /* add user code end ADC1_2_IRQ 0 */

  /* add user code begin ADC1_2_IRQ 1 */

  /* add user code end ADC1_2_IRQ 1 */
}

/**
  * @brief  this function handles SPI1 handler.
  * @param  none
  * @retval none
  */
void SPI1_IRQHandler(void)
{
  /* add user code begin SPI1_IRQ 0 */

  /* add user code end SPI1_IRQ 0 */

  /* add user code begin SPI1_IRQ 1 */

  /* add user code end SPI1_IRQ 1 */
}

/**
  * @brief  this function handles USART1 handler.
  * @param  none
  * @retval none
  */
void USART1_IRQHandler(void)
{
  /* add user code begin USART1_IRQ 0 */

  /* add user code end USART1_IRQ 0 */

  /* add user code begin USART1_IRQ 1 */

  /* add user code end USART1_IRQ 1 */
}

/**
  * @brief  this function handles TMR6 & DAC handler.
  * @param  none
  * @retval none
  */
void TMR6_DAC_GLOBAL_IRQHandler(void)
{
  /* add user code begin TMR6_DAC_GLOBAL_IRQ 0 */

  /* add user code end TMR6_DAC_GLOBAL_IRQ 0 */

  wk_timebase_handler();

  /* add user code begin TMR6_DAC_GLOBAL_IRQ 1 */

  /* add user code end TMR6_DAC_GLOBAL_IRQ 1 */
}

/* add user code begin 1 */

/* add user code end 1 */
