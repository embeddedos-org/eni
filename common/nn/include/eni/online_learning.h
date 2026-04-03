// SPDX-License-Identifier: MIT
// eNI Online Learning — Incremental model updates for MCU targets

#ifndef ENI_ONLINE_LEARNING_H
#define ENI_ONLINE_LEARNING_H

#include <stdint.h>
#include <stddef.h>

#define ENI_OL_MAX_PARAMS       4096
#define ENI_OL_MAX_BATCH        32
#define ENI_OL_REPLAY_CAPACITY  128

typedef struct {
    float   learning_rate;
    float   momentum;
    float   weight_decay;
    int     batch_size;
    int     max_epochs;
} eni_ol_config_t;

typedef struct {
    float   features[64];
    int     label;
    float   weight;
} eni_ol_sample_t;

typedef struct {
    eni_ol_sample_t samples[ENI_OL_REPLAY_CAPACITY];
    int             count;
    int             head;
    int             capacity;
} eni_ol_replay_buffer_t;

typedef struct {
    float               params[ENI_OL_MAX_PARAMS];
    float               gradients[ENI_OL_MAX_PARAMS];
    float               velocity[ENI_OL_MAX_PARAMS];
    int                 num_params;
    eni_ol_config_t     config;
    eni_ol_replay_buffer_t replay;
    uint32_t            update_count;
    float               running_loss;
} eni_ol_model_t;

int     eni_ol_init(eni_ol_model_t *model, const eni_ol_config_t *config, int num_params);
int     eni_ol_update(eni_ol_model_t *model, const float *features, int label, int num_features);
float   eni_ol_predict(const eni_ol_model_t *model, const float *features, int num_features, int num_classes);
int     eni_ol_replay_add(eni_ol_model_t *model, const float *features, int label, int num_features);
int     eni_ol_replay_train(eni_ol_model_t *model, int num_steps, int num_features, int num_classes);
int     eni_ol_save_checkpoint(const eni_ol_model_t *model, void *buffer, size_t buffer_size);
int     eni_ol_load_checkpoint(eni_ol_model_t *model, const void *buffer, size_t buffer_size);
float   eni_ol_get_loss(const eni_ol_model_t *model);
void    eni_ol_reset(eni_ol_model_t *model);

#endif /* ENI_ONLINE_LEARNING_H */
