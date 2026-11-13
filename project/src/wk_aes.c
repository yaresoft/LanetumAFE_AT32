/* add user code begin Header */
/**
  **************************************************************************
  * @file     wk_aes.c
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
#include "wk_aes.h"

/* add user code begin 0 */

/* add user code end 0 */

uint32_t key[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
/**
  * @brief  init aes function.
  * @param  none
  * @retval none
  */
void wk_aes_init(void)
{
  /* add user code begin aes_init 0 */

  /* add user code end aes_init 0 */
  
  aes_init_type aes_init_struct; 
 
  aes_reset();
  aes_default_para_init(&aes_init_struct);
  aes_init_struct.operate_mode = AES_OPMODE_ENCRYPT;
  aes_init_struct.chaining_mode = AES_CHMODE_ECB;
  aes_init_struct.key_len = AES_KEY_LENGTH_128;
  aes_init_struct.key_buf = (uint32_t *)key;
  aes_init_struct.data_swap_mode = AES_SWAP_TYPE_NONE;
  aes_init(&aes_init_struct);
  
  /* add user code begin aes_init 1 */

  /* add user code end aes_init 1 */
}

/* add user code begin 1 */

/* add user code end 1 */
