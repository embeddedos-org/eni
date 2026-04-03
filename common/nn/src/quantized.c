/**
 * @file quantized.c
 * @brief INT8 quantised inference implementation.
 *
 * SPDX-License-Identifier: MIT
 */

#include "eni/quantized.h"
#include <string.h>
#include <math.h>

/* ---- helpers ------------------------------------------------------------ */

static inline int8_t clamp_i8(int32_t v)
{
    if (v > 127)  return 127;
    if (v < -128) return -128;
    return (int8_t)v;
}

static inline int32_t roundf_to_i32(float v)
{
    return (int32_t)roundf(v);
}

/* ---- Public API --------------------------------------------------------- */

int eni_quant_linear(const eni_quant_tensor_t *input,
                     const eni_quant_tensor_t *weight,
                     const int32_t *bias,
                     eni_quant_tensor_t *output,
                     const eni_quant_params_t *out_params)
{
    if (!input || !weight || !output || !out_params) return -1;

    uint32_t K = input->size;
    if (weight->num_dims < 2) return -2;
    uint32_t M = weight->shape[0];
    uint32_t wK = weight->shape[1];
    if (wK != K) return -2;
    if (M > ENI_QUANT_MAX_SIZE) return -2;

    float s_in = input->params.scale;
    int32_t zp_in = input->params.zero_point;
    float s_w = weight->params.scale;
    int32_t zp_w = weight->params.zero_point;
    float s_out = out_params->scale;
    int32_t zp_out = out_params->zero_point;

    if (s_out < 1e-12f) return -2;
    float combined_scale = (s_in * s_w) / s_out;

    for (uint32_t m = 0; m < M; m++) {
        int32_t acc = 0;
        const int8_t *w_row = weight->data + m * K;
        for (uint32_t k = 0; k < K; k++) {
            int32_t a = (int32_t)input->data[k] - zp_in;
            int32_t b = (int32_t)w_row[k] - zp_w;
            acc += a * b;
        }
        if (bias) {
            acc += bias[m];
        }
        int32_t q = roundf_to_i32((float)acc * combined_scale) + zp_out;
        output->data[m] = clamp_i8(q);
    }

    output->shape[0] = M;
    output->num_dims = 1;
    output->size     = M;
    output->params   = *out_params;
    return 0;
}

int eni_quant_conv1d(const eni_quant_tensor_t *input,
                     const eni_quant_tensor_t *weight,
                     const int32_t *bias,
                     eni_quant_tensor_t *output,
                     uint32_t stride,
                     uint32_t padding,
                     const eni_quant_params_t *out_params)
{
    if (!input || !weight || !output || !out_params) return -1;
    if (stride == 0) return -2;

    /* input: [C_in][W_in], weight: [C_out][C_in][K] */
    if (input->num_dims < 2 || weight->num_dims < 3) return -2;

    uint32_t C_in  = input->shape[0];
    uint32_t W_in  = input->shape[1];
    uint32_t C_out = weight->shape[0];
    uint32_t wC_in = weight->shape[1];
    uint32_t K     = weight->shape[2];

    if (wC_in != C_in) return -2;
    if (K > ENI_QUANT_MAX_KERNEL) return -2;
    if (C_out > ENI_QUANT_MAX_CHANNELS) return -2;

    uint32_t W_out = (W_in + 2 * padding - K) / stride + 1;
    if (C_out * W_out > ENI_QUANT_MAX_SIZE) return -2;

    float s_in = input->params.scale;
    int32_t zp_in = input->params.zero_point;
    float s_w = weight->params.scale;
    int32_t zp_w = weight->params.zero_point;
    float s_out = out_params->scale;
    int32_t zp_out = out_params->zero_point;

    if (s_out < 1e-12f) return -2;
    float combined_scale = (s_in * s_w) / s_out;

    for (uint32_t co = 0; co < C_out; co++) {
        for (uint32_t ow = 0; ow < W_out; ow++) {
            int32_t acc = 0;
            int32_t iw_start = (int32_t)(ow * stride) - (int32_t)padding;

            for (uint32_t ci = 0; ci < C_in; ci++) {
                for (uint32_t k = 0; k < K; k++) {
                    int32_t iw = iw_start + (int32_t)k;
                    int32_t in_val = 0;
                    if (iw >= 0 && (uint32_t)iw < W_in) {
                        in_val = (int32_t)input->data[ci * W_in + (uint32_t)iw] - zp_in;
                    } else {
                        in_val = -zp_in;
                    }
                    int32_t w_val = (int32_t)weight->data[
                        co * C_in * K + ci * K + k] - zp_w;
                    acc += in_val * w_val;
                }
            }

            if (bias) {
                acc += bias[co];
            }

            int32_t q = roundf_to_i32((float)acc * combined_scale) + zp_out;
            output->data[co * W_out + ow] = clamp_i8(q);
        }
    }

    output->shape[0] = C_out;
    output->shape[1] = W_out;
    output->num_dims = 2;
    output->size     = C_out * W_out;
    output->params   = *out_params;
    return 0;
}

int eni_quant_dequantize(const eni_quant_tensor_t *input,
                         float *output,
                         uint32_t max_len)
{
    if (!input || !output) return -1;
    if (input->size > max_len) return -2;

    float scale = input->params.scale;
    int32_t zp  = input->params.zero_point;

    for (uint32_t i = 0; i < input->size; i++) {
        output[i] = scale * ((float)((int32_t)input->data[i] - zp));
    }

    return 0;
}

int eni_quant_quantize(const float *input,
                       uint32_t len,
                       eni_quant_tensor_t *output,
                       const eni_quant_params_t *params)
{
    if (!input || !output || !params) return -1;
    if (len > ENI_QUANT_MAX_SIZE) return -2;
    if (params->scale < 1e-12f) return -2;

    float inv_scale = 1.0f / params->scale;
    int32_t zp = params->zero_point;

    for (uint32_t i = 0; i < len; i++) {
        int32_t q = roundf_to_i32(input[i] * inv_scale) + zp;
        output->data[i] = clamp_i8(q);
    }

    output->shape[0] = len;
    output->num_dims = 1;
    output->size     = len;
    output->params   = *params;
    return 0;
}
