/**
 * @file wireless_bci.c
 * @brief Wireless BCI provider implementation.
 *
 * SPDX-License-Identifier: MIT
 */

#include "wireless_bci.h"
#include <string.h>
#include <math.h>

/* ---- helpers ------------------------------------------------------------ */

static inline uint32_t ring_next(uint32_t idx, uint32_t size)
{
    return (idx + 1) % size;
}

static void decode_raw_to_samples(const eni_wbci_raw_pkt_t *pkt,
                                  float *samples,
                                  uint32_t num_channels)
{
    /*
     * Protocol: each channel encoded as 3-byte signed 24-bit big-endian.
     * Header: 1 byte seq + 1 byte status = 2 bytes offset.
     */
    uint32_t offset = 2;
    for (uint32_t ch = 0; ch < num_channels; ch++) {
        if (offset + 3 > pkt->len) {
            samples[ch] = 0.0f;
            continue;
        }
        int32_t raw = ((int32_t)pkt->raw[offset] << 16) |
                      ((int32_t)pkt->raw[offset + 1] << 8) |
                      ((int32_t)pkt->raw[offset + 2]);
        /* Sign extension from 24-bit */
        if (raw & 0x800000) {
            raw |= (int32_t)0xFF000000;
        }
        /* Convert to µV: ±187.5 µV full-scale for 24-bit ADC */
        samples[ch] = (float)raw * (187.5f / 8388607.0f);
        offset += 3;
    }
}

static uint8_t compute_checksum(const uint8_t *data, uint32_t len)
{
    uint8_t sum = 0;
    for (uint32_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return ~sum;
}

/* ---- Provider vtable wrappers ------------------------------------------- */

static int wbci_vtable_init(void *ctx, const void *config)
{
    return eni_wbci_init((eni_wbci_t *)ctx, (const eni_wbci_config_t *)config);
}

static int wbci_vtable_connect(void *ctx)
{
    return eni_wbci_connect((eni_wbci_t *)ctx);
}

static int wbci_vtable_poll(void *ctx, float *samples,
                            uint32_t *num_channels, uint64_t *timestamp)
{
    return eni_wbci_poll((eni_wbci_t *)ctx, samples, num_channels, timestamp);
}

static int wbci_vtable_disconnect(void *ctx)
{
    return eni_wbci_disconnect((eni_wbci_t *)ctx);
}

static int wbci_vtable_deinit(void *ctx)
{
    return eni_wbci_deinit((eni_wbci_t *)ctx);
}

static const eni_wbci_ops_t wbci_ops = {
    .init       = wbci_vtable_init,
    .connect    = wbci_vtable_connect,
    .poll       = wbci_vtable_poll,
    .disconnect = wbci_vtable_disconnect,
    .deinit     = wbci_vtable_deinit,
    .name       = "wireless_bci"
};

/* ---- Public API --------------------------------------------------------- */

int eni_wbci_init(eni_wbci_t *bci, const eni_wbci_config_t *config)
{
    if (!bci || !config) return ENI_WBCI_ERR_NULL;
    if (config->num_channels == 0 ||
        config->num_channels > ENI_WBCI_MAX_CHANNELS) return ENI_WBCI_ERR_PARAM;

    memset(bci, 0, sizeof(*bci));
    memcpy(&bci->config, config, sizeof(*config));

    if (bci->config.sample_rate_hz == 0) {
        bci->config.sample_rate_hz = ENI_WBCI_SAMPLE_RATE_HZ;
    }
    if (bci->config.connect_timeout_ms == 0) {
        bci->config.connect_timeout_ms = 5000;
    }
    if (bci->config.poll_interval_ms == 0) {
        bci->config.poll_interval_ms = 4; /* 250 Hz */
    }

    bci->state = ENI_WBCI_STATE_IDLE;
    bci->initialised = true;
    return ENI_WBCI_OK;
}

int eni_wbci_connect(eni_wbci_t *bci)
{
    if (!bci) return ENI_WBCI_ERR_NULL;
    if (!bci->initialised) return ENI_WBCI_ERR_INIT;

    if (bci->state == ENI_WBCI_STATE_CONNECTED ||
        bci->state == ENI_WBCI_STATE_STREAMING) {
        return ENI_WBCI_ERR_BUSY;
    }

    bci->state = ENI_WBCI_STATE_SCANNING;

    /*
     * In a real implementation this would:
     * 1. Initiate BLE/WiFi scan via HAL
     * 2. Match device_addr
     * 3. Establish connection
     * 4. Negotiate data stream parameters
     *
     * For now, transition directly to connected (stub for simulation).
     */

    bci->state = ENI_WBCI_STATE_CONNECTED;
    bci->rssi = 80;
    bci->link_quality = 95;

    return ENI_WBCI_OK;
}

int eni_wbci_poll(eni_wbci_t *bci, float *samples,
                  uint32_t *num_channels, uint64_t *timestamp)
{
    if (!bci || !samples) return ENI_WBCI_ERR_NULL;
    if (!bci->initialised) return ENI_WBCI_ERR_INIT;

    if (bci->state != ENI_WBCI_STATE_CONNECTED &&
        bci->state != ENI_WBCI_STATE_STREAMING) {
        return ENI_WBCI_ERR_INIT;
    }

    bci->state = ENI_WBCI_STATE_STREAMING;

    /* Check receive ring buffer */
    if (bci->rx_count > 0) {
        eni_wbci_raw_pkt_t *pkt = &bci->rx_ring[bci->rx_tail];

        /* Verify checksum */
        if (pkt->len > 1) {
            uint8_t expected = compute_checksum(pkt->raw, pkt->len - 1);
            if (pkt->raw[pkt->len - 1] != expected) {
                bci->crc_errors++;
                bci->rx_tail = ring_next(bci->rx_tail, ENI_WBCI_RX_RING_SIZE);
                bci->rx_count--;
                return ENI_WBCI_ERR_IO;
            }
        }

        decode_raw_to_samples(pkt, bci->samples, bci->config.num_channels);
        bci->sample_seq++;
        bci->pkts_received++;

        bci->rx_tail = ring_next(bci->rx_tail, ENI_WBCI_RX_RING_SIZE);
        bci->rx_count--;

        memcpy(samples, bci->samples,
               bci->config.num_channels * sizeof(float));
        if (num_channels) *num_channels = bci->config.num_channels;
        if (timestamp)    *timestamp = pkt->timestamp;

        return ENI_WBCI_OK;
    }

    /* No data available — return last known samples */
    memcpy(samples, bci->samples,
           bci->config.num_channels * sizeof(float));
    if (num_channels) *num_channels = bci->config.num_channels;
    if (timestamp)    *timestamp = 0;

    return ENI_WBCI_ERR_TIMEOUT;
}

int eni_wbci_disconnect(eni_wbci_t *bci)
{
    if (!bci) return ENI_WBCI_ERR_NULL;
    if (!bci->initialised) return ENI_WBCI_ERR_INIT;

    bci->state = ENI_WBCI_STATE_IDLE;
    bci->rx_head  = 0;
    bci->rx_tail  = 0;
    bci->rx_count = 0;

    return ENI_WBCI_OK;
}

int eni_wbci_deinit(eni_wbci_t *bci)
{
    if (!bci) return ENI_WBCI_ERR_NULL;

    if (bci->state == ENI_WBCI_STATE_CONNECTED ||
        bci->state == ENI_WBCI_STATE_STREAMING) {
        eni_wbci_disconnect(bci);
    }

    memset(bci, 0, sizeof(*bci));
    return ENI_WBCI_OK;
}

const eni_wbci_ops_t *eni_wbci_get_ops(void)
{
    return &wbci_ops;
}
