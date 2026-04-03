/**
 * @file sim_bci_generator.c
 * @brief Synthetic BCI signal generator with motor-imagery events.
 *
 * Generates multi-channel signals with µ (8–12 Hz) and β (18–26 Hz)
 * rhythms. Scheduled events modulate amplitude to simulate motor imagery
 * event-related desynchronisation (ERD) and synchronisation (ERS).
 *
 * SPDX-License-Identifier: MIT
 */

#include "eni/sim.h"
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* ---- Lightweight PRNG --------------------------------------------------- */

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

static float rand_gaussian(uint32_t *state)
{
    float u1 = rand_uniform(state);
    float u2 = rand_uniform(state);
    if (u1 < 1e-10f) u1 = 1e-10f;
    return sqrtf(-2.0f * logf(u1)) * cosf(2.0f * (float)M_PI * u2);
}

/* ---- Event-driven amplitude modulation ---------------------------------- */

/**
 * Compute the ERD/ERS modulation factor for the current sample.
 * During an active event, the µ/β power is suppressed (ERD) on
 * contralateral channels and enhanced (ERS) on ipsilateral channels.
 */
static float compute_modulation(const eni_sim_bci_gen_t *gen,
                                uint32_t channel)
{
    if (gen->num_events == 0) return 1.0f;

    uint64_t t = gen->sample_count;

    for (uint32_t i = 0; i < gen->num_events; i++) {
        const eni_sim_bci_event_t *ev = &gen->events[i];
        if (t >= ev->start_tick && t < ev->start_tick + ev->duration_ticks) {
            float strength = ev->strength;

            /* Simple contralateral model:
             * LEFT_HAND  → suppress right-side channels (even idx)
             * RIGHT_HAND → suppress left-side channels (odd idx)
             * FEET/TONGUE → suppress central channels
             */
            bool suppress = false;
            switch (ev->intent) {
            case ENI_SIM_BCI_LEFT_HAND:
                suppress = (channel % 2 == 0);
                break;
            case ENI_SIM_BCI_RIGHT_HAND:
                suppress = (channel % 2 == 1);
                break;
            case ENI_SIM_BCI_FEET:
            case ENI_SIM_BCI_TONGUE:
                suppress = (channel < gen->config.num_channels / 2);
                break;
            default:
                break;
            }

            if (suppress) {
                return 1.0f - strength * 0.7f; /* ERD: reduce amplitude */
            } else {
                return 1.0f + strength * 0.3f; /* ERS: boost amplitude */
            }
        }
    }

    return 1.0f;
}

/* ---- Public API --------------------------------------------------------- */

int eni_sim_bci_init(eni_sim_bci_gen_t *gen,
                     const eni_sim_bci_config_t *config)
{
    if (!gen || !config) return -1;
    if (config->num_channels == 0 ||
        config->num_channels > ENI_SIM_MAX_CHANNELS) return -2;
    if (config->sample_rate_hz == 0) return -2;

    memset(gen, 0, sizeof(*gen));
    memcpy(&gen->config, config, sizeof(*config));

    /* Defaults */
    if (gen->config.mu_frequency <= 0.0f) gen->config.mu_frequency = 10.0f;
    if (gen->config.beta_frequency <= 0.0f) gen->config.beta_frequency = 22.0f;
    if (gen->config.base_amplitude <= 0.0f) gen->config.base_amplitude = 10.0f;

    gen->rng_state = 0xCAFEBABE;

    /* Randomise initial phases */
    for (uint32_t ch = 0; ch < config->num_channels; ch++) {
        gen->phase_mu[ch]   = rand_uniform(&gen->rng_state) * 2.0f * (float)M_PI;
        gen->phase_beta[ch] = rand_uniform(&gen->rng_state) * 2.0f * (float)M_PI;
    }

    gen->initialised = true;
    return 0;
}

int eni_sim_bci_add_event(eni_sim_bci_gen_t *gen,
                          const eni_sim_bci_event_t *event)
{
    if (!gen || !event) return -1;
    if (!gen->initialised) return -3;
    if (gen->num_events >= ENI_SIM_MAX_EVENTS) return -2;

    memcpy(&gen->events[gen->num_events], event, sizeof(*event));
    gen->num_events++;
    return 0;
}

int eni_sim_bci_generate(eni_sim_bci_gen_t *gen,
                         float *samples, uint32_t num_channels)
{
    if (!gen || !samples) return -1;
    if (!gen->initialised) return -3;

    uint32_t nc = num_channels;
    if (nc > gen->config.num_channels) nc = gen->config.num_channels;

    float dt = 1.0f / (float)gen->config.sample_rate_hz;
    float amp = gen->config.base_amplitude;
    float f_mu = gen->config.mu_frequency;
    float f_beta = gen->config.beta_frequency;

    for (uint32_t ch = 0; ch < nc; ch++) {
        float mod = compute_modulation(gen, ch);

        /* µ rhythm */
        float mu_val = amp * mod * sinf(gen->phase_mu[ch]);
        gen->phase_mu[ch] += 2.0f * (float)M_PI * f_mu * dt;
        if (gen->phase_mu[ch] > 2.0f * (float)M_PI) {
            gen->phase_mu[ch] -= 2.0f * (float)M_PI;
        }

        /* β rhythm */
        float beta_val = amp * 0.5f * mod * sinf(gen->phase_beta[ch]);
        gen->phase_beta[ch] += 2.0f * (float)M_PI * f_beta * dt;
        if (gen->phase_beta[ch] > 2.0f * (float)M_PI) {
            gen->phase_beta[ch] -= 2.0f * (float)M_PI;
        }

        /* Combine with noise */
        float noise = 0.0f;
        if (gen->config.noise_amplitude > 0.0f) {
            noise = gen->config.noise_amplitude * rand_gaussian(&gen->rng_state);
        }

        samples[ch] = mu_val + beta_val + noise;
    }

    gen->sample_count++;
    return 0;
}

int eni_sim_bci_reset(eni_sim_bci_gen_t *gen)
{
    if (!gen) return -1;
    if (!gen->initialised) return -3;

    gen->sample_count  = 0;
    gen->current_event = 0;
    gen->rng_state     = 0xCAFEBABE;

    for (uint32_t ch = 0; ch < gen->config.num_channels; ch++) {
        gen->phase_mu[ch]   = rand_uniform(&gen->rng_state) * 2.0f * (float)M_PI;
        gen->phase_beta[ch] = rand_uniform(&gen->rng_state) * 2.0f * (float)M_PI;
    }

    return 0;
}
