// SPDX-License-Identifier: MIT
// eNI TensorFlow Lite Micro Integration

#ifndef ENI_TFLITE_MICRO_H
#define ENI_TFLITE_MICRO_H

#include <stdint.h>
#include <stddef.h>

#define ENI_TFLITE_MAX_ARENA_SIZE  (64 * 1024)
#define ENI_TFLITE_MAX_INPUTS     8
#define ENI_TFLITE_MAX_OUTPUTS    8

typedef enum {
    ENI_TFLITE_OK = 0,
    ENI_TFLITE_ERR_INIT,
    ENI_TFLITE_ERR_MODEL,
    ENI_TFLITE_ERR_INVOKE,
    ENI_TFLITE_ERR_TENSOR,
    ENI_TFLITE_ERR_ARENA,
} eni_tflite_status_t;

typedef enum {
    ENI_TFLITE_TYPE_FLOAT32 = 0,
    ENI_TFLITE_TYPE_INT8,
    ENI_TFLITE_TYPE_UINT8,
    ENI_TFLITE_TYPE_INT32,
} eni_tflite_tensor_type_t;

typedef struct {
    uint32_t                arena_size;
    const uint8_t          *model_data;
    uint32_t                model_size;
    int                     num_threads;
} eni_tflite_config_t;

typedef struct {
    eni_tflite_tensor_type_t type;
    int                      dims[4];
    int                      num_dims;
    int                      bytes;
} eni_tflite_tensor_info_t;

typedef struct {
    uint8_t                  arena[ENI_TFLITE_MAX_ARENA_SIZE];
    uint32_t                 arena_size;
    int                      initialized;
    int                      num_inputs;
    int                      num_outputs;
    eni_tflite_tensor_info_t input_info[ENI_TFLITE_MAX_INPUTS];
    eni_tflite_tensor_info_t output_info[ENI_TFLITE_MAX_OUTPUTS];
} eni_tflite_interpreter_t;

eni_tflite_status_t eni_tflite_init(eni_tflite_interpreter_t *interp, const eni_tflite_config_t *config);
eni_tflite_status_t eni_tflite_set_input(eni_tflite_interpreter_t *interp, int index, const void *data, size_t size);
eni_tflite_status_t eni_tflite_invoke(eni_tflite_interpreter_t *interp);
eni_tflite_status_t eni_tflite_get_output(const eni_tflite_interpreter_t *interp, int index, void *data, size_t size);
eni_tflite_status_t eni_tflite_get_input_info(const eni_tflite_interpreter_t *interp, int index, eni_tflite_tensor_info_t *info);
eni_tflite_status_t eni_tflite_get_output_info(const eni_tflite_interpreter_t *interp, int index, eni_tflite_tensor_info_t *info);
void                eni_tflite_deinit(eni_tflite_interpreter_t *interp);

#endif /* ENI_TFLITE_MICRO_H */
