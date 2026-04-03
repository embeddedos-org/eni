/**
 * @file template_provider.h
 * @brief Template provider for third-party eNI development.
 *
 * Copy this file and its .c counterpart to create a new provider.
 * Replace "template" with your provider name throughout.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ENI_TEMPLATE_PROVIDER_H
#define ENI_TEMPLATE_PROVIDER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Constants — TODO: adjust for your device
 * ------------------------------------------------------------------------ */
#define ENI_TEMPLATE_MAX_CHANNELS   8
#define ENI_TEMPLATE_SAMPLE_RATE  256

/* ---------------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------------ */

typedef struct {
    uint32_t num_channels;
    uint32_t sample_rate_hz;
    /* TODO: add device-specific configuration fields */
} eni_template_config_t;

typedef struct {
    eni_template_config_t config;
    float                 samples[ENI_TEMPLATE_MAX_CHANNELS];
    bool                  connected;
    bool                  initialised;
    /* TODO: add device-specific state */
} eni_template_t;

/** Provider vtable (compatible with eni_provider_ops_t). */
typedef struct {
    int (*init)(void *ctx, const void *config);
    int (*connect)(void *ctx);
    int (*poll)(void *ctx, float *samples, uint32_t *num_channels,
                uint64_t *timestamp);
    int (*disconnect)(void *ctx);
    int (*deinit)(void *ctx);
    const char *name;
} eni_template_ops_t;

/* ---------------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------------ */

int eni_template_init(eni_template_t *dev, const eni_template_config_t *config);
int eni_template_connect(eni_template_t *dev);
int eni_template_poll(eni_template_t *dev, float *samples,
                      uint32_t *num_channels, uint64_t *timestamp);
int eni_template_disconnect(eni_template_t *dev);
int eni_template_deinit(eni_template_t *dev);

/** Get the provider ops vtable. */
const eni_template_ops_t *eni_template_get_ops(void);

#ifdef __cplusplus
}
#endif

#endif /* ENI_TEMPLATE_PROVIDER_H */
