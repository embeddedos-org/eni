/**
 * @file artifact_removal.c
 * @brief Real-time artifact removal implementation.
 *
 * Sliding-window blink detector, adaptive threshold, linear interpolation
 * for artifact correction. No heap allocation.
 *
 * SPDX-License-Identifier: MIT
 */

#include "eni/artifact_removal.h"
#include <string.h>
#include <math.h>

/* ---- helpers ------------------------------------------------------------ */

static inline float clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static void chan_state_reset(eni_artifact_chan_state_t *s)
{
    memset(s->ring, 0, sizeof(s->ring));
    s->running_mean   = 0.0f;
    s->running_var    = 0.0f;
    s->adaptive_thresh = 0.0f;
    s->head           = 0;
    s->count          = 0;
    s->artifact_count = 0;
}

/** Push a sample into the ring buffer and update running statistics. */
static void chan_push(eni_artifact_chan_state_t *s,
                      float sample,
                      uint32_t window_size)
{
    if (window_size == 0 || window_size > ENI_ARTIFACT_WINDOW_SIZE) {
        window_size = ENI_ARTIFACT_WINDOW_SIZE;
    }

    float old = s->ring[s->head];
    s->ring[s->head] = sample;
    s->head = (s->head + 1) % window_size;
    if (s->count < window_size) {
        s->count++;
    }

    /* Incremental mean update */
    float n = (float)s->count;
    s->running_mean += (sample - old) / n;

    /* Incremental variance (Welford-ish, simplified for ring) */
    float delta = sample - s->running_mean;
    s->running_var = s->running_var * 0.99f + delta * delta * 0.01f;
}

/** Linear interpolation between two values. */
static inline float lerpf(float a, float b, float t)
{
    return a + t * (b - a);
}

/**
 * Interpolate artifact region: replace current sample with blend of
 * the two neighbours in the ring buffer.
 */
static float interpolate_artifact(const eni_artifact_chan_state_t *s,
                                  uint32_t window_size,
                                  float ratio)
{
    if (s->count < 3) {
        return s->running_mean;
    }
    uint32_t ws = (window_size > 0 && window_size <= ENI_ARTIFACT_WINDOW_SIZE)
                  ? window_size : ENI_ARTIFACT_WINDOW_SIZE;

    uint32_t cur = (s->head + ws - 1) % ws;
    uint32_t prev = (cur + ws - 1) % ws;
    uint32_t prev2 = (prev + ws - 1) % ws;

    float interp = lerpf(s->ring[prev2], s->ring[prev], 0.5f);
    return lerpf(s->ring[cur], interp, clampf(ratio, 0.0f, 1.0f));
}

/* ---- public API --------------------------------------------------------- */

int eni_artifact_init(eni_artifact_engine_t *engine,
                      const eni_artifact_config_t *config)
{
    if (!engine || !config) {
        return -1;
    }
    if (config->num_channels == 0 ||
        config->num_channels > ENI_ARTIFACT_MAX_CHANNELS) {
        return -2;
    }
    if (config->window_size == 0 ||
        config->window_size > ENI_ARTIFACT_WINDOW_SIZE) {
        return -2;
    }

    memcpy(&engine->config, config, sizeof(*config));

    for (uint32_t ch = 0; ch < config->num_channels; ch++) {
        chan_state_reset(&engine->state[ch]);
        engine->state[ch].adaptive_thresh =
            config->channels[ch].threshold > 0.0f
                ? config->channels[ch].threshold
                : config->global_threshold;
    }

    engine->initialised = true;
    return 0;
}

int eni_artifact_process(eni_artifact_engine_t *engine,
                         float *samples,
                         uint32_t num_channels,
                         eni_artifact_flag_t *flags)
{
    if (!engine || !samples) {
        return -1;
    }
    if (!engine->initialised) {
        return -3;
    }
    if (num_channels > engine->config.num_channels) {
        num_channels = engine->config.num_channels;
    }

    uint32_t ws = engine->config.window_size;

    for (uint32_t ch = 0; ch < num_channels; ch++) {
        eni_artifact_flag_t f = ENI_ARTIFACT_NONE;
        eni_artifact_chan_state_t *st = &engine->state[ch];
        const eni_artifact_channel_cfg_t *cfg = &engine->config.channels[ch];

        if (!cfg->enabled) {
            if (flags) flags[ch] = ENI_ARTIFACT_NONE;
            chan_push(st, samples[ch], ws);
            continue;
        }

        float sample = samples[ch];
        chan_push(st, sample, ws);

        float abs_val = fabsf(sample - st->running_mean);
        float std_dev = sqrtf(st->running_var + 1e-12f);

        /* Adaptive threshold update */
        st->adaptive_thresh = st->adaptive_thresh * (1.0f - engine->config.adaptive_rate)
                            + (st->running_mean + 3.0f * std_dev) * engine->config.adaptive_rate;

        /* --- Blink detection --- */
        if (abs_val > cfg->blink_threshold && cfg->blink_threshold > 0.0f) {
            f |= ENI_ARTIFACT_BLINK;
        }

        /* --- EMG detection (high-freq proxy: diff from mean) --- */
        if (abs_val > cfg->emg_threshold && cfg->emg_threshold > 0.0f) {
            /* Simple high-frequency check: large deviation with small window variance */
            if (std_dev > 0.0f && abs_val / std_dev > 4.0f) {
                f |= ENI_ARTIFACT_EMG;
            }
        }

        /* --- Saturation detection --- */
        if (fabsf(sample) > st->adaptive_thresh * 2.0f) {
            f |= ENI_ARTIFACT_SAT;
        }

        /* --- Generic threshold crossing --- */
        if (abs_val > st->adaptive_thresh) {
            f |= ENI_ARTIFACT_MOTION;
        }

        /* --- Artifact correction via interpolation --- */
        if (f != ENI_ARTIFACT_NONE) {
            st->artifact_count++;
            float ratio = cfg->interp_ratio > 0.0f ? cfg->interp_ratio : 0.8f;
            samples[ch] = interpolate_artifact(st, ws, ratio);
        }

        if (flags) {
            flags[ch] = f;
        }
    }

    return 0;
}

bool eni_artifact_detect_blink(const eni_artifact_engine_t *engine,
                               uint32_t channel)
{
    if (!engine || !engine->initialised) return false;
    if (channel >= engine->config.num_channels) return false;

    const eni_artifact_chan_state_t *st = &engine->state[channel];
    const eni_artifact_channel_cfg_t *cfg = &engine->config.channels[channel];

    if (!cfg->enabled || cfg->blink_threshold <= 0.0f) return false;
    if (st->count < 3) return false;

    uint32_t ws = engine->config.window_size;
    uint32_t latest = (st->head + ws - 1) % ws;
    float abs_val = fabsf(st->ring[latest] - st->running_mean);

    return abs_val > cfg->blink_threshold;
}

bool eni_artifact_detect_emg(const eni_artifact_engine_t *engine,
                             uint32_t channel)
{
    if (!engine || !engine->initialised) return false;
    if (channel >= engine->config.num_channels) return false;

    const eni_artifact_chan_state_t *st = &engine->state[channel];
    const eni_artifact_channel_cfg_t *cfg = &engine->config.channels[channel];

    if (!cfg->enabled || cfg->emg_threshold <= 0.0f) return false;
    if (st->count < 3) return false;

    uint32_t ws = engine->config.window_size;
    uint32_t latest = (st->head + ws - 1) % ws;
    float abs_val = fabsf(st->ring[latest] - st->running_mean);
    float std_dev = sqrtf(st->running_var + 1e-12f);

    return (abs_val > cfg->emg_threshold) && (abs_val / std_dev > 4.0f);
}

int eni_artifact_reset(eni_artifact_engine_t *engine)
{
    if (!engine) return -1;
    if (!engine->initialised) return -3;

    for (uint32_t ch = 0; ch < engine->config.num_channels; ch++) {
        chan_state_reset(&engine->state[ch]);
        engine->state[ch].adaptive_thresh =
            engine->config.channels[ch].threshold > 0.0f
                ? engine->config.channels[ch].threshold
                : engine->config.global_threshold;
    }

    return 0;
}
