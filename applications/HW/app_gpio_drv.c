#include "rtthread.h"

#include "app_gpio_drv.h"

/*===========================================================================*/
app_gpio_ops_t app_gpio_ops;
/*===========================================================================*/

static void io_init(void)
{
    rt_pin_mode(USB3_CONNECT, PIN_MODE_OUTPUT);
    rt_pin_write(USB3_CONNECT, PIN_LOW);

    rt_pin_mode(USB2_CONNECT, PIN_MODE_OUTPUT);
    rt_pin_write(USB2_CONNECT, PIN_LOW);

    rt_pin_mode(USB_EN, PIN_MODE_OUTPUT);
    rt_pin_write(USB_EN, PIN_LOW);

    rt_pin_mode(TYPEC_DETECT, PIN_MODE_INPUT);
}

static void io_high(app_gpio_pin_t pin)
{
    rt_pin_write(pin, PIN_HIGH);
}

static void io_low(app_gpio_pin_t pin)
{
    rt_pin_write(pin, PIN_LOW);
}

static void io_toggle(app_gpio_pin_t pin)
{
    rt_pin_write(pin, !rt_pin_read(pin));
}

static void io_enable(app_gpio_pin_t pin)
{
    rt_pin_write(pin, PIN_HIGH);
}

static void io_disable(app_gpio_pin_t pin)
{
    rt_pin_write(pin, PIN_LOW);
}

static int io_read(app_gpio_pin_t pin)
{
    return rt_pin_read(pin);
}

static int rt_app_gpio_init(void)
{
    io_init();
    app_gpio_ops.set_high = io_high;
    app_gpio_ops.set_low = io_low;
    app_gpio_ops.toggle = io_toggle;
    app_gpio_ops.enable = io_enable;
    app_gpio_ops.disable = io_disable;
    app_gpio_ops.read = io_read;
    return 0;
}
INIT_PREV_EXPORT(rt_app_gpio_init);