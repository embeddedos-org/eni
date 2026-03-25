#ifndef NIA_CONFIG_H
#define NIA_CONFIG_H

#include "nia/types.h"

#define NIA_CONFIG_MAX_PROVIDERS 8
#define NIA_CONFIG_MAX_ALLOW     32
#define NIA_CONFIG_MAX_DENY      32

typedef struct {
    float    min_confidence;
    uint32_t debounce_ms;
} nia_filter_config_t;

typedef struct {
    const char *allow[NIA_CONFIG_MAX_ALLOW];
    int         allow_count;
    const char *deny[NIA_CONFIG_MAX_DENY];
    int         deny_count;
    bool        require_confirmation;
} nia_policy_config_t;

typedef struct {
    bool metrics;
    bool audit;
    bool trace;
} nia_observability_config_t;

typedef struct {
    const char     *name;
    nia_transport_t transport;
} nia_provider_config_t;

typedef struct {
    uint32_t max_latency_ms;
    bool     local_only;
} nia_routing_class_config_t;

typedef struct {
    nia_variant_t variant;
    nia_mode_t    mode;

    /* Provider(s) */
    nia_provider_config_t providers[NIA_CONFIG_MAX_PROVIDERS];
    int                   provider_count;

    /* Filters (NIA-Min) */
    nia_filter_config_t filter;

    /* Policy */
    nia_policy_config_t policy;

    /* Routing (NIA-Framework) */
    nia_routing_class_config_t routing;

    /* Observability (NIA-Framework) */
    nia_observability_config_t observability;
} nia_config_t;

nia_status_t nia_config_init(nia_config_t *cfg);
nia_status_t nia_config_load_file(nia_config_t *cfg, const char *path);
nia_status_t nia_config_load_defaults(nia_config_t *cfg, nia_variant_t variant);
void         nia_config_dump(const nia_config_t *cfg);

#endif /* NIA_CONFIG_H */
