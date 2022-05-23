# stm32f429_sd_w25q128
stm32f429创建块设备,挂载sd卡,挂载w25q128
# stm32f407igt6 片上FLASH读写测试

STM32F429igt6官方说有1MB的flash，但是实际上内部有2MB的flash，相当于STM32F429iit6吧。

使用 STM32F429igt6的开发板， 实现Flash抽象层FAL。

## 复制修改文件

先添加fal的软件包，并复制fal_cfg.h到drivers目录下：

复制C:\RT-ThreadStudio\download\rt-thread-sdk\rt-thread-src\v4.0.2\bsp\stm32\libraries\HAL_Drivers\drv_flash目录下的drv_flash.h文件和对应的flash驱动文件如drv_flash_f4.c到drivers目录下，复制上一级目录的drv_log.h到drivers目录下，并注释drv_flash_f4.c文件中的#include "drv_config.h"。

![image-20220522233641793](https://cdn.jsdelivr.net/gh/KingingWang/myImage@main/images/1653233835060image-20220522233641793.png)

### 修改fal_cfg.h为以下内容：

``` c
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

/* flash device table */
#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
    &stm32_onchip_flash_512k0,                                         \
    &stm32_onchip_flash_512k1,                                         \
    &stm32_onchip_flash_1024k,                                        \
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
}

#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */

```

这里的建议是他们根据最小擦除粒度分为3个区域，刚好2MB。

### 修改drv_flash_f4c.h

```c
/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-12-5      SummerGift   first version
 */

#include "board.h"

#ifdef BSP_USING_ON_CHIP_FLASH
//#include "drv_config.h"
#include "drv_flash.h"

#if defined(PKG_USING_FAL)
#include "fal.h"
#endif

//#define DRV_DEBUG
#define LOG_TAG                "drv.flash"
#include <drv_log.h>

/* Base address of the Flash sectors Bank 1 */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

/* Base address of the Flash sectors Bank 2 */
#define ADDR_FLASH_SECTOR_12     ((uint32_t)0x08100000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_13     ((uint32_t)0x08104000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_14     ((uint32_t)0x08108000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_15     ((uint32_t)0x0810C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_16     ((uint32_t)0x08110000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_17     ((uint32_t)0x08120000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_18     ((uint32_t)0x08140000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_19     ((uint32_t)0x08160000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_20     ((uint32_t)0x08180000) /* Base @ of Sector 8, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_21     ((uint32_t)0x081A0000) /* Base @ of Sector 9, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_22     ((uint32_t)0x081C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_23     ((uint32_t)0x081E0000) /* Base @ of Sector 11, 128 Kbytes */

/* Base address of the Flash sectors Bank 3 */
#define ADDR_FLASH_SECTOR_24     ((uint32_t)0x08200000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_25     ((uint32_t)0x08204000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_26     ((uint32_t)0x08208000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_27     ((uint32_t)0x0820C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_28    ((uint32_t)0x08210000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_29     ((uint32_t)0x08220000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_30     ((uint32_t)0x08240000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_31     ((uint32_t)0x08260000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_32     ((uint32_t)0x08280000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_33     ((uint32_t)0x082A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_34    ((uint32_t)0x082C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_35    ((uint32_t)0x082E0000) /* Base @ of Sector 11, 128 Kbytes */

/* Base address of the Flash sectors Bank 4 */
#define ADDR_FLASH_SECTOR_36     ((uint32_t)0x08300000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_37     ((uint32_t)0x08304000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_38     ((uint32_t)0x08308000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_39     ((uint32_t)0x0830C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_40     ((uint32_t)0x08310000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_41     ((uint32_t)0x08320000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_42     ((uint32_t)0x08340000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_43     ((uint32_t)0x08360000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_44     ((uint32_t)0x08380000) /* Base @ of Sector 8, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_45     ((uint32_t)0x083A0000) /* Base @ of Sector 9, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_46     ((uint32_t)0x083C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_47     ((uint32_t)0x083E0000) /* Base @ of Sector 11, 128 Kbytes */

/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
static rt_uint32_t GetSector(rt_uint32_t Address)
{
    rt_uint32_t sector = 0;

    if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
    {
        sector = FLASH_SECTOR_0;
    }
    else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
    {
        sector = FLASH_SECTOR_1;
    }
    else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
    {
        sector = FLASH_SECTOR_2;
    }
    else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
    {
        sector = FLASH_SECTOR_3;
    }
    else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
    {
        sector = FLASH_SECTOR_4;
    }
    else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
    {
        sector = FLASH_SECTOR_5;
    }
    else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
    {
        sector = FLASH_SECTOR_6;
    }
    else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
    {
        sector = FLASH_SECTOR_7;
    }
#if defined(FLASH_SECTOR_8)
    else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
    {
        sector = FLASH_SECTOR_8;
    }
#endif
#if defined(FLASH_SECTOR_9)
    else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
    {
        sector = FLASH_SECTOR_9;
    }
#endif
#if defined(FLASH_SECTOR_10)
    else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
    {
        sector = FLASH_SECTOR_10;
    }
#endif
#if defined(FLASH_SECTOR_11)
    else if((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11))
    {
        sector = FLASH_SECTOR_11;
    }
#endif
#if defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx)|| defined(STM32F439xx) || defined(STM32F469xx) || defined(STM32F479xx)
    else if((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
    {
        sector = FLASH_SECTOR_12;
    }
    else if((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
    {
        sector = FLASH_SECTOR_13;
    }
    else if((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
    {
        sector = FLASH_SECTOR_14;
    }
    else if((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
    {
        sector = FLASH_SECTOR_15;
    }
    else if((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
    {
        sector = FLASH_SECTOR_16;
    }
    else if((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
    {
        sector = FLASH_SECTOR_17;
    }
    else if((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18))
    {
        sector = FLASH_SECTOR_18;
    }
    else if((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19))
    {
        sector = FLASH_SECTOR_19;
    }
    else if((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20))
    {
        sector = FLASH_SECTOR_20;
    }
    else if((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21))
    {
        sector = FLASH_SECTOR_21;
    }
    else if((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22))
    {
        sector = FLASH_SECTOR_22;
    }
    else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23) */
    {
        sector = FLASH_SECTOR_23;
    }
#endif
    return sector;
}

/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
int stm32_flash_read(rt_uint32_t addr, rt_uint8_t *buf, size_t size)
{
    size_t i;

    if ((addr + size) > STM32_FLASH_END_ADDRESS)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -1;
    }

    for (i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(rt_uint8_t *) addr;
    }

    return size;
}

/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
int stm32_flash_write(rt_uint32_t addr, const rt_uint8_t *buf, size_t size)
{
    rt_err_t result      = RT_EOK;
    rt_uint32_t end_addr = addr + size;

    if ((end_addr) > STM32_FLASH_END_ADDRESS)
    {
        LOG_E("write outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }

    if (size < 1)
    {
        return -RT_EINVAL;
    }

    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    for (size_t i = 0; i < size; i++, addr++, buf++)
    {
        /* write data to flash */
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr, (rt_uint64_t)(*buf)) == HAL_OK)
        {
            if (*(rt_uint8_t *)addr != *buf)
            {
                result = -RT_ERROR;
                break;
            }
        }
        else
        {
            result = -RT_ERROR;
            break;
        }
    }

    HAL_FLASH_Lock();

    if (result != RT_EOK)
    {
        return result;
    }

    return size;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
int stm32_flash_erase(rt_uint32_t addr, size_t size)
{
    rt_err_t result = RT_EOK;
    rt_uint32_t FirstSector = 0, NbOfSectors = 0;
    rt_uint32_t SECTORError = 0;

    if ((addr + size) > STM32_FLASH_END_ADDRESS)
    {
        LOG_E("ERROR: erase outrange flash size! addr is (0x%p)\n", (void*)(addr + size));
        return -RT_EINVAL;
    }

    /*Variable used for Erase procedure*/
    FLASH_EraseInitTypeDef EraseInitStruct;

    /* Unlock the Flash to enable the flash control register access */
    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    /* Get the 1st sector to erase */
    FirstSector = GetSector(addr);
    /* Get the number of sector to erase from 1st sector*/
    NbOfSectors = GetSector(addr + size - 1) - FirstSector + 1;
    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector        = FirstSector;
    EraseInitStruct.NbSectors     = NbOfSectors;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, (uint32_t *)&SECTORError) != HAL_OK)
    {
        result = -RT_ERROR;
        goto __exit;
    }

__exit:
    HAL_FLASH_Lock();

    if (result != RT_EOK)
    {
        return result;
    }

    LOG_D("erase done: addr (0x%p), size %d", (void*)addr, size);
    return size;
}

#if defined(PKG_USING_FAL)

//static int fal_flash_read_16k(long offset, rt_uint8_t *buf, size_t size);
//static int fal_flash_read_64k(long offset, rt_uint8_t *buf, size_t size);
//static int fal_flash_read_128k(long offset, rt_uint8_t *buf, size_t size);
//static int fal_flash_read_256k(long offset, rt_uint8_t *buf, size_t size);
static int fal_flash_read_512k0(long offset, rt_uint8_t *buf, size_t size);
static int fal_flash_read_512k1(long offset, rt_uint8_t *buf, size_t size);
static int fal_flash_read_1024k(long offset, rt_uint8_t *buf, size_t size);

//static int fal_flash_write_16k(long offset, const rt_uint8_t *buf, size_t size);
//static int fal_flash_write_64k(long offset, const rt_uint8_t *buf, size_t size);
//static int fal_flash_write_128k(long offset, const rt_uint8_t *buf, size_t size);
//static int fal_flash_write_256k(long offset, const rt_uint8_t *buf, size_t size);
static int fal_flash_write_512k0(long offset, const rt_uint8_t *buf, size_t size);
static int fal_flash_write_512k1(long offset, const rt_uint8_t *buf, size_t size);
static int fal_flash_write_1024k(long offset, const rt_uint8_t *buf, size_t size);

//static int fal_flash_erase_16k(long offset, size_t size);
//static int fal_flash_erase_64k(long offset, size_t size);
//static int fal_flash_erase_128k(long offset, size_t size);
//static int fal_flash_erase_256k(long offset, size_t size);
static int fal_flash_erase_512k0(long offset, size_t size);
static int fal_flash_erase_512k1(long offset, size_t size);
static int fal_flash_erase_1024k(long offset, size_t size);

//const struct fal_flash_dev stm32_onchip_flash_16k = { "onchip_flash_16k", STM32_FLASH_START_ADRESS_16K, FLASH_SIZE_GRANULARITY_16K, (16 * 1024), {NULL, fal_flash_read_16k, fal_flash_write_16k, fal_flash_erase_16k} };
//const struct fal_flash_dev stm32_onchip_flash_64k = { "onchip_flash_64k", STM32_FLASH_START_ADRESS_64K, FLASH_SIZE_GRANULARITY_64K, (64 * 1024), {NULL, fal_flash_read_64k, fal_flash_write_64k, fal_flash_erase_64k} };
//const struct fal_flash_dev stm32_onchip_flash_128k = { "onchip_flash_128k", STM32_FLASH_START_ADRESS_128K, FLASH_SIZE_GRANULARITY_128K, (128 * 1024), {NULL, fal_flash_read_128k, fal_flash_write_128k, fal_flash_erase_128k} };
//const struct fal_flash_dev stm32_onchip_flash_256k = { "onchip_flash_256k", STM32_FLASH_START_ADRESS_256K, FLASH_SIZE_GRANULARITY_256K, (256 * 1024), {NULL, fal_flash_read_256k, fal_flash_write_256k, fal_flash_erase_256k} };
const struct fal_flash_dev stm32_onchip_flash_512k0 = { "onchip_flash_512k0", STM32_FLASH_START_ADRESS_512K0, FLASH_SIZE_GRANULARITY_512K0, (512 * 1024), {NULL, fal_flash_read_512k0, fal_flash_write_512k0, fal_flash_erase_512k0} };
const struct fal_flash_dev stm32_onchip_flash_512k1 = { "onchip_flash_512k1", STM32_FLASH_START_ADRESS_512K1, FLASH_SIZE_GRANULARITY_512K1, (512 * 1024), {NULL, fal_flash_read_512k1, fal_flash_write_512k1, fal_flash_erase_512k1} };
const struct fal_flash_dev stm32_onchip_flash_1024k = { "onchip_flash_1024k", STM32_FLASH_START_ADRESS_1024K, FLASH_SIZE_GRANULARITY_1024K, (1024 * 1024), {NULL, fal_flash_read_1024k, fal_flash_write_1024k, fal_flash_erase_1024k} };

//static int fal_flash_read_16k(long offset, rt_uint8_t *buf, size_t size)
//{
//    return stm32_flash_read(stm32_onchip_flash_16k.addr + offset, buf, size);
//}
//static int fal_flash_read_64k(long offset, rt_uint8_t *buf, size_t size)
//{
//    return stm32_flash_read(stm32_onchip_flash_64k.addr + offset, buf, size);
//}
//static int fal_flash_read_128k(long offset, rt_uint8_t *buf, size_t size)
//{
//    return stm32_flash_read(stm32_onchip_flash_128k.addr + offset, buf, size);
//}
//static int fal_flash_read_256k(long offset, rt_uint8_t *buf, size_t size)
//{
//    return stm32_flash_read(stm32_onchip_flash_256k.addr + offset, buf, size);
//}

static int fal_flash_read_512k0(long offset, rt_uint8_t *buf, size_t size)
{
    return stm32_flash_read(stm32_onchip_flash_512k0.addr + offset, buf, size);
}
static int fal_flash_read_512k1(long offset, rt_uint8_t *buf, size_t size)
{
    return stm32_flash_read(stm32_onchip_flash_512k1.addr + offset, buf, size);
}
static int fal_flash_read_1024k(long offset, rt_uint8_t *buf, size_t size)
{
    return stm32_flash_read(stm32_onchip_flash_1024k.addr + offset, buf, size);
}


//static int fal_flash_write_16k(long offset, const rt_uint8_t *buf, size_t size)
//{
//    return stm32_flash_write(stm32_onchip_flash_16k.addr + offset, buf, size);
//}
//static int fal_flash_write_64k(long offset, const rt_uint8_t *buf, size_t size)
//{
//    return stm32_flash_write(stm32_onchip_flash_64k.addr + offset, buf, size);
//}
//static int fal_flash_write_128k(long offset, const rt_uint8_t *buf, size_t size)
//{
//    return stm32_flash_write(stm32_onchip_flash_128k.addr + offset, buf, size);
//}
//static int fal_flash_write_256k(long offset, const rt_uint8_t *buf, size_t size)
//{
//    return stm32_flash_write(stm32_onchip_flash_256k.addr + offset, buf, size);
//}
static int fal_flash_write_512k0(long offset, const rt_uint8_t *buf, size_t size)
{
    return stm32_flash_write(stm32_onchip_flash_512k0.addr + offset, buf, size);
}
static int fal_flash_write_512k1(long offset, const rt_uint8_t *buf, size_t size)
{
    return stm32_flash_write(stm32_onchip_flash_512k1.addr + offset, buf, size);
}
static int fal_flash_write_1024k(long offset, const rt_uint8_t *buf, size_t size)
{
    return stm32_flash_write(stm32_onchip_flash_1024k.addr + offset, buf, size);
}


//static int fal_flash_erase_16k(long offset, size_t size)
//{
//    return stm32_flash_erase(stm32_onchip_flash_16k.addr + offset, size);
//}
//static int fal_flash_erase_64k(long offset, size_t size)
//{
//    return stm32_flash_erase(stm32_onchip_flash_64k.addr + offset, size);
//}
//static int fal_flash_erase_128k(long offset, size_t size)
//{
//    return stm32_flash_erase(stm32_onchip_flash_128k.addr + offset, size);
//}
//static int fal_flash_erase_256k(long offset, size_t size)
//{
//    return stm32_flash_erase(stm32_onchip_flash_256k.addr + offset, size);
//}
static int fal_flash_erase_512k0(long offset, size_t size)
{
    return stm32_flash_erase(stm32_onchip_flash_512k0.addr + offset, size);
}
static int fal_flash_erase_512k1(long offset, size_t size)
{
    return stm32_flash_erase(stm32_onchip_flash_512k1.addr + offset, size);
}
static int fal_flash_erase_1024k(long offset, size_t size)
{
    return stm32_flash_erase(stm32_onchip_flash_1024k.addr + offset, size);
}

#endif
#endif /* BSP_USING_ON_CHIP_FLASH */

```



### 创建onchipFlash.c文件

```c
/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-05-22     12645       the first version
 */
#include <rtthread.h>
#include <fal.h>
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
void onchipFlash_init()
{
    fal_init();
}
INIT_DEVICE_EXPORT(onchipFlash_init);

#define BUF_SIZE 1024

static int fal_test(uint8_t argc, char **argv )
{
    if (argc < 2) {
            rt_kprintf("Usage:fal_test app1\n");
            }
    char *partiton_name=argv[1];
    LOG_I("%s",partiton_name);
    int ret;
    int i, j, len;
    char buf[BUF_SIZE];
    const struct fal_flash_dev *flash_dev = RT_NULL;
    const struct fal_partition *partition = RT_NULL;

    if (!partiton_name)
    {
        rt_kprintf("Input param partition name is null!\n");
        return -1;
    }

    partition = fal_partition_find(partiton_name);
    if (partition == RT_NULL)
    {
        rt_kprintf("Find partition (%s) failed!\n", partiton_name);
        ret = -1;
        return ret;
    }

    flash_dev = fal_flash_device_find(partition->flash_name);
    if (flash_dev == RT_NULL)
    {
        rt_kprintf("Find flash device (%s) failed!\n", partition->flash_name);
        ret = -1;
        return ret;
    }

    rt_kprintf("Flash device : %s   "
               "Flash size : %dK   \n"
               "Partition : %s   "
               "Partition size: %dK\n",
                partition->flash_name,
                flash_dev->len/1024,
                partition->name,
                partition->len/1024);

    /* erase all partition */
    ret = fal_partition_erase_all(partition);
    if (ret < 0)
    {
        rt_kprintf("Partition (%s) erase failed!\n", partition->name);
        ret = -1;
        return ret;
    }
    rt_kprintf("Erase (%s) partition finish!\n", partiton_name);

    /* read the specified partition and check data */
    for (i = 0; i < partition->len;)
    {
        rt_memset(buf, 0x00, BUF_SIZE);

        len = (partition->len - i) > BUF_SIZE ? BUF_SIZE : (partition->len - i);

        ret = fal_partition_read(partition, i, buf, len);
        if (ret < 0)
        {
            rt_kprintf("Partition (%s) read failed!\n", partition->name);
            ret = -1;
            return ret;
        }

        for(j = 0; j < len; j++)
        {
            if (buf[j] != 0xFF)
            {
                rt_kprintf("The erase operation did not really succeed!\n");
                ret = -1;
                return ret;
            }
        }
        i += len;
    }

    /* write 0x00 to the specified partition */
    for (i = 0; i < partition->len;)
    {
        rt_memset(buf, 0x00, BUF_SIZE);

        len = (partition->len - i) > BUF_SIZE ? BUF_SIZE : (partition->len - i);

        ret = fal_partition_write(partition, i, buf, len);
        if (ret < 0)
        {
            rt_kprintf("Partition (%s) write failed!\n", partition->name);
            ret = -1;
            return ret;
        }

        i += len;
    }
    rt_kprintf("Write (%s) partition finish! Write size %d(%dK).\n", partiton_name, i, i/1024);

    /* read the specified partition and check data */
    for (i = 0; i < partition->len;)
    {
        rt_memset(buf, 0xFF, BUF_SIZE);

        len = (partition->len - i) > BUF_SIZE ? BUF_SIZE : (partition->len - i);

        ret = fal_partition_read(partition, i, buf, len);
        if (ret < 0)
        {
            rt_kprintf("Partition (%s) read failed!\n", partition->name);
            ret = -1;
            return ret;
        }

        for(j = 0; j < len; j++)
        {
            if (buf[j] != 0x00)
            {
                rt_kprintf("The write operation did not really succeed!\n");
                ret = -1;
                return ret;
            }
        }

        i += len;
    }
    rt_kprintf("Fal partition (%s) test success!\n", partiton_name);
    ret = 0;
    return ret;
}
MSH_CMD_EXPORT(fal_test,test_flash);

```

## 修改链接脚本，将rom大小修改为2048

![image-20220522234333350](https://cdn.jsdelivr.net/gh/KingingWang/myImage@main/images/1653234255062image-20220522234333350.png)

## 修改board.h，将rom大小修改为2M，其实选芯片的时候选stm32f429iit6应该也可以，没有测试，请自行测试。

![image-20220522234528683](https://cdn.jsdelivr.net/gh/KingingWang/myImage@main/images/1653234375056image-20220522234528683.png)

## 下载验证：

![image-20220522234701507](https://cdn.jsdelivr.net/gh/KingingWang/myImage@main/images/1653234435050image-20220522234701507.png)

在进行读写测试的时候，这里是对第三个分区进行的测试，如果对第一个分区进行测试，擦除了部分固件，程序的运行可能会出现预想不到的结果。

*由于stm32f429igt6后面1MB的rom官方未经过测试，请自行测试，如果测试失败也是有可能的*

实现片上Flash的fal工程下载

————————————————————————————————————————————

下面开启外部flash的fal功能。

开启fatfs和dfs，设置fatfs最大要处理扇区大小为4096，再稍微修改一下分区表：

![image-20220523092244049](https://cdn.jsdelivr.net/gh/KingingWang/myImage@main/images/1653268994923image-20220523092244049.png)

修改分区表：

![image-20220523092334880](https://cdn.jsdelivr.net/gh/KingingWang/myImage@main/images/1653269054922image-20220523092334880.png)

修改设备表：

![image-20220523092421719](https://cdn.jsdelivr.net/gh/KingingWang/myImage@main/images/1653269114928image-20220523092421719.png)

增加函数并添加到命令行：

*由于我的工程将sd卡挂载到了根目录，所以这里我挂载到了/w25dl目录,必须在根目录下创建这个目录才能成功，没挂载sd卡的话请挂载到根目录*

``` c
#define FS_PARTITION_NAME  "download"
int W25Q128Mount()
{
    /* 在 spi flash 中名为 "download" 的分区上创建一个块设备 */
    struct rt_device *flash_dev = fal_blk_device_create(FS_PARTITION_NAME);
    if (flash_dev == NULL)
    {
        LOG_E("Can't create a block device on '%s' partition.", FS_PARTITION_NAME);
    }
    else
    {
        LOG_D("Create a block device on the %s partition of flash successful.", FS_PARTITION_NAME);
    }

    /* 挂载 spi flash 中名为 "filesystem" 的分区上的文件系统 */
    LOG_I("%s",flash_dev->parent.name);
    if (dfs_mount(flash_dev->parent.name, "/w25dl", "elm", 0, 0) == 0)
    {
        LOG_I("Filesystem initialized!");
    }
    else
    {
        LOG_E("Failed to initialize filesystem!");
        LOG_D("You should create a filesystem on the block device first!");
    }
}
MSH_CMD_EXPORT(W25Q128Mount,"mount w25q128")
```

启动系统,输入W25Q128Mount

```bash
msh />W25Q128Mount
[I/FAL] The FAL block device (download) created successfully
[D/DBG] Create a block device on the download partition of flash successful.
[I/DBG] download▒
[I/DBG] Filesystem initialized!
msh />
```

如果失败,请创建文件系统

```
mkfs -t elm download
```

