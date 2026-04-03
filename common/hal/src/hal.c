/**
 * @file hal.c
 * @brief HAL registry and dispatch implementation.
 *
 * SPDX-License-Identifier: MIT
 */

#include "eni/hal.h"
#include <string.h>

/* ---- helpers ------------------------------------------------------------ */

static inline const eni_hal_backend_t *active_backend(const eni_hal_t *hal)
{
    if (!hal || !hal->initialised) return NULL;
    if (hal->active_idx >= hal->num_backends) return NULL;
    return &hal->backends[hal->active_idx];
}

#define HAL_DISPATCH(hal, op, ...)                                  \
    do {                                                            \
        const eni_hal_backend_t *be = active_backend(hal);          \
        if (!be) return ENI_HAL_ERR_NOT_INIT;                       \
        if (!be->ops || !be->ops->op) return ENI_HAL_ERR_NULL;      \
        return be->ops->op(be->ctx, __VA_ARGS__);                   \
    } while (0)

/* ---- Public API --------------------------------------------------------- */

eni_hal_status_t eni_hal_init(eni_hal_t *hal)
{
    if (!hal) return ENI_HAL_ERR_NULL;
    memset(hal, 0, sizeof(*hal));
    hal->initialised = true;
    return ENI_HAL_OK;
}

eni_hal_status_t eni_hal_register(eni_hal_t *hal,
                                  const eni_hal_ops_t *ops,
                                  void *ctx)
{
    if (!hal || !ops) return ENI_HAL_ERR_NULL;
    if (!hal->initialised) return ENI_HAL_ERR_NOT_INIT;
    if (hal->num_backends >= ENI_HAL_MAX_BACKENDS) return ENI_HAL_ERR_FULL;

    eni_hal_backend_t *be = &hal->backends[hal->num_backends];
    be->ops    = ops;
    be->ctx    = ctx;
    be->active = true;

    /* First registered backend becomes active */
    if (hal->num_backends == 0) {
        hal->active_idx = 0;
    }

    hal->num_backends++;
    return ENI_HAL_OK;
}

eni_hal_status_t eni_hal_select(eni_hal_t *hal, uint32_t index)
{
    if (!hal) return ENI_HAL_ERR_NULL;
    if (!hal->initialised) return ENI_HAL_ERR_NOT_INIT;
    if (index >= hal->num_backends) return ENI_HAL_ERR_PARAM;

    hal->active_idx = index;
    return ENI_HAL_OK;
}

eni_hal_status_t eni_hal_spi_transfer(eni_hal_t *hal, uint8_t bus,
                                      const uint8_t *tx, uint8_t *rx,
                                      uint32_t len)
{
    HAL_DISPATCH(hal, spi_transfer, bus, tx, rx, len);
}

eni_hal_status_t eni_hal_i2c_read(eni_hal_t *hal, uint8_t bus, uint8_t addr,
                                   uint8_t *data, uint32_t len)
{
    HAL_DISPATCH(hal, i2c_read, bus, addr, data, len);
}

eni_hal_status_t eni_hal_i2c_write(eni_hal_t *hal, uint8_t bus, uint8_t addr,
                                    const uint8_t *data, uint32_t len)
{
    HAL_DISPATCH(hal, i2c_write, bus, addr, data, len);
}

eni_hal_status_t eni_hal_gpio_set(eni_hal_t *hal, uint32_t pin, bool value)
{
    HAL_DISPATCH(hal, gpio_set, pin, value);
}

eni_hal_status_t eni_hal_gpio_get(eni_hal_t *hal, uint32_t pin, bool *value)
{
    HAL_DISPATCH(hal, gpio_get, pin, value);
}

eni_hal_status_t eni_hal_uart_send(eni_hal_t *hal, uint8_t port,
                                    const uint8_t *data, uint32_t len)
{
    HAL_DISPATCH(hal, uart_send, port, data, len);
}

eni_hal_status_t eni_hal_uart_recv(eni_hal_t *hal, uint8_t port,
                                    uint8_t *data, uint32_t len,
                                    uint32_t timeout_ms)
{
    HAL_DISPATCH(hal, uart_recv, port, data, len, timeout_ms);
}

eni_hal_status_t eni_hal_adc_read(eni_hal_t *hal, uint8_t channel,
                                   uint16_t *value)
{
    HAL_DISPATCH(hal, adc_read, channel, value);
}

eni_hal_status_t eni_hal_delay_ms(eni_hal_t *hal, uint32_t ms)
{
    HAL_DISPATCH(hal, delay_ms, ms);
}

uint64_t eni_hal_get_tick_us(eni_hal_t *hal)
{
    const eni_hal_backend_t *be = active_backend(hal);
    if (!be || !be->ops || !be->ops->get_tick_us) return 0;
    return be->ops->get_tick_us(be->ctx);
}
