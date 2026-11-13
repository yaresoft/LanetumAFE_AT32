/**
  **************************************************************************
  * @file     flash.c
  * @brief    flash program
  **************************************************************************
  *
  * Copyright (c) 2025, Artery Technology, All rights reserved.
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */


#include "flash.h"

/** @addtogroup AT32F455_periph_examples
  * @{
  */

/** @addtogroup 455_FLASH_write_read
  * @{
  */

#define SECTOR_SIZE                      0x800   /* this parameter depends on the specific model of the chip */

uint16_t flash_buf[SECTOR_SIZE / 2];

/**
  * @brief  read data using halfword mode
  * @param  read_addr: the address of reading
  * @param  p_buffer: the buffer of reading data
  * @param  num_read: the number of reading data
  * @retval none
  */
void flash_read(uint32_t read_addr, uint16_t *p_buffer, uint16_t num_read)
{
  uint16_t i;
  for(i = 0; i < num_read; i++)
  {
    p_buffer[i] = *(uint16_t*)(read_addr);
    read_addr += 2;
  }
}

/**
  * @brief  write data using halfword mode without checking
  * @param  write_addr: the address of writing
  * @param  p_buffer: the buffer of writing data
  * @param  num_write: the number of writing data
  * @retval result
  */
error_status flash_write_nocheck(uint32_t write_addr, uint16_t *p_buffer, uint16_t num_write)
{
  uint16_t i;
  flash_status_type status = FLASH_OPERATE_DONE; 
  for(i = 0; i < num_write; i++)
  {
    status = flash_halfword_program(write_addr, p_buffer[i]);
    if(status != FLASH_OPERATE_DONE)
      return ERROR;
    write_addr += 2;
  }
  return SUCCESS;
}

/**
  * @brief  write data using halfword mode with checking
  * @param  write_addr: the address of writing
  * @param  p_buffer: the buffer of writing data
  * @param  num_write: the number of writing data
  * @retval result
  */
error_status flash_write(uint32_t write_addr, uint16_t *p_buffer, uint16_t num_write)
{
  uint32_t offset_addr;
  uint32_t sector_position;
  uint16_t sector_offset;
  uint16_t sector_remain;
  uint16_t i;
  flash_status_type status = FLASH_OPERATE_DONE;
 
  flash_unlock();
  offset_addr = write_addr - FLASH_BASE;
  sector_position = offset_addr / SECTOR_SIZE;
  sector_offset = (offset_addr % SECTOR_SIZE) / 2;
  sector_remain = SECTOR_SIZE / 2 - sector_offset;
  if(num_write <= sector_remain)
    sector_remain = num_write;
  while(1)
  {
    flash_read(sector_position * SECTOR_SIZE + FLASH_BASE, flash_buf, SECTOR_SIZE / 2);
    for(i = 0; i < sector_remain; i++)
    {
      if(flash_buf[sector_offset + i] != 0xFFFF)
        break;
    }
    if(i < sector_remain)
    {
      /* wait for operation to be completed */
      status = flash_operation_wait_for(ERASE_TIMEOUT);
     
      if((status == FLASH_PROGRAM_ERROR) || (status == FLASH_EPP_ERROR))
        flash_flag_clear(FLASH_PRGMERR_FLAG | FLASH_EPPERR_FLAG);
      else if(status == FLASH_OPERATE_TIMEOUT)
      {	flash_lock();
        return ERROR;
      }
      status = flash_sector_erase(sector_position * SECTOR_SIZE + FLASH_BASE);
      if(status != FLASH_OPERATE_DONE)
      {	flash_lock();
        return ERROR;
      }
      for(i = 0; i < sector_remain; i++)
      {
        flash_buf[i + sector_offset] = p_buffer[i];
      }
      if(flash_write_nocheck(sector_position * SECTOR_SIZE + FLASH_BASE, flash_buf, SECTOR_SIZE / 2) != SUCCESS)
      {	flash_lock();
        return ERROR;
      }
    }
    else
    {
      if(flash_write_nocheck(write_addr, p_buffer, sector_remain) != SUCCESS)
      {flash_lock();
        return ERROR;
      }
    }
    if(num_write == sector_remain)
      break;
    else
    {
      sector_position++;
      sector_offset = 0;
      p_buffer += sector_remain;
      write_addr += (sector_remain * 2);
      num_write -= sector_remain;
      if(num_write > (SECTOR_SIZE / 2))
        sector_remain = SECTOR_SIZE / 2;
      else
        sector_remain = num_write;
    }
  }
  flash_lock();
  return SUCCESS;
}


error_status flash_erase(uint32_t start_addr, uint32_t size)
{
  uint32_t offset_addr;
  uint32_t sector_position;
  uint32_t sector_byte_offset;
  uint32_t sector_remain_bytes;
  uint32_t i;
  flash_status_type status = FLASH_OPERATE_DONE;
  uint8_t *buf_bytes;

  if (start_addr % 2 != 0) {
    return ERROR;
  }
  if (size == 0) {
    return SUCCESS;
  }

  flash_unlock();

  offset_addr = start_addr - FLASH_BASE;
  sector_position = offset_addr / SECTOR_SIZE;
  sector_byte_offset = offset_addr % SECTOR_SIZE;
  sector_remain_bytes = SECTOR_SIZE - sector_byte_offset;
  if (size <= sector_remain_bytes)
    sector_remain_bytes = size;

  while (1)
  {

    flash_read(sector_position * SECTOR_SIZE + FLASH_BASE,
               flash_buf, SECTOR_SIZE / 2);


    buf_bytes = (uint8_t *)flash_buf;


    for (i = 0; i < sector_remain_bytes; i++)
    {
      if (buf_bytes[sector_byte_offset + i] != 0xFF)
        break;
    }

    if (i < sector_remain_bytes)
    {

      status = flash_operation_wait_for(ERASE_TIMEOUT);
      if ((status == FLASH_PROGRAM_ERROR) || (status == FLASH_EPP_ERROR))
        flash_flag_clear(FLASH_PRGMERR_FLAG | FLASH_EPPERR_FLAG);
      else if (status == FLASH_OPERATE_TIMEOUT)
      {
        flash_lock();
        return ERROR;
      }

      status = flash_sector_erase(sector_position * SECTOR_SIZE + FLASH_BASE);
      if (status != FLASH_OPERATE_DONE)
      {
        flash_lock();
        return ERROR;
      }


      for (i = 0; i < sector_remain_bytes; i++)
      {
        buf_bytes[sector_byte_offset + i] = 0xFF;
      }


      if (flash_write_nocheck(sector_position * SECTOR_SIZE + FLASH_BASE,
                              flash_buf, SECTOR_SIZE / 2) != SUCCESS)
      {
        flash_lock();
        return ERROR;
      }
    }

    if (size == sector_remain_bytes)
      break;
    else
    {
      sector_position++;
      sector_byte_offset = 0;
      start_addr += sector_remain_bytes;
      size -= sector_remain_bytes;
      if (size > SECTOR_SIZE)
        sector_remain_bytes = SECTOR_SIZE;
      else
        sector_remain_bytes = size;
    }
  }

  flash_lock();
  return SUCCESS;
}
/**
  * @}
  */

/**
  * @}
  */
