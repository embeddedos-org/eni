/**
 * @file hal_stub.c
 * @brief Stub HAL backend for simulation and testing.
 *
 * All operations return success with zero/default data.
 * Useful for host-side unit tests and simulation environments.
 *
 * SPDX-License-Identifier: MIT
 */

#include "eni/hal.h"
#include <string.h>

/* ---- Stub state --------------------------------------------------------- */

static uint64_t stub_tick_counter = 0;

/* ---- Stub implementations ----------------------------------------------- */

static eni_hal_status_t stub_spi_transfer(void *ctx, uint8_t bus,
                                          const uint8_t *tx, uint8_t *rx,
                                          uint32_t len)
{
    (void)ctx; (void)bus; (void)tx;
    if (rx && len > 0) {
        memset(rx, 0, len);
    }
    return ENI_HAL_OK;
}

static eni_hal_status_t stub_i2c_read(void *ctx, uint8_t bus, uint8_t addr,
                                      uint8_t *data, uint32_t len)
{
    (void)ctx; (void)bus; (void)addr;
    if (data && len > 0) {
        memset(data, 0, len);
    }
    return ENI_HAL_OK;
}

static eni_hal_status_t stub_i2c_write(void *ctx, uint8_t bus, uint8_t addr,
                                       const uint8_t *data, uint32_t len)
{
    (void)ctx; (void)bus; (void)addr; (void)data; (void)len;
    return ENI_HAL_OK;
}

static eni_hal_status_t stub_gpio_set(void *ctx, uint32_t pin, bool value)
{
    (void)ctx; (void)pin; (void)value;
    return ENI_HAL_OK;
}

static eni_hal_status_t stub_gpio_get(void *ctx, uint32_t pin, bool *value)
{
    (void)ctx; (void)pin;
    if (value) *value = false;
    return ENI_HAL_OK;
}

static eni_hal_status_t stub_uart_send(void *ctx, uint8_t port,
                                       const uint8_t *data, uint32_t len)
{
    (void)ctx; (void)port; (void)data; (void)len;
    return ENI_HAL_OK;
}

static eni_hal_status_t stub_uart_recv(void *ctx, uint8_t port,
                                       uint8_t *data, uint32_t len,
                                       uint32_t timeout_ms)
{
    (void)ctx; (void)port; (void)timeout_ms;
    if (data && len > 0) {
        memset(data, 0, len);
    }
    return ENI_HAL_OK;
}

static eni_hal_status_t stub_adc_read(void *ctx, uint8_t channel,
                                      uint16_t *value)
{
    (void)ctx; (void)channel;
    if (value) *value = 0;
    return ENI_HAL_OK;
}

static eni_hal_status_t stub_delay_ms(void *ctx, uint32_t ms)
{
    (void)ctx;
    stub_tick_counter += (uint64_t)ms * 1000;
    return ENI_HAL_OK;
}

static uint64_t stub_get_tick_us(void *ctx)
{
    (void)ctx;
    return stub_tick_counter++;
}

/* ---- Public stub ops table ---------------------------------------------- */

const eni_hal_ops_t eni_hal_stub_ops = {
    .spi_transfer = stub_spi_transfer,
    .i2c_read     = stub_i2c_read,
    .i2c_write    = stub_i2c_write,
    .gpio_set     = stub_gpio_set,
    .gpio_get     = stub_gpio_get,
    .uart_send    = stub_uart_send,
    .uart_recv    = stub_uart_recv,
    .adc_read     = stub_adc_read,
    .delay_ms     = stub_delay_ms,
    .get_tick_us  = stub_get_tick_us,
    .name         = "stub"
};

/**
 * Convenience: register the stub backend on a HAL instance.
 */
eni_hal_status_t eni_hal_register_stub(eni_hal_t *hal)
{
    return eni_hal_register(hal, &eni_hal_stub_ops, NULL);
}
