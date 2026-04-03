/**
 * @file sim_eeg_generator.c
 * @brief Synthetic EEG signal generator.
 *
 * Generates multi-channel EEG with configurable band amplitudes,
 * Gaussian noise, and occasional blink artifacts.
 *
 * SPDX-License-Identifier: MIT
 */

#include "eni/sim.h"
#include <string.h>
#include <math.h>

/* ---- Constants ---------------------------------------------------------- */

static const float BAND_FREQ_LO[ENI_SIM_BAND_COUNT] = {
    0.5f, 4.0f, 8.0f, 13.0f, 30.0f
};
static const float BAND_FREQ_HI[ENI_SIM_BAND_COUNT] = {
    4.0f, 8.0f, 13.0f, 30.0f, 100.0f
};

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* ---- Lightweight PRNG (xorshift32) -------------------------------------- */

static uint32_t xorshift32(uint32_t *state)
{
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static float rand_uniform(uint32_t *state)
{
    return (float)(xorshift32(state) & 0xFFFFFF) / 16777216.0f;
}

/** Box-Muller transform for Gaussian noise. */
static float rand_gaussian(uint32_t *state)
{
    float u1 = rand_uniform(state);
    float u2 = rand_uniform(state);
    if (u1 < 1e-10f) u1 = 1e-10f;
    return sqrtf(-2.0f * logf(u1)) * cosf(2.0f * (float)M_PI * u2);
}

/* ---- Public API --------------------------------------------------------- */

int eni_sim_eeg_init(eni_sim_eeg_gen_t *gen,
                     const eni_sim_eeg_config_t *config)
{
    if (!gen || !config) return -1;
    if (config->num_channels == 0 ||
        config->num_channels > ENI_SIM_MAX_CHANNELS) return -2;
    if (config->sample_rate_hz == 0) return -2;

    memset(gen, 0, sizeof(*gen));
    memcpy(&gen->config, config, sizeof(*config));

    /* Seed PRNG */
    gen->rng_state = 0xDEADBEEF;

    /* Randomise initial phases per channel/band */
    for (uint32_t ch = 0; ch < config->num_channels; ch++) {
        for (uint32_t b = 0; b < ENI_SIM_BAND_COUNT; b++) {
            gen->phase[ch][b] = rand_uniform(&gen->rng_state) * 2.0f * (float)M_PI;
        }
    }

    gen->initialised = true;
    return 0;
}

int eni_sim_eeg_generate(eni_sim_eeg_gen_t *gen,
                         float *samples, uint32_t num_channels)
{
    if (!gen || !samples) return -1;
    if (!gen->initialised) return -3;

    uint32_t nc = num_channels;
    if (nc > gen->config.num_channels) nc = gen->config.num_channels;

    float dt = 1.0f / (float)gen->config.sample_rate_hz;

    for (uint32_t ch = 0; ch < nc; ch++) {
        const eni_sim_eeg_channel_t *cfg = &gen->config.channels[ch];
        if (!cfg->enabled) {
            samples[ch] = 0.0f;
            continue;
        }

        float val = 0.0f;

        /* Sum band oscillations */
        for (uint32_t b = 0; b < ENI_SIM_BAND_COUNT; b++) {
            float freq = (BAND_FREQ_LO[b] + BAND_FREQ_HI[b]) * 0.5f;
            val += cfg->amplitude[b] * sinf(gen->phase[ch][b]);
            gen->phase[ch][b] += 2.0f * (float)M_PI * freq * dt;

            /* Wrap phase to prevent float overflow */
            if (gen->phase[ch][b] > 2.0f * (float)M_PI) {
                gen->phase[ch][b] -= 2.0f * (float)M_PI;
            }
        }

        /* Add noise */
        if (cfg->noise_amplitude > 0.0f) {
            val += cfg->noise_amplitude * rand_gaussian(&gen->rng_state);
        }

        /* Blink artifact (random occurrence) */
        if (cfg->blink_probability > 0.0f && cfg->blink_amplitude > 0.0f) {
            if (rand_uniform(&gen->rng_state) < cfg->blink_probability) {
                /* Gaussian-shaped blink pulse */
                float blink_phase = rand_uniform(&gen->rng_state) * (float)M_PI;
                float blink = cfg->blink_amplitude *
                              expf(-0.5f * blink_phase * blink_phase);
                val += blink;
            }
        }

        samples[ch] = val;
    }

    gen->sample_count++;
    return 0;
}

int eni_sim_eeg_reset(eni_sim_eeg_gen_t *gen)
{
    if (!gen) return -1;
    if (!gen->initialised) return -3;

    gen->sample_count = 0;
    gen->rng_state = 0xDEADBEEF;

    for (uint32_t ch = 0; ch < gen->config.num_channels; ch++) {
        for (uint32_t b = 0; b < ENI_SIM_BAND_COUNT; b++) {
            gen->phase[ch][b] = rand_uniform(&gen->rng_state) * 2.0f * (float)M_PI;
        }
    }

    return 0;
}
