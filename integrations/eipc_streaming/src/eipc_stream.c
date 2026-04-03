// SPDX-License-Identifier: MIT
// eNI EIPC Streaming — Real-time neural data streaming implementation

#include "eni/eipc_stream.h"
#include <string.h>

eni_stream_status_t eni_stream_init(eni_stream_session_t *session, const eni_stream_config_t *config)
{
    if (!session || !config) return ENI_STREAM_ERR_INIT;
    memset(session, 0, sizeof(*session));
    session->config = *config;

    if (session->config.buffer_size == 0) session->config.buffer_size = ENI_STREAM_MAX_PACKET_SIZE;
    if (session->config.max_backpressure == 0) session->config.max_backpressure = ENI_STREAM_RING_CAPACITY / 2;

    session->initialized = 1;
    return ENI_STREAM_OK;
}

eni_stream_status_t eni_stream_connect(eni_stream_session_t *session)
{
    if (!session || !session->initialized) return ENI_STREAM_ERR_INIT;

    /* TODO: Establish connection via EIPC bridge to endpoint:port */
    session->connected = 1;
    return ENI_STREAM_OK;
}

static eni_stream_status_t delta_compress(const float *samples, uint32_t count, uint8_t *out, uint32_t *out_len, uint32_t max_len)
{
    if (count * sizeof(float) > max_len) return ENI_STREAM_ERR_COMPRESS;

    /* Simple delta encoding: store first sample as-is, then deltas */
    if (count == 0) { *out_len = 0; return ENI_STREAM_OK; }

    memcpy(out, &samples[0], sizeof(float));
    uint32_t pos = sizeof(float);

    for (uint32_t i = 1; i < count && pos + sizeof(float) <= max_len; i++) {
        float delta = samples[i] - samples[i - 1];
        memcpy(out + pos, &delta, sizeof(float));
        pos += sizeof(float);
    }
    *out_len = pos;
    return ENI_STREAM_OK;
}

eni_stream_status_t eni_stream_send(eni_stream_session_t *session, const float *samples, uint32_t count, uint64_t timestamp)
{
    if (!session || !session->initialized || !samples) return ENI_STREAM_ERR_INIT;
    if (!session->connected) return ENI_STREAM_ERR_CONNECT;

    if ((uint32_t)session->ring_count >= session->config.max_backpressure) {
        session->drops++;
        return ENI_STREAM_ERR_BACKPRESSURE;
    }

    eni_stream_packet_t *pkt = &session->ring[session->ring_head];
    pkt->timestamp = timestamp;
    pkt->sequence = session->sequence++;
    pkt->num_samples = count;

    if (session->config.compression == ENI_STREAM_COMPRESS_DELTA) {
        eni_stream_status_t st = delta_compress(samples, count, pkt->data, &pkt->data_len, ENI_STREAM_MAX_PACKET_SIZE);
        if (st != ENI_STREAM_OK) return st;
    } else {
        uint32_t bytes = count * sizeof(float);
        if (bytes > ENI_STREAM_MAX_PACKET_SIZE) bytes = ENI_STREAM_MAX_PACKET_SIZE;
        memcpy(pkt->data, samples, bytes);
        pkt->data_len = bytes;
    }

    session->ring_head = (session->ring_head + 1) % ENI_STREAM_RING_CAPACITY;
    session->ring_count++;

    /* TODO: Actually send via EIPC transport */
    session->bytes_sent += pkt->data_len;
    session->packets_sent++;
    session->ring_count--;
    session->ring_tail = (session->ring_tail + 1) % ENI_STREAM_RING_CAPACITY;

    return ENI_STREAM_OK;
}

eni_stream_status_t eni_stream_flush(eni_stream_session_t *session)
{
    if (!session || !session->initialized) return ENI_STREAM_ERR_INIT;
    while (session->ring_count > 0) {
        session->ring_tail = (session->ring_tail + 1) % ENI_STREAM_RING_CAPACITY;
        session->ring_count--;
    }
    return ENI_STREAM_OK;
}

eni_stream_status_t eni_stream_disconnect(eni_stream_session_t *session)
{
    if (!session || !session->initialized) return ENI_STREAM_ERR_INIT;
    eni_stream_flush(session);
    session->connected = 0;
    return ENI_STREAM_OK;
}

void eni_stream_get_stats(const eni_stream_session_t *session, uint64_t *bytes, uint32_t *packets, uint32_t *drops)
{
    if (!session) return;
    if (bytes) *bytes = session->bytes_sent;
    if (packets) *packets = session->packets_sent;
    if (drops) *drops = session->drops;
}

void eni_stream_deinit(eni_stream_session_t *session)
{
    if (session) {
        memset(session, 0, sizeof(*session));
    }
}
