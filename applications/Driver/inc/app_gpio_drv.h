#ifndef __APP_GPIO_DRV_H__
#define __APP_GPIO_DRV_H__

#include <board.h>
#include <rtdevice.h>

typedef enum
{
    // 电源控制类
    USB3_CONNECT = GET_PIN(C, 6),
    USB2_CONNECT = GET_PIN(C, 7),
    USB_EN = GET_PIN(B, 1),

    // 状态检测类
    TYPEC_DETECT = GET_PIN(B, 2),
} app_gpio_pin_t;

typedef struct
{
    void (*set_high)(app_gpio_pin_t pin);
    void (*set_low)(app_gpio_pin_t pin);
    void (*toggle)(app_gpio_pin_t pin);
    void (*enable)(app_gpio_pin_t pin);
    void (*disable)(app_gpio_pin_t pin);
    int (*read)(app_gpio_pin_t pin);
} app_gpio_ops_t;

extern app_gpio_ops_t app_gpio_ops;

#endif