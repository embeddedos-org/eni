#include "nia/config.h"
#include "nia/log.h"
#include <string.h>
#include <stdio.h>

nia_status_t nia_config_init(nia_config_t *cfg)
{
    if (!cfg) return NIA_ERR_INVALID;
    memset(cfg, 0, sizeof(*cfg));
    cfg->variant = NIA_VARIANT_MIN;
    cfg->mode    = NIA_MODE_INTENT;
    cfg->filter.min_confidence = 0.80f;
    cfg->filter.debounce_ms    = 100;
    return NIA_OK;
}

nia_status_t nia_config_load_defaults(nia_config_t *cfg, nia_variant_t variant)
{
    if (!cfg) return NIA_ERR_INVALID;

    nia_config_init(cfg);
    cfg->variant = variant;

    if (variant == NIA_VARIANT_MIN) {
        cfg->mode = NIA_MODE_INTENT;
        cfg->providers[0].name      = "simulator";
        cfg->providers[0].transport = NIA_TRANSPORT_UNIX_SOCKET;
        cfg->provider_count         = 1;
        cfg->filter.min_confidence  = 0.80f;
        cfg->filter.debounce_ms     = 100;
    } else {
        cfg->mode = NIA_MODE_FEATURES_INTENT;
        cfg->providers[0].name      = "generic-decoder";
        cfg->providers[0].transport = NIA_TRANSPORT_GRPC;
        cfg->provider_count         = 1;
        cfg->routing.max_latency_ms = 20;
        cfg->routing.local_only     = true;
        cfg->observability.metrics  = true;
        cfg->observability.audit    = true;
        cfg->observability.trace    = false;
    }

    return NIA_OK;
}

nia_status_t nia_config_load_file(nia_config_t *cfg, const char *path)
{
    if (!cfg || !path) return NIA_ERR_INVALID;
    /* TODO: YAML/JSON file parsing */
    NIA_LOG_WARN("config", "file loading not yet implemented: %s", path);
    return NIA_ERR_UNSUPPORTED;
}

void nia_config_dump(const nia_config_t *cfg)
{
    if (!cfg) return;

    const char *var_str = cfg->variant == NIA_VARIANT_MIN ? "min" : "framework";
    const char *mode_str;
    switch (cfg->mode) {
    case NIA_MODE_INTENT:          mode_str = "intent";          break;
    case NIA_MODE_FEATURES:        mode_str = "features";        break;
    case NIA_MODE_RAW:             mode_str = "raw";             break;
    case NIA_MODE_FEATURES_INTENT: mode_str = "features+intent"; break;
    default:                       mode_str = "unknown";         break;
    }

    printf("[config] variant=%s mode=%s providers=%d\n",
           var_str, mode_str, cfg->provider_count);

    for (int i = 0; i < cfg->provider_count; i++) {
        printf("  provider[%d]: %s\n", i,
               cfg->providers[i].name ? cfg->providers[i].name : "(null)");
    }

    printf("  filter: confidence=%.2f debounce=%u ms\n",
           cfg->filter.min_confidence, cfg->filter.debounce_ms);
    printf("  observability: metrics=%d audit=%d trace=%d\n",
           cfg->observability.metrics, cfg->observability.audit,
           cfg->observability.trace);
}
