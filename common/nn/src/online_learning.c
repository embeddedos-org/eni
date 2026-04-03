// SPDX-License-Identifier: MIT
// eNI Online Learning — Incremental SGD for lightweight models

#include "eni/online_learning.h"
#include <string.h>
#include <math.h>

int eni_ol_init(eni_ol_model_t *model, const eni_ol_config_t *config, int num_params)
{
    if (!model || !config) return -1;
    if (num_params <= 0 || num_params > ENI_OL_MAX_PARAMS) return -1;

    memset(model, 0, sizeof(*model));
    model->config = *config;
    model->num_params = num_params;
    model->replay.capacity = ENI_OL_REPLAY_CAPACITY;

    if (model->config.learning_rate <= 0.0f) model->config.learning_rate = 0.01f;
    if (model->config.batch_size <= 0) model->config.batch_size = 1;

    for (int i = 0; i < num_params; i++) {
        model->params[i] = 0.001f * ((float)(i % 100) / 100.0f - 0.5f);
    }

    return 0;
}

static float sigmoid(float x)
{
    if (x > 10.0f) return 1.0f;
    if (x < -10.0f) return 0.0f;
    return 1.0f / (1.0f + expf(-x));
}

int eni_ol_update(eni_ol_model_t *model, const float *features, int label, int num_features)
{
    if (!model || !features) return -1;
    if (num_features <= 0 || num_features > 64) return -1;

    float logit = 0.0f;
    for (int i = 0; i < num_features && i < model->num_params; i++) {
        logit += model->params[i] * features[i];
    }

    float pred = sigmoid(logit);
    float target = (label > 0) ? 1.0f : 0.0f;
    float error = pred - target;

    model->running_loss = 0.99f * model->running_loss + 0.01f * (error * error);

    for (int i = 0; i < num_features && i < model->num_params; i++) {
        float grad = error * features[i] + model->config.weight_decay * model->params[i];
        model->velocity[i] = model->config.momentum * model->velocity[i] + grad;
        model->params[i] -= model->config.learning_rate * model->velocity[i];
    }

    model->update_count++;
    return 0;
}

float eni_ol_predict(const eni_ol_model_t *model, const float *features, int num_features, int num_classes)
{
    if (!model || !features) return 0.0f;
    (void)num_classes;

    float logit = 0.0f;
    for (int i = 0; i < num_features && i < model->num_params; i++) {
        logit += model->params[i] * features[i];
    }
    return sigmoid(logit);
}

int eni_ol_replay_add(eni_ol_model_t *model, const float *features, int label, int num_features)
{
    if (!model || !features) return -1;
    if (num_features <= 0 || num_features > 64) return -1;

    int idx = model->replay.head;
    eni_ol_sample_t *s = &model->replay.samples[idx];
    memset(s, 0, sizeof(*s));
    for (int i = 0; i < num_features && i < 64; i++) {
        s->features[i] = features[i];
    }
    s->label = label;
    s->weight = 1.0f;

    model->replay.head = (model->replay.head + 1) % model->replay.capacity;
    if (model->replay.count < model->replay.capacity) {
        model->replay.count++;
    }
    return 0;
}

int eni_ol_replay_train(eni_ol_model_t *model, int num_steps, int num_features, int num_classes)
{
    if (!model || model->replay.count == 0) return -1;
    (void)num_classes;

    for (int step = 0; step < num_steps; step++) {
        int idx = step % model->replay.count;
        eni_ol_sample_t *s = &model->replay.samples[idx];
        eni_ol_update(model, s->features, s->label, num_features);
    }
    return 0;
}

int eni_ol_save_checkpoint(const eni_ol_model_t *model, void *buffer, size_t buffer_size)
{
    if (!model || !buffer) return -1;
    if (buffer_size < sizeof(eni_ol_model_t)) return -1;
    memcpy(buffer, model, sizeof(eni_ol_model_t));
    return 0;
}

int eni_ol_load_checkpoint(eni_ol_model_t *model, const void *buffer, size_t buffer_size)
{
    if (!model || !buffer) return -1;
    if (buffer_size < sizeof(eni_ol_model_t)) return -1;
    memcpy(model, buffer, sizeof(eni_ol_model_t));
    return 0;
}

float eni_ol_get_loss(const eni_ol_model_t *model)
{
    if (!model) return 0.0f;
    return model->running_loss;
}

void eni_ol_reset(eni_ol_model_t *model)
{
    if (model) {
        eni_ol_config_t cfg = model->config;
        int np = model->num_params;
        memset(model, 0, sizeof(*model));
        model->config = cfg;
        model->num_params = np;
        model->replay.capacity = ENI_OL_REPLAY_CAPACITY;
    }
}
