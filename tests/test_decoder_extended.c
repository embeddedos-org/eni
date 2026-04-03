// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// Extended decoder tests: edge cases, boundary conditions, multi-decoder

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "eni/decoder.h"
#include "eni/dsp.h"

static int passed = 0;
#define PASS(name) do { printf("[PASS] %s\n", name); passed++; } while(0)

static void test_energy_decoder_motor_imagine(void) {
    eni_decoder_t dec;
    eni_decoder_config_t cfg = {0};
    cfg.confidence_threshold = 0.5f;
    eni_decoder_init(&dec, &eni_decoder_energy_ops, &cfg);
    eni_dsp_features_t feat = {0};
    feat.total_power = 50.0f;
    feat.band_power[3] = 30.0f; /* beta dominant */
    eni_decode_result_t result = {0};
    eni_status_t rc = eni_decoder_decode(&dec, &feat, &result);
    assert(rc == ENI_OK);
    assert(result.count > 0);
    assert(result.best_idx >= 0 && result.best_idx < result.count);
    eni_decoder_shutdown(&dec);
    PASS("energy_decoder_motor_imagine");
}

static void test_energy_decoder_attention(void) {
    eni_decoder_t dec;
    eni_decoder_config_t cfg = {0};
    cfg.confidence_threshold = 0.3f;
    eni_decoder_init(&dec, &eni_decoder_energy_ops, &cfg);
    eni_dsp_features_t feat = {0};
    feat.total_power = 30.0f;
    feat.band_power[2] = 20.0f; /* alpha */
    feat.band_power[3] = 5.0f;  /* some beta */
    eni_decode_result_t result = {0};
    eni_decoder_decode(&dec, &feat, &result);
    assert(result.count > 0);
    int found = 0;
    for (int i = 0; i < result.count; i++) {
        if (result.intents[i].confidence > 0.0f) found++;
    }
    assert(found > 0);
    eni_decoder_shutdown(&dec);
    PASS("energy_decoder_attention");
}

static void test_energy_decoder_confidence_sum(void) {
    eni_decoder_t dec;
    eni_decoder_config_t cfg = {0};
    cfg.confidence_threshold = 0.0f;
    eni_decoder_init(&dec, &eni_decoder_energy_ops, &cfg);
    eni_dsp_features_t feat = {0};
    feat.total_power = 25.0f;
    feat.band_power[0] = 5.0f;
    feat.band_power[1] = 5.0f;
    feat.band_power[2] = 5.0f;
    feat.band_power[3] = 5.0f;
    feat.band_power[4] = 5.0f;
    eni_decode_result_t result = {0};
    eni_decoder_decode(&dec, &feat, &result);
    float sum = 0.0f;
    for (int i = 0; i < result.count; i++) sum += result.intents[i].confidence;
    assert(sum > 0.0f);
    eni_decoder_shutdown(&dec);
    PASS("energy_decoder_confidence_sum");
}

static void test_nn_decoder_zero_classes(void) {
    eni_decoder_t dec;
    eni_decoder_config_t cfg = {0};
    cfg.num_classes = 0;
    eni_status_t rc = eni_decoder_init(&dec, &eni_decoder_nn_ops, &cfg);
    assert(rc == ENI_OK);
    eni_decoder_shutdown(&dec);
    PASS("nn_decoder_zero_classes");
}

static void test_nn_decoder_max_classes(void) {
    eni_decoder_t dec;
    eni_decoder_config_t cfg = {0};
    cfg.num_classes = ENI_DECODE_MAX_CLASSES;
    eni_status_t rc = eni_decoder_init(&dec, &eni_decoder_nn_ops, &cfg);
    assert(rc == ENI_OK);
    eni_dsp_features_t feat = {0};
    feat.total_power = 10.0f;
    eni_decode_result_t result = {0};
    rc = eni_decoder_decode(&dec, &feat, &result);
    assert(rc == ENI_OK);
    assert(result.count <= ENI_DECODE_MAX_CLASSES);
    eni_decoder_shutdown(&dec);
    PASS("nn_decoder_max_classes");
}

static void test_decoder_reinit(void) {
    eni_decoder_t dec;
    eni_decoder_config_t cfg = {0};
    cfg.confidence_threshold = 0.5f;
    assert(eni_decoder_init(&dec, &eni_decoder_energy_ops, &cfg) == ENI_OK);
    eni_decoder_shutdown(&dec);
    cfg.confidence_threshold = 0.8f;
    assert(eni_decoder_init(&dec, &eni_decoder_energy_ops, &cfg) == ENI_OK);
    eni_dsp_features_t feat = {0};
    feat.total_power = 1.0f;
    eni_decode_result_t result = {0};
    assert(eni_decoder_decode(&dec, &feat, &result) == ENI_OK);
    eni_decoder_shutdown(&dec);
    PASS("decoder_reinit");
}

static void test_decoder_name_preserved(void) {
    eni_decoder_t dec;
    eni_decoder_config_t cfg = {0};
    eni_decoder_init(&dec, &eni_decoder_energy_ops, &cfg);
    assert(strcmp(dec.name, "energy") == 0);
    eni_decoder_shutdown(&dec);
    eni_decoder_init(&dec, &eni_decoder_nn_ops, &cfg);
    assert(strcmp(dec.name, "nn") == 0);
    eni_decoder_shutdown(&dec);
    PASS("decoder_name_preserved");
}

static void test_energy_decoder_low_confidence_threshold(void) {
    eni_decoder_t dec;
    eni_decoder_config_t cfg = {0};
    cfg.confidence_threshold = 0.01f;
    eni_decoder_init(&dec, &eni_decoder_energy_ops, &cfg);
    eni_dsp_features_t feat = {0};
    feat.total_power = 0.001f;
    eni_decode_result_t result = {0};
    eni_decoder_decode(&dec, &feat, &result);
    assert(result.count > 0);
    eni_decoder_shutdown(&dec);
    PASS("energy_decoder_low_confidence_threshold");
}

static void test_decoder_sequential_decodes(void) {
    eni_decoder_t dec;
    eni_decoder_config_t cfg = {0};
    cfg.confidence_threshold = 0.5f;
    eni_decoder_init(&dec, &eni_decoder_energy_ops, &cfg);

    for (int iter = 0; iter < 10; iter++) {
        eni_dsp_features_t feat = {0};
        feat.total_power = (float)(iter * 10);
        eni_decode_result_t result = {0};
        assert(eni_decoder_decode(&dec, &feat, &result) == ENI_OK);
        assert(result.count > 0);
    }
    eni_decoder_shutdown(&dec);
    PASS("decoder_sequential_decodes");
}

int main(void) {
    printf("=== ENI Decoder Extended Tests ===\n");
    test_energy_decoder_motor_imagine();
    test_energy_decoder_attention();
    test_energy_decoder_confidence_sum();
    test_nn_decoder_zero_classes();
    test_nn_decoder_max_classes();
    test_decoder_reinit();
    test_decoder_name_preserved();
    test_energy_decoder_low_confidence_threshold();
    test_decoder_sequential_decodes();
    printf("\n=== ALL %d TESTS PASSED ===\n", passed);
    return 0;
}
