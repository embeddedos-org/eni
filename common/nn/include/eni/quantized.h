/**
 * @file quantized.h
 * @brief INT8 quantized neural network inference.
 *
 * Provides quantised linear, conv1d, and conversion utilities.
 * All operations use fixed-size buffers — MCU-safe.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ENI_QUANTIZED_H
#define ENI_QUANTIZED_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------ */
#define ENI_QUANT_MAX_SIZE      4096
#define ENI_QUANT_MAX_DIMS         4
#define ENI_QUANT_MAX_CHANNELS    64
#define ENI_QUANT_MAX_KERNEL      16

/* ---------------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------------ */

/** Quantisation parameters (affine: real = scale * (q - zero_point)). */
typedef struct {
    float   scale;
    int32_t zero_point;
    bool    per_channel;        /**< If true, scale/zp are per output channel. */
} eni_quant_params_t;

/** Quantised tensor (INT8). */
typedef struct {
    int8_t              data[ENI_QUANT_MAX_SIZE];
    uint32_t            shape[ENI_QUANT_MAX_DIMS];
    uint32_t            num_dims;
    uint32_t            size;       /**< Total elements. */
    eni_quant_params_t  params;
} eni_quant_tensor_t;

/** Per-channel quantisation table (for weights). */
typedef struct {
    float   scales[ENI_QUANT_MAX_CHANNELS];
    int32_t zero_points[ENI_QUANT_MAX_CHANNELS];
    uint32_t num_channels;
} eni_quant_per_channel_t;

/* ---------------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------------ */

/**
 * Quantised fully-connected (linear) layer.
 * out[M] = sum_k( input[K] * weight[M][K] ) + bias[M]
 * All tensors in INT8; accumulation in INT32.
 */
int eni_quant_linear(const eni_quant_tensor_t *input,
                     const eni_quant_tensor_t *weight,
                     const int32_t *bias,
                     eni_quant_tensor_t *output,
                     const eni_quant_params_t *out_params);

/**
 * Quantised 1-D convolution.
 * input: [channels_in][width], weight: [channels_out][channels_in][kernel].
 */
int eni_quant_conv1d(const eni_quant_tensor_t *input,
                     const eni_quant_tensor_t *weight,
                     const int32_t *bias,
                     eni_quant_tensor_t *output,
                     uint32_t stride,
                     uint32_t padding,
                     const eni_quant_params_t *out_params);

/**
 * De-quantise INT8 tensor to float32 buffer.
 */
int eni_quant_dequantize(const eni_quant_tensor_t *input,
                         float *output,
                         uint32_t max_len);

/**
 * Quantise float32 buffer to INT8 tensor.
 */
int eni_quant_quantize(const float *input,
                       uint32_t len,
                       eni_quant_tensor_t *output,
                       const eni_quant_params_t *params);

#ifdef __cplusplus
}
#endif

#endif /* ENI_QUANTIZED_H */
