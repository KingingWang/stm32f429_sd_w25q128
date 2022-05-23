/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-30     12645       the first version
 */
#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>
#include "drv_spi.h"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include <spi_flash_sfud.h>
static int rt_hw_spi_flash_init(void)
{
    __HAL_RCC_GPIOF_CLK_ENABLE()
    ;
    rt_hw_spi_device_attach("spi5", "spi50", GPIOF, GPIO_PIN_6); // spi10 表示挂载在 spi3 总线上的 0 号设备,PC0是片选，这一步就可以将从设备挂在到总线中。

    if (RT_NULL == rt_sfud_flash_probe("W25Q1281", "spi50"))  //注册块设备，这一步可以将外部flash抽象为系统的块设备
    {
        return -RT_ERROR;
    };

//    __HAL_RCC_GPIOA_CLK_ENABLE()
//    ;
//    rt_hw_spi_device_attach("spi1", "spi10", GPIOA, GPIO_PIN_4); // spi10 表示挂载在 spi3 总线上的 0 号设备,PC0是片选，这一步就可以将从设备挂在到总线中。
//
//    if (RT_NULL == rt_sfud_flash_probe("W25Q1282", "spi10"))  //注册块设备，这一步可以将外部flash抽象为系统的块设备
//    {
//        return -RT_ERROR;
//    };

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_hw_spi_flash_init);

//int dfs_mount_init(void)
//{
//    dfs_mkfs("elm", "W25Q1281");
//
//    if (dfs_mount("W25Q1281", "/", "elm", 0, 0))
//    {
//        rt_kprintf("W25Q1281 mount success\n");
//        return RT_EOK;
//    }
//    else
//    {
//        rt_kprintf("W25Q1281 mount fail\n");
//        return -RT_ERROR;
//    }
//}
//INIT_COMPONENT_EXPORT(dfs_mount_init);

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



