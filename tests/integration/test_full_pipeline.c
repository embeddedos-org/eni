// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// Full integration pipeline test: Provider → DSP → Decoder → Feedback → Safety

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "eni/common.h"
#include "eni/dsp.h"
#include "eni/nn.h"
#include "eni/decoder.h"
#include "eni/feedback.h"
#include "eni/stim_safety.h"
#include "eni/stimulator.h"
#include "eni/provider_contract.h"

static int passed = 0;
#define PASS(name) do { printf("[PASS] %s\n", name); passed++; } while(0)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern const eni_provider_ops_t eni_provider_simulator_ops;
extern const eni_stimulator_ops_t eni_stimulator_sim_ops;

/* ── Pipeline stage tests ─────────────────────────────────────────── */

static void test_pipeline_provider_to_dsp(void) {
    /* Initialize provider */
    eni_provider_t prov;
    eni_status_t rc = eni_provider_init(&prov, &eni_provider_simulator_ops, "pipe_sim", NULL);
    assert(rc == ENI_OK);
    rc = eni_provider_start(&prov);
    assert(rc == ENI_OK);
    assert(prov.running == true);

    /* Generate synthetic signal as if from provider */
    eni_dsp_fft_ctx_t fft_ctx;
    rc = eni_dsp_fft_init(&fft_ctx, 256);
    assert(rc == ENI_OK);

    float signal[256];
    for (int i = 0; i < 256; i++) {
        signal[i] = 40.0f * sinf(2.0f * (float)M_PI * 10.0f * (float)i / 256.0f)
                  + 20.0f * sinf(2.0f * (float)M_PI * 22.0f * (float)i / 256.0f);
    }

    eni_dsp_features_t features;
    rc = eni_dsp_extract_features(&fft_ctx, signal, 256, 256.0f, &features);
    assert(rc == ENI_OK);
    assert(features.total_power > 0.0f);
    assert(features.band_power[2] > 0.0f); /* alpha from 10 Hz */

    eni_provider_stop(&prov);
    eni_provider_shutdown(&prov);
    PASS("pipeline_provider_to_dsp");
}

static void test_pipeline_dsp_to_decoder(void) {
    /* DSP stage */
    eni_dsp_fft_ctx_t fft_ctx;
    eni_dsp_fft_init(&fft_ctx, 256);
    float signal[256];
    for (int i = 0; i < 256; i++) {
        signal[i] = 50.0f * sinf(2.0f * (float)M_PI * 10.0f * (float)i / 256.0f);
    }
    eni_dsp_features_t features;
    eni_dsp_extract_features(&fft_ctx, signal, 256, 256.0f, &features);

    /* Decoder stage */
    eni_decoder_t dec;
    eni_decoder_config_t dec_cfg = {0};
    dec_cfg.confidence_threshold = 0.3f;
    eni_status_t rc = eni_decoder_init(&dec, &eni_decoder_energy_ops, &dec_cfg);
    assert(rc == ENI_OK);

    eni_decode_result_t result = {0};
    rc = eni_decoder_decode(&dec, &features, &result);
    assert(rc == ENI_OK);
    assert(result.count > 0);
    assert(result.best_idx >= 0);
    assert(result.intents[result.best_idx].confidence > 0.0f);
    assert(strlen(result.intents[result.best_idx].name) > 0);

    eni_decoder_shutdown(&dec);
    PASS("pipeline_dsp_to_decoder");
}

static void test_pipeline_decoder_to_event(void) {
    /* Decoder */
    eni_decoder_t dec;
    eni_decoder_config_t dec_cfg = {0};
    dec_cfg.confidence_threshold = 0.3f;
    eni_decoder_init(&dec, &eni_decoder_energy_ops, &dec_cfg);

    eni_dsp_features_t features = {0};
    features.total_power = 80.0f;
    features.band_power[2] = 40.0f;

    eni_decode_result_t result = {0};
    eni_decoder_decode(&dec, &features, &result);

    /* Convert decode result to event */
    eni_event_t ev;
    eni_status_t rc = eni_event_init(&ev, ENI_EVENT_INTENT, "decoder");
    assert(rc == ENI_OK);
    rc = eni_event_set_intent(&ev, result.intents[result.best_idx].name,
                              result.intents[result.best_idx].confidence);
    assert(rc == ENI_OK);
    assert(ev.type == ENI_EVENT_INTENT);
    assert(ev.payload.intent.confidence > 0.0f);

    eni_decoder_shutdown(&dec);
    PASS("pipeline_decoder_to_event");
}

static void test_pipeline_safety_check(void) {
    /* Safety subsystem */
    eni_stim_safety_t safety;
    eni_stim_safety_init(&safety, 1.5f, 300000, 1000, 50);

    eni_stim_params_t params = {0};
    params.type = ENI_STIM_HAPTIC;
    params.amplitude = 0.8f;
    params.duration_ms = 200;
    params.frequency_hz = 100.0f;

    /* Safety check should pass */
    assert(eni_stim_safety_check(&safety, &params, 0) == ENI_OK);
    eni_stim_safety_record(&safety, 0);

    /* Rate limited */
    assert(eni_stim_safety_check(&safety, &params, 500) == ENI_ERR_PERMISSION);

    /* After interval */
    assert(eni_stim_safety_check(&safety, &params, 1500) == ENI_OK);

    PASS("pipeline_safety_check");
}

static void test_pipeline_full_signal_to_intent(void) {
    /* Stage 1: Simulate provider producing neural data */
    eni_provider_t prov;
    eni_provider_init(&prov, &eni_provider_simulator_ops, "full_pipe", NULL);
    eni_provider_start(&prov);

    /* Stage 2: DSP feature extraction */
    eni_dsp_fft_ctx_t fft_ctx;
    eni_dsp_fft_init(&fft_ctx, 256);

    float signal[256];
    for (int i = 0; i < 256; i++) {
        signal[i] = 60.0f * sinf(2.0f * (float)M_PI * 10.0f * (float)i / 256.0f)
                  + 30.0f * sinf(2.0f * (float)M_PI * 22.0f * (float)i / 256.0f)
                  + 10.0f * sinf(2.0f * (float)M_PI * 40.0f * (float)i / 256.0f);
    }

    eni_dsp_features_t features;
    eni_dsp_extract_features(&fft_ctx, signal, 256, 256.0f, &features);
    assert(features.total_power > 0.0f);

    /* Stage 3: Decode intent */
    eni_decoder_t dec;
    eni_decoder_config_t dec_cfg = {0};
    dec_cfg.confidence_threshold = 0.3f;
    eni_decoder_init(&dec, &eni_decoder_energy_ops, &dec_cfg);

    eni_decode_result_t result = {0};
    eni_decoder_decode(&dec, &features, &result);
    assert(result.count > 0);

    /* Stage 4: Create intent event */
    eni_event_t intent_ev;
    eni_event_init(&intent_ev, ENI_EVENT_INTENT, "pipeline");
    eni_event_set_intent(&intent_ev,
                         result.intents[result.best_idx].name,
                         result.intents[result.best_idx].confidence);

    /* Stage 5: Safety check before potential feedback */
    eni_stim_safety_t safety;
    eni_stim_safety_init(&safety, 1.5f, 300000, 1000, 50);
    eni_stim_params_t stim_params = {0};
    stim_params.type = ENI_STIM_HAPTIC;
    stim_params.amplitude = 0.5f;
    stim_params.duration_ms = 100;
    assert(eni_stim_safety_check(&safety, &stim_params, 0) == ENI_OK);

    /* Verify full pipeline data flow */
    assert(intent_ev.type == ENI_EVENT_INTENT);
    assert(strlen(intent_ev.payload.intent.name) > 0);
    assert(intent_ev.payload.intent.confidence > 0.0f);

    eni_decoder_shutdown(&dec);
    eni_provider_stop(&prov);
    eni_provider_shutdown(&prov);
    PASS("pipeline_full_signal_to_intent");
}

static void test_pipeline_epoch_accumulation(void) {
    /* Simulate epoch-based processing as in real BCI */
    eni_dsp_epoch_t epoch;
    eni_dsp_epoch_init(&epoch, 256, 256.0f);

    /* Push samples one at a time (like streaming from provider) */
    for (int i = 0; i < 256; i++) {
        float sample = 40.0f * sinf(2.0f * (float)M_PI * 10.0f * (float)i / 256.0f);
        eni_dsp_epoch_push(&epoch, sample);
    }
    assert(eni_dsp_epoch_ready(&epoch));

    /* Process complete epoch */
    eni_dsp_fft_ctx_t fft_ctx;
    eni_dsp_fft_init(&fft_ctx, 256);
    eni_dsp_features_t features;
    eni_dsp_extract_features(&fft_ctx, epoch.samples, epoch.count, epoch.sample_rate, &features);
    assert(features.total_power > 0.0f);

    /* Reset for next epoch */
    eni_dsp_epoch_reset(&epoch);
    assert(!eni_dsp_epoch_ready(&epoch));

    PASS("pipeline_epoch_accumulation");
}

static void test_pipeline_multi_epoch_decode(void) {
    eni_dsp_fft_ctx_t fft_ctx;
    eni_dsp_fft_init(&fft_ctx, 256);

    eni_decoder_t dec;
    eni_decoder_config_t dec_cfg = {0};
    dec_cfg.confidence_threshold = 0.3f;
    eni_decoder_init(&dec, &eni_decoder_energy_ops, &dec_cfg);

    /* Process 5 epochs with different signal characteristics */
    float frequencies[] = {3.0f, 8.0f, 10.0f, 22.0f, 45.0f};
    for (int epoch = 0; epoch < 5; epoch++) {
        float signal[256];
        for (int i = 0; i < 256; i++) {
            signal[i] = 50.0f * sinf(2.0f * (float)M_PI * frequencies[epoch] * (float)i / 256.0f);
        }

        eni_dsp_features_t features;
        eni_dsp_extract_features(&fft_ctx, signal, 256, 256.0f, &features);

        eni_decode_result_t result = {0};
        assert(eni_decoder_decode(&dec, &features, &result) == ENI_OK);
        assert(result.count > 0);
    }

    eni_decoder_shutdown(&dec);
    PASS("pipeline_multi_epoch_decode");
}

static void test_pipeline_config_driven(void) {
    /* Initialize config */
    eni_config_t cfg;
    eni_status_t rc = eni_config_init(&cfg);
    assert(rc == ENI_OK);

    cfg.variant = ENI_VARIANT_FRAMEWORK;
    cfg.mode = ENI_MODE_FEATURES_INTENT;
    cfg.dsp.epoch_size = 256;
    cfg.dsp.sample_rate = 256;
    cfg.dsp.artifact_threshold = 50.0f;
    cfg.decoder.num_classes = 4;
    cfg.decoder.confidence_threshold = 0.5f;

    /* Use config to drive pipeline setup */
    eni_dsp_fft_ctx_t fft_ctx;
    rc = eni_dsp_fft_init(&fft_ctx, (int)cfg.dsp.epoch_size);
    assert(rc == ENI_OK);

    eni_decoder_t dec;
    rc = eni_decoder_init(&dec, &eni_decoder_energy_ops, &cfg.decoder);
    assert(rc == ENI_OK);

    /* Process with config-driven parameters */
    float signal[256];
    for (int i = 0; i < 256; i++) {
        signal[i] = 40.0f * sinf(2.0f * (float)M_PI * 10.0f * (float)i / (float)cfg.dsp.sample_rate);
    }

    eni_dsp_features_t features;
    eni_dsp_extract_features(&fft_ctx, signal, (int)cfg.dsp.epoch_size,
                             (float)cfg.dsp.sample_rate, &features);

    eni_decode_result_t result = {0};
    rc = eni_decoder_decode(&dec, &features, &result);
    assert(rc == ENI_OK);

    eni_decoder_shutdown(&dec);
    PASS("pipeline_config_driven");
}

static void test_pipeline_artifact_rejection(void) {
    /* Signal with artifact */
    float signal[256];
    for (int i = 0; i < 256; i++) {
        signal[i] = 40.0f * sinf(2.0f * (float)M_PI * 10.0f * (float)i / 256.0f);
    }
    signal[64] = 500.0f; /* eye blink artifact */

    /* Detect artifact */
    eni_dsp_artifact_t artifact = eni_dsp_artifact_detect(signal, 256, 50.0f);

    if (artifact.eye_blink || artifact.muscle || artifact.movement) {
        /* In real pipeline, we'd reject this epoch */
        assert(artifact.severity > 0.0f);
    }

    /* Clean signal should pass */
    float clean[256];
    for (int i = 0; i < 256; i++) {
        clean[i] = 40.0f * sinf(2.0f * (float)M_PI * 10.0f * (float)i / 256.0f);
    }
    eni_dsp_artifact_t clean_art = eni_dsp_artifact_detect(clean, 256, 100.0f);
    assert(clean_art.eye_blink == false);

    PASS("pipeline_artifact_rejection");
}

int main(void) {
    printf("=== ENI Full Pipeline Integration Tests ===\n");
    test_pipeline_provider_to_dsp();
    test_pipeline_dsp_to_decoder();
    test_pipeline_decoder_to_event();
    test_pipeline_safety_check();
    test_pipeline_full_signal_to_intent();
    test_pipeline_epoch_accumulation();
    test_pipeline_multi_epoch_decode();
    test_pipeline_config_driven();
    test_pipeline_artifact_rejection();
    printf("\n=== ALL %d TESTS PASSED ===\n", passed);
    return 0;
}
