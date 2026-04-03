// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// C++17 RAII umbrella header for eNI

#ifndef ENI_CPP_ENI_HPP
#define ENI_CPP_ENI_HPP

extern "C" {
#include "eni/common.h"
#include "eni/dsp.h"
#include "eni/nn.h"
#include "eni/decoder.h"
#include "eni/feedback.h"
#include "eni/stim_safety.h"
#include "eni/stimulator.h"
}

#include "eni/types.hpp"
#include "eni/provider.hpp"

namespace eni {

inline constexpr int version_major = ENI_VERSION_MAJOR;
inline constexpr int version_minor = ENI_VERSION_MINOR;
inline constexpr int version_patch = ENI_VERSION_PATCH;
inline constexpr const char* version_string = ENI_VERSION_STRING;

inline constexpr int max_fft_size   = ENI_DSP_MAX_FFT_SIZE;
inline constexpr int max_psd_bins   = ENI_DSP_MAX_PSD_BINS;
inline constexpr int max_nn_weights = ENI_NN_MAX_WEIGHTS;
inline constexpr int max_nn_layers  = ENI_NN_MAX_LAYERS;

class Pipeline {
public:
    struct Stage {
        enum class Type { DSP, NN, DECODER, FEEDBACK };
        Type        type;
        std::string label;
    };

    Pipeline() = default;

    Pipeline& add_provider(const eni_provider_ops_t& ops, std::string_view name,
                           const void* config = nullptr) {
        provider_.emplace(ops, name, config);
        return *this;
    }

    Pipeline& add_dsp(int fft_size) {
        dsp_.emplace(fft_size);
        stages_.push_back({Stage::Type::DSP, "dsp"});
        return *this;
    }

    Pipeline& add_decoder(const eni_decoder_ops_t& ops, const eni_decoder_config_t& cfg) {
        decoder_.emplace(ops, cfg);
        stages_.push_back({Stage::Type::DECODER, "decoder"});
        return *this;
    }

    [[nodiscard]] bool has_provider() const noexcept { return provider_.has_value(); }
    [[nodiscard]] bool has_dsp() const noexcept { return dsp_.has_value(); }
    [[nodiscard]] bool has_decoder() const noexcept { return decoder_.has_value(); }

    [[nodiscard]] Provider& provider() { return *provider_; }
    [[nodiscard]] DspEngine& dsp() { return *dsp_; }
    [[nodiscard]] Decoder& decoder() { return *decoder_; }

    [[nodiscard]] const std::vector<Stage>& stages() const noexcept { return stages_; }

private:
    std::optional<Provider>  provider_;
    std::optional<DspEngine> dsp_;
    std::optional<Decoder>   decoder_;
    std::vector<Stage>       stages_;
};

} // namespace eni

#endif // ENI_CPP_ENI_HPP
