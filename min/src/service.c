#include "nia_min/service.h"
#include "nia/log.h"
#include <string.h>
#include <stdio.h>

nia_status_t nia_min_service_init(nia_min_service_t *svc, const nia_config_t *cfg,
                                   const nia_provider_ops_t *provider_ops)
{
    if (!svc || !cfg || !provider_ops) return NIA_ERR_INVALID;

    memset(svc, 0, sizeof(*svc));
    memcpy(&svc->config, cfg, sizeof(*cfg));
    svc->state = NIA_MIN_STATE_INIT;

    nia_status_t st;

    const char *prov_name = cfg->provider_count > 0 && cfg->providers[0].name
                          ? cfg->providers[0].name : "default";
    nia_transport_t transport = cfg->provider_count > 0
                              ? cfg->providers[0].transport : NIA_TRANSPORT_UNIX_SOCKET;

    st = nia_min_input_init(&svc->input, provider_ops, prov_name, transport);
    if (st != NIA_OK) return st;

    st = nia_min_normalizer_init(&svc->normalizer, cfg->mode);
    if (st != NIA_OK) return st;

    st = nia_min_filter_init(&svc->filter, cfg->filter.min_confidence,
                              cfg->filter.debounce_ms);
    if (st != NIA_OK) return st;

    st = nia_min_mapper_init(&svc->mapper);
    if (st != NIA_OK) return st;

    st = nia_policy_init(&svc->policy);
    if (st != NIA_OK) return st;

    /* Load policy rules from config */
    for (int i = 0; i < cfg->policy.allow_count; i++) {
        nia_policy_add_rule(&svc->policy, cfg->policy.allow[i],
                            NIA_POLICY_ALLOW, NIA_ACTION_SAFE);
    }
    for (int i = 0; i < cfg->policy.deny_count; i++) {
        nia_policy_add_rule(&svc->policy, cfg->policy.deny[i],
                            NIA_POLICY_DENY, NIA_ACTION_RESTRICTED);
    }

    st = nia_min_tool_bridge_init(&svc->tool_bridge, &svc->policy);
    if (st != NIA_OK) return st;

    NIA_LOG_INFO("min.service", "initialized (provider=%s)", prov_name);
    return NIA_OK;
}

nia_status_t nia_min_service_start(nia_min_service_t *svc)
{
    if (!svc) return NIA_ERR_INVALID;

    nia_status_t st = nia_min_input_connect(&svc->input);
    if (st != NIA_OK) {
        svc->state = NIA_MIN_STATE_ERROR;
        return st;
    }

    svc->state = NIA_MIN_STATE_RUNNING;
    NIA_LOG_INFO("min.service", "started");
    return NIA_OK;
}

nia_status_t nia_min_service_tick(nia_min_service_t *svc)
{
    if (!svc) return NIA_ERR_INVALID;
    if (svc->state != NIA_MIN_STATE_RUNNING) return NIA_ERR_RUNTIME;

    nia_event_t raw_ev;
    nia_status_t st = nia_min_input_poll(&svc->input, &raw_ev);
    if (st == NIA_ERR_TIMEOUT) return NIA_OK; /* no event available */
    if (st != NIA_OK) return st;

    svc->events_processed++;

    /* Normalize */
    nia_event_t norm_ev;
    st = nia_min_normalizer_process(&svc->normalizer, &raw_ev, &norm_ev);
    if (st != NIA_OK) return st;

    /* Filter */
    if (!nia_min_filter_accept(&svc->filter, &norm_ev)) {
        svc->events_filtered++;
        return NIA_OK;
    }

    /* Map intent to tool call */
    nia_tool_call_t call;
    st = nia_min_mapper_resolve(&svc->mapper, &norm_ev, &call);
    if (st != NIA_OK) return st;

    /* Execute */
    nia_tool_result_t result;
    st = nia_min_tool_bridge_exec(&svc->tool_bridge, &call, &result);
    if (st == NIA_OK) {
        svc->events_executed++;
    }

    return st;
}

nia_status_t nia_min_service_stop(nia_min_service_t *svc)
{
    if (!svc) return NIA_ERR_INVALID;

    nia_min_input_disconnect(&svc->input);
    svc->state = NIA_MIN_STATE_STOPPED;
    NIA_LOG_INFO("min.service", "stopped");
    return NIA_OK;
}

void nia_min_service_shutdown(nia_min_service_t *svc)
{
    if (!svc) return;

    if (svc->state == NIA_MIN_STATE_RUNNING) {
        nia_min_service_stop(svc);
    }
    nia_min_input_shutdown(&svc->input);
    NIA_LOG_INFO("min.service", "shutdown complete");
}

void nia_min_service_stats(const nia_min_service_t *svc)
{
    if (!svc) return;

    const char *state_str;
    switch (svc->state) {
    case NIA_MIN_STATE_INIT:    state_str = "init";    break;
    case NIA_MIN_STATE_RUNNING: state_str = "running"; break;
    case NIA_MIN_STATE_STOPPED: state_str = "stopped"; break;
    case NIA_MIN_STATE_ERROR:   state_str = "error";   break;
    default:                    state_str = "unknown";  break;
    }

    printf("[nia-min] state=%s processed=%llu filtered=%llu executed=%llu\n",
           state_str,
           (unsigned long long)svc->events_processed,
           (unsigned long long)svc->events_filtered,
           (unsigned long long)svc->events_executed);
}
