// SPDX-License-Identifier: MIT
// eNI Feedback Loop — Multi-modal feedback implementation

#include "eni/feedback_loop.h"
#include <string.h>

int eni_fl_init(eni_fl_controller_t *ctrl, const eni_fl_config_t *config)
{
    if (!ctrl || !config) return -1;
    memset(ctrl, 0, sizeof(*ctrl));
    ctrl->config = *config;

    if (ctrl->config.max_electrical_ma <= 0.0f || ctrl->config.max_electrical_ma > 2.0f) {
        ctrl->config.max_electrical_ma = 2.0f;
    }
    if (ctrl->config.min_interval_ms == 0) {
        ctrl->config.min_interval_ms = 100;
    }

    ctrl->initialized = 1;
    return 0;
}

int eni_fl_is_safe(const eni_fl_controller_t *ctrl, const eni_fl_command_t *cmd)
{
    if (!ctrl || !ctrl->initialized || !cmd) return 0;
    if (!ctrl->config.safety_interlock) return 1;

    if (cmd->modality == ENI_FL_ELECTRICAL) {
        if (cmd->params.electrical.amplitude_ma > ctrl->config.max_electrical_ma) return 0;
        if (cmd->params.electrical.amplitude_ma < 0.0f) return 0;
    }

    if (!ctrl->config.enabled_modalities[cmd->modality]) return 0;

    return 1;
}

int eni_fl_send(eni_fl_controller_t *ctrl, const eni_fl_command_t *cmd, uint64_t now_us)
{
    if (!ctrl || !ctrl->initialized || !cmd) return -1;

    if (ctrl->config.safety_interlock && !eni_fl_is_safe(ctrl, cmd)) return -2;

    uint64_t elapsed_us = now_us - ctrl->last_stim_time_us;
    uint64_t min_interval_us = (uint64_t)ctrl->config.min_interval_ms * 1000ULL;
    if (ctrl->last_stim_time_us > 0 && elapsed_us < min_interval_us) return -3;

    ctrl->last_stim_time_us = now_us;

    int idx = ctrl->history_head;
    ctrl->history[idx].stimulus_time_us = now_us;
    ctrl->history[idx].response_time_us = 0;
    ctrl->history[idx].latency_us = 0;
    ctrl->history_head = (ctrl->history_head + 1) % ENI_FL_HISTORY_SIZE;
    if (ctrl->history_count < ENI_FL_HISTORY_SIZE) ctrl->history_count++;

    return 0;
}

int eni_fl_record_response(eni_fl_controller_t *ctrl, uint64_t response_time_us)
{
    if (!ctrl || !ctrl->initialized) return -1;
    if (ctrl->history_count == 0) return -1;

    int last_idx = (ctrl->history_head - 1 + ENI_FL_HISTORY_SIZE) % ENI_FL_HISTORY_SIZE;
    ctrl->history[last_idx].response_time_us = response_time_us;

    if (response_time_us > ctrl->history[last_idx].stimulus_time_us) {
        ctrl->history[last_idx].latency_us = (uint32_t)(response_time_us - ctrl->history[last_idx].stimulus_time_us);
    }

    float sum = 0.0f;
    int count = 0;
    for (int i = 0; i < ctrl->history_count; i++) {
        if (ctrl->history[i].latency_us > 0) {
            sum += (float)ctrl->history[i].latency_us;
            count++;
        }
    }
    ctrl->avg_latency_us = (count > 0) ? sum / (float)count : 0.0f;

    return 0;
}

float eni_fl_get_avg_latency(const eni_fl_controller_t *ctrl)
{
    if (!ctrl) return 0.0f;
    return ctrl->avg_latency_us;
}

void eni_fl_reset(eni_fl_controller_t *ctrl)
{
    if (ctrl) {
        eni_fl_config_t cfg = ctrl->config;
        memset(ctrl, 0, sizeof(*ctrl));
        ctrl->config = cfg;
        ctrl->initialized = 1;
    }
}
