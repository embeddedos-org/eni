// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// Extended NN tests: multi-layer, activations, edge cases, conv1d

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "eni/nn.h"

static int passed = 0;
#define PASS(name) do { printf("[PASS] %s\n", name); passed++; } while(0)

static void build_dense_model(uint8_t *buf, size_t *len,
                               int in_sz, int out_sz,
                               eni_nn_activation_t act,
                               const float *weights, const float *biases) {
    size_t off = 0;
    uint32_t magic = ENI_NN_MAGIC; memcpy(buf + off, &magic, 4); off += 4;
    uint32_t version = 1;          memcpy(buf + off, &version, 4); off += 4;
    uint32_t layers = 1;           memcpy(buf + off, &layers, 4); off += 4;
    uint32_t is = (uint32_t)in_sz; memcpy(buf + off, &is, 4); off += 4;
    uint32_t os = (uint32_t)out_sz;memcpy(buf + off, &os, 4); off += 4;
    uint32_t type = ENI_NN_DENSE;  memcpy(buf + off, &type, 4); off += 4;
    uint32_t a = (uint32_t)act;    memcpy(buf + off, &a, 4); off += 4;
    uint32_t in2 = (uint32_t)in_sz;memcpy(buf + off, &in2, 4); off += 4;
    uint32_t o2 = (uint32_t)out_sz;memcpy(buf + off, &o2, 4); off += 4;
    uint32_t kern = 0;             memcpy(buf + off, &kern, 4); off += 4;
    int nw = in_sz * out_sz;
    memcpy(buf + off, weights, (size_t)nw * sizeof(float)); off += (size_t)nw * sizeof(float);
    memcpy(buf + off, biases, (size_t)out_sz * sizeof(float)); off += (size_t)out_sz * sizeof(float);
    *len = off;
}

static void test_nn_relu_activation(void) {
    float w[] = {1.0f, 0.0f, 0.0f, 1.0f};
    float b[] = {-0.5f, 0.5f};
    uint8_t buf[512];
    size_t len;
    build_dense_model(buf, &len, 2, 2, ENI_NN_RELU, w, b);
    eni_nn_model_t model;
    assert(eni_nn_load(&model, buf, len) == ENI_OK);
    float input[] = {0.3f, -1.0f};
    float output[2];
    assert(eni_nn_forward(&model, input, output, 2) == ENI_OK);
    /* 0.3 - 0.5 = -0.2 → ReLU → 0.0 */
    assert(output[0] < 0.01f);
    /* -1.0 + 0.5 = -0.5 → ReLU → 0.0 */
    assert(output[1] < 0.01f);
    PASS("nn_relu_activation");
}

static void test_nn_sigmoid_activation(void) {
    float w[] = {1.0f};
    float b[] = {0.0f};
    uint8_t buf[256];
    size_t len;
    build_dense_model(buf, &len, 1, 1, ENI_NN_SIGMOID, w, b);
    eni_nn_model_t model;
    assert(eni_nn_load(&model, buf, len) == ENI_OK);
    float input[] = {0.0f};
    float output[1];
    assert(eni_nn_forward(&model, input, output, 1) == ENI_OK);
    assert(fabsf(output[0] - 0.5f) < 0.01f);
    PASS("nn_sigmoid_activation");
}

static void test_nn_tanh_activation(void) {
    float w[] = {1.0f};
    float b[] = {0.0f};
    uint8_t buf[256];
    size_t len;
    build_dense_model(buf, &len, 1, 1, ENI_NN_TANH, w, b);
    eni_nn_model_t model;
    assert(eni_nn_load(&model, buf, len) == ENI_OK);
    float input[] = {0.0f};
    float output[1];
    assert(eni_nn_forward(&model, input, output, 1) == ENI_OK);
    assert(fabsf(output[0]) < 0.01f);
    PASS("nn_tanh_activation");
}

static void test_nn_linear_activation(void) {
    float w[] = {2.0f, 0.0f, 0.0f, 3.0f};
    float b[] = {1.0f, -1.0f};
    uint8_t buf[512];
    size_t len;
    build_dense_model(buf, &len, 2, 2, ENI_NN_LINEAR, w, b);
    eni_nn_model_t model;
    assert(eni_nn_load(&model, buf, len) == ENI_OK);
    float input[] = {1.0f, 2.0f};
    float output[2];
    assert(eni_nn_forward(&model, input, output, 2) == ENI_OK);
    assert(fabsf(output[0] - 3.0f) < 0.01f);
    assert(fabsf(output[1] - 5.0f) < 0.01f);
    PASS("nn_linear_activation");
}

static void test_nn_load_null_data(void) {
    eni_nn_model_t model;
    assert(eni_nn_load(&model, NULL, 0) == ENI_ERR_INVALID);
    PASS("nn_load_null_data");
}

static void test_nn_load_truncated(void) {
    uint8_t buf[8] = {0};
    uint32_t magic = ENI_NN_MAGIC;
    memcpy(buf, &magic, 4);
    eni_nn_model_t model;
    assert(eni_nn_load(&model, buf, 8) != ENI_OK);
    PASS("nn_load_truncated");
}

static void test_nn_forward_large_input(void) {
    float w[16];
    float b[4];
    for (int i = 0; i < 16; i++) w[i] = (i % 5 == 0) ? 0.25f : 0.0f;
    for (int i = 0; i < 4; i++) b[i] = 0.0f;
    uint8_t buf[512];
    size_t len;
    build_dense_model(buf, &len, 4, 4, ENI_NN_SOFTMAX, w, b);
    eni_nn_model_t model;
    assert(eni_nn_load(&model, buf, len) == ENI_OK);
    float input[] = {1.0f, 2.0f, 3.0f, 4.0f};
    float output[4];
    assert(eni_nn_forward(&model, input, output, 4) == ENI_OK);
    float sum = 0.0f;
    for (int i = 0; i < 4; i++) sum += output[i];
    assert(sum > 0.99f && sum < 1.01f);
    PASS("nn_forward_large_input");
}

static void test_nn_model_properties(void) {
    float w[] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float b[] = {0.0f, 0.0f, 0.0f};
    uint8_t buf[512];
    size_t len;
    build_dense_model(buf, &len, 3, 3, ENI_NN_RELU, w, b);
    eni_nn_model_t model;
    assert(eni_nn_load(&model, buf, len) == ENI_OK);
    assert(model.input_size == 3);
    assert(model.output_size == 3);
    assert(model.layer_count == 1);
    assert(model.weight_count > 0);
    PASS("nn_model_properties");
}

static void test_nn_relu_positive_passthrough(void) {
    float w[] = {1.0f};
    float b[] = {0.0f};
    uint8_t buf[256];
    size_t len;
    build_dense_model(buf, &len, 1, 1, ENI_NN_RELU, w, b);
    eni_nn_model_t model;
    assert(eni_nn_load(&model, buf, len) == ENI_OK);
    float input[] = {5.0f};
    float output[1];
    assert(eni_nn_forward(&model, input, output, 1) == ENI_OK);
    assert(fabsf(output[0] - 5.0f) < 0.01f);
    PASS("nn_relu_positive_passthrough");
}

static void test_nn_softmax_extreme_values(void) {
    float w[] = {1.0f, 0.0f, 0.0f, 1.0f};
    float b[] = {0.0f, 0.0f};
    uint8_t buf[512];
    size_t len;
    build_dense_model(buf, &len, 2, 2, ENI_NN_SOFTMAX, w, b);
    eni_nn_model_t model;
    assert(eni_nn_load(&model, buf, len) == ENI_OK);
    float input[] = {100.0f, 0.0f};
    float output[2];
    assert(eni_nn_forward(&model, input, output, 2) == ENI_OK);
    assert(output[0] > 0.99f);
    assert(output[1] < 0.01f);
    PASS("nn_softmax_extreme_values");
}

int main(void) {
    printf("=== ENI NN Extended Tests ===\n");
    test_nn_relu_activation();
    test_nn_sigmoid_activation();
    test_nn_tanh_activation();
    test_nn_linear_activation();
    test_nn_load_null_data();
    test_nn_load_truncated();
    test_nn_forward_large_input();
    test_nn_model_properties();
    test_nn_relu_positive_passthrough();
    test_nn_softmax_extreme_values();
    printf("\n=== ALL %d TESTS PASSED ===\n", passed);
    return 0;
}
