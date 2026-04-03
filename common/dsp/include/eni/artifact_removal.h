/**
 * @file artifact_removal.h
 * @brief Real-time artifact removal for neural signal processing.
 *
 * Provides sliding-window blink detection, adaptive threshold EMG removal,
 * and interpolation-based artifact correction. MCU-safe: no dynamic allocation.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ENI_ARTIFACT_REMOVAL_H
#define ENI_ARTIFACT_REMOVAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------ */
#define ENI_ARTIFACT_MAX_CHANNELS   64
#define ENI_ARTIFACT_WINDOW_SIZE    128
#define ENI_ARTIFACT_HISTORY_LEN    16

/* ---------------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------------ */

/** Per-channel artifact configuration. */
typedef struct {
    float   threshold;          /**< Amplitude threshold (µV). */
    float   blink_threshold;    /**< Blink-specific threshold. */
    float   emg_threshold;      /**< EMG-specific high-freq threshold. */
    float   interp_ratio;       /**< Interpolation blend ratio [0..1]. */
    bool    enabled;            /**< Enable artifact removal on channel. */
} eni_artifact_channel_cfg_t;

/** Top-level artifact removal configuration. */
typedef struct {
    uint32_t                    num_channels;
    uint32_t                    window_size;    /**< Sliding window length. */
    float                       global_threshold;
    float                       adaptive_rate;  /**< Threshold adaptation speed. */
    eni_artifact_channel_cfg_t  channels[ENI_ARTIFACT_MAX_CHANNELS];
} eni_artifact_config_t;

/** Detection result flags. */
typedef enum {
    ENI_ARTIFACT_NONE   = 0,
    ENI_ARTIFACT_BLINK  = (1 << 0),
    ENI_ARTIFACT_EMG    = (1 << 1),
    ENI_ARTIFACT_MOTION = (1 << 2),
    ENI_ARTIFACT_SAT    = (1 << 3)
} eni_artifact_flag_t;

/** Internal per-channel state. */
typedef struct {
    float   ring[ENI_ARTIFACT_WINDOW_SIZE]; /**< Circular sample buffer. */
    float   running_mean;
    float   running_var;
    float   adaptive_thresh;
    uint32_t head;                          /**< Ring write index. */
    uint32_t count;                         /**< Samples seen. */
    uint32_t artifact_count;
} eni_artifact_chan_state_t;

/** Artifact removal engine (stack-allocated). */
typedef struct {
    eni_artifact_config_t       config;
    eni_artifact_chan_state_t    state[ENI_ARTIFACT_MAX_CHANNELS];
    bool                        initialised;
} eni_artifact_engine_t;

/* ---------------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------------ */

/**
 * Initialise the artifact removal engine.
 * @return 0 on success, negative on error.
 */
int eni_artifact_init(eni_artifact_engine_t *engine,
                      const eni_artifact_config_t *config);

/**
 * Process samples in-place: detect and remove artifacts.
 * @param samples   Array [num_channels] of new samples.
 * @param flags     Output flags per channel (caller allocates).
 * @return 0 on success.
 */
int eni_artifact_process(eni_artifact_engine_t *engine,
                         float *samples,
                         uint32_t num_channels,
                         eni_artifact_flag_t *flags);

/**
 * Detect blink artifact on a single channel.
 * @return true if blink detected.
 */
bool eni_artifact_detect_blink(const eni_artifact_engine_t *engine,
                               uint32_t channel);

/**
 * Detect EMG artifact on a single channel.
 * @return true if EMG detected.
 */
bool eni_artifact_detect_emg(const eni_artifact_engine_t *engine,
                             uint32_t channel);

/**
 * Reset engine state (keep configuration).
 */
int eni_artifact_reset(eni_artifact_engine_t *engine);

#ifdef __cplusplus
}
#endif

#endif /* ENI_ARTIFACT_REMOVAL_H */
