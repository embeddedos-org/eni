/**
 * @file template_provider.c
 * @brief Template provider implementation for third-party development.
 *
 * Copy this file and replace every "template" with your provider name.
 * Fill in the TODO sections with device-specific logic.
 *
 * SPDX-License-Identifier: MIT
 */

#include "template_provider.h"
#include <string.h>

/* ---- Provider vtable wrappers ------------------------------------------- */

static int tpl_vtable_init(void *ctx, const void *config)
{
    return eni_template_init((eni_template_t *)ctx,
                             (const eni_template_config_t *)config);
}

static int tpl_vtable_connect(void *ctx)
{
    return eni_template_connect((eni_template_t *)ctx);
}

static int tpl_vtable_poll(void *ctx, float *samples,
                           uint32_t *num_channels, uint64_t *timestamp)
{
    return eni_template_poll((eni_template_t *)ctx, samples,
                             num_channels, timestamp);
}

static int tpl_vtable_disconnect(void *ctx)
{
    return eni_template_disconnect((eni_template_t *)ctx);
}

static int tpl_vtable_deinit(void *ctx)
{
    return eni_template_deinit((eni_template_t *)ctx);
}

static const eni_template_ops_t template_ops = {
    .init       = tpl_vtable_init,
    .connect    = tpl_vtable_connect,
    .poll       = tpl_vtable_poll,
    .disconnect = tpl_vtable_disconnect,
    .deinit     = tpl_vtable_deinit,
    .name       = "template"
};

/* ---- Public API --------------------------------------------------------- */

int eni_template_init(eni_template_t *dev, const eni_template_config_t *config)
{
    if (!dev || !config) return -1;
    if (config->num_channels == 0 ||
        config->num_channels > ENI_TEMPLATE_MAX_CHANNELS) return -2;

    memset(dev, 0, sizeof(*dev));
    memcpy(&dev->config, config, sizeof(*config));

    if (dev->config.sample_rate_hz == 0) {
        dev->config.sample_rate_hz = ENI_TEMPLATE_SAMPLE_RATE;
    }

    /* TODO: perform device-specific initialisation here.
     *  - Configure communication interface (SPI, I2C, UART) via HAL
     *  - Verify device ID / firmware version
     *  - Set up default registers
     */

    dev->initialised = true;
    return 0;
}

int eni_template_connect(eni_template_t *dev)
{
    if (!dev) return -1;
    if (!dev->initialised) return -3;
    if (dev->connected) return -4;  /* Already connected */

    /* TODO: establish connection to your device.
     *  - Power on sequence
     *  - Handshake / authentication
     *  - Configure streaming parameters
     *  - Start data acquisition
     */

    dev->connected = true;
    return 0;
}

int eni_template_poll(eni_template_t *dev, float *samples,
                      uint32_t *num_channels, uint64_t *timestamp)
{
    if (!dev || !samples) return -1;
    if (!dev->initialised) return -3;
    if (!dev->connected) return -4;

    /* TODO: read samples from your device.
     *  - Read raw data from SPI/I2C/UART via HAL
     *  - Parse device-specific packet format
     *  - Convert raw ADC values to float (µV)
     *  - Handle packet sequencing and loss detection
     *  - Apply device-specific calibration
     */

    /* Placeholder: return zeros */
    memcpy(samples, dev->samples,
           dev->config.num_channels * sizeof(float));

    if (num_channels) *num_channels = dev->config.num_channels;
    if (timestamp)    *timestamp = 0;  /* TODO: provide real timestamp */

    return 0;
}

int eni_template_disconnect(eni_template_t *dev)
{
    if (!dev) return -1;
    if (!dev->initialised) return -3;

    /* TODO: gracefully disconnect from device.
     *  - Stop data streaming
     *  - Power down sequence
     *  - Release communication interface
     */

    dev->connected = false;
    return 0;
}

int eni_template_deinit(eni_template_t *dev)
{
    if (!dev) return -1;

    if (dev->connected) {
        eni_template_disconnect(dev);
    }

    /* TODO: release any device-specific resources.
     *  - De-initialise HAL peripherals
     *  - Release GPIO pins
     */

    memset(dev, 0, sizeof(*dev));
    return 0;
}

const eni_template_ops_t *eni_template_get_ops(void)
{
    return &template_ops;
}
