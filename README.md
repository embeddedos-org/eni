# ENI — Embedded Neural Interface


## Install

```bash
# Clone
git clone https://github.com/embeddedos-org/eni.git
cd eni

# Build (minimal + framework + neuralink)
cmake -B build -DENI_BUILD_TESTS=ON
cmake --build build

# Minimal only (MCU targets)
cmake -B build -DENI_BUILD_MIN=ON -DENI_BUILD_FRAMEWORK=OFF

# Without Neuralink
cmake -B build -DENI_PROVIDER_NEURALINK=OFF

# Cross-compile for ARM
cmake -B build-arm -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_SYSTEM_NAME=Linux
cmake --build build-arm
```

**Real-time neural, BCI, and assistive-input integration layer for EoS.**

ENI provides a standardized, vendor-neutral framework to integrate brain-computer interfaces (BCI), neural decoders, and assistive input systems into the EoS embedded OS platform.

## Configurations


## Install

```bash
# Clone
git clone https://github.com/embeddedos-org/eni.git
cd eni

# Build (minimal + framework + neuralink)
cmake -B build -DENI_BUILD_TESTS=ON
cmake --build build

# Minimal only (MCU targets)
cmake -B build -DENI_BUILD_MIN=ON -DENI_BUILD_FRAMEWORK=OFF

# Without Neuralink
cmake -B build -DENI_PROVIDER_NEURALINK=OFF

# Cross-compile for ARM
cmake -B build-arm -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_SYSTEM_NAME=Linux
cmake --build build-arm
```

ENI supports two build configurations:

| Config | Option | Description |
|---|---|---|
| **Minimal** | `ENI_BUILD_MIN=ON` | Lightweight runtime for resource-constrained MCUs. Input normalization, signal filtering, event mapping, tool bridge. Minimal RAM footprint. |
| **Framework** | `ENI_BUILD_FRAMEWORK=ON` | Full industrial platform with connectors, pipeline orchestration, advanced signal processing. For application processors (RPi, i.MX8M, x86). |

Both configurations share the common library (events, providers, policy, EIPC bridge).

## Neuralink Support


## Install

```bash
# Clone
git clone https://github.com/embeddedos-org/eni.git
cd eni

# Build (minimal + framework + neuralink)
cmake -B build -DENI_BUILD_TESTS=ON
cmake --build build

# Minimal only (MCU targets)
cmake -B build -DENI_BUILD_MIN=ON -DENI_BUILD_FRAMEWORK=OFF

# Without Neuralink
cmake -B build -DENI_PROVIDER_NEURALINK=OFF

# Cross-compile for ARM
cmake -B build-arm -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_SYSTEM_NAME=Linux
cmake --build build-arm
```

ENI includes a **Neuralink adapter** (`providers/neuralink/`) that provides:

- **1024-channel electrode support** at 30kHz sampling rate
- **4 operating modes**: raw data, decoded signals, intent classification, motor output
- **Signal processing**: bandpass filter (0.5–300Hz), 60Hz notch filter, baseline calibration
- **Intent decoder**: classifies neural energy into `idle`, `attention`, `motor_intent`, `motor_execute`
- **Calibration**: automatic baseline computation over configurable duration
- **Streaming API**: packet-based data acquisition with callback support
- **Provider integration**: registers as standard ENI provider, pluggable alongside simulator/generic

### Usage


## Install

```bash
# Clone
git clone https://github.com/embeddedos-org/eni.git
cd eni

# Build (minimal + framework + neuralink)
cmake -B build -DENI_BUILD_TESTS=ON
cmake --build build

# Minimal only (MCU targets)
cmake -B build -DENI_BUILD_MIN=ON -DENI_BUILD_FRAMEWORK=OFF

# Without Neuralink
cmake -B build -DENI_PROVIDER_NEURALINK=OFF

# Cross-compile for ARM
cmake -B build-arm -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_SYSTEM_NAME=Linux
cmake --build build-arm
```

```c
#include "neuralink.h"

eni_nl_config_t cfg = {
    .mode = ENI_NL_MODE_INTENT,
    .channels = 256,
    .sample_rate = 30000,
    .filter_enabled = 1,
    .bandpass_low_hz = 0.5f,
    .bandpass_high_hz = 300.0f,
    .auto_calibrate = 1,
    .on_intent = my_intent_handler,
};

eni_neuralink_init(&cfg);
eni_neuralink_connect("neuralink-n1-001");
eni_neuralink_calibrate(5000);  // 5 second baseline
eni_neuralink_start_stream();

while (running) {
    eni_nl_packet_t pkt;
    eni_neuralink_read_packet(&pkt);

    char intent[32];
    float confidence;
    eni_neuralink_decode_intent(&pkt, intent, sizeof(intent), &confidence);
    // intent = "motor_intent", confidence = 0.78
}
```

## Providers


## Install

```bash
# Clone
git clone https://github.com/embeddedos-org/eni.git
cd eni

# Build (minimal + framework + neuralink)
cmake -B build -DENI_BUILD_TESTS=ON
cmake --build build

# Minimal only (MCU targets)
cmake -B build -DENI_BUILD_MIN=ON -DENI_BUILD_FRAMEWORK=OFF

# Without Neuralink
cmake -B build -DENI_PROVIDER_NEURALINK=OFF

# Cross-compile for ARM
cmake -B build-arm -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_SYSTEM_NAME=Linux
cmake --build build-arm
```

| Provider | Description | Status |
|---|---|---|
| **neuralink** | Neuralink BCI adapter — 1024 channels, intent decoding, calibration | ✅ Implemented |
| **simulator** | BCI signal simulator for testing without hardware | ✅ Implemented |
| **generic** | Generic neural signal decoder — vendor-agnostic | ✅ Implemented |

New adapters can be added by implementing the `eni_provider_ops_t` interface:

```c
typedef struct {
    const char *name;
    int  (*init)(void *ctx);
    int  (*read)(void *ctx, void *buf, int len);
    void (*deinit)(void *ctx);
} eni_provider_ops_t;
```

## Architecture


## Install

```bash
# Clone
git clone https://github.com/embeddedos-org/eni.git
cd eni

# Build (minimal + framework + neuralink)
cmake -B build -DENI_BUILD_TESTS=ON
cmake --build build

# Minimal only (MCU targets)
cmake -B build -DENI_BUILD_MIN=ON -DENI_BUILD_FRAMEWORK=OFF

# Without Neuralink
cmake -B build -DENI_PROVIDER_NEURALINK=OFF

# Cross-compile for ARM
cmake -B build-arm -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_SYSTEM_NAME=Linux
cmake --build build-arm
```

```
┌──────────────────────────────────────────────────────────────┐
│                    ENI Neural Interface                        │
│                                                                │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐│
│  │   Neuralink   │  │  Simulator   │  │  Generic / Custom    ││
│  │  1024ch 30kHz │  │  Test data   │  │  Vendor-agnostic     ││
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────────────┘│
│         │                  │                  │                │
│  ┌──────▼──────────────────▼──────────────────▼───────────┐   │
│  │              Provider Framework                         │   │
│  │         eni_provider_ops_t (init/read/deinit)          │   │
│  └──────────────────────┬─────────────────────────────────┘   │
│                         │                                      │
│  ┌──────────────────────▼─────────────────────────────────┐   │
│  │              Common Layer                               │   │
│  │  Events · Policy · Config · Logging · EIPC Bridge       │   │
│  └──────────────────────┬─────────────────────────────────┘   │
│                         │                                      │
│  ┌─────────────┐  ┌────▼────────┐                             │
│  │  ENI-Min    │  │ ENI-Framework│                             │
│  │  (MCU)      │  │ (App Proc)   │                             │
│  │  filter     │  │ connectors   │                             │
│  │  normalizer │  │ pipeline     │                             │
│  │  mapper     │  │ orchestrator │                             │
│  │  tool bridge│  │              │                             │
│  └─────────────┘  └─────────────┘                             │
│         │                  │                                   │
│  ┌──────▼──────────────────▼──────────────────────────────┐   │
│  │              EIPC Bridge → EAI Agent                    │   │
│  │         Neural intents → AI tool calls                  │   │
│  └────────────────────────────────────────────────────────┘   │
└──────────────────────────────────────────────────────────────┘
```

## Build


## Install

```bash
# Clone
git clone https://github.com/embeddedos-org/eni.git
cd eni

# Build (minimal + framework + neuralink)
cmake -B build -DENI_BUILD_TESTS=ON
cmake --build build

# Minimal only (MCU targets)
cmake -B build -DENI_BUILD_MIN=ON -DENI_BUILD_FRAMEWORK=OFF

# Without Neuralink
cmake -B build -DENI_PROVIDER_NEURALINK=OFF

# Cross-compile for ARM
cmake -B build-arm -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_SYSTEM_NAME=Linux
cmake --build build-arm
```

```bash
# Full build (minimal + framework + neuralink)


## Install

```bash
# Clone
git clone https://github.com/embeddedos-org/eni.git
cd eni

# Build (minimal + framework + neuralink)
cmake -B build -DENI_BUILD_TESTS=ON
cmake --build build

# Minimal only (MCU targets)
cmake -B build -DENI_BUILD_MIN=ON -DENI_BUILD_FRAMEWORK=OFF

# Without Neuralink
cmake -B build -DENI_PROVIDER_NEURALINK=OFF

# Cross-compile for ARM
cmake -B build-arm -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_SYSTEM_NAME=Linux
cmake --build build-arm
```
cmake -B build -DENI_BUILD_TESTS=ON
cmake --build build

# Minimal only (for MCU targets)


## Install

```bash
# Clone
git clone https://github.com/embeddedos-org/eni.git
cd eni

# Build (minimal + framework + neuralink)
cmake -B build -DENI_BUILD_TESTS=ON
cmake --build build

# Minimal only (MCU targets)
cmake -B build -DENI_BUILD_MIN=ON -DENI_BUILD_FRAMEWORK=OFF

# Without Neuralink
cmake -B build -DENI_PROVIDER_NEURALINK=OFF

# Cross-compile for ARM
cmake -B build-arm -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_SYSTEM_NAME=Linux
cmake --build build-arm
```
cmake -B build -DENI_BUILD_MIN=ON -DENI_BUILD_FRAMEWORK=OFF

# With EIPC integration


## Install

```bash
# Clone
git clone https://github.com/embeddedos-org/eni.git
cd eni

# Build (minimal + framework + neuralink)
cmake -B build -DENI_BUILD_TESTS=ON
cmake --build build

# Minimal only (MCU targets)
cmake -B build -DENI_BUILD_MIN=ON -DENI_BUILD_FRAMEWORK=OFF

# Without Neuralink
cmake -B build -DENI_PROVIDER_NEURALINK=OFF

# Cross-compile for ARM
cmake -B build-arm -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_SYSTEM_NAME=Linux
cmake --build build-arm
```
cmake -B build -DENI_EIPC_ENABLED=ON

# Without Neuralink (generic/simulator only)


## Install

```bash
# Clone
git clone https://github.com/embeddedos-org/eni.git
cd eni

# Build (minimal + framework + neuralink)
cmake -B build -DENI_BUILD_TESTS=ON
cmake --build build

# Minimal only (MCU targets)
cmake -B build -DENI_BUILD_MIN=ON -DENI_BUILD_FRAMEWORK=OFF

# Without Neuralink
cmake -B build -DENI_PROVIDER_NEURALINK=OFF

# Cross-compile for ARM
cmake -B build-arm -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_SYSTEM_NAME=Linux
cmake --build build-arm
```
cmake -B build -DENI_PROVIDER_NEURALINK=OFF
```

## Data Flow


## Install

```bash
# Clone
git clone https://github.com/embeddedos-org/eni.git
cd eni

# Build (minimal + framework + neuralink)
cmake -B build -DENI_BUILD_TESTS=ON
cmake --build build

# Minimal only (MCU targets)
cmake -B build -DENI_BUILD_MIN=ON -DENI_BUILD_FRAMEWORK=OFF

# Without Neuralink
cmake -B build -DENI_PROVIDER_NEURALINK=OFF

# Cross-compile for ARM
cmake -B build-arm -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_SYSTEM_NAME=Linux
cmake --build build-arm
```

```
Neuralink N1 → electrodes → raw spike data (30kHz × 1024ch)
    ↓
ENI Neuralink Adapter → bandpass filter → calibration → packet
    ↓
ENI Provider Framework → event generation → policy check
    ↓
ENI-Min (filter → normalize → map → tool bridge)
  or
ENI-Framework (connectors → pipeline → orchestrator)
    ↓
EIPC Bridge → EAI Agent → tool execution (mqtt.publish, device.read_sensor)
```

## License


## Install

```bash
# Clone
git clone https://github.com/embeddedos-org/eni.git
cd eni

# Build (minimal + framework + neuralink)
cmake -B build -DENI_BUILD_TESTS=ON
cmake --build build

# Minimal only (MCU targets)
cmake -B build -DENI_BUILD_MIN=ON -DENI_BUILD_FRAMEWORK=OFF

# Without Neuralink
cmake -B build -DENI_PROVIDER_NEURALINK=OFF

# Cross-compile for ARM
cmake -B build-arm -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_SYSTEM_NAME=Linux
cmake --build build-arm
```

MIT