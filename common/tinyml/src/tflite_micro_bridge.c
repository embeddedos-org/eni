// SPDX-License-Identifier: MIT
// eNI TFLite Micro Bridge — C bridge to TFLite Micro C++ API

#include "eni/tflite_micro.h"
#include <string.h>

eni_tflite_status_t eni_tflite_init(eni_tflite_interpreter_t *interp, const eni_tflite_config_t *config)
{
    if (!interp || !config) return ENI_TFLITE_ERR_INIT;
    if (!config->model_data || config->model_size == 0) return ENI_TFLITE_ERR_MODEL;
    if (config->arena_size > ENI_TFLITE_MAX_ARENA_SIZE) return ENI_TFLITE_ERR_ARENA;

    memset(interp, 0, sizeof(*interp));
    interp->arena_size = config->arena_size > 0 ? config->arena_size : ENI_TFLITE_MAX_ARENA_SIZE;

    /* TODO: When TFLite Micro is available, initialize:
     * - tflite::MicroInterpreter
     * - tflite::MicroMutableOpResolver
     * - tflite::GetModel(config->model_data)
     * - interpreter->AllocateTensors()
     */

    interp->num_inputs = 1;
    interp->num_outputs = 1;
    interp->input_info[0].type = ENI_TFLITE_TYPE_FLOAT32;
    interp->input_info[0].dims[0] = 1;
    interp->input_info[0].num_dims = 1;
    interp->output_info[0].type = ENI_TFLITE_TYPE_FLOAT32;
    interp->output_info[0].dims[0] = 1;
    interp->output_info[0].num_dims = 1;
    interp->initialized = 1;

    return ENI_TFLITE_OK;
}

eni_tflite_status_t eni_tflite_set_input(eni_tflite_interpreter_t *interp, int index, const void *data, size_t size)
{
    if (!interp || !interp->initialized) return ENI_TFLITE_ERR_INIT;
    if (index < 0 || index >= interp->num_inputs) return ENI_TFLITE_ERR_TENSOR;
    if (!data || size == 0) return ENI_TFLITE_ERR_TENSOR;

    /* TODO: Copy data to input tensor via interpreter->input(index)->data */
    (void)data;
    (void)size;

    return ENI_TFLITE_OK;
}

eni_tflite_status_t eni_tflite_invoke(eni_tflite_interpreter_t *interp)
{
    if (!interp || !interp->initialized) return ENI_TFLITE_ERR_INIT;

    /* TODO: Call interpreter->Invoke() and check return status */

    return ENI_TFLITE_OK;
}

eni_tflite_status_t eni_tflite_get_output(const eni_tflite_interpreter_t *interp, int index, void *data, size_t size)
{
    if (!interp || !interp->initialized) return ENI_TFLITE_ERR_INIT;
    if (index < 0 || index >= interp->num_outputs) return ENI_TFLITE_ERR_TENSOR;
    if (!data || size == 0) return ENI_TFLITE_ERR_TENSOR;

    /* TODO: Copy output tensor data via interpreter->output(index)->data */
    memset(data, 0, size);

    return ENI_TFLITE_OK;
}

eni_tflite_status_t eni_tflite_get_input_info(const eni_tflite_interpreter_t *interp, int index, eni_tflite_tensor_info_t *info)
{
    if (!interp || !interp->initialized || !info) return ENI_TFLITE_ERR_INIT;
    if (index < 0 || index >= interp->num_inputs) return ENI_TFLITE_ERR_TENSOR;
    *info = interp->input_info[index];
    return ENI_TFLITE_OK;
}

eni_tflite_status_t eni_tflite_get_output_info(const eni_tflite_interpreter_t *interp, int index, eni_tflite_tensor_info_t *info)
{
    if (!interp || !interp->initialized || !info) return ENI_TFLITE_ERR_INIT;
    if (index < 0 || index >= interp->num_outputs) return ENI_TFLITE_ERR_TENSOR;
    *info = interp->output_info[index];
    return ENI_TFLITE_OK;
}

void eni_tflite_deinit(eni_tflite_interpreter_t *interp)
{
    if (interp) {
        interp->initialized = 0;
    }
}
