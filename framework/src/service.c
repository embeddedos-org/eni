#include "nia_fw/service.h"
#include "nia/log.h"
#include <string.h>
#include <stdio.h>

nia_status_t nia_fw_service_init(nia_fw_service_t *svc, const nia_config_t *cfg)
{
    if (!svc || !cfg) return NIA_ERR_INVALID;

    memset(svc, 0, sizeof(*svc));
    memcpy(&svc->config, cfg, sizeof(*cfg));
    svc->state = NIA_FW_STATE_INIT;

    nia_status_t st;

    st = nia_fw_provider_manager_init(&svc->providers);
    if (st != NIA_OK) return st;

    st = nia_fw_stream_bus_init(&svc->stream_bus);
    if (st != NIA_OK) return st;

    st = nia_fw_router_init(&svc->router);
    if (st != NIA_OK) return st;

    st = nia_policy_init(&svc->policy);
    if (st != NIA_OK) return st;

    /* Load policy from config */
    for (int i = 0; i < cfg->policy.allow_count; i++) {
        nia_policy_add_rule(&svc->policy, cfg->policy.allow[i],
                            NIA_POLICY_ALLOW, NIA_ACTION_SAFE);
    }
    for (int i = 0; i < cfg->policy.deny_count; i++) {
        nia_policy_add_rule(&svc->policy, cfg->policy.deny[i],
                            NIA_POLICY_DENY, NIA_ACTION_RESTRICTED);
    }

    st = nia_fw_orchestrator_init(&svc->orchestrator, &svc->policy, &svc->router);
    if (st != NIA_OK) return st;

    st = nia_fw_connector_manager_init(&svc->connectors);
    if (st != NIA_OK) return st;

    st = nia_fw_observability_init(&svc->observability,
                                    cfg->observability.metrics,
                                    cfg->observability.audit,
                                    cfg->observability.trace);
    if (st != NIA_OK) return st;

    /* Default routing: control events are critical */
    nia_fw_router_add_rule(&svc->router, NIA_EVENT_CONTROL,
                           NIA_ROUTE_CRITICAL, cfg->routing.max_latency_ms, true);
    nia_fw_router_add_rule(&svc->router, NIA_EVENT_INTENT,
                           NIA_ROUTE_NORMAL, 50, false);
    nia_fw_router_add_rule(&svc->router, NIA_EVENT_FEATURES,
                           NIA_ROUTE_LOW, 100, false);

    NIA_LOG_INFO("fw.service", "initialized");
    return NIA_OK;
}

nia_status_t nia_fw_service_add_provider(nia_fw_service_t *svc,
                                          const nia_provider_ops_t *ops,
                                          const char *name)
{
    if (!svc || !ops || !name) return NIA_ERR_INVALID;
    return nia_fw_provider_manager_add(&svc->providers, ops, name, NULL);
}

nia_status_t nia_fw_service_start(nia_fw_service_t *svc)
{
    if (!svc) return NIA_ERR_INVALID;

    nia_status_t st = nia_fw_provider_manager_start_all(&svc->providers);
    if (st != NIA_OK) {
        svc->state = NIA_FW_STATE_ERROR;
        return st;
    }

    svc->state = NIA_FW_STATE_RUNNING;
    NIA_LOG_INFO("fw.service", "started");
    return NIA_OK;
}

nia_status_t nia_fw_service_tick(nia_fw_service_t *svc)
{
    if (!svc) return NIA_ERR_INVALID;
    if (svc->state != NIA_FW_STATE_RUNNING) return NIA_ERR_RUNTIME;

    /* Ingest from providers into stream bus */
    nia_event_t ev;
    int prov_idx = -1;
    nia_status_t st = nia_fw_provider_manager_poll_any(&svc->providers, &ev, &prov_idx);
    if (st == NIA_OK) {
        nia_fw_observability_record_event(&svc->observability);
        st = nia_fw_stream_bus_push(&svc->stream_bus, &ev);
        if (st != NIA_OK) {
            nia_fw_observability_record_drop(&svc->observability);
        }
    }

    /* Process one event from the bus */
    if (!nia_fw_stream_bus_empty(&svc->stream_bus)) {
        nia_event_t bus_ev;
        st = nia_fw_stream_bus_pop(&svc->stream_bus, &bus_ev);
        if (st == NIA_OK) {
            /* Route and classify */
            nia_route_priority_t pri = nia_fw_router_classify(&svc->router, &bus_ev);
            (void)pri; /* used for future priority queuing */

            /* Broadcast to connectors */
            nia_fw_connector_broadcast(&svc->connectors, &bus_ev);

            /* Audit */
            if (bus_ev.type == NIA_EVENT_INTENT) {
                nia_fw_observability_audit(&svc->observability,
                                           bus_ev.payload.intent.name,
                                           bus_ev.source, NIA_OK);
            }
        }
    }

    return NIA_OK;
}

nia_status_t nia_fw_service_stop(nia_fw_service_t *svc)
{
    if (!svc) return NIA_ERR_INVALID;

    nia_fw_provider_manager_stop_all(&svc->providers);
    svc->state = NIA_FW_STATE_STOPPED;
    NIA_LOG_INFO("fw.service", "stopped");
    return NIA_OK;
}

void nia_fw_service_shutdown(nia_fw_service_t *svc)
{
    if (!svc) return;

    if (svc->state == NIA_FW_STATE_RUNNING) {
        nia_fw_service_stop(svc);
    }
    nia_fw_provider_manager_shutdown(&svc->providers);
    nia_fw_connector_manager_shutdown(&svc->connectors);
    NIA_LOG_INFO("fw.service", "shutdown complete");
}

void nia_fw_service_stats(const nia_fw_service_t *svc)
{
    if (!svc) return;

    const char *state_str;
    switch (svc->state) {
    case NIA_FW_STATE_INIT:    state_str = "init";    break;
    case NIA_FW_STATE_RUNNING: state_str = "running"; break;
    case NIA_FW_STATE_STOPPED: state_str = "stopped"; break;
    case NIA_FW_STATE_ERROR:   state_str = "error";   break;
    default:                   state_str = "unknown";  break;
    }

    printf("[nia-framework] state=%s\n", state_str);
    nia_fw_stream_bus_stats(&svc->stream_bus);
    nia_fw_orchestrator_stats(&svc->orchestrator);
    nia_fw_observability_dump_metrics(&svc->observability);
}
