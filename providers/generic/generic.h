#ifndef NIA_PROVIDER_GENERIC_H
#define NIA_PROVIDER_GENERIC_H

#include "nia/provider_contract.h"

extern const nia_provider_ops_t nia_provider_generic_ops;

typedef struct {
    nia_transport_t transport;
    const char     *endpoint;
    uint32_t        timeout_ms;
} nia_generic_config_t;

#endif /* NIA_PROVIDER_GENERIC_H */
