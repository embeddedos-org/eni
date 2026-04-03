// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// Mock neural input generator implementation

#include "mock_neural_input.h"
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static uint32_t mock_rng(uint32_t *state) {
    *state ^= *state << 13;
    *state ^= *state >> 17;
    *state ^= *state << 5;
    return *state;
}

static float mock_rng_float(uint32_t *state) {
    return (float)(mock_rng(state) & 0xFFFF) / 65536.0f;
}

static float mock_rng_gaussian(uint32_t *state) {
    float u1 = mock_rng_float(state);
    float u2 = mock_rng_float(state);
    if (u1 < 1e-7f) u1 = 1e-7f;
    return sqrtf(-2.0f * logf(u1)) * cosf(2.0f * (float)M_PI * u2);
}

void mock_neural_config_defaults(mock_neural_config_t *cfg) {
    if (!cfg) return;
    memset(cfg, 0, sizeof(*cfg));
    cfg->pattern = MOCK_PATTERN_ALPHA_DOMINANT;
    cfg->amplitude = 50.0f;
    cfg->frequency_hz = 10.0f;
    cfg->sample_rate = MOCK_DEFAULT_RATE;
    cfg->noise_level = 5.0f;
    cfg->num_channels = 1;
    cfg->num_samples = 256;
    cfg->seed = 42;
}

eni_status_t mock_neural_init(mock_neural_input_t *input,
                              const mock_neural_config_t *cfg) {
    if (!input || !cfg) return ENI_ERR_INVALID;
    if (cfg->num_channels <= 0 || cfg->num_channels > MOCK_MAX_CHANNELS)
        return ENI_ERR_INVALID;
    if (cfg->num_samples <= 0 || cfg->num_samples > MOCK_MAX_SAMPLES)
        return ENI_ERR_INVALID;
    if (cfg->sample_rate <= 0.0f) return ENI_ERR_INVALID;

    memset(input, 0, sizeof(*input));
    input->config = *cfg;
    input->num_channels = cfg->num_channels;
    input->num_samples = cfg->num_samples;
    input->sample_rate = cfg->sample_rate;
    input->current_index = 0;
    input->rng_state = cfg->seed;

    return ENI_OK;
}

eni_status_t mock_neural_generate(mock_neural_input_t *input) {
    if (!input) return ENI_ERR_INVALID;

    const mock_neural_config_t *cfg = &input->config;
    float amp = cfg->amplitude;
    float sr = cfg->sample_rate;
    int n = input->num_samples;

    for (int ch = 0; ch < input->num_channels; ++ch) {
        float phase_offset = (float)ch * 0.1f;

        for (int i = 0; i < n; ++i) {
            float t = (float)i / sr;
            float sample = 0.0f;

            switch (cfg->pattern) {
            case MOCK_PATTERN_SINE:
                sample = amp * sinf(2.0f * (float)M_PI * cfg->frequency_hz * t + phase_offset);
                break;

            case MOCK_PATTERN_ALPHA_DOMINANT:
                sample = amp * sinf(2.0f * (float)M_PI * 10.0f * t + phase_offset)
                       + 0.3f * amp * sinf(2.0f * (float)M_PI * 20.0f * t)
                       + 0.1f * amp * sinf(2.0f * (float)M_PI * 5.0f * t);
                break;

            case MOCK_PATTERN_BETA_DOMINANT:
                sample = amp * sinf(2.0f * (float)M_PI * 22.0f * t + phase_offset)
                       + 0.2f * amp * sinf(2.0f * (float)M_PI * 10.0f * t)
                       + 0.15f * amp * sinf(2.0f * (float)M_PI * 35.0f * t);
                break;

            case MOCK_PATTERN_MOTOR_IMAGERY:
                sample = 0.5f * amp * sinf(2.0f * (float)M_PI * 12.0f * t)
                       + amp * sinf(2.0f * (float)M_PI * 22.0f * t + phase_offset)
                       + 0.3f * amp * sinf(2.0f * (float)M_PI * 70.0f * t);
                break;

            case MOCK_PATTERN_IDLE:
                sample = 0.1f * amp * sinf(2.0f * (float)M_PI * 3.0f * t)
                       + 0.05f * amp * sinf(2.0f * (float)M_PI * 8.0f * t);
                break;

            case MOCK_PATTERN_ARTIFACT_BLINK:
                sample = amp * sinf(2.0f * (float)M_PI * 10.0f * t);
                if (i > n / 4 && i < n / 4 + 10) sample += 10.0f * amp;
                break;

            case MOCK_PATTERN_ARTIFACT_MUSCLE:
                sample = amp * sinf(2.0f * (float)M_PI * 10.0f * t);
                if (i > n / 3 && i < n / 3 + n / 6)
                    sample += 3.0f * amp * mock_rng_gaussian(&input->rng_state);
                break;

            case MOCK_PATTERN_WHITE_NOISE:
                sample = amp * mock_rng_gaussian(&input->rng_state);
                break;

            case MOCK_PATTERN_CHIRP:
                {
                    float f_inst = 1.0f + (50.0f - 1.0f) * t * sr / (float)n;
                    sample = amp * sinf(2.0f * (float)M_PI * f_inst * t);
                }
                break;

            case MOCK_PATTERN_ZERO:
                sample = 0.0f;
                break;
            }

            if (cfg->noise_level > 0.0f && cfg->pattern != MOCK_PATTERN_ZERO) {
                sample += cfg->noise_level * mock_rng_gaussian(&input->rng_state);
            }

            input->samples[ch][i] = sample;
        }
    }

    input->current_index = 0;
    return ENI_OK;
}

eni_status_t mock_neural_get_channel(const mock_neural_input_t *input,
                                     int channel, float *out, int max_samples) {
    if (!input || !out) return ENI_ERR_INVALID;
    if (channel < 0 || channel >= input->num_channels) return ENI_ERR_INVALID;

    int count = input->num_samples < max_samples ? input->num_samples : max_samples;
    memcpy(out, input->samples[channel], (size_t)count * sizeof(float));
    return ENI_OK;
}

eni_status_t mock_neural_get_epoch(const mock_neural_input_t *input,
                                   int channel, eni_dsp_epoch_t *epoch) {
    if (!input || !epoch) return ENI_ERR_INVALID;
    if (channel < 0 || channel >= input->num_channels) return ENI_ERR_INVALID;

    int count = input->num_samples;
    if (count > ENI_DSP_MAX_FFT_SIZE) count = ENI_DSP_MAX_FFT_SIZE;

    eni_dsp_epoch_init(epoch, count, input->sample_rate);
    for (int i = 0; i < count; ++i) {
        eni_dsp_epoch_push(epoch, input->samples[channel][i]);
    }
    return ENI_OK;
}

float mock_neural_next_sample(mock_neural_input_t *input, int channel) {
    if (!input || channel < 0 || channel >= input->num_channels) return 0.0f;
    if (input->current_index >= input->num_samples) input->current_index = 0;
    return input->samples[channel][input->current_index++];
}

void mock_neural_reset(mock_neural_input_t *input) {
    if (!input) return;
    input->current_index = 0;
    input->rng_state = input->config.seed;
}
