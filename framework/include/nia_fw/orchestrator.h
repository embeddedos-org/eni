#ifndef NIA_FW_ORCHESTRATOR_H
#define NIA_FW_ORCHESTRATOR_H

#include "nia/common.h"
#include "nia_fw/router.h"

typedef struct {
    nia_tool_registry_t   registry;
    nia_policy_engine_t  *policy;
    nia_fw_router_t      *router;
    uint64_t              exec_count;
    uint64_t              denied_count;
} nia_fw_orchestrator_t;

nia_status_t nia_fw_orchestrator_init(nia_fw_orchestrator_t *orch,
                                       nia_policy_engine_t *policy,
                                       nia_fw_router_t *router);
nia_status_t nia_fw_orchestrator_register_tool(nia_fw_orchestrator_t *orch,
                                                const nia_tool_entry_t *entry);
nia_status_t nia_fw_orchestrator_dispatch(nia_fw_orchestrator_t *orch,
                                           const nia_event_t *ev,
                                           const nia_tool_call_t *call,
                                           nia_tool_result_t *result);
void         nia_fw_orchestrator_stats(const nia_fw_orchestrator_t *orch);

#endif /* NIA_FW_ORCHESTRATOR_H */
