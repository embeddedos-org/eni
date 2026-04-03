/**
 * @file onnx_loader.c
 * @brief Minimal ONNX protobuf subset parser.
 *
 * Parses wire-format protobuf for the ONNX ModelProto / GraphProto / NodeProto
 * structures. No external dependencies — fully self-contained.
 *
 * SPDX-License-Identifier: MIT
 */

#include "eni/onnx_loader.h"
#include <string.h>
#include <stdio.h>

/* ---- Protobuf wire types ------------------------------------------------ */

#define PB_WIRE_VARINT   0
#define PB_WIRE_64BIT    1
#define PB_WIRE_LEN      2
#define PB_WIRE_32BIT    5

/* ---- Protobuf mini-decoder --------------------------------------------- */

typedef struct {
    const uint8_t *data;
    uint32_t       pos;
    uint32_t       end;
} pb_reader_t;

static inline bool pb_has_data(const pb_reader_t *r)
{
    return r->pos < r->end;
}

static uint64_t pb_read_varint(pb_reader_t *r)
{
    uint64_t val = 0;
    uint32_t shift = 0;
    while (r->pos < r->end && shift < 64) {
        uint8_t b = r->data[r->pos++];
        val |= (uint64_t)(b & 0x7F) << shift;
        if ((b & 0x80) == 0) break;
        shift += 7;
    }
    return val;
}

static uint32_t pb_read_tag(pb_reader_t *r, uint32_t *wire_type)
{
    uint64_t v = pb_read_varint(r);
    *wire_type = (uint32_t)(v & 0x07);
    return (uint32_t)(v >> 3);
}

static void pb_skip_field(pb_reader_t *r, uint32_t wire_type)
{
    switch (wire_type) {
    case PB_WIRE_VARINT:
        pb_read_varint(r);
        break;
    case PB_WIRE_64BIT:
        r->pos += 8;
        break;
    case PB_WIRE_LEN: {
        uint32_t len = (uint32_t)pb_read_varint(r);
        r->pos += len;
        break;
    }
    case PB_WIRE_32BIT:
        r->pos += 4;
        break;
    default:
        break;
    }
    if (r->pos > r->end) r->pos = r->end;
}

static bool pb_read_bytes(pb_reader_t *r, const uint8_t **out, uint32_t *len)
{
    *len = (uint32_t)pb_read_varint(r);
    if (r->pos + *len > r->end) {
        *len = 0;
        return false;
    }
    *out = r->data + r->pos;
    r->pos += *len;
    return true;
}

static void pb_read_string(pb_reader_t *r, char *buf, uint32_t buf_size)
{
    const uint8_t *data;
    uint32_t len;
    if (pb_read_bytes(r, &data, &len)) {
        uint32_t copy = (len < buf_size - 1) ? len : buf_size - 1;
        memcpy(buf, data, copy);
        buf[copy] = '\0';
    }
}

/* ---- Op-type string lookup ---------------------------------------------- */

static eni_onnx_op_t parse_op_type(const char *name)
{
    if (!name) return ENI_ONNX_OP_UNKNOWN;

    struct { const char *str; eni_onnx_op_t op; } table[] = {
        {"Conv",      ENI_ONNX_OP_CONV},
        {"Relu",      ENI_ONNX_OP_RELU},
        {"Sigmoid",   ENI_ONNX_OP_SIGMOID},
        {"Tanh",      ENI_ONNX_OP_TANH},
        {"MatMul",    ENI_ONNX_OP_MATMUL},
        {"Add",       ENI_ONNX_OP_ADD},
        {"LSTM",      ENI_ONNX_OP_LSTM},
        {"Softmax",   ENI_ONNX_OP_SOFTMAX},
        {"Flatten",   ENI_ONNX_OP_FLATTEN},
        {"MaxPool",   ENI_ONNX_OP_MAXPOOL},
        {"AveragePool", ENI_ONNX_OP_AVGPOOL},
        {"BatchNormalization", ENI_ONNX_OP_BATCHNORM},
        {"Gemm",      ENI_ONNX_OP_GEMM},
    };

    for (uint32_t i = 0; i < sizeof(table) / sizeof(table[0]); i++) {
        if (strcmp(name, table[i].str) == 0) {
            return table[i].op;
        }
    }
    return ENI_ONNX_OP_UNKNOWN;
}

/* ---- Parse a NodeProto (ONNX graph node) -------------------------------- */

static eni_onnx_status_t parse_node(pb_reader_t *r, eni_onnx_layer_t *layer)
{
    memset(layer, 0, sizeof(*layer));
    layer->weight_idx = UINT32_MAX;
    layer->bias_idx   = UINT32_MAX;
    layer->stride     = 1;

    while (pb_has_data(r)) {
        uint32_t wire;
        uint32_t field = pb_read_tag(r, &wire);

        switch (field) {
        case 1: /* input (repeated string) */
            layer->input_count++;
            pb_skip_field(r, wire);
            break;
        case 2: /* output (repeated string) */
            layer->output_count++;
            pb_skip_field(r, wire);
            break;
        case 3: /* name */
            pb_read_string(r, layer->name, ENI_ONNX_MAX_NAME_LEN);
            break;
        case 4: { /* op_type */
            char op_name[ENI_ONNX_MAX_NAME_LEN];
            pb_read_string(r, op_name, ENI_ONNX_MAX_NAME_LEN);
            layer->op_type = parse_op_type(op_name);
            break;
        }
        default:
            pb_skip_field(r, wire);
            break;
        }
    }
    return ENI_ONNX_OK;
}

/* ---- Parse a TensorProto ------------------------------------------------ */

static eni_onnx_status_t parse_tensor(pb_reader_t *r,
                                      eni_onnx_tensor_t *tensor,
                                      uint8_t *raw_buf,
                                      uint32_t *raw_used,
                                      uint32_t raw_max)
{
    memset(tensor, 0, sizeof(*tensor));

    while (pb_has_data(r)) {
        uint32_t wire;
        uint32_t field = pb_read_tag(r, &wire);

        switch (field) {
        case 1: /* dims (repeated int64) */
            if (wire == PB_WIRE_VARINT && tensor->num_dims < ENI_ONNX_MAX_DIMS) {
                tensor->dims[tensor->num_dims++] = (uint32_t)pb_read_varint(r);
            } else {
                pb_skip_field(r, wire);
            }
            break;
        case 2: /* data_type */
            tensor->dtype = (eni_onnx_dtype_t)pb_read_varint(r);
            break;
        case 8: /* name */
            pb_read_string(r, tensor->name, ENI_ONNX_MAX_NAME_LEN);
            break;
        case 4: /* float_data */
        case 5: /* int32_data */
        case 9: { /* raw_data */
            const uint8_t *data;
            uint32_t len;
            if (pb_read_bytes(r, &data, &len)) {
                if (*raw_used + len <= raw_max) {
                    memcpy(raw_buf + *raw_used, data, len);
                    tensor->data_offset = *raw_used;
                    tensor->data_size   = len;
                    *raw_used += len;
                }
            }
            break;
        }
        default:
            pb_skip_field(r, wire);
            break;
        }
    }
    return ENI_ONNX_OK;
}

/* ---- Parse GraphProto --------------------------------------------------- */

static eni_onnx_status_t parse_graph(pb_reader_t *r, eni_onnx_model_t *model)
{
    while (pb_has_data(r)) {
        uint32_t wire;
        uint32_t field = pb_read_tag(r, &wire);

        switch (field) {
        case 1: { /* node (repeated NodeProto) */
            const uint8_t *sub_data;
            uint32_t sub_len;
            if (!pb_read_bytes(r, &sub_data, &sub_len)) break;

            if (model->num_layers >= ENI_ONNX_MAX_LAYERS) {
                return ENI_ONNX_ERR_LAYER_COUNT;
            }
            pb_reader_t sub = {sub_data, 0, sub_len};
            eni_onnx_status_t st = parse_node(&sub,
                &model->layers[model->num_layers]);
            if (st != ENI_ONNX_OK) return st;
            model->num_layers++;
            break;
        }
        case 5: { /* initializer (repeated TensorProto) */
            const uint8_t *sub_data;
            uint32_t sub_len;
            if (!pb_read_bytes(r, &sub_data, &sub_len)) break;

            if (model->num_tensors >= ENI_ONNX_MAX_LAYERS * 2) break;

            pb_reader_t sub = {sub_data, 0, sub_len};
            parse_tensor(&sub,
                         &model->tensors[model->num_tensors],
                         model->raw_data,
                         &model->raw_data_used,
                         ENI_ONNX_MAX_TENSOR_SIZE);
            model->num_tensors++;
            break;
        }
        default:
            pb_skip_field(r, wire);
            break;
        }
    }
    return ENI_ONNX_OK;
}

/* ---- Public API --------------------------------------------------------- */

eni_onnx_status_t eni_onnx_load_from_buffer(eni_onnx_model_t *model,
                                            const uint8_t *buffer,
                                            uint32_t size,
                                            const eni_onnx_config_t *config)
{
    if (!model || !buffer) return ENI_ONNX_ERR_NULL;

    uint32_t max_size = config ? config->max_model_size : ENI_ONNX_MAX_MODEL_SIZE;
    if (size > max_size) return ENI_ONNX_ERR_SIZE;

    memset(model, 0, sizeof(*model));

    pb_reader_t r = {buffer, 0, size};

    /* Parse ModelProto top-level fields */
    while (pb_has_data(&r)) {
        uint32_t wire;
        uint32_t field = pb_read_tag(&r, &wire);

        switch (field) {
        case 1: /* ir_version */
            model->ir_version = (uint32_t)pb_read_varint(&r);
            break;
        case 7: { /* graph (GraphProto) */
            const uint8_t *sub_data;
            uint32_t sub_len;
            if (!pb_read_bytes(&r, &sub_data, &sub_len)) break;

            pb_reader_t sub = {sub_data, 0, sub_len};
            eni_onnx_status_t st = parse_graph(&sub, model);
            if (st != ENI_ONNX_OK) return st;
            break;
        }
        case 8: /* opset_import */
        {
            const uint8_t *sub_data;
            uint32_t sub_len;
            if (pb_read_bytes(&r, &sub_data, &sub_len)) {
                pb_reader_t sub = {sub_data, 0, sub_len};
                while (pb_has_data(&sub)) {
                    uint32_t sw;
                    uint32_t sf = pb_read_tag(&sub, &sw);
                    if (sf == 2) {
                        model->opset_version = (uint32_t)pb_read_varint(&sub);
                    } else {
                        pb_skip_field(&sub, sw);
                    }
                }
            }
            break;
        }
        default:
            pb_skip_field(&r, wire);
            break;
        }
    }

    model->valid = true;
    return ENI_ONNX_OK;
}

eni_onnx_status_t eni_onnx_load_from_file(eni_onnx_model_t *model,
                                          const char *path,
                                          const eni_onnx_config_t *config)
{
    if (!model || !path) return ENI_ONNX_ERR_NULL;

    FILE *f = fopen(path, "rb");
    if (!f) return ENI_ONNX_ERR_IO;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0 || (uint32_t)fsize > ENI_ONNX_MAX_MODEL_SIZE) {
        fclose(f);
        return ENI_ONNX_ERR_SIZE;
    }

    /* Stack-allocate read buffer (up to 128 KiB). On constrained MCUs
       the caller should use load_from_buffer with pre-loaded data. */
    uint8_t file_buf[ENI_ONNX_MAX_MODEL_SIZE];
    size_t read = fread(file_buf, 1, (size_t)fsize, f);
    fclose(f);

    if (read != (size_t)fsize) return ENI_ONNX_ERR_IO;

    return eni_onnx_load_from_buffer(model, file_buf, (uint32_t)read, config);
}

eni_onnx_status_t eni_onnx_validate(const eni_onnx_model_t *model)
{
    if (!model) return ENI_ONNX_ERR_NULL;
    if (!model->valid) return ENI_ONNX_ERR_FORMAT;
    if (model->num_layers == 0) return ENI_ONNX_ERR_FORMAT;

    /* Check for unsupported ops */
    for (uint32_t i = 0; i < model->num_layers; i++) {
        if (model->layers[i].op_type == ENI_ONNX_OP_UNKNOWN) {
            return ENI_ONNX_ERR_UNSUPPORTED_OP;
        }
    }

    return ENI_ONNX_OK;
}

uint32_t eni_onnx_get_layer_count(const eni_onnx_model_t *model)
{
    if (!model || !model->valid) return 0;
    return model->num_layers;
}

const eni_onnx_layer_t *eni_onnx_get_layer(const eni_onnx_model_t *model,
                                           uint32_t index)
{
    if (!model || !model->valid) return NULL;
    if (index >= model->num_layers) return NULL;
    return &model->layers[index];
}
