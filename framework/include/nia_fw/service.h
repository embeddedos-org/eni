#ifndef NIA_FW_SERVICE_H
#define NIA_FW_SERVICE_H

#include "nia/common.h"
#include "nia_fw/provider_manager.h"
#include "nia_fw/stream_bus.h"
#include "nia_fw/router.h"
#include "nia_fw/orchestrator.h"
#include "nia_fw/connectors.h"
#include "nia_fw/observability.h"

typedef enum {
    NIA_FW_STATE_INIT,
    NIA_FW_STATE_RUNNING,
    NIA_FW_STATE_STOPPED,
    NIA_FW_STATE_ERROR,
} nia_fw_state_t;

typedef struct {
    nia_config_t                config;
    nia_fw_provider_manager_t   providers;
    nia_fw_stream_bus_t         stream_bus;
    nia_fw_router_t             router;
    nia_policy_engine_t         policy;
    nia_fw_orchestrator_t       orchestrator;
    nia_fw_connector_manager_t  connectors;
    nia_fw_observability_t      observability;
    nia_fw_state_t              state;
} nia_fw_service_t;

nia_status_t nia_fw_service_init(nia_fw_service_t *svc, const nia_config_t *cfg);
nia_status_t nia_fw_service_add_provider(nia_fw_service_t *svc,
                                          const nia_provider_ops_t *ops,
                                          const char *name);
nia_status_t nia_fw_service_start(nia_fw_service_t *svc);
nia_status_t nia_fw_service_tick(nia_fw_service_t *svc);
nia_status_t nia_fw_service_stop(nia_fw_service_t *svc);
void         nia_fw_service_shutdown(nia_fw_service_t *svc);
void         nia_fw_service_stats(const nia_fw_service_t *svc);

#endif /* NIA_FW_SERVICE_H */
