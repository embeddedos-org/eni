// SPDX-License-Identifier: MIT
// eNI EIPC Streaming — Real-time neural data streaming to cloud/edge

#ifndef ENI_EIPC_STREAM_H
#define ENI_EIPC_STREAM_H

#include <stdint.h>
#include <stddef.h>

#define ENI_STREAM_MAX_PACKET_SIZE  4096
#define ENI_STREAM_RING_CAPACITY    64

typedef enum {
    ENI_STREAM_OK = 0,
    ENI_STREAM_ERR_INIT,
    ENI_STREAM_ERR_CONNECT,
    ENI_STREAM_ERR_SEND,
    ENI_STREAM_ERR_BACKPRESSURE,
    ENI_STREAM_ERR_COMPRESS,
} eni_stream_status_t;

typedef enum {
    ENI_STREAM_COMPRESS_NONE = 0,
    ENI_STREAM_COMPRESS_DELTA,
    ENI_STREAM_COMPRESS_RLE,
} eni_stream_compress_t;

typedef struct {
    const char            *endpoint;
    uint16_t               port;
    eni_stream_compress_t  compression;
    uint32_t               buffer_size;
    uint32_t               max_backpressure;
    int                    auto_reconnect;
} eni_stream_config_t;

typedef struct {
    uint64_t timestamp;
    uint32_t sequence;
    uint32_t num_samples;
    uint8_t  data[ENI_STREAM_MAX_PACKET_SIZE];
    uint32_t data_len;
} eni_stream_packet_t;

typedef struct {
    eni_stream_config_t  config;
    eni_stream_packet_t  ring[ENI_STREAM_RING_CAPACITY];
    int                  ring_head;
    int                  ring_tail;
    int                  ring_count;
    uint32_t             sequence;
    uint64_t             bytes_sent;
    uint32_t             packets_sent;
    uint32_t             drops;
    int                  connected;
    int                  initialized;
} eni_stream_session_t;

eni_stream_status_t eni_stream_init(eni_stream_session_t *session, const eni_stream_config_t *config);
eni_stream_status_t eni_stream_connect(eni_stream_session_t *session);
eni_stream_status_t eni_stream_send(eni_stream_session_t *session, const float *samples, uint32_t count, uint64_t timestamp);
eni_stream_status_t eni_stream_flush(eni_stream_session_t *session);
eni_stream_status_t eni_stream_disconnect(eni_stream_session_t *session);
void                eni_stream_get_stats(const eni_stream_session_t *session, uint64_t *bytes, uint32_t *packets, uint32_t *drops);
void                eni_stream_deinit(eni_stream_session_t *session);

#endif /* ENI_EIPC_STREAM_H */
