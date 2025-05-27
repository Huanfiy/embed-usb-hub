/*
 * Copyright (c) 2006-2025, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-05-20     RT-Thread    first version
 */
#include <rtthread.h>

#include "app_gpio_drv.h"

static void typec_irq_callback(void *args);
static void typec_timer_callback(void *args);
static rt_timer_t typec_timer;

int main(void)
{
    rt_pin_attach_irq(TYPEC_DETECT, PIN_IRQ_MODE_RISING, typec_irq_callback, (void *)TYPEC_DETECT);
    rt_pin_irq_enable(TYPEC_DETECT, PIN_IRQ_ENABLE);

    typec_timer = rt_timer_create("typec_timer", typec_timer_callback, NULL, 500,
                                  RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    if (typec_timer == RT_NULL)
    {
        rt_kprintf("typec_timer create failed!\n");
    }

    app_gpio_ops.set_high(USB_EN);

    while (1) { rt_thread_mdelay(1000); }

    return RT_EOK;
}

static void typec_irq_callback(void *args)
{
    RT_UNUSED(args);

    rt_kprintf("typec_irq_callback\n");
    app_gpio_ops.set_high(USB3_CONNECT);
    rt_timer_start(typec_timer);
}

static void typec_timer_callback(void *args)
{
    RT_UNUSED(args);

    rt_kprintf("typec_timer\n");
    app_gpio_ops.set_high(USB2_CONNECT);
}