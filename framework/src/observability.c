#include "nia_fw/observability.h"
#include "nia/log.h"
#include <string.h>
#include <stdio.h>

nia_status_t nia_fw_observability_init(nia_fw_observability_t *obs,
                                        bool metrics, bool audit, bool trace)
{
    if (!obs) return NIA_ERR_INVALID;
    memset(obs, 0, sizeof(*obs));
    obs->metrics_enabled = metrics;
    obs->audit_enabled   = audit;
    obs->trace_enabled   = trace;
    return NIA_OK;
}

void nia_fw_observability_record_event(nia_fw_observability_t *obs)
{
    if (!obs || !obs->metrics_enabled) return;
    obs->metrics.events_ingested++;
}

void nia_fw_observability_record_exec(nia_fw_observability_t *obs, uint32_t latency_ms)
{
    if (!obs || !obs->metrics_enabled) return;
    obs->metrics.events_executed++;
    obs->metrics.total_latency_ms += latency_ms;
}

void nia_fw_observability_record_denied(nia_fw_observability_t *obs)
{
    if (!obs || !obs->metrics_enabled) return;
    obs->metrics.events_denied++;
}

void nia_fw_observability_record_drop(nia_fw_observability_t *obs)
{
    if (!obs || !obs->metrics_enabled) return;
    obs->metrics.events_dropped++;
}

void nia_fw_observability_audit(nia_fw_observability_t *obs,
                                 const char *action, const char *source,
                                 nia_status_t result)
{
    if (!obs || !obs->audit_enabled) return;

    int idx = (obs->audit_head + obs->audit_count) % NIA_FW_AUDIT_LOG_MAX;
    if (obs->audit_count >= NIA_FW_AUDIT_LOG_MAX) {
        obs->audit_head = (obs->audit_head + 1) % NIA_FW_AUDIT_LOG_MAX;
    } else {
        obs->audit_count++;
    }

    nia_fw_audit_entry_t *entry = &obs->audit_log[idx];
    entry->timestamp = nia_timestamp_now();
    entry->result    = result;

    if (action) {
        size_t len = strlen(action);
        if (len >= NIA_POLICY_ACTION_MAX) len = NIA_POLICY_ACTION_MAX - 1;
        memcpy(entry->action, action, len);
        entry->action[len] = '\0';
    } else {
        entry->action[0] = '\0';
    }

    if (source) {
        size_t len = strlen(source);
        if (len >= NIA_EVENT_SOURCE_MAX) len = NIA_EVENT_SOURCE_MAX - 1;
        memcpy(entry->source, source, len);
        entry->source[len] = '\0';
    } else {
        entry->source[0] = '\0';
    }
}

void nia_fw_observability_dump_metrics(const nia_fw_observability_t *obs)
{
    if (!obs) return;

    const nia_fw_metrics_t *m = &obs->metrics;
    printf("[metrics] ingested=%llu routed=%llu executed=%llu denied=%llu dropped=%llu avg_latency=%llu ms\n",
           (unsigned long long)m->events_ingested,
           (unsigned long long)m->events_routed,
           (unsigned long long)m->events_executed,
           (unsigned long long)m->events_denied,
           (unsigned long long)m->events_dropped,
           m->events_executed > 0
               ? (unsigned long long)(m->total_latency_ms / m->events_executed)
               : 0ULL);
}

void nia_fw_observability_dump_audit(const nia_fw_observability_t *obs, int last_n)
{
    if (!obs || obs->audit_count == 0) return;

    int start = 0;
    int count = obs->audit_count;
    if (last_n > 0 && last_n < count) {
        start = count - last_n;
        count = last_n;
    }

    printf("[audit] last %d entries:\n", count);
    for (int i = 0; i < count; i++) {
        int idx = (obs->audit_head + start + i) % NIA_FW_AUDIT_LOG_MAX;
        const nia_fw_audit_entry_t *e = &obs->audit_log[idx];
        printf("  [%llu.%u] %s from %s → %s\n",
               (unsigned long long)e->timestamp.sec,
               e->timestamp.nsec,
               e->action, e->source,
               nia_status_str(e->result));
    }
}
