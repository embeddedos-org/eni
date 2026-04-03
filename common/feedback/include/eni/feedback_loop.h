// SPDX-License-Identifier: MIT
// eNI Feedback Loop — Multi-modal feedback with latency tracking

#ifndef ENI_FEEDBACK_LOOP_H
#define ENI_FEEDBACK_LOOP_H

#include <stdint.h>

#define ENI_FL_MAX_MODALITIES  4
#define ENI_FL_HISTORY_SIZE    64

typedef enum {
    ENI_FL_VISUAL = 0,
    ENI_FL_HAPTIC,
    ENI_FL_AUDITORY,
    ENI_FL_ELECTRICAL,
} eni_fl_modality_t;

typedef struct {
    uint8_t  pattern[16];
    uint8_t  pattern_len;
    uint16_t duration_ms;
    uint8_t  intensity;
} eni_fl_visual_params_t;

typedef struct {
    uint16_t frequency_hz;
    uint16_t duration_ms;
    uint8_t  intensity;
} eni_fl_haptic_params_t;

typedef struct {
    float    amplitude_ma;
    uint16_t frequency_hz;
    uint16_t duration_ms;
    uint8_t  waveform;
} eni_fl_electrical_params_t;

typedef struct {
    eni_fl_modality_t modality;
    union {
        eni_fl_visual_params_t     visual;
        eni_fl_haptic_params_t     haptic;
        eni_fl_electrical_params_t electrical;
    } params;
} eni_fl_command_t;

typedef struct {
    uint64_t stimulus_time_us;
    uint64_t response_time_us;
    uint32_t latency_us;
} eni_fl_latency_entry_t;

typedef struct {
    int                    enabled_modalities[ENI_FL_MAX_MODALITIES];
    float                  max_electrical_ma;
    uint32_t               min_interval_ms;
    int                    safety_interlock;
} eni_fl_config_t;

typedef struct {
    eni_fl_config_t        config;
    eni_fl_latency_entry_t history[ENI_FL_HISTORY_SIZE];
    int                    history_count;
    int                    history_head;
    uint64_t               last_stim_time_us;
    float                  avg_latency_us;
    int                    initialized;
} eni_fl_controller_t;

int      eni_fl_init(eni_fl_controller_t *ctrl, const eni_fl_config_t *config);
int      eni_fl_send(eni_fl_controller_t *ctrl, const eni_fl_command_t *cmd, uint64_t now_us);
int      eni_fl_record_response(eni_fl_controller_t *ctrl, uint64_t response_time_us);
float    eni_fl_get_avg_latency(const eni_fl_controller_t *ctrl);
int      eni_fl_is_safe(const eni_fl_controller_t *ctrl, const eni_fl_command_t *cmd);
void     eni_fl_reset(eni_fl_controller_t *ctrl);

#endif /* ENI_FEEDBACK_LOOP_H */
