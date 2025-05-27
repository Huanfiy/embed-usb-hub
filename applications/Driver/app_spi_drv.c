#include "rtdevice.h"
#include "rtthread.h"

#include "drv_spi.h"

#include "app_gpio_drv.h"
#include "app_spi_drv.h"

spi_handle_t spi_handle;

// SPI1设备定义 - 存储器
#define W25Q128_SPI_DEVICE_NAME "spi10"

// SPI2设备定义 - 扩展设备
#define EXT_SPI_DEVICE_NAME "spi20"

// SPI3设备定义 - 显示屏
#define ST7735_SPI_DEVICE_NAME "spi30"

// CS引脚定义
#define W25Q128_SPI_CS GET_PIN(A, 4)  // PA4
#define EXT_SPI_CS     GET_PIN(B, 12) // PB12
#define ST7735_SPI_CS  GET_PIN(A, 15) // PA15

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    RT_ASSERT(hspi != RT_NULL);
    rt_sem_release((rt_sem_t)((struct stm32_spi *)hspi)->spi_bus.owner->user_data);
}
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    RT_ASSERT(hspi != RT_NULL);
    rt_sem_release((rt_sem_t)((struct stm32_spi *)hspi)->spi_bus.owner->user_data);
}
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    RT_ASSERT(hspi != RT_NULL);
    rt_sem_release((rt_sem_t)((struct stm32_spi *)hspi)->spi_bus.owner->user_data);
}

static rt_err_t spi_device_init(void)
{
    rt_err_t result = RT_EOK;

    // 初始化SPI1设备 - 传感器
    result = rt_hw_spi_device_attach("spi1", W25Q128_SPI_DEVICE_NAME, GPIOA, W25Q128_SPI_CS);
    if (result != RT_EOK)
    {
        rt_kprintf("W25Q128 SPI device attach failed!\n");
        return result;
    }

    // 初始化SPI2设备 - 存储器
    result = rt_hw_spi_device_attach("spi2", EXT_SPI_DEVICE_NAME, GPIOB, EXT_SPI_CS);
    if (result != RT_EOK)
    {
        rt_kprintf("EXT SPI device attach failed!\n");
        return result;
    }

    // 初始化SPI3设备 - 显示屏
    result = rt_hw_spi_device_attach("spi3", ST7735_SPI_DEVICE_NAME, GPIOA, ST7735_SPI_CS);
    if (result != RT_EOK)
    {
        rt_kprintf("ST7735 SPI device attach failed!\n");
        return result;
    }

    return result;
}

static rt_err_t spi_device_find(void)
{
    // 查找SPI1设备
    spi_handle.spi1_dev.w25q128 = (struct rt_spi_device *)rt_device_find(W25Q128_SPI_DEVICE_NAME);
    if (spi_handle.spi1_dev.w25q128 == RT_NULL)
    {
        rt_kprintf("W25Q128 SPI device not found!\n");
        return -RT_ERROR;
    }

    // 查找SPI2设备
    spi_handle.spi2_dev.ext_device = (struct rt_spi_device *)rt_device_find(EXT_SPI_DEVICE_NAME);
    if (spi_handle.spi2_dev.ext_device == RT_NULL)
    {
        rt_kprintf("EXT SPI device not found!\n");
        return -RT_ERROR;
    }

    // 查找SPI3设备
    spi_handle.spi3_dev.st7735 = (struct rt_spi_device *)rt_device_find(ST7735_SPI_DEVICE_NAME);
    if (spi_handle.spi3_dev.st7735 == RT_NULL)
    {
        rt_kprintf("ST7735 SPI device not found!\n");
        return -RT_ERROR;
    }

    return RT_EOK;
}

static rt_err_t spi_device_config(void)
{
    struct rt_spi_configuration cfg;

    // 配置SPI1设备 - 存储器 (高速模式)
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_3 | RT_SPI_MSB;
    cfg.max_hz = 10 * 1000 * 1000; // 10MHz

    rt_spi_configure(spi_handle.spi1_dev.w25q128, &cfg);

    // 配置SPI2设备 - 扩展设备 (中速模式)
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = 20 * 1000 * 1000; // 20MHz

    rt_spi_configure(spi_handle.spi2_dev.ext_device, &cfg);

    // 配置SPI3设备 - 显示屏 (中速模式)
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = 36 * 1000 * 1000; // 36MHz

    rt_spi_configure(spi_handle.spi3_dev.st7735, &cfg);

    return RT_EOK;
}

static rt_err_t spi_sem_init(void)
{
    // 创建SPI1信号量
    spi_handle.spi1_dev.sem = rt_sem_create("spi1_sem", 0, RT_IPC_FLAG_FIFO);
    if (spi_handle.spi1_dev.sem == RT_NULL)
    {
        rt_kprintf("SPI1 semaphore create failed!\n");
        return -RT_ERROR;
    }

    // 创建SPI2信号量
    spi_handle.spi2_dev.sem = rt_sem_create("spi2_sem", 0, RT_IPC_FLAG_FIFO);
    if (spi_handle.spi2_dev.sem == RT_NULL)
    {
        rt_kprintf("SPI2 semaphore create failed!\n");
        return -RT_ERROR;
    }

    // 创建SPI3信号量
    spi_handle.spi3_dev.sem = rt_sem_create("spi3_sem", 0, RT_IPC_FLAG_FIFO);
    if (spi_handle.spi3_dev.sem == RT_NULL)
    {
        rt_kprintf("SPI3 semaphore create failed!\n");
        return -RT_ERROR;
    }

    return RT_EOK;
}

static int app_spi_init(void)
{
    rt_err_t result = RT_EOK;

    // 初始化SPI设备
    result = spi_device_init();
    if (result != RT_EOK)
    {
        rt_kprintf("SPI device init failed!\n");
        return result;
    }

    // 查找SPI设备
    result = spi_device_find();
    if (result != RT_EOK)
    {
        rt_kprintf("SPI device find failed!\n");
        return result;
    }

    // 配置SPI设备
    result = spi_device_config();
    if (result != RT_EOK)
    {
        rt_kprintf("SPI device config failed!\n");
        return result;
    }

    // 初始化信号量
    result = spi_sem_init();
    if (result != RT_EOK)
    {
        rt_kprintf("SPI semaphore init failed!\n");
        return result;
    }

    spi_handle.transfer = rt_spi_transfer;
    spi_handle.write_then_read = rt_spi_send_then_recv;
    spi_handle.write_then_write = rt_spi_send_then_send;

    rt_kprintf("SPI init success!\n");
    return RT_EOK;
}

INIT_DEVICE_EXPORT(app_spi_init);

static void app_spi_test(int argc, char **argv)
{
    if (argc < 2)
    {
        rt_kprintf("Usage: spi_test <device_name>\n");
        rt_kprintf("Available devices:\n");
        rt_kprintf("  icm42688  - ICM42688 sensor on SPI1\n");
        rt_kprintf("  ms5611    - MS5611 sensor on SPI1\n");
        rt_kprintf("  icm20602  - ICM20602 sensor on SPI1\n");
        rt_kprintf("  fm25v05   - FM25V05 memory on SPI2\n");
        rt_kprintf("  st7735    - ST7735 display on SPI3\n");
        rt_kprintf("  st7789    - ST7789 display on SPI3\n");
        return;
    }

    struct rt_spi_device *spi_device = RT_NULL;
    rt_uint8_t tx_data = 0x00; // WHO_AM_I register for sensors
    rt_uint8_t rx_data = 0x00;

    if (rt_strcmp(argv[1], "w25q128") == 0)
    {
        spi_device = spi_handle.spi1_dev.w25q128;
        tx_data = 0x00; // TODO: 待补充
    }
    else if (rt_strcmp(argv[1], "ext_device") == 0)
    {
        spi_device = spi_handle.spi2_dev.ext_device;
        tx_data = 0x00; // TODO: 待补充
    }
    else if (rt_strcmp(argv[1], "st7735") == 0)
    {
        spi_device = spi_handle.spi3_dev.st7735;
        tx_data = 0x00; // TODO: 待补充
    }
    else
    {
        rt_kprintf("Unknown device: %s\n", argv[1]);
        return;
    }

    if (spi_device == RT_NULL)
    {
        rt_kprintf("SPI device %s not found!\n", argv[1]);
        return;
    }

    // 执行SPI传输测试
    rt_ssize_t result = spi_handle.transfer(spi_device, &tx_data, &rx_data, 1);
    if (result == 1)
    {
        rt_kprintf("SPI test for %s successful!\n", argv[1]);
        rt_kprintf("TX: 0x%02X, RX: 0x%02X\n", tx_data, rx_data);
    }
    else
    {
        rt_kprintf("SPI test for %s failed!\n", argv[1]);
    }
}

MSH_CMD_EXPORT(app_spi_test, SPI device test command);