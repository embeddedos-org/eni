// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// Mock neural input generator for testing

#ifndef ENI_MOCK_NEURAL_INPUT_H
#define ENI_MOCK_NEURAL_INPUT_H

#include "eni/types.h"
#include "eni/dsp.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MOCK_MAX_CHANNELS      64
#define MOCK_MAX_SAMPLES       512
#define MOCK_DEFAULT_RATE      256.0f

typedef enum {
    MOCK_PATTERN_SINE,
    MOCK_PATTERN_ALPHA_DOMINANT,
    MOCK_PATTERN_BETA_DOMINANT,
    MOCK_PATTERN_MOTOR_IMAGERY,
    MOCK_PATTERN_IDLE,
    MOCK_PATTERN_ARTIFACT_BLINK,
    MOCK_PATTERN_ARTIFACT_MUSCLE,
    MOCK_PATTERN_WHITE_NOISE,
    MOCK_PATTERN_CHIRP,
    MOCK_PATTERN_ZERO,
} mock_pattern_t;

typedef struct {
    mock_pattern_t pattern;
    float          amplitude;
    float          frequency_hz;
    float          sample_rate;
    float          noise_level;
    int            num_channels;
    int            num_samples;
    uint32_t       seed;
} mock_neural_config_t;

typedef struct {
    float   samples[MOCK_MAX_CHANNELS][MOCK_MAX_SAMPLES];
    int     num_channels;
    int     num_samples;
    float   sample_rate;
    int     current_index;
    uint32_t rng_state;
    mock_neural_config_t config;
} mock_neural_input_t;

void mock_neural_config_defaults(mock_neural_config_t *cfg);

eni_status_t mock_neural_init(mock_neural_input_t *input,
                              const mock_neural_config_t *cfg);

eni_status_t mock_neural_generate(mock_neural_input_t *input);

eni_status_t mock_neural_get_channel(const mock_neural_input_t *input,
                                     int channel, float *out, int max_samples);

eni_status_t mock_neural_get_epoch(const mock_neural_input_t *input,
                                   int channel, eni_dsp_epoch_t *epoch);

float mock_neural_next_sample(mock_neural_input_t *input, int channel);

void mock_neural_reset(mock_neural_input_t *input);

#ifdef __cplusplus
}
#endif

#endif /* ENI_MOCK_NEURAL_INPUT_H */
