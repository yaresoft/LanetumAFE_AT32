/* add user code begin Header */
/**
  **************************************************************************
  * @file     wk_trng.c
  * @brief    work bench config program
  **************************************************************************
  *
  * Copyright (c) 2025, Artery Technology, All rights reserved.
  * SPDX-License-Identifier: BSD-3-Clause
  *
  **************************************************************************
  */
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "wk_trng.h"

/* add user code begin 0 */

/* add user code end 0 */

/**
  * @brief  init trng function.
  * @param  none
  * @retval none
  */
void wk_trng_init(void)
{
  /* add user code begin trng_init 0 */

  /* add user code end trng_init 0 */
 
  /* check if ckfes has been set */
  while(trng_flag_get(TRNG_CKFES_FLAG) == SET)
  {
    /* add user code begin trng_init 1 */

   /* add user code end trng_init 1 */
  }
  
  /* check if ESFES has been set */
  while(trng_flag_get(TRNG_ESFES_FLAG) == SET)
  {
    /* add user code begin trng_init 2 */

   /* add user code end trng_init 2 */
  }

  trng_clock_divider_set(0);
  
  /* enable trng noise source */
  trng_enable(TRUE);

  /* add user code begin trng_init 3 */

  /* add user code end trng_init 3 */
}

/* add user code begin 1 */

/* add user code end 1 */
