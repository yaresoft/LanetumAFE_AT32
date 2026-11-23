/* add user code begin Header */
/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
  **************************************************************************
  *
  * Copyright (c) 2025, Artery Technology, All rights reserved.
  * SPDX-License-Identifier: BSD-3-Clause
  *
  **************************************************************************
  */
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "at32f45x_wk_config.h"
#include "wk_adc.h"
#include "wk_aes.h"
#include "wk_crc.h"
#include "wk_exint.h"
#include "wk_spi.h"
#include "wk_trng.h"
#include "wk_usart.h"
#include "wk_dma.h"
#include "wk_system.h"
#include "freertos_app.h"
#include "arm_math.h"

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
volatile uint8_t spi_buffer[2048];
volatile uint16_t adc1_buffer[2032];
volatile uint16_t adc2_buffer[2032];
/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */

/* add user code end 0 */

/**
  * @brief main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  /* add user code begin 1 */

  /* add user code end 1 */

  /* system clock config. */
  wk_system_clock_config();

  /* config periph clock. */
  wk_periph_clock_config();

  /* nvic config. */
  wk_nvic_config();

  /* timebase config for
     void wk_delay_ms(uint32_t delay); */
  wk_timebase_init();

  /* init adc-common function. */
  wk_adc_common_init();

  /* init adc1 function. */
  wk_adc1_init();

  /* init adc2 function. */
  wk_adc2_init();

  /* init spi1 function. */
  wk_spi1_init();

  /* usart1 already supports printf. */
  /* init usart1 function. */
//  wk_usart1_init();

  /* init crc function. */
  wk_crc_init();

  /* init aes function. */
  wk_aes_init();

  /* init trng function. */
  wk_trng_init();

  /* init dma2 channel1 */
  wk_dma2_channel1_init();
  /* config dma channel transfer parameter */
  /* user need to modify define values DMAx_CHANNELy_XXX_BASE_ADDR 
     and DMAx_CHANNELy_BUFFER_SIZE in at32xxx_wk_config.h */
  wk_dma_channel_config(DMA2_CHANNEL1, 
                        (uint32_t)&SPI1->dt, 
						(uint32_t)spi_buffer,
						sizeof(spi_buffer));
  dma_channel_enable(DMA2_CHANNEL1, TRUE);

  /* init dma2 channel2 */
  wk_dma2_channel2_init();
  /* config dma channel transfer parameter */
  /* user need to modify define values DMAx_CHANNELy_XXX_BASE_ADDR 
     and DMAx_CHANNELy_BUFFER_SIZE in at32xxx_wk_config.h */
  wk_dma_channel_config(DMA2_CHANNEL2, 
                        (uint32_t)&SPI1->dt, 
						(uint32_t)spi_buffer,
						sizeof(spi_buffer));
  dma_channel_enable(DMA2_CHANNEL2, TRUE);

  /* init dma1 channel1 */
  wk_dma1_channel1_init();
  /* config dma channel transfer parameter */
  /* user need to modify define values DMAx_CHANNELy_XXX_BASE_ADDR 
     and DMAx_CHANNELy_BUFFER_SIZE in at32xxx_wk_config.h */
  wk_dma_channel_config(DMA1_CHANNEL1, 
                        (uint32_t)&ADC1->odt, 
						(uint32_t)adc1_buffer,
                        sizeof(adc1_buffer)/sizeof(adc1_buffer[0]));
  dma_channel_enable(DMA1_CHANNEL1, TRUE);

  /* init dma1 channel2 */
  wk_dma1_channel2_init();
  /* config dma channel transfer parameter */
  /* user need to modify define values DMAx_CHANNELy_XXX_BASE_ADDR 
     and DMAx_CHANNELy_BUFFER_SIZE in at32xxx_wk_config.h */
  wk_dma_channel_config(DMA1_CHANNEL2, 
                        (uint32_t)&ADC2->odt, 
						(uint32_t)adc2_buffer,
						sizeof(adc2_buffer)/sizeof(adc1_buffer[0]));
  dma_channel_enable(DMA1_CHANNEL2, TRUE);

  /* init dma2 channel3 */
  wk_dma2_channel3_init();
  /* config dma channel transfer parameter */
  /* user need to modify define values DMAx_CHANNELy_XXX_BASE_ADDR 
     and DMAx_CHANNELy_BUFFER_SIZE in at32xxx_wk_config.h */
  wk_dma_channel_config(DMA2_CHANNEL3, 
                        (uint32_t)&USART1->dt, 
                        DMA2_CHANNEL3_MEMORY_BASE_ADDR, 
                        DMA2_CHANNEL3_BUFFER_SIZE);
  dma_channel_enable(DMA2_CHANNEL3, TRUE);

  /* init dma2 channel4 */
  wk_dma2_channel4_init();
  /* config dma channel transfer parameter */
  /* user need to modify define values DMAx_CHANNELy_XXX_BASE_ADDR 
     and DMAx_CHANNELy_BUFFER_SIZE in at32xxx_wk_config.h */
  wk_dma_channel_config(DMA2_CHANNEL4, 
                        (uint32_t)&USART1->dt, 
                        DMA2_CHANNEL4_MEMORY_BASE_ADDR, 
                        DMA2_CHANNEL4_BUFFER_SIZE);
  dma_channel_enable(DMA2_CHANNEL4, TRUE);

  /* add user code begin 2 */
  /* init exint function. */
  wk_exint_config();

  adc_flag_clear(ADC1, ADC_OCCO_FLAG | ADC_TCF_FLAG);
  adc_flag_clear(ADC2, ADC_OCCO_FLAG | ADC_TCF_FLAG);
  /* add user code end 2 */

  /* init freertos function. */
  wk_freertos_init();

  while(1)
  {
    /* add user code begin 3 */

    /* add user code end 3 */
  }
}

  /* add user code begin 4 */

  /* add user code end 4 */
