#ifndef NIA_PROVIDER_SIMULATOR_H
#define NIA_PROVIDER_SIMULATOR_H

#include "nia/provider_contract.h"

extern const nia_provider_ops_t nia_provider_simulator_ops;

typedef struct {
    uint32_t event_interval_ms;
    uint32_t tick_count;
} nia_simulator_config_t;

#endif /* NIA_PROVIDER_SIMULATOR_H */
