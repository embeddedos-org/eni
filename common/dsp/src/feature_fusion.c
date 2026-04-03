/**
 * @file feature_fusion.c
 * @brief Multi-channel feature fusion implementation.
 *
 * SPDX-License-Identifier: MIT
 */

#include "eni/feature_fusion.h"
#include <string.h>
#include <math.h>

/* ---- helpers ------------------------------------------------------------ */

static inline float safe_sqrt(float x)
{
    return (x > 0.0f) ? sqrtf(x) : 0.0f;
}

static inline uint32_t min_u32(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

/* ---- API ---------------------------------------------------------------- */

int eni_fusion_init(eni_fusion_engine_t *engine,
                    const eni_fusion_config_t *config)
{
    if (!engine || !config) return -1;
    if (config->num_channels == 0 ||
        config->num_channels > ENI_FUSION_MAX_CHANNELS) return -2;
    if (config->feature_size == 0 ||
        config->feature_size > ENI_FUSION_MAX_FEATURES) return -2;

    memset(engine, 0, sizeof(*engine));
    memcpy(&engine->config, config, sizeof(*config));

    /* Identity spatial filter by default */
    for (uint32_t i = 0; i < config->num_channels; i++) {
        engine->W[i][i] = 1.0f;
        engine->selected[i] = true;
    }
    engine->num_selected = config->num_channels;
    engine->initialised  = true;
    return 0;
}

int eni_fusion_cross_coherence(eni_fusion_engine_t *engine,
                               const float *data,
                               uint32_t num_samples)
{
    if (!engine || !data) return -1;
    if (!engine->initialised) return -3;
    if (num_samples == 0 || num_samples > ENI_FUSION_MAX_SAMPLES) return -2;

    uint32_t nc = engine->config.num_channels;
    float inv_n = 1.0f / (float)num_samples;

    /* Compute per-channel mean and variance */
    float ch_mean[ENI_FUSION_MAX_CHANNELS];
    float ch_var[ENI_FUSION_MAX_CHANNELS];
    memset(ch_mean, 0, sizeof(ch_mean));
    memset(ch_var, 0, sizeof(ch_var));

    for (uint32_t c = 0; c < nc; c++) {
        const float *row = data + c * num_samples;
        float sum = 0.0f;
        for (uint32_t s = 0; s < num_samples; s++) {
            sum += row[s];
        }
        ch_mean[c] = sum * inv_n;

        float var = 0.0f;
        for (uint32_t s = 0; s < num_samples; s++) {
            float d = row[s] - ch_mean[c];
            var += d * d;
        }
        ch_var[c] = var * inv_n;
    }

    /* Cross-coherence (normalised cross-correlation magnitude) */
    for (uint32_t i = 0; i < nc; i++) {
        engine->coherence[i][i] = 1.0f;
        for (uint32_t j = i + 1; j < nc; j++) {
            const float *ri = data + i * num_samples;
            const float *rj = data + j * num_samples;
            float cross = 0.0f;
            for (uint32_t s = 0; s < num_samples; s++) {
                cross += (ri[s] - ch_mean[i]) * (rj[s] - ch_mean[j]);
            }
            cross *= inv_n;

            float denom = safe_sqrt(ch_var[i]) * safe_sqrt(ch_var[j]);
            float coh = (denom > 1e-12f) ? fabsf(cross / denom) : 0.0f;
            if (coh > 1.0f) coh = 1.0f;

            engine->coherence[i][j] = coh;
            engine->coherence[j][i] = coh;
        }
    }

    return 0;
}

int eni_fusion_csp_transform(eni_fusion_engine_t *engine,
                             const float *input,
                             float *output,
                             uint32_t num_samples)
{
    if (!engine || !input || !output) return -1;
    if (!engine->initialised) return -3;
    if (num_samples == 0 || num_samples > ENI_FUSION_MAX_SAMPLES) return -2;

    uint32_t nc = engine->config.num_channels;

    /* output[i][s] = sum_j( W[i][j] * input[j][s] ) */
    for (uint32_t i = 0; i < nc; i++) {
        for (uint32_t s = 0; s < num_samples; s++) {
            float val = 0.0f;
            for (uint32_t j = 0; j < nc; j++) {
                val += engine->W[i][j] * input[j * num_samples + s];
            }
            output[i * num_samples + s] = val;
        }
    }

    return 0;
}

int eni_fusion_normalize(eni_fusion_engine_t *engine,
                         float *features,
                         uint32_t len)
{
    if (!engine || !features) return -1;
    if (!engine->initialised) return -3;
    if (len == 0) return -2;

    /* Compute mean */
    float sum = 0.0f;
    for (uint32_t i = 0; i < len; i++) {
        sum += features[i];
    }
    float mean = sum / (float)len;

    /* Compute standard deviation */
    float var = 0.0f;
    for (uint32_t i = 0; i < len; i++) {
        float d = features[i] - mean;
        var += d * d;
    }
    float std = safe_sqrt(var / (float)len);
    if (std < 1e-12f) std = 1e-12f;

    /* Z-score normalisation */
    float inv_std = 1.0f / std;
    for (uint32_t i = 0; i < len; i++) {
        features[i] = (features[i] - mean) * inv_std;
    }

    /* Update running statistics (exponential moving average) */
    engine->sample_count++;
    float alpha = (engine->sample_count > 1) ? 0.01f : 1.0f;
    uint32_t nc = min_u32(len, ENI_FUSION_MAX_CHANNELS);
    for (uint32_t i = 0; i < nc; i++) {
        engine->mean[i]    = engine->mean[i] * (1.0f - alpha) + mean * alpha;
        engine->std_dev[i] = engine->std_dev[i] * (1.0f - alpha) + std * alpha;
    }

    return 0;
}

int eni_fusion_select_channels(eni_fusion_engine_t *engine,
                               uint32_t max_channels)
{
    if (!engine) return -1;
    if (!engine->initialised) return -3;

    uint32_t nc = engine->config.num_channels;
    if (max_channels == 0 || max_channels > nc) {
        max_channels = nc;
    }

    /* Compute average coherence per channel */
    float scores[ENI_FUSION_MAX_CHANNELS];
    for (uint32_t i = 0; i < nc; i++) {
        float s = 0.0f;
        for (uint32_t j = 0; j < nc; j++) {
            if (i != j) {
                s += engine->coherence[i][j];
            }
        }
        scores[i] = (nc > 1) ? s / (float)(nc - 1) : 0.0f;
        engine->selected[i] = false;
    }

    /* Select top-k channels by score (simple selection sort) */
    uint32_t selected = 0;
    bool used[ENI_FUSION_MAX_CHANNELS];
    memset(used, 0, sizeof(used));

    while (selected < max_channels) {
        float best = -1.0f;
        uint32_t best_idx = 0;
        for (uint32_t i = 0; i < nc; i++) {
            if (!used[i] && scores[i] > best) {
                best = scores[i];
                best_idx = i;
            }
        }
        if (best < engine->config.coherence_threshold && selected > 0) {
            break;
        }
        engine->selected[best_idx] = true;
        used[best_idx] = true;
        selected++;
    }

    engine->num_selected = selected;
    return 0;
}
