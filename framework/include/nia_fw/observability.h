#ifndef NIA_FW_OBSERVABILITY_H
#define NIA_FW_OBSERVABILITY_H

#include "nia/common.h"

typedef struct {
    uint64_t events_ingested;
    uint64_t events_routed;
    uint64_t events_executed;
    uint64_t events_denied;
    uint64_t events_dropped;
    uint64_t total_latency_ms;
} nia_fw_metrics_t;

typedef struct {
    nia_timestamp_t timestamp;
    char            action[NIA_POLICY_ACTION_MAX];
    char            source[NIA_EVENT_SOURCE_MAX];
    nia_status_t    result;
} nia_fw_audit_entry_t;

#define NIA_FW_AUDIT_LOG_MAX 512

typedef struct {
    bool                  metrics_enabled;
    bool                  audit_enabled;
    bool                  trace_enabled;
    nia_fw_metrics_t      metrics;
    nia_fw_audit_entry_t  audit_log[NIA_FW_AUDIT_LOG_MAX];
    int                   audit_head;
    int                   audit_count;
} nia_fw_observability_t;

nia_status_t nia_fw_observability_init(nia_fw_observability_t *obs,
                                        bool metrics, bool audit, bool trace);
void         nia_fw_observability_record_event(nia_fw_observability_t *obs);
void         nia_fw_observability_record_exec(nia_fw_observability_t *obs, uint32_t latency_ms);
void         nia_fw_observability_record_denied(nia_fw_observability_t *obs);
void         nia_fw_observability_record_drop(nia_fw_observability_t *obs);
void         nia_fw_observability_audit(nia_fw_observability_t *obs,
                                         const char *action, const char *source,
                                         nia_status_t result);
void         nia_fw_observability_dump_metrics(const nia_fw_observability_t *obs);
void         nia_fw_observability_dump_audit(const nia_fw_observability_t *obs, int last_n);

#endif /* NIA_FW_OBSERVABILITY_H */
