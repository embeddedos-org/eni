// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// Basic C++ pipeline example demonstrating RAII wrappers

#include "eni/eni.hpp"

#include <cstdio>
#include <cmath>

extern "C" {
#include "eni/provider_contract.h"
}

extern const eni_provider_ops_t eni_provider_simulator_ops;

static void example_provider_polling() {
    std::printf("--- Provider Polling Example ---\n");

    eni::Provider prov(eni_provider_simulator_ops, "sim_cpp");
    std::printf("  Provider '%.*s' created\n",
                static_cast<int>(prov.name().size()), prov.name().data());

    prov.start();
    std::printf("  Provider started (running=%d)\n", prov.running());

    for (int i = 0; i < 5; ++i) {
        auto ev = prov.poll();
        if (ev) {
            std::printf("  Event: type=%d source=%.*s\n",
                        ev->type(),
                        static_cast<int>(ev->source().size()),
                        ev->source().data());
        } else {
            std::printf("  No event available\n");
        }
    }

    prov.stop();
    std::printf("  Provider stopped\n\n");
}

static void example_dsp_processing() {
    std::printf("--- DSP Processing Example ---\n");

    constexpr int N = 256;
    constexpr float SAMPLE_RATE = 256.0f;
    constexpr float PI = 3.14159265358979f;

    float signal[N];
    for (int i = 0; i < N; ++i) {
        signal[i] = 40.0f * std::sin(2.0f * PI * 10.0f * static_cast<float>(i) / SAMPLE_RATE)
                  + 20.0f * std::sin(2.0f * PI * 22.0f * static_cast<float>(i) / SAMPLE_RATE);
    }

    eni::DspEngine dsp(N);
    auto features = dsp.extract_features(signal, N, SAMPLE_RATE);

    std::printf("  Alpha power:       %.4f\n", features.alpha());
    std::printf("  Beta power:        %.4f\n", features.beta());
    std::printf("  Total power:       %.4f\n", features.total_power());
    std::printf("  Spectral entropy:  %.4f\n", features.spectral_entropy());
    std::printf("  Hjorth activity:   %.4f\n", features.hjorth_activity());
    std::printf("\n");
}

static void example_decoder() {
    std::printf("--- Decoder Example ---\n");

    eni_decoder_config_t cfg{};
    cfg.confidence_threshold = 0.5f;

    eni::Decoder decoder(eni_decoder_energy_ops, cfg);
    std::printf("  Decoder '%.*s' initialized\n",
                static_cast<int>(decoder.name().size()), decoder.name().data());

    eni::DspFeatures feat;
    feat.raw().total_power = 75.0f;
    feat.raw().band_power[2] = 30.0f; // alpha

    auto result = decoder.decode(feat);
    std::printf("  Best intent: %s (confidence=%.3f)\n",
                result.best_intent.c_str(), result.best_confidence);
    std::printf("  Total intents: %d\n", result.count);
    std::printf("\n");
}

static void example_pipeline() {
    std::printf("--- Pipeline Example ---\n");

    eni_decoder_config_t dec_cfg{};
    dec_cfg.confidence_threshold = 0.5f;

    eni::Pipeline pipeline;
    pipeline.add_provider(eni_provider_simulator_ops, "sim_pipeline")
            .add_dsp(256)
            .add_decoder(eni_decoder_energy_ops, dec_cfg);

    std::printf("  Pipeline stages: %zu\n", pipeline.stages().size());
    std::printf("  Has provider: %d\n", pipeline.has_provider());
    std::printf("  Has DSP: %d\n", pipeline.has_dsp());
    std::printf("  Has decoder: %d\n", pipeline.has_decoder());
    std::printf("\n");
}

static void example_error_handling() {
    std::printf("--- Error Handling Example ---\n");

    try {
        eni::DspEngine bad_dsp(100); // not power of 2
        std::printf("  ERROR: should have thrown\n");
    } catch (const eni::Error& e) {
        std::printf("  Caught expected error: %s (code=%d)\n", e.what(), e.code());
    }

    try {
        eni::NnModel model;
        uint8_t bad_data[32] = {0};
        model.load(bad_data, sizeof(bad_data));
        std::printf("  ERROR: should have thrown\n");
    } catch (const eni::Error& e) {
        std::printf("  Caught expected error: %s (code=%d)\n", e.what(), e.code());
    }

    std::printf("\n");
}

int main() {
    std::printf("=== eNI C++ Bindings Example ===\n");
    std::printf("  Version: %s\n\n", eni::version_string);

    example_provider_polling();
    example_dsp_processing();
    example_decoder();
    example_pipeline();
    example_error_handling();

    std::printf("=== All examples completed ===\n");
    return 0;
}
