/* add user code begin Header */
/**
  **************************************************************************
  * @file     wk_crc.c
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
#include "wk_crc.h"

/* add user code begin 0 */

/* add user code end 0 */

/**
  * @brief  init crc function.
  * @param  none
  * @retval none
  */
void wk_crc_init(void)
{
  /* add user code begin crc_init 0 */

  /* add user code end crc_init 0 */

  crc_init_data_set(0xFFFFFFFF);
  crc_poly_size_set(CRC_POLY_SIZE_32B);
  crc_poly_value_set(0x04C11DB7);
  crc_reverse_input_data_set(CRC_REVERSE_INPUT_NO_AFFECTE);
  crc_reverse_output_data_set(CRC_REVERSE_OUTPUT_NO_AFFECTE);
  crc_data_reset();

  /* add user code begin crc_init 1 */

  /* add user code end crc_init 1 */
}

/* add user code begin 1 */

/* add user code end 1 */
