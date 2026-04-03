// SPDX-License-Identifier: MIT
// eNI ONI Compliance — Wraps eNI providers into ONI-compatible contexts

#include "eni/oni_compliance.h"
#include <string.h>

eni_oni_status_t eni_oni_init(eni_oni_context_t *ctx, uint32_t device_id)
{
    if (!ctx) return ENI_ONI_ERR_INIT;
    memset(ctx, 0, sizeof(*ctx));
    ctx->device_id = device_id;
    ctx->firmware_version = 0x00020000; /* v0.2.0 */
    ctx->initialized = 1;
    return ENI_ONI_OK;
}

eni_oni_status_t eni_oni_configure_stream(eni_oni_context_t *ctx, const eni_oni_stream_config_t *config)
{
    if (!ctx || !ctx->initialized || !config) return ENI_ONI_ERR_CONFIG;
    if (ctx->num_streams >= ENI_ONI_MAX_STREAMS) return ENI_ONI_ERR_STREAM;

    ctx->streams[ctx->num_streams] = *config;
    ctx->streams[ctx->num_streams].enabled = 1;
    ctx->num_streams++;
    return ENI_ONI_OK;
}

eni_oni_status_t eni_oni_start(eni_oni_context_t *ctx)
{
    if (!ctx || !ctx->initialized) return ENI_ONI_ERR_INIT;
    if (ctx->num_streams == 0) return ENI_ONI_ERR_STREAM;
    return ENI_ONI_OK;
}

eni_oni_status_t eni_oni_read_frame(eni_oni_context_t *ctx, uint32_t stream_id, eni_oni_frame_t *frame)
{
    if (!ctx || !ctx->initialized || !frame) return ENI_ONI_ERR_FRAME;

    int found = 0;
    for (uint32_t i = 0; i < ctx->num_streams; i++) {
        if (ctx->streams[i].stream_id == stream_id && ctx->streams[i].enabled) {
            found = 1;
            break;
        }
    }
    if (!found) return ENI_ONI_ERR_STREAM;

    memset(frame, 0, sizeof(*frame));
    frame->stream_id = stream_id;
    frame->num_samples = 0;
    return ENI_ONI_OK;
}

eni_oni_status_t eni_oni_reg_read(const eni_oni_context_t *ctx, uint32_t addr, uint32_t *value)
{
    if (!ctx || !ctx->initialized || !value) return ENI_ONI_ERR_REG;
    if (addr >= ENI_ONI_MAX_REG_ADDR) return ENI_ONI_ERR_REG;
    *value = ctx->registers[addr];
    return ENI_ONI_OK;
}

eni_oni_status_t eni_oni_reg_write(eni_oni_context_t *ctx, uint32_t addr, uint32_t value)
{
    if (!ctx || !ctx->initialized) return ENI_ONI_ERR_REG;
    if (addr >= ENI_ONI_MAX_REG_ADDR) return ENI_ONI_ERR_REG;
    ctx->registers[addr] = value;
    return ENI_ONI_OK;
}

eni_oni_status_t eni_oni_stop(eni_oni_context_t *ctx)
{
    if (!ctx || !ctx->initialized) return ENI_ONI_ERR_INIT;
    for (uint32_t i = 0; i < ctx->num_streams; i++) {
        ctx->streams[i].enabled = 0;
    }
    return ENI_ONI_OK;
}

void eni_oni_deinit(eni_oni_context_t *ctx)
{
    if (ctx) {
        memset(ctx, 0, sizeof(*ctx));
    }
}
