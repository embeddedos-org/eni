/**
 * @file hal.h
 * @brief Hardware Abstraction Layer for eNI.
 *
 * Function-pointer based HAL with registry for SPI, I2C, GPIO, UART, ADC.
 * MCU-safe: no dynamic allocation.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ENI_HAL_H
#define ENI_HAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------ */
#define ENI_HAL_MAX_BACKENDS    4

/* ---------------------------------------------------------------------------
 * Status
 * ------------------------------------------------------------------------ */
typedef enum {
    ENI_HAL_OK = 0,
    ENI_HAL_ERR_NULL,
    ENI_HAL_ERR_PARAM,
    ENI_HAL_ERR_IO,
    ENI_HAL_ERR_BUSY,
    ENI_HAL_ERR_TIMEOUT,
    ENI_HAL_ERR_NOT_INIT,
    ENI_HAL_ERR_FULL
} eni_hal_status_t;

/* ---------------------------------------------------------------------------
 * Hardware operation function pointers
 * ------------------------------------------------------------------------ */
typedef struct {
    /* SPI */
    eni_hal_status_t (*spi_transfer)(void *ctx, uint8_t bus,
                                     const uint8_t *tx, uint8_t *rx,
                                     uint32_t len);

    /* I2C */
    eni_hal_status_t (*i2c_read)(void *ctx, uint8_t bus, uint8_t addr,
                                 uint8_t *data, uint32_t len);
    eni_hal_status_t (*i2c_write)(void *ctx, uint8_t bus, uint8_t addr,
                                  const uint8_t *data, uint32_t len);

    /* GPIO */
    eni_hal_status_t (*gpio_set)(void *ctx, uint32_t pin, bool value);
    eni_hal_status_t (*gpio_get)(void *ctx, uint32_t pin, bool *value);

    /* UART */
    eni_hal_status_t (*uart_send)(void *ctx, uint8_t port,
                                  const uint8_t *data, uint32_t len);
    eni_hal_status_t (*uart_recv)(void *ctx, uint8_t port,
                                  uint8_t *data, uint32_t len,
                                  uint32_t timeout_ms);

    /* ADC */
    eni_hal_status_t (*adc_read)(void *ctx, uint8_t channel,
                                 uint16_t *value);

    /* Timer / delay */
    eni_hal_status_t (*delay_ms)(void *ctx, uint32_t ms);
    uint64_t         (*get_tick_us)(void *ctx);

    const char *name;
} eni_hal_ops_t;

/** HAL instance. */
typedef struct {
    const eni_hal_ops_t *ops;
    void                *ctx;       /**< Platform-specific context. */
    bool                 active;
} eni_hal_backend_t;

/** HAL registry (supports multiple backends). */
typedef struct {
    eni_hal_backend_t  backends[ENI_HAL_MAX_BACKENDS];
    uint32_t           num_backends;
    uint32_t           active_idx;
    bool               initialised;
} eni_hal_t;

/* ---------------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------------ */

eni_hal_status_t eni_hal_init(eni_hal_t *hal);

eni_hal_status_t eni_hal_register(eni_hal_t *hal,
                                  const eni_hal_ops_t *ops,
                                  void *ctx);

eni_hal_status_t eni_hal_select(eni_hal_t *hal, uint32_t index);

/* Convenience wrappers */
eni_hal_status_t eni_hal_spi_transfer(eni_hal_t *hal, uint8_t bus,
                                      const uint8_t *tx, uint8_t *rx,
                                      uint32_t len);

eni_hal_status_t eni_hal_i2c_read(eni_hal_t *hal, uint8_t bus, uint8_t addr,
                                   uint8_t *data, uint32_t len);

eni_hal_status_t eni_hal_i2c_write(eni_hal_t *hal, uint8_t bus, uint8_t addr,
                                    const uint8_t *data, uint32_t len);

eni_hal_status_t eni_hal_gpio_set(eni_hal_t *hal, uint32_t pin, bool value);
eni_hal_status_t eni_hal_gpio_get(eni_hal_t *hal, uint32_t pin, bool *value);

eni_hal_status_t eni_hal_uart_send(eni_hal_t *hal, uint8_t port,
                                    const uint8_t *data, uint32_t len);
eni_hal_status_t eni_hal_uart_recv(eni_hal_t *hal, uint8_t port,
                                    uint8_t *data, uint32_t len,
                                    uint32_t timeout_ms);

eni_hal_status_t eni_hal_adc_read(eni_hal_t *hal, uint8_t channel,
                                   uint16_t *value);

eni_hal_status_t eni_hal_delay_ms(eni_hal_t *hal, uint32_t ms);
uint64_t         eni_hal_get_tick_us(eni_hal_t *hal);

#ifdef __cplusplus
}
#endif

#endif /* ENI_HAL_H */
