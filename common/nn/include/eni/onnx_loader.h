/**
 * @file onnx_loader.h
 * @brief Minimal ONNX model loader for embedded neural network inference.
 *
 * Parses a subset of ONNX protobuf format. No external dependencies.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ENI_ONNX_LOADER_H
#define ENI_ONNX_LOADER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------ */
#define ENI_ONNX_MAX_LAYERS         32
#define ENI_ONNX_MAX_NAME_LEN       64
#define ENI_ONNX_MAX_DIMS            8
#define ENI_ONNX_MAX_TENSOR_SIZE  8192
#define ENI_ONNX_MAX_MODEL_SIZE  (128 * 1024)

/* ---------------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------------ */

typedef enum {
    ENI_ONNX_OK = 0,
    ENI_ONNX_ERR_NULL,
    ENI_ONNX_ERR_SIZE,
    ENI_ONNX_ERR_FORMAT,
    ENI_ONNX_ERR_VERSION,
    ENI_ONNX_ERR_LAYER_COUNT,
    ENI_ONNX_ERR_TENSOR_SIZE,
    ENI_ONNX_ERR_UNSUPPORTED_OP,
    ENI_ONNX_ERR_IO
} eni_onnx_status_t;

typedef enum {
    ENI_ONNX_DTYPE_FLOAT32 = 1,
    ENI_ONNX_DTYPE_INT8    = 3,
    ENI_ONNX_DTYPE_INT32   = 6,
    ENI_ONNX_DTYPE_INT64   = 7
} eni_onnx_dtype_t;

typedef enum {
    ENI_ONNX_OP_UNKNOWN = 0,
    ENI_ONNX_OP_CONV,
    ENI_ONNX_OP_RELU,
    ENI_ONNX_OP_SIGMOID,
    ENI_ONNX_OP_TANH,
    ENI_ONNX_OP_MATMUL,
    ENI_ONNX_OP_ADD,
    ENI_ONNX_OP_LSTM,
    ENI_ONNX_OP_SOFTMAX,
    ENI_ONNX_OP_FLATTEN,
    ENI_ONNX_OP_MAXPOOL,
    ENI_ONNX_OP_AVGPOOL,
    ENI_ONNX_OP_BATCHNORM,
    ENI_ONNX_OP_GEMM
} eni_onnx_op_t;

typedef struct {
    char        name[ENI_ONNX_MAX_NAME_LEN];
    eni_onnx_dtype_t dtype;
    uint32_t    dims[ENI_ONNX_MAX_DIMS];
    uint32_t    num_dims;
    uint32_t    data_offset;    /**< Offset into raw data buffer. */
    uint32_t    data_size;      /**< Size in bytes. */
} eni_onnx_tensor_t;

typedef struct {
    char            name[ENI_ONNX_MAX_NAME_LEN];
    eni_onnx_op_t   op_type;
    uint32_t        input_count;
    uint32_t        output_count;
    uint32_t        weight_idx;     /**< Index into tensor table, or UINT32_MAX. */
    uint32_t        bias_idx;
    /* Conv/Pool attributes */
    uint32_t        kernel_size;
    uint32_t        stride;
    uint32_t        padding;
} eni_onnx_layer_t;

typedef struct {
    uint32_t    max_model_size;
    uint32_t    max_layers;
} eni_onnx_config_t;

/** Parsed ONNX model. */
typedef struct {
    char                name[ENI_ONNX_MAX_NAME_LEN];
    uint32_t            ir_version;
    uint32_t            opset_version;

    eni_onnx_layer_t    layers[ENI_ONNX_MAX_LAYERS];
    uint32_t            num_layers;

    eni_onnx_tensor_t   tensors[ENI_ONNX_MAX_LAYERS * 2];
    uint32_t            num_tensors;

    uint8_t             raw_data[ENI_ONNX_MAX_TENSOR_SIZE];
    uint32_t            raw_data_used;

    bool                valid;
} eni_onnx_model_t;

/* ---------------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------------ */

eni_onnx_status_t eni_onnx_load_from_buffer(eni_onnx_model_t *model,
                                            const uint8_t *buffer,
                                            uint32_t size,
                                            const eni_onnx_config_t *config);

eni_onnx_status_t eni_onnx_load_from_file(eni_onnx_model_t *model,
                                          const char *path,
                                          const eni_onnx_config_t *config);

eni_onnx_status_t eni_onnx_validate(const eni_onnx_model_t *model);

/**
 * Get layer count from parsed model.
 */
uint32_t eni_onnx_get_layer_count(const eni_onnx_model_t *model);

/**
 * Get a specific layer descriptor.
 */
const eni_onnx_layer_t *eni_onnx_get_layer(const eni_onnx_model_t *model,
                                           uint32_t index);

#ifdef __cplusplus
}
#endif

#endif /* ENI_ONNX_LOADER_H */
