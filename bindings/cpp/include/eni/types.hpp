// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// C++17 type wrappers for eNI C API

#ifndef ENI_CPP_TYPES_HPP
#define ENI_CPP_TYPES_HPP

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <optional>
#include <stdexcept>
#include <array>
#include <span>
#include <chrono>

extern "C" {
#include "eni/types.h"
#include "eni/event.h"
#include "eni/dsp.h"
#include "eni/nn.h"
#include "eni/decoder.h"
#include "eni/config.h"
#include "eni/stim_safety.h"
#include "eni/stimulator.h"
}

namespace eni {

class Error : public std::runtime_error {
public:
    explicit Error(eni_status_t code)
        : std::runtime_error(eni_status_str(code)), code_(code) {}

    Error(eni_status_t code, std::string_view detail)
        : std::runtime_error(std::string(eni_status_str(code)) + ": " + std::string(detail)),
          code_(code) {}

    [[nodiscard]] eni_status_t code() const noexcept { return code_; }

private:
    eni_status_t code_;
};

inline void check(eni_status_t st) {
    if (st != ENI_OK) throw Error(st);
}

inline void check(eni_status_t st, std::string_view context) {
    if (st != ENI_OK) throw Error(st, context);
}

using Status = eni_status_t;

class Timestamp {
public:
    Timestamp() noexcept : ts_{0, 0} {}
    explicit Timestamp(eni_timestamp_t ts) noexcept : ts_(ts) {}

    static Timestamp now() noexcept { return Timestamp(eni_timestamp_now()); }

    [[nodiscard]] uint64_t sec() const noexcept { return ts_.sec; }
    [[nodiscard]] uint32_t nsec() const noexcept { return ts_.nsec; }
    [[nodiscard]] const eni_timestamp_t& raw() const noexcept { return ts_; }

    [[nodiscard]] double to_seconds() const noexcept {
        return static_cast<double>(ts_.sec) + static_cast<double>(ts_.nsec) * 1e-9;
    }

    [[nodiscard]] uint64_t to_ms() const noexcept {
        return ts_.sec * 1000 + ts_.nsec / 1000000;
    }

    auto operator<=>(const Timestamp& o) const noexcept {
        if (ts_.sec != o.ts_.sec) return ts_.sec <=> o.ts_.sec;
        return ts_.nsec <=> o.ts_.nsec;
    }

    bool operator==(const Timestamp& o) const noexcept {
        return ts_.sec == o.ts_.sec && ts_.nsec == o.ts_.nsec;
    }

private:
    eni_timestamp_t ts_;
};

class Event {
public:
    Event() noexcept { std::memset(&ev_, 0, sizeof(ev_)); }

    Event(eni_event_type_t type, std::string_view source) {
        check(eni_event_init(&ev_, type, source.data()), "Event::init");
    }

    void set_intent(std::string_view intent, float confidence) {
        check(eni_event_set_intent(&ev_, intent.data(), confidence), "Event::set_intent");
    }

    void add_feature(std::string_view name, float value) {
        check(eni_event_add_feature(&ev_, name.data(), value), "Event::add_feature");
    }

    void set_raw(const uint8_t* data, size_t len) {
        check(eni_event_set_raw(&ev_, data, len), "Event::set_raw");
    }

    [[nodiscard]] eni_event_type_t type() const noexcept { return ev_.type; }
    [[nodiscard]] uint32_t id() const noexcept { return ev_.id; }
    [[nodiscard]] std::string_view source() const noexcept { return ev_.source; }
    [[nodiscard]] Timestamp timestamp() const noexcept { return Timestamp(ev_.timestamp); }

    [[nodiscard]] std::string_view intent_name() const noexcept {
        return ev_.payload.intent.name;
    }

    [[nodiscard]] float intent_confidence() const noexcept {
        return ev_.payload.intent.confidence;
    }

    [[nodiscard]] eni_event_t& raw() noexcept { return ev_; }
    [[nodiscard]] const eni_event_t& raw() const noexcept { return ev_; }

private:
    eni_event_t ev_;
};

class Config {
public:
    Config() { check(eni_config_init(&cfg_), "Config::init"); }

    explicit Config(eni_variant_t variant) {
        check(eni_config_init(&cfg_), "Config::init");
        check(eni_config_load_defaults(&cfg_, variant), "Config::load_defaults");
    }

    void load_file(std::string_view path) {
        check(eni_config_load_file(&cfg_, path.data()), "Config::load_file");
    }

    void set_variant(eni_variant_t v) noexcept { cfg_.variant = v; }
    void set_mode(eni_mode_t m) noexcept { cfg_.mode = m; }

    void set_dsp(uint32_t epoch_size, uint32_t overlap, uint32_t sample_rate,
                 float artifact_threshold = 50.0f) noexcept {
        cfg_.dsp.epoch_size = epoch_size;
        cfg_.dsp.overlap = overlap;
        cfg_.dsp.sample_rate = sample_rate;
        cfg_.dsp.artifact_threshold = artifact_threshold;
    }

    void set_decoder(std::string_view model_path, int num_classes,
                     float confidence_threshold = 0.5f) noexcept {
        std::strncpy(cfg_.decoder.model_path, model_path.data(),
                     sizeof(cfg_.decoder.model_path) - 1);
        cfg_.decoder.num_classes = num_classes;
        cfg_.decoder.confidence_threshold = confidence_threshold;
    }

    [[nodiscard]] eni_variant_t variant() const noexcept { return cfg_.variant; }
    [[nodiscard]] eni_mode_t mode() const noexcept { return cfg_.mode; }
    [[nodiscard]] eni_config_t& raw() noexcept { return cfg_; }
    [[nodiscard]] const eni_config_t& raw() const noexcept { return cfg_; }

private:
    eni_config_t cfg_;
};

class DspFeatures {
public:
    DspFeatures() noexcept { std::memset(&feat_, 0, sizeof(feat_)); }
    explicit DspFeatures(const eni_dsp_features_t& f) noexcept : feat_(f) {}

    [[nodiscard]] float delta() const noexcept { return feat_.band_power[0]; }
    [[nodiscard]] float theta() const noexcept { return feat_.band_power[1]; }
    [[nodiscard]] float alpha() const noexcept { return feat_.band_power[2]; }
    [[nodiscard]] float beta() const noexcept { return feat_.band_power[3]; }
    [[nodiscard]] float gamma() const noexcept { return feat_.band_power[4]; }
    [[nodiscard]] float total_power() const noexcept { return feat_.total_power; }
    [[nodiscard]] float spectral_entropy() const noexcept { return feat_.spectral_entropy; }
    [[nodiscard]] float hjorth_activity() const noexcept { return feat_.hjorth_activity; }
    [[nodiscard]] float hjorth_mobility() const noexcept { return feat_.hjorth_mobility; }
    [[nodiscard]] float hjorth_complexity() const noexcept { return feat_.hjorth_complexity; }

    [[nodiscard]] eni_dsp_features_t& raw() noexcept { return feat_; }
    [[nodiscard]] const eni_dsp_features_t& raw() const noexcept { return feat_; }

private:
    eni_dsp_features_t feat_;
};

class DspEngine {
public:
    explicit DspEngine(int fft_size) {
        check(eni_dsp_fft_init(&ctx_, fft_size), "DspEngine::init");
    }

    DspEngine(const DspEngine&) = delete;
    DspEngine& operator=(const DspEngine&) = delete;
    DspEngine(DspEngine&& o) noexcept : ctx_(o.ctx_) { o.ctx_.initialized = 0; }

    DspFeatures extract_features(const float* signal, int n, float sample_rate) {
        eni_dsp_features_t feat{};
        check(eni_dsp_extract_features(&ctx_, signal, n, sample_rate, &feat),
              "DspEngine::extract_features");
        return DspFeatures(feat);
    }

    void fft(float* re, float* im, int n) {
        check(eni_dsp_fft(&ctx_, re, im, n), "DspEngine::fft");
    }

    eni_dsp_psd_result_t psd(const float* signal, int n, float sample_rate) {
        eni_dsp_psd_result_t result{};
        check(eni_dsp_psd(&ctx_, signal, n, sample_rate, &result), "DspEngine::psd");
        return result;
    }

    [[nodiscard]] int size() const noexcept { return ctx_.size; }
    [[nodiscard]] eni_dsp_fft_ctx_t& raw() noexcept { return ctx_; }

private:
    eni_dsp_fft_ctx_t ctx_;
};

class NnModel {
public:
    NnModel() noexcept { std::memset(&model_, 0, sizeof(model_)); }

    void load(const uint8_t* data, size_t len) {
        check(eni_nn_load(&model_, data, len), "NnModel::load");
    }

    void forward(const float* input, float* output, int max_output) const {
        check(eni_nn_forward(&model_, input, output, max_output), "NnModel::forward");
    }

    [[nodiscard]] int input_size() const noexcept { return model_.input_size; }
    [[nodiscard]] int output_size() const noexcept { return model_.output_size; }
    [[nodiscard]] int layer_count() const noexcept { return model_.layer_count; }
    [[nodiscard]] eni_nn_model_t& raw() noexcept { return model_; }
    [[nodiscard]] const eni_nn_model_t& raw() const noexcept { return model_; }

private:
    eni_nn_model_t model_;
};

struct DecodeResult {
    std::string best_intent;
    float       best_confidence;
    int         count;

    struct Intent {
        std::string name;
        float       confidence;
    };
    std::array<Intent, ENI_DECODE_MAX_CLASSES> intents;
};

class Decoder {
public:
    Decoder(const eni_decoder_ops_t& ops, const eni_decoder_config_t& cfg) {
        check(eni_decoder_init(&dec_, &ops, &cfg), "Decoder::init");
    }

    ~Decoder() { eni_decoder_shutdown(&dec_); }

    Decoder(const Decoder&) = delete;
    Decoder& operator=(const Decoder&) = delete;

    DecodeResult decode(const DspFeatures& features) {
        eni_decode_result_t result{};
        check(eni_decoder_decode(&dec_, &features.raw(), &result), "Decoder::decode");

        DecodeResult out{};
        out.count = result.count;
        out.best_intent = result.intents[result.best_idx].name;
        out.best_confidence = result.intents[result.best_idx].confidence;
        for (int i = 0; i < result.count && i < ENI_DECODE_MAX_CLASSES; ++i) {
            out.intents[static_cast<size_t>(i)].name = result.intents[i].name;
            out.intents[static_cast<size_t>(i)].confidence = result.intents[i].confidence;
        }
        return out;
    }

    [[nodiscard]] std::string_view name() const noexcept { return dec_.name; }
    [[nodiscard]] eni_decoder_t& raw() noexcept { return dec_; }

private:
    eni_decoder_t dec_;
};

class StimSafety {
public:
    StimSafety(float max_amp, uint32_t max_dur_ms,
               uint32_t min_interval_ms = 1000, uint32_t max_daily = 100) noexcept {
        eni_stim_safety_init(&safety_, max_amp, max_dur_ms, min_interval_ms, max_daily);
    }

    [[nodiscard]] bool check(const eni_stim_params_t& params, uint64_t current_time_ms) const noexcept {
        return eni_stim_safety_check(&safety_, &params, current_time_ms) == ENI_OK;
    }

    void record(uint64_t current_time_ms) noexcept {
        eni_stim_safety_record(&safety_, current_time_ms);
    }

    void reset_daily() noexcept {
        eni_stim_safety_reset_daily(&safety_);
    }

    [[nodiscard]] eni_stim_safety_t& raw() noexcept { return safety_; }
    [[nodiscard]] const eni_stim_safety_t& raw() const noexcept { return safety_; }

private:
    eni_stim_safety_t safety_;
};

} // namespace eni

#endif // ENI_CPP_TYPES_HPP
