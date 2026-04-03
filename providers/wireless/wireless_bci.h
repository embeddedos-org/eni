/**
 * @file wireless_bci.h
 * @brief Wireless BCI provider for eNI.
 *
 * Implements the eni_provider_ops_t vtable for wireless BCI headsets.
 * Supports BLE/WiFi transport via HAL.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ENI_WIRELESS_BCI_H
#define ENI_WIRELESS_BCI_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------ */
#define ENI_WBCI_MAX_CHANNELS       32
#define ENI_WBCI_SAMPLE_RATE_HZ    250
#define ENI_WBCI_PACKET_SIZE       256
#define ENI_WBCI_RX_RING_SIZE       16

/* ---------------------------------------------------------------------------
 * Status (maps to eni_status_t values)
 * ------------------------------------------------------------------------ */
typedef enum {
    ENI_WBCI_OK = 0,
    ENI_WBCI_ERR_INIT,
    ENI_WBCI_ERR_NULL,
    ENI_WBCI_ERR_PARAM,
    ENI_WBCI_ERR_TIMEOUT,
    ENI_WBCI_ERR_BUSY,
    ENI_WBCI_ERR_OVERFLOW,
    ENI_WBCI_ERR_NOT_FOUND,
    ENI_WBCI_ERR_IO
} eni_wbci_status_t;

/* ---------------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------------ */

typedef enum {
    ENI_WBCI_TRANSPORT_BLE,
    ENI_WBCI_TRANSPORT_WIFI,
    ENI_WBCI_TRANSPORT_SERIAL
} eni_wbci_transport_t;

typedef struct {
    eni_wbci_transport_t transport;
    uint32_t            num_channels;
    uint32_t            sample_rate_hz;
    uint8_t             device_addr[6];     /**< MAC or BLE address. */
    uint32_t            connect_timeout_ms;
    uint32_t            poll_interval_ms;
} eni_wbci_config_t;

typedef enum {
    ENI_WBCI_STATE_IDLE,
    ENI_WBCI_STATE_SCANNING,
    ENI_WBCI_STATE_CONNECTING,
    ENI_WBCI_STATE_CONNECTED,
    ENI_WBCI_STATE_STREAMING,
    ENI_WBCI_STATE_ERROR
} eni_wbci_state_t;

/** Raw packet from wireless device. */
typedef struct {
    uint8_t     raw[ENI_WBCI_PACKET_SIZE];
    uint32_t    len;
    uint64_t    timestamp;
    uint8_t     seq;
} eni_wbci_raw_pkt_t;

/** Wireless BCI provider instance. */
typedef struct {
    eni_wbci_config_t   config;
    eni_wbci_state_t    state;

    /* Receive ring buffer */
    eni_wbci_raw_pkt_t  rx_ring[ENI_WBCI_RX_RING_SIZE];
    uint32_t            rx_head;
    uint32_t            rx_tail;
    uint32_t            rx_count;

    /* Decoded sample buffer */
    float               samples[ENI_WBCI_MAX_CHANNELS];
    uint32_t            sample_seq;

    /* Statistics */
    uint32_t            pkts_received;
    uint32_t            pkts_dropped;
    uint32_t            crc_errors;

    /* Link quality (0-100) */
    uint8_t             rssi;
    uint8_t             link_quality;

    bool                initialised;
} eni_wbci_t;

/* Provider vtable (compatible with eni_provider_ops_t) */
typedef struct {
    int (*init)(void *ctx, const void *config);
    int (*connect)(void *ctx);
    int (*poll)(void *ctx, float *samples, uint32_t *num_channels,
                uint64_t *timestamp);
    int (*disconnect)(void *ctx);
    int (*deinit)(void *ctx);
    const char *name;
} eni_wbci_ops_t;

/* ---------------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------------ */

int eni_wbci_init(eni_wbci_t *bci, const eni_wbci_config_t *config);
int eni_wbci_connect(eni_wbci_t *bci);
int eni_wbci_poll(eni_wbci_t *bci, float *samples,
                  uint32_t *num_channels, uint64_t *timestamp);
int eni_wbci_disconnect(eni_wbci_t *bci);
int eni_wbci_deinit(eni_wbci_t *bci);

/** Get the provider ops vtable. */
const eni_wbci_ops_t *eni_wbci_get_ops(void);

#ifdef __cplusplus
}
#endif

#endif /* ENI_WIRELESS_BCI_H */
