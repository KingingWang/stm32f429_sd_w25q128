/*
 * File      : fal_cfg.h
 * This file is part of FAL (Flash Abstraction Layer) package
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-17     armink       the first version
 * 2020-03-20     ShineRoyal   change for stm32f411rc
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include <rtconfig.h>
#include <board.h>

//#define FLASH_SIZE_GRANULARITY_16K   (4 * 16 * 1024)
//#define FLASH_SIZE_GRANULARITY_64K   (1 * 64 * 1024)
//#define FLASH_SIZE_GRANULARITY_128K  (1 * 128 * 1024)
//#define FLASH_SIZE_GRANULARITY_256K  (1 * 256 * 1024)
#define FLASH_SIZE_GRANULARITY_512K0  (1 * 512 * 1024)
#define FLASH_SIZE_GRANULARITY_512K1  (1 * 512 * 1024)
#define FLASH_SIZE_GRANULARITY_1024K  (1 * 1024 * 1024)

//#define STM32_FLASH_START_ADRESS_16K  STM32_FLASH_START_ADRESS
//#define STM32_FLASH_START_ADRESS_64K  (STM32_FLASH_START_ADRESS_16K + FLASH_SIZE_GRANULARITY_16K)
//#define STM32_FLASH_START_ADRESS_128K (STM32_FLASH_START_ADRESS_64K + FLASH_SIZE_GRANULARITY_64K)
//#define STM32_FLASH_START_ADRESS_256K (STM32_FLASH_START_ADRESS_128K + FLASH_SIZE_GRANULARITY_128K)
#define STM32_FLASH_START_ADRESS_512K0 (STM32_FLASH_START_ADRESS)
#define STM32_FLASH_START_ADRESS_512K1 (STM32_FLASH_START_ADRESS_512K0 + FLASH_SIZE_GRANULARITY_512K0)
#define STM32_FLASH_START_ADRESS_1024K (STM32_FLASH_START_ADRESS_512K1 + FLASH_SIZE_GRANULARITY_512K1)

/* ===================== Flash device Configuration ========================= */
//extern const struct fal_flash_dev stm32_onchip_flash_16k;
//extern const struct fal_flash_dev stm32_onchip_flash_64k;
//extern const struct fal_flash_dev stm32_onchip_flash_128k;
//extern const struct fal_flash_dev stm32_onchip_flash_256k;
extern const struct fal_flash_dev stm32_onchip_flash_512k0;
extern const struct fal_flash_dev stm32_onchip_flash_512k1;
extern const struct fal_flash_dev stm32_onchip_flash_1024k;
extern  struct fal_flash_dev nor_flash0;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
    &stm32_onchip_flash_512k0,                                         \
    &stm32_onchip_flash_512k1,                                         \
    &stm32_onchip_flash_1024k,                                        \
    &nor_flash0,                                        \
}

/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
/* partition table */
#define FAL_PART_TABLE                                                                                                     \
{                                                                                                                          \
{FAL_PART_MAGIC_WROD, "bootloader",        "onchip_flash_512k0", 0 , FLASH_SIZE_GRANULARITY_512K0, 0}, \
    {FAL_PART_MAGIC_WROD, "app1",        "onchip_flash_512k1", 0 , FLASH_SIZE_GRANULARITY_512K1, 0}, \
    {FAL_PART_MAGIC_WROD, "app2",        "onchip_flash_1024k", 0 , FLASH_SIZE_GRANULARITY_1024K, 0}, \
    {FAL_PART_MAGIC_WROD, "easyflash",        "W25Q1281", 0 , 4096*1024, 0}, \
    {FAL_PART_MAGIC_WROD, "download",        "W25Q1281", 4096*1024 , (12288)*1024, 0}, \
}

#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
