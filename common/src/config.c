#include "eni/config.h"
#include "eni/log.h"
#include <string.h>
#include <stdio.h>

eni_status_t eni_config_init(eni_config_t *cfg)
{
    if (!cfg) return ENI_ERR_INVALID;
    memset(cfg, 0, sizeof(*cfg));
    cfg->variant = ENI_VARIANT_MIN;
    cfg->mode    = ENI_MODE_INTENT;
    cfg->filter.min_confidence = 0.80f;
    cfg->filter.debounce_ms    = 100;
    return ENI_OK;
}

eni_status_t eni_config_load_defaults(eni_config_t *cfg, eni_variant_t variant)
{
    if (!cfg) return ENI_ERR_INVALID;

    eni_config_init(cfg);
    cfg->variant = variant;

    if (variant == ENI_VARIANT_MIN) {
        cfg->mode = ENI_MODE_INTENT;
        cfg->providers[0].name      = "simulator";
        cfg->providers[0].transport = ENI_TRANSPORT_UNIX_SOCKET;
        cfg->provider_count         = 1;
        cfg->filter.min_confidence  = 0.80f;
        cfg->filter.debounce_ms     = 100;
    } else {
        cfg->mode = ENI_MODE_FEATURES_INTENT;
        cfg->providers[0].name      = "generic-decoder";
        cfg->providers[0].transport = ENI_TRANSPORT_GRPC;
        cfg->provider_count         = 1;
        cfg->routing.max_latency_ms = 20;
        cfg->routing.local_only     = true;
        cfg->observability.metrics  = true;
        cfg->observability.audit    = true;
        cfg->observability.trace    = false;
    }

    return ENI_OK;
}

eni_status_t eni_config_load_file(eni_config_t *cfg, const char *path)
{
    if (!cfg || !path) return ENI_ERR_INVALID;
    /* TODO: YAML/JSON file parsing */
    ENI_LOG_WARN("config", "file loading not yet implemented: %s", path);
    return ENI_ERR_UNSUPPORTED;
}

void eni_config_dump(const eni_config_t *cfg)
{
    if (!cfg) return;

    const char *var_str = cfg->variant == ENI_VARIANT_MIN ? "min" : "framework";
    const char *mode_str;
    switch (cfg->mode) {
    case ENI_MODE_INTENT:          mode_str = "intent";          break;
    case ENI_MODE_FEATURES:        mode_str = "features";        break;
    case ENI_MODE_RAW:             mode_str = "raw";             break;
    case ENI_MODE_FEATURES_INTENT: mode_str = "features+intent"; break;
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
