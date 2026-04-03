/**
 * @file sim_environment.c
 * @brief Simulation environment engine implementation.
 *
 * SPDX-License-Identifier: MIT
 */

#include "eni/sim.h"
#include <string.h>

/* ---- Public API --------------------------------------------------------- */

int eni_sim_init(eni_sim_env_t *env, uint32_t sample_rate_hz,
                 eni_sim_clock_mode_t clock_mode)
{
    if (!env) return -1;
    if (sample_rate_hz == 0) return -2;

    memset(env, 0, sizeof(*env));
    env->clock_mode     = clock_mode;
    env->sample_rate_hz = sample_rate_hz;
    env->tick_step      = 1000000ULL / (uint64_t)sample_rate_hz;
    env->initialised    = true;
    return 0;
}

int eni_sim_step(eni_sim_env_t *env)
{
    if (!env) return -1;
    if (!env->initialised) return -3;

    env->tick += env->tick_step;

    /* Generate EEG samples if attached */
    if (env->eeg_gen && env->eeg_gen->initialised) {
        float buf[ENI_SIM_MAX_CHANNELS];
        eni_sim_eeg_generate(env->eeg_gen, buf,
                             env->eeg_gen->config.num_channels);
    }

    /* Generate BCI samples if attached */
    if (env->bci_gen && env->bci_gen->initialised) {
        float buf[ENI_SIM_MAX_CHANNELS];
        eni_sim_bci_generate(env->bci_gen, buf,
                             env->bci_gen->config.num_channels);
    }

    return 0;
}

int eni_sim_run(eni_sim_env_t *env, uint32_t num_steps)
{
    if (!env) return -1;
    if (!env->initialised) return -3;

    env->running = true;
    for (uint32_t i = 0; i < num_steps && env->running; i++) {
        int rc = eni_sim_step(env);
        if (rc != 0) return rc;
    }
    env->running = false;
    return 0;
}

int eni_sim_stop(eni_sim_env_t *env)
{
    if (!env) return -1;
    env->running = false;
    return 0;
}

int eni_sim_reset(eni_sim_env_t *env)
{
    if (!env) return -1;
    if (!env->initialised) return -3;

    env->tick    = 0;
    env->running = false;

    if (env->eeg_gen) eni_sim_eeg_reset(env->eeg_gen);
    if (env->bci_gen) eni_sim_bci_reset(env->bci_gen);

    return 0;
}

uint64_t eni_sim_get_tick(const eni_sim_env_t *env)
{
    if (!env || !env->initialised) return 0;
    return env->tick;
}
