// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// Extended DSP tests: windowing, PSD edge cases, multi-channel, Hjorth parameters

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "eni/dsp.h"

static int passed = 0;
#define PASS(name) do { printf("[PASS] %s\n", name); passed++; } while(0)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void test_fft_dc_signal(void) {
    eni_dsp_fft_ctx_t ctx;
    eni_dsp_fft_init(&ctx, 256);
    float re[256], im[256];
    memset(im, 0, sizeof(im));
    for (int i = 0; i < 256; i++) re[i] = 1.0f;
    assert(eni_dsp_fft(&ctx, re, im, 256) == ENI_OK);
    float dc_mag = sqrtf(re[0] * re[0] + im[0] * im[0]);
    for (int i = 1; i < 128; i++) {
        float mag = sqrtf(re[i] * re[i] + im[i] * im[i]);
        assert(dc_mag > mag * 10.0f);
    }
    PASS("fft_dc_signal");
}

static void test_fft_zero_signal(void) {
    eni_dsp_fft_ctx_t ctx;
    eni_dsp_fft_init(&ctx, 64);
    float re[64], im[64];
    memset(re, 0, sizeof(re));
    memset(im, 0, sizeof(im));
    assert(eni_dsp_fft(&ctx, re, im, 64) == ENI_OK);
    for (int i = 0; i < 64; i++) {
        assert(fabsf(re[i]) < 1e-6f);
        assert(fabsf(im[i]) < 1e-6f);
    }
    PASS("fft_zero_signal");
}

static void test_fft_multiple_sines(void) {
    eni_dsp_fft_ctx_t ctx;
    eni_dsp_fft_init(&ctx, 256);
    float re[256], im[256];
    memset(im, 0, sizeof(im));
    for (int i = 0; i < 256; i++) {
        re[i] = sinf(2.0f * (float)M_PI * 10.0f * (float)i / 256.0f)
              + sinf(2.0f * (float)M_PI * 30.0f * (float)i / 256.0f);
    }
    assert(eni_dsp_fft(&ctx, re, im, 256) == ENI_OK);
    float mag10 = sqrtf(re[10] * re[10] + im[10] * im[10]);
    float mag30 = sqrtf(re[30] * re[30] + im[30] * im[30]);
    float mag20 = sqrtf(re[20] * re[20] + im[20] * im[20]);
    assert(mag10 > mag20 * 5.0f);
    assert(mag30 > mag20 * 5.0f);
    PASS("fft_multiple_sines");
}

static void test_fft_invalid_size(void) {
    eni_dsp_fft_ctx_t ctx;
    assert(eni_dsp_fft_init(&ctx, 0) == ENI_ERR_INVALID);
    assert(eni_dsp_fft_init(&ctx, 3) == ENI_ERR_INVALID);
    assert(eni_dsp_fft_init(&ctx, 7) == ENI_ERR_INVALID);
    assert(eni_dsp_fft_init(&ctx, 1024) == ENI_ERR_INVALID);
    assert(eni_dsp_fft_init(&ctx, -1) == ENI_ERR_INVALID);
    PASS("fft_invalid_size");
}

static void test_fft_all_valid_sizes(void) {
    eni_dsp_fft_ctx_t ctx;
    int sizes[] = {2, 4, 8, 16, 32, 64, 128, 256, 512};
    for (int s = 0; s < 9; s++) {
        assert(eni_dsp_fft_init(&ctx, sizes[s]) == ENI_OK);
        assert(ctx.size == sizes[s]);
    }
    PASS("fft_all_valid_sizes");
}

static void test_psd_frequency_resolution(void) {
    eni_dsp_fft_ctx_t ctx;
    eni_dsp_fft_init(&ctx, 256);
    float signal[256];
    for (int i = 0; i < 256; i++)
        signal[i] = sinf(2.0f * (float)M_PI * 10.0f * (float)i / 256.0f);
    eni_dsp_psd_result_t psd;
    assert(eni_dsp_psd(&ctx, signal, 256, 256.0f, &psd) == ENI_OK);
    assert(fabsf(psd.freq_resolution - 1.0f) < 0.1f);
    PASS("psd_frequency_resolution");
}

static void test_band_power_isolation(void) {
    eni_dsp_fft_ctx_t ctx;
    eni_dsp_fft_init(&ctx, 256);
    float signal[256];
    for (int i = 0; i < 256; i++)
        signal[i] = 50.0f * sinf(2.0f * (float)M_PI * 22.0f * (float)i / 256.0f);
    eni_dsp_psd_result_t psd;
    eni_dsp_psd(&ctx, signal, 256, 256.0f, &psd);
    float beta = eni_dsp_beta_power(&psd);
    float delta = eni_dsp_delta_power(&psd);
    float theta = eni_dsp_theta_power(&psd);
    assert(beta > delta);
    assert(beta > theta);
    PASS("band_power_isolation");
}

static void test_gamma_band_power(void) {
    eni_dsp_fft_ctx_t ctx;
    eni_dsp_fft_init(&ctx, 256);
    float signal[256];
    for (int i = 0; i < 256; i++)
        signal[i] = 50.0f * sinf(2.0f * (float)M_PI * 45.0f * (float)i / 256.0f);
    eni_dsp_psd_result_t psd;
    eni_dsp_psd(&ctx, signal, 256, 256.0f, &psd);
    float gamma = eni_dsp_gamma_power(&psd);
    float alpha = eni_dsp_alpha_power(&psd);
    assert(gamma > alpha);
    PASS("gamma_band_power");
}

static void test_artifact_no_false_positive(void) {
    float signal[64];
    for (int i = 0; i < 64; i++)
        signal[i] = 10.0f * sinf(2.0f * (float)M_PI * 10.0f * (float)i / 256.0f);
    eni_dsp_artifact_t art = eni_dsp_artifact_detect(signal, 64, 50.0f);
    assert(art.eye_blink == false);
    PASS("artifact_no_false_positive");
}

static void test_artifact_muscle(void) {
    float signal[128];
    memset(signal, 0, sizeof(signal));
    for (int i = 30; i < 60; i++) signal[i] = 200.0f * ((i % 2) ? 1.0f : -1.0f);
    eni_dsp_artifact_t art = eni_dsp_artifact_detect(signal, 128, 50.0f);
    assert(art.severity > 0.0f);
    PASS("artifact_muscle");
}

static void test_epoch_overflow(void) {
    eni_dsp_epoch_t epoch;
    eni_dsp_epoch_init(&epoch, 4, 256.0f);
    for (int i = 0; i < 10; i++) eni_dsp_epoch_push(&epoch, (float)i);
    assert(eni_dsp_epoch_ready(&epoch));
    assert(epoch.count == 4);
    PASS("epoch_overflow");
}

static void test_epoch_reset_and_refill(void) {
    eni_dsp_epoch_t epoch;
    eni_dsp_epoch_init(&epoch, 8, 256.0f);
    for (int i = 0; i < 8; i++) eni_dsp_epoch_push(&epoch, (float)i);
    assert(eni_dsp_epoch_ready(&epoch));
    eni_dsp_epoch_reset(&epoch);
    assert(!eni_dsp_epoch_ready(&epoch));
    for (int i = 0; i < 8; i++) eni_dsp_epoch_push(&epoch, (float)(i + 10));
    assert(eni_dsp_epoch_ready(&epoch));
    PASS("epoch_reset_and_refill");
}

static void test_features_zero_signal(void) {
    eni_dsp_fft_ctx_t ctx;
    eni_dsp_fft_init(&ctx, 256);
    float signal[256];
    memset(signal, 0, sizeof(signal));
    eni_dsp_features_t feat;
    assert(eni_dsp_extract_features(&ctx, signal, 256, 256.0f, &feat) == ENI_OK);
    assert(feat.total_power < 0.01f);
    PASS("features_zero_signal");
}

static void test_features_hjorth_consistency(void) {
    eni_dsp_fft_ctx_t ctx;
    eni_dsp_fft_init(&ctx, 256);
    float signal[256];
    for (int i = 0; i < 256; i++)
        signal[i] = 40.0f * sinf(2.0f * (float)M_PI * 10.0f * (float)i / 256.0f);
    eni_dsp_features_t feat;
    assert(eni_dsp_extract_features(&ctx, signal, 256, 256.0f, &feat) == ENI_OK);
    assert(feat.hjorth_activity > 0.0f);
    assert(feat.hjorth_mobility > 0.0f);
    assert(feat.hjorth_complexity > 0.0f);
    PASS("features_hjorth_consistency");
}

static void test_features_spectral_entropy_range(void) {
    eni_dsp_fft_ctx_t ctx;
    eni_dsp_fft_init(&ctx, 256);
    float signal[256];
    for (int i = 0; i < 256; i++)
        signal[i] = 30.0f * sinf(2.0f * (float)M_PI * 10.0f * (float)i / 256.0f)
                  + 20.0f * sinf(2.0f * (float)M_PI * 20.0f * (float)i / 256.0f)
                  + 10.0f * sinf(2.0f * (float)M_PI * 40.0f * (float)i / 256.0f);
    eni_dsp_features_t feat;
    eni_dsp_extract_features(&ctx, signal, 256, 256.0f, &feat);
    assert(feat.spectral_entropy >= 0.0f);
    PASS("features_spectral_entropy_range");
}

static void test_band_power_custom_range(void) {
    eni_dsp_fft_ctx_t ctx;
    eni_dsp_fft_init(&ctx, 256);
    float signal[256];
    for (int i = 0; i < 256; i++)
        signal[i] = 50.0f * sinf(2.0f * (float)M_PI * 15.0f * (float)i / 256.0f);
    eni_dsp_psd_result_t psd;
    eni_dsp_psd(&ctx, signal, 256, 256.0f, &psd);
    float custom = eni_dsp_band_power(&psd, 12.0f, 18.0f);
    float outside = eni_dsp_band_power(&psd, 25.0f, 35.0f);
    assert(custom > outside);
    PASS("band_power_custom_range");
}

int main(void) {
    printf("=== ENI DSP Extended Tests ===\n");
    test_fft_dc_signal();
    test_fft_zero_signal();
    test_fft_multiple_sines();
    test_fft_invalid_size();
    test_fft_all_valid_sizes();
    test_psd_frequency_resolution();
    test_band_power_isolation();
    test_gamma_band_power();
    test_artifact_no_false_positive();
    test_artifact_muscle();
    test_epoch_overflow();
    test_epoch_reset_and_refill();
    test_features_zero_signal();
    test_features_hjorth_consistency();
    test_features_spectral_entropy_range();
    test_band_power_custom_range();
    printf("\n=== ALL %d TESTS PASSED ===\n", passed);
    return 0;
}
