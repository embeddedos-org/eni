#ifndef NIA_MIN_SERVICE_H
#define NIA_MIN_SERVICE_H

#include "nia/common.h"
#include "nia_min/input.h"
#include "nia_min/normalizer.h"
#include "nia_min/filter.h"
#include "nia_min/mapper.h"
#include "nia_min/tool_bridge.h"

typedef enum {
    NIA_MIN_STATE_INIT,
    NIA_MIN_STATE_RUNNING,
    NIA_MIN_STATE_STOPPED,
    NIA_MIN_STATE_ERROR,
} nia_min_state_t;

typedef struct {
    nia_config_t           config;
    nia_min_input_t        input;
    nia_min_normalizer_t   normalizer;
    nia_min_filter_t       filter;
    nia_min_mapper_t       mapper;
    nia_policy_engine_t    policy;
    nia_min_tool_bridge_t  tool_bridge;
    nia_min_state_t        state;
    uint64_t               events_processed;
    uint64_t               events_filtered;
    uint64_t               events_executed;
} nia_min_service_t;

nia_status_t nia_min_service_init(nia_min_service_t *svc, const nia_config_t *cfg,
                                   const nia_provider_ops_t *provider_ops);
nia_status_t nia_min_service_start(nia_min_service_t *svc);
nia_status_t nia_min_service_tick(nia_min_service_t *svc);
nia_status_t nia_min_service_stop(nia_min_service_t *svc);
void         nia_min_service_shutdown(nia_min_service_t *svc);
void         nia_min_service_stats(const nia_min_service_t *svc);

#endif /* NIA_MIN_SERVICE_H */
