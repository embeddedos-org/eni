/**
 * @file cnn_decoder.c
 * @brief CNN intent decoder implementation with 1-D temporal convolution.
 *
 * SPDX-License-Identifier: MIT
 */

#include "eni/cnn_decoder.h"
#include <string.h>
#include <math.h>

/* ---- helpers ------------------------------------------------------------ */

static inline float relu_f(float x) { return x > 0.0f ? x : 0.0f; }

static inline uint32_t min_u32(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

/* ---- conv1d ------------------------------------------------------------- */

static void conv1d_forward(const eni_cnn_conv_layer_t *conv,
                           const float *input,
                           float output[][ENI_CNN_MAX_SAMPLES],
                           uint32_t in_length,
                           uint32_t *out_length)
{
    uint32_t nf = conv->num_filters;
    uint32_t ci = conv->in_channels;
    uint32_t ks = conv->kernel_size;
    uint32_t st = conv->stride > 0 ? conv->stride : 1;
    uint32_t pad = conv->padding;

    uint32_t olen = (in_length + 2 * pad - ks) / st + 1;
    if (olen > ENI_CNN_MAX_SAMPLES) olen = ENI_CNN_MAX_SAMPLES;
    *out_length = olen;

    for (uint32_t f = 0; f < nf; f++) {
        for (uint32_t o = 0; o < olen; o++) {
            float sum = conv->bias[f];
            int32_t iw_start = (int32_t)(o * st) - (int32_t)pad;

            for (uint32_t c = 0; c < ci; c++) {
                for (uint32_t k = 0; k < ks; k++) {
                    int32_t iw = iw_start + (int32_t)k;
                    if (iw >= 0 && (uint32_t)iw < in_length) {
                        sum += conv->weights[f][c][k] *
                               input[c * in_length + (uint32_t)iw];
                    }
                }
            }
            output[f][o] = relu_f(sum);
        }
    }
}

/* ---- pooling ------------------------------------------------------------ */

static void pool1d_forward(const eni_cnn_pool_layer_t *pool,
                           const float input[][ENI_CNN_MAX_SAMPLES],
                           float output[][ENI_CNN_MAX_SAMPLES],
                           uint32_t num_channels,
                           uint32_t in_length,
                           uint32_t *out_length)
{
    uint32_t ks = pool->kernel_size > 0 ? pool->kernel_size : 2;
    uint32_t st = pool->stride > 0 ? pool->stride : ks;

    uint32_t olen = (in_length >= ks) ? (in_length - ks) / st + 1 : 1;
    if (olen > ENI_CNN_MAX_SAMPLES) olen = ENI_CNN_MAX_SAMPLES;
    *out_length = olen;

    for (uint32_t c = 0; c < num_channels; c++) {
        for (uint32_t o = 0; o < olen; o++) {
            uint32_t start = o * st;
            uint32_t end = min_u32(start + ks, in_length);

            if (pool->type == ENI_CNN_POOL_MAX) {
                float mx = -1e30f;
                for (uint32_t i = start; i < end; i++) {
                    if (input[c][i] > mx) mx = input[c][i];
                }
                output[c][o] = mx;
            } else {
                float sum = 0.0f;
                uint32_t count = 0;
                for (uint32_t i = start; i < end; i++) {
                    sum += input[c][i];
                    count++;
                }
                output[c][o] = (count > 0) ? sum / (float)count : 0.0f;
            }
        }
    }
}

/* ---- softmax ------------------------------------------------------------ */

static void softmax(float *v, uint32_t n)
{
    if (n == 0) return;

    float mx = v[0];
    for (uint32_t i = 1; i < n; i++) {
        if (v[i] > mx) mx = v[i];
    }

    float sum = 0.0f;
    for (uint32_t i = 0; i < n; i++) {
        v[i] = expf(v[i] - mx);
        sum += v[i];
    }

    float inv_sum = 1.0f / (sum + 1e-12f);
    for (uint32_t i = 0; i < n; i++) {
        v[i] *= inv_sum;
    }
}

/* ---- Public API --------------------------------------------------------- */

int eni_cnn_decoder_init(eni_cnn_decoder_t *dec,
                         uint32_t input_channels,
                         uint32_t input_length,
                         uint32_t num_classes)
{
    if (!dec) return -1;
    if (input_channels == 0 || input_channels > ENI_CNN_MAX_CHANNELS) return -2;
    if (input_length == 0 || input_length > ENI_CNN_MAX_SAMPLES) return -2;
    if (num_classes == 0 || num_classes > ENI_CNN_MAX_CLASSES) return -2;

    memset(dec, 0, sizeof(*dec));
    dec->input_channels = input_channels;
    dec->input_length   = input_length;
    dec->num_classes    = num_classes;

    /* Default conv: 8 filters, kernel 3, stride 1, no padding */
    dec->conv.num_filters = min_u32(8, ENI_CNN_MAX_FILTERS);
    dec->conv.in_channels = input_channels;
    dec->conv.kernel_size = 3;
    dec->conv.stride      = 1;
    dec->conv.padding     = 0;

    /* Default pool: max, kernel 2, stride 2 */
    dec->pool.type        = ENI_CNN_POOL_MAX;
    dec->pool.kernel_size = 2;
    dec->pool.stride      = 2;

    /* Dense layer dimensions computed at forward time */
    dec->dense.out_features = num_classes;

    dec->initialised = true;
    return 0;
}

int eni_cnn_decoder_forward(eni_cnn_decoder_t *dec,
                            const float *input)
{
    if (!dec || !input) return -1;
    if (!dec->initialised) return -3;

    uint32_t conv_len = 0;
    uint32_t pool_len = 0;

    /* 1. Conv1D + ReLU */
    conv1d_forward(&dec->conv, input, dec->conv_out,
                   dec->input_length, &conv_len);

    /* 2. Pooling */
    pool1d_forward(&dec->pool, dec->conv_out, dec->pool_out,
                   dec->conv.num_filters, conv_len, &pool_len);

    /* 3. Flatten */
    uint32_t flat_size = dec->conv.num_filters * pool_len;
    if (flat_size > ENI_CNN_MAX_DENSE) flat_size = ENI_CNN_MAX_DENSE;

    uint32_t idx = 0;
    for (uint32_t f = 0; f < dec->conv.num_filters && idx < flat_size; f++) {
        for (uint32_t s = 0; s < pool_len && idx < flat_size; s++) {
            dec->flat_buf[idx++] = dec->pool_out[f][s];
        }
    }

    dec->dense.in_features = flat_size;

    /* 4. Dense layer */
    uint32_t nc = dec->num_classes;
    for (uint32_t c = 0; c < nc; c++) {
        float sum = dec->dense.bias[c];
        uint32_t lim = min_u32(flat_size, ENI_CNN_MAX_DENSE);
        for (uint32_t j = 0; j < lim; j++) {
            sum += dec->dense.weights[c][j] * dec->flat_buf[j];
        }
        dec->softmax_out[c] = sum;
    }

    /* 5. Softmax */
    softmax(dec->softmax_out, nc);

    return 0;
}

int eni_cnn_decoder_get_intent(const eni_cnn_decoder_t *dec,
                               eni_cnn_intent_t *intent)
{
    if (!dec || !intent) return -1;
    if (!dec->initialised) return -3;

    uint32_t best = 0;
    float best_conf = dec->softmax_out[0];
    for (uint32_t i = 1; i < dec->num_classes; i++) {
        if (dec->softmax_out[i] > best_conf) {
            best_conf = dec->softmax_out[i];
            best = i;
        }
    }

    intent->class_id   = best;
    intent->confidence = best_conf;
    return 0;
}
