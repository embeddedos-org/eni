/**
 * @file feature_fusion.h
 * @brief Multi-channel feature fusion for neural signal processing.
 *
 * Cross-coherence, CSP transform, normalisation, and channel selection.
 * All buffers are compile-time sized for MCU deployment.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ENI_FEATURE_FUSION_H
#define ENI_FEATURE_FUSION_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------ */
#define ENI_FUSION_MAX_CHANNELS     32
#define ENI_FUSION_MAX_FEATURES     64
#define ENI_FUSION_MAX_SAMPLES      256

/* ---------------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------------ */

typedef struct {
    uint32_t num_channels;
    uint32_t feature_size;          /**< Features per channel. */
    uint32_t num_csp_pairs;         /**< CSP filter pairs to extract. */
    float    coherence_threshold;   /**< Min coherence to keep channel. */
    bool     auto_select;           /**< Enable automatic channel selection. */
} eni_fusion_config_t;

typedef struct {
    eni_fusion_config_t config;

    /* CSP spatial filters W (num_channels x num_channels) */
    float W[ENI_FUSION_MAX_CHANNELS][ENI_FUSION_MAX_CHANNELS];

    /* Cross-coherence matrix */
    float coherence[ENI_FUSION_MAX_CHANNELS][ENI_FUSION_MAX_CHANNELS];

    /* Feature buffer: fused features */
    float features[ENI_FUSION_MAX_CHANNELS * ENI_FUSION_MAX_FEATURES];

    /* Channel selection mask */
    bool  selected[ENI_FUSION_MAX_CHANNELS];
    uint32_t num_selected;

    /* Normalisation statistics */
    float mean[ENI_FUSION_MAX_CHANNELS];
    float std_dev[ENI_FUSION_MAX_CHANNELS];
    uint32_t sample_count;

    bool initialised;
} eni_fusion_engine_t;

/* ---------------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------------ */

int eni_fusion_init(eni_fusion_engine_t *engine,
                    const eni_fusion_config_t *config);

/**
 * Compute cross-coherence matrix between all channel pairs.
 * @param data   [num_channels][num_samples] row-major.
 */
int eni_fusion_cross_coherence(eni_fusion_engine_t *engine,
                               const float *data,
                               uint32_t num_samples);

/**
 * Apply Common Spatial Pattern (CSP) transform.
 * @param input  [num_channels][num_samples] row-major.
 * @param output [num_channels][num_samples] row-major.
 */
int eni_fusion_csp_transform(eni_fusion_engine_t *engine,
                             const float *input,
                             float *output,
                             uint32_t num_samples);

/**
 * Normalise feature vector (zero-mean, unit-variance).
 */
int eni_fusion_normalize(eni_fusion_engine_t *engine,
                         float *features,
                         uint32_t len);

/**
 * Select best channels based on coherence scores.
 * @param max_channels  Maximum channels to keep.
 */
int eni_fusion_select_channels(eni_fusion_engine_t *engine,
                               uint32_t max_channels);

#ifdef __cplusplus
}
#endif

#endif /* ENI_FEATURE_FUSION_H */
