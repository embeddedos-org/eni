// SPDX-License-Identifier: MIT
// eNI ONI (Open Neural Interface) Compliance Layer

#ifndef ENI_ONI_COMPLIANCE_H
#define ENI_ONI_COMPLIANCE_H

#include <stdint.h>
#include <stddef.h>

#define ENI_ONI_MAX_STREAMS    8
#define ENI_ONI_MAX_REG_ADDR   256
#define ENI_ONI_FRAME_SIZE     4096

typedef enum {
    ENI_ONI_OK = 0,
    ENI_ONI_ERR_INIT,
    ENI_ONI_ERR_CONFIG,
    ENI_ONI_ERR_STREAM,
    ENI_ONI_ERR_REG,
    ENI_ONI_ERR_FRAME,
} eni_oni_status_t;

typedef struct {
    uint32_t stream_id;
    uint32_t sample_rate;
    uint32_t num_channels;
    uint32_t data_type;
    int      enabled;
} eni_oni_stream_config_t;

typedef struct {
    uint32_t device_id;
    uint32_t firmware_version;
    uint32_t num_streams;
    eni_oni_stream_config_t streams[ENI_ONI_MAX_STREAMS];
    uint32_t registers[ENI_ONI_MAX_REG_ADDR];
    int      initialized;
} eni_oni_context_t;

typedef struct {
    uint32_t stream_id;
    uint64_t timestamp;
    uint32_t num_samples;
    float    data[ENI_ONI_FRAME_SIZE];
} eni_oni_frame_t;

eni_oni_status_t eni_oni_init(eni_oni_context_t *ctx, uint32_t device_id);
eni_oni_status_t eni_oni_configure_stream(eni_oni_context_t *ctx, const eni_oni_stream_config_t *config);
eni_oni_status_t eni_oni_start(eni_oni_context_t *ctx);
eni_oni_status_t eni_oni_read_frame(eni_oni_context_t *ctx, uint32_t stream_id, eni_oni_frame_t *frame);
eni_oni_status_t eni_oni_reg_read(const eni_oni_context_t *ctx, uint32_t addr, uint32_t *value);
eni_oni_status_t eni_oni_reg_write(eni_oni_context_t *ctx, uint32_t addr, uint32_t value);
eni_oni_status_t eni_oni_stop(eni_oni_context_t *ctx);
void             eni_oni_deinit(eni_oni_context_t *ctx);

#endif /* ENI_ONI_COMPLIANCE_H */
