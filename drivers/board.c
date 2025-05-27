/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-05-20     RealThread   first version
 */

#include <rtthread.h>

#include <board.h>
#include <drv_common.h>

RT_WEAK void rt_hw_board_init()
{
    extern void hw_board_init(char *clock_src, int32_t clock_src_freq, int32_t clock_target_freq);

    /* Heap initialization */
#if defined(RT_USING_HEAP)
    rt_system_heap_init((void *)HEAP_BEGIN, (void *)HEAP_END);
#endif

    hw_board_init(BSP_CLOCK_SOURCE, BSP_CLOCK_SOURCE_FREQ_MHZ, BSP_CLOCK_SYSTEM_FREQ_MHZ);

    /* Set the shell console output device */
#if defined(RT_USING_DEVICE) && defined(RT_USING_CONSOLE)
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

    /* Board underlying hardware initialization */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef gpio_config = {0};

    if (hspi->Instance == SPI1)
    {
        /* Peripheral clock enable */
        __SPI1_CLK_ENABLE(); // forget to open clock so that i waste one hour
        __GPIOA_CLK_ENABLE();
        /**SPI1 GPIO Configuration
        PA6     ------> SPI1_MISO
        PA5     ------> SPI1_SCLK
        PA7     ------> SPI1_MOSI
        */
        gpio_config.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
        gpio_config.Mode = GPIO_MODE_AF_PP;
        gpio_config.Pull = GPIO_PULLUP;
        gpio_config.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio_config.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOA, &gpio_config);
    }
    if (hspi->Instance == SPI2)
    {
        /* Peripheral clock enable */
        __SPI2_CLK_ENABLE();
        __GPIOB_CLK_ENABLE();
        /**SPI2 GPIO Configuration
        PB14     ------> SPI2_MISO
        PB13     ------> SPI2_SCLK
        PB15     ------> SPI2_MOSI
        */
        gpio_config.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
        gpio_config.Mode = GPIO_MODE_AF_PP;
        gpio_config.Pull = GPIO_PULLUP;
        gpio_config.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio_config.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &gpio_config);
    }
    if (hspi->Instance == SPI3)
    {
        /* Peripheral clock enable */
        __SPI3_CLK_ENABLE();
        __GPIOB_CLK_ENABLE();
        /**SPI3 GPIO Configuration
        PB3     ------> SPI3_SCLK
        PB4     ------> SPI3_MISO
        PB5     ------> SPI3_MOSI
        */
        gpio_config.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
        gpio_config.Mode = GPIO_MODE_AF_PP;
        gpio_config.Pull = GPIO_PULLUP;
        gpio_config.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio_config.Alternate = GPIO_AF6_SPI3;
        HAL_GPIO_Init(GPIOB, &gpio_config);
    }
}