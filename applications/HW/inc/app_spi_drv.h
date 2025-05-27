#ifndef __APP_SPI_DRV_H__
#define __APP_SPI_DRV_H__

#include <board.h>
#include <rtdevice.h>

typedef struct
{
    struct rt_spi_device *w25q128;
    rt_sem_t sem;
} spi1_dev_t;

typedef struct
{
    struct rt_spi_device *ext_device;
    rt_sem_t sem;
} spi2_dev_t;

typedef struct
{
    struct rt_spi_device *st7735;
    rt_sem_t sem;
} spi3_dev_t;

typedef struct
{
    spi1_dev_t spi1_dev;
    spi2_dev_t spi2_dev;
    spi3_dev_t spi3_dev;

    rt_ssize_t (*transfer)(struct rt_spi_device *device, const void *send_buf, void *recv_buf, rt_size_t length);
    rt_err_t (*write_then_read)(struct rt_spi_device *device, const void *send_buf, rt_size_t send_length,
                                void *recv_buf, rt_size_t recv_length);
    rt_err_t (*write_then_write)(struct rt_spi_device *device, const void *send_buf1, rt_size_t send_length1,
                                 const void *send_buf2, rt_size_t send_length2);
} spi_handle_t;
extern spi_handle_t spi_handle;

#endif /* __APP_SPI_DRV_H__ */
