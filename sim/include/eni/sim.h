/**
 * @file sim.h
 * @brief Real-time simulation environment API for eNI.
 *
 * Provides synthetic EEG and BCI signal generators, a simulation clock,
 * and an environment wrapper for integration testing without hardware.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ENI_SIM_H
#define ENI_SIM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------ */
#define ENI_SIM_MAX_CHANNELS    64
#define ENI_SIM_MAX_EVENTS      32

/* ---------------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------------ */

/** Simulation clock mode. */
typedef enum {
    ENI_SIM_CLOCK_REALTIME,     /**< Wall-clock pacing. */
    ENI_SIM_CLOCK_FAST,         /**< As fast as possible. */
    ENI_SIM_CLOCK_STEPPED       /**< Manual tick advance. */
} eni_sim_clock_mode_t;

/** EEG band specification for synthetic generator. */
typedef enum {
    ENI_SIM_BAND_DELTA = 0,     /**< 0.5–4 Hz */
    ENI_SIM_BAND_THETA,         /**< 4–8 Hz */
    ENI_SIM_BAND_ALPHA,         /**< 8–13 Hz */
    ENI_SIM_BAND_BETA,          /**< 13–30 Hz */
    ENI_SIM_BAND_GAMMA,         /**< 30–100 Hz */
    ENI_SIM_BAND_COUNT
} eni_sim_eeg_band_t;

/** Per-channel EEG generator config. */
typedef struct {
    float amplitude[ENI_SIM_BAND_COUNT]; /**< Band amplitudes (µV). */
    float noise_amplitude;               /**< Gaussian noise amplitude. */
    float blink_probability;             /**< Blink artifact probability. */
    float blink_amplitude;               /**< Blink artifact amplitude. */
    bool  enabled;
} eni_sim_eeg_channel_t;

typedef struct {
    uint32_t                num_channels;
    uint32_t                sample_rate_hz;
    eni_sim_eeg_channel_t   channels[ENI_SIM_MAX_CHANNELS];
} eni_sim_eeg_config_t;

/** BCI event for simulation. */
typedef enum {
    ENI_SIM_BCI_IDLE = 0,
    ENI_SIM_BCI_LEFT_HAND,
    ENI_SIM_BCI_RIGHT_HAND,
    ENI_SIM_BCI_FEET,
    ENI_SIM_BCI_TONGUE,
    ENI_SIM_BCI_REST
} eni_sim_bci_intent_t;

typedef struct {
    eni_sim_bci_intent_t intent;
    uint64_t             start_tick;
    uint64_t             duration_ticks;
    float                strength;          /**< Modulation strength [0..1]. */
} eni_sim_bci_event_t;

/** BCI generator config. */
typedef struct {
    uint32_t            num_channels;
    uint32_t            sample_rate_hz;
    float               mu_frequency;       /**< µ rhythm centre (Hz). */
    float               beta_frequency;     /**< β rhythm centre (Hz). */
    float               base_amplitude;     /**< Baseline amplitude (µV). */
    float               noise_amplitude;
} eni_sim_bci_config_t;

/** EEG generator state. */
typedef struct {
    eni_sim_eeg_config_t config;
    float    phase[ENI_SIM_MAX_CHANNELS][ENI_SIM_BAND_COUNT];
    uint32_t rng_state;
    uint64_t sample_count;
    bool     initialised;
} eni_sim_eeg_gen_t;

/** BCI generator state. */
typedef struct {
    eni_sim_bci_config_t config;
    eni_sim_bci_event_t  events[ENI_SIM_MAX_EVENTS];
    uint32_t             num_events;
    uint32_t             current_event;
    float                phase_mu[ENI_SIM_MAX_CHANNELS];
    float                phase_beta[ENI_SIM_MAX_CHANNELS];
    uint32_t             rng_state;
    uint64_t             sample_count;
    bool                 initialised;
} eni_sim_bci_gen_t;

/** Simulation environment. */
typedef struct {
    eni_sim_clock_mode_t clock_mode;
    uint64_t             tick;              /**< Current sim tick (µs). */
    uint64_t             tick_step;         /**< Microseconds per step. */
    uint32_t             sample_rate_hz;

    eni_sim_eeg_gen_t   *eeg_gen;          /**< Optional EEG generator. */
    eni_sim_bci_gen_t   *bci_gen;          /**< Optional BCI generator. */

    bool                 running;
    bool                 initialised;
} eni_sim_env_t;

/* ---------------------------------------------------------------------------
 * Simulation environment API
 * ------------------------------------------------------------------------ */

int  eni_sim_init(eni_sim_env_t *env, uint32_t sample_rate_hz,
                  eni_sim_clock_mode_t clock_mode);
int  eni_sim_step(eni_sim_env_t *env);
int  eni_sim_run(eni_sim_env_t *env, uint32_t num_steps);
int  eni_sim_stop(eni_sim_env_t *env);
int  eni_sim_reset(eni_sim_env_t *env);
uint64_t eni_sim_get_tick(const eni_sim_env_t *env);

/* ---------------------------------------------------------------------------
 * EEG generator API
 * ------------------------------------------------------------------------ */

int  eni_sim_eeg_init(eni_sim_eeg_gen_t *gen,
                      const eni_sim_eeg_config_t *config);
int  eni_sim_eeg_generate(eni_sim_eeg_gen_t *gen,
                          float *samples, uint32_t num_channels);
int  eni_sim_eeg_reset(eni_sim_eeg_gen_t *gen);

/* ---------------------------------------------------------------------------
 * BCI generator API
 * ------------------------------------------------------------------------ */

int  eni_sim_bci_init(eni_sim_bci_gen_t *gen,
                      const eni_sim_bci_config_t *config);
int  eni_sim_bci_add_event(eni_sim_bci_gen_t *gen,
                           const eni_sim_bci_event_t *event);
int  eni_sim_bci_generate(eni_sim_bci_gen_t *gen,
                          float *samples, uint32_t num_channels);
int  eni_sim_bci_reset(eni_sim_bci_gen_t *gen);

#ifdef __cplusplus
}
#endif

#endif /* ENI_SIM_H */
