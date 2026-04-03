/**
 * @file cnn_decoder.h
 * @brief CNN intent decoder for BCI classification.
 *
 * 1-D temporal convolution → pooling → dense → softmax.
 * Fixed-size arrays — MCU-safe, no dynamic allocation.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ENI_CNN_DECODER_H
#define ENI_CNN_DECODER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------ */
#define ENI_CNN_MAX_CHANNELS    32
#define ENI_CNN_MAX_SAMPLES    256
#define ENI_CNN_MAX_FILTERS     16
#define ENI_CNN_MAX_KERNEL      16
#define ENI_CNN_MAX_CLASSES     16
#define ENI_CNN_MAX_DENSE      128

/* ---------------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------------ */

typedef struct {
    float   weights[ENI_CNN_MAX_FILTERS][ENI_CNN_MAX_CHANNELS][ENI_CNN_MAX_KERNEL];
    float   bias[ENI_CNN_MAX_FILTERS];
    uint32_t num_filters;
    uint32_t in_channels;
    uint32_t kernel_size;
    uint32_t stride;
    uint32_t padding;
} eni_cnn_conv_layer_t;

typedef enum {
    ENI_CNN_POOL_MAX,
    ENI_CNN_POOL_AVG
} eni_cnn_pool_type_t;

typedef struct {
    eni_cnn_pool_type_t type;
    uint32_t kernel_size;
    uint32_t stride;
} eni_cnn_pool_layer_t;

typedef struct {
    float    weights[ENI_CNN_MAX_CLASSES][ENI_CNN_MAX_DENSE];
    float    bias[ENI_CNN_MAX_CLASSES];
    uint32_t in_features;
    uint32_t out_features;
} eni_cnn_dense_layer_t;

typedef struct {
    eni_cnn_conv_layer_t    conv;
    eni_cnn_pool_layer_t    pool;
    eni_cnn_dense_layer_t   dense;

    /* Intermediate buffers */
    float conv_out[ENI_CNN_MAX_FILTERS][ENI_CNN_MAX_SAMPLES];
    float pool_out[ENI_CNN_MAX_FILTERS][ENI_CNN_MAX_SAMPLES];
    float flat_buf[ENI_CNN_MAX_DENSE];
    float softmax_out[ENI_CNN_MAX_CLASSES];

    uint32_t input_channels;
    uint32_t input_length;
    uint32_t num_classes;

    bool initialised;
} eni_cnn_decoder_t;

typedef struct {
    uint32_t class_id;
    float    confidence;
} eni_cnn_intent_t;

/* ---------------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------------ */

int eni_cnn_decoder_init(eni_cnn_decoder_t *dec,
                         uint32_t input_channels,
                         uint32_t input_length,
                         uint32_t num_classes);

/**
 * Run forward pass.
 * @param input  [input_channels][input_length] row-major.
 * @return 0 on success.
 */
int eni_cnn_decoder_forward(eni_cnn_decoder_t *dec,
                            const float *input);

/**
 * Get classification result after forward pass.
 */
int eni_cnn_decoder_get_intent(const eni_cnn_decoder_t *dec,
                               eni_cnn_intent_t *intent);

#ifdef __cplusplus
}
#endif

#endif /* ENI_CNN_DECODER_H */
