// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// C++17 RAII provider wrapper for eNI

#ifndef ENI_CPP_PROVIDER_HPP
#define ENI_CPP_PROVIDER_HPP

#include "eni/types.hpp"

extern "C" {
#include "eni/provider_contract.h"
}

#include <functional>
#include <string>
#include <string_view>
#include <optional>
#include <utility>

namespace eni {

class Provider {
public:
    Provider(const eni_provider_ops_t& ops, std::string_view name,
             const void* config = nullptr) {
        check(eni_provider_init(&prov_, &ops, name.data(), config), "Provider::init");
    }

    ~Provider() {
        if (prov_.ops) {
            if (prov_.running) {
                eni_provider_stop(&prov_);
            }
            eni_provider_shutdown(&prov_);
        }
    }

    Provider(const Provider&) = delete;
    Provider& operator=(const Provider&) = delete;

    Provider(Provider&& o) noexcept : prov_(o.prov_) {
        o.prov_.ops = nullptr;
    }

    Provider& operator=(Provider&& o) noexcept {
        if (this != &o) {
            if (prov_.ops) {
                if (prov_.running) eni_provider_stop(&prov_);
                eni_provider_shutdown(&prov_);
            }
            prov_ = o.prov_;
            o.prov_.ops = nullptr;
        }
        return *this;
    }

    void start() {
        check(eni_provider_start(&prov_), "Provider::start");
    }

    void stop() {
        check(eni_provider_stop(&prov_), "Provider::stop");
    }

    [[nodiscard]] std::optional<Event> poll() {
        Event ev;
        eni_status_t st = eni_provider_poll(&prov_, &ev.raw());
        if (st == ENI_OK) return ev;
        if (st == ENI_ERR_NOT_FOUND || st == ENI_ERR_TIMEOUT) return std::nullopt;
        throw Error(st, "Provider::poll");
    }

    [[nodiscard]] bool running() const noexcept { return prov_.running; }
    [[nodiscard]] std::string_view name() const noexcept { return prov_.name; }
    [[nodiscard]] eni_transport_t transport() const noexcept { return prov_.transport; }
    [[nodiscard]] eni_provider_t& raw() noexcept { return prov_; }
    [[nodiscard]] const eni_provider_t& raw() const noexcept { return prov_; }

private:
    eni_provider_t prov_;
};

class ProviderGuard {
public:
    explicit ProviderGuard(Provider& prov) : prov_(prov) {
        prov_.start();
    }

    ~ProviderGuard() {
        try { prov_.stop(); } catch (...) {}
    }

    ProviderGuard(const ProviderGuard&) = delete;
    ProviderGuard& operator=(const ProviderGuard&) = delete;

    [[nodiscard]] Provider& provider() noexcept { return prov_; }

    [[nodiscard]] std::optional<Event> poll() { return prov_.poll(); }

private:
    Provider& prov_;
};

template<typename PollCallback>
void run_provider_loop(Provider& prov, PollCallback&& on_event,
                       int max_iterations = -1) {
    ProviderGuard guard(prov);
    int count = 0;
    while (max_iterations < 0 || count < max_iterations) {
        auto ev = guard.poll();
        if (ev) {
            if (!std::forward<PollCallback>(on_event)(*ev)) break;
        }
        ++count;
    }
}

} // namespace eni

#endif // ENI_CPP_PROVIDER_HPP
