#include "nia_fw/orchestrator.h"
#include "nia/log.h"
#include <string.h>
#include <stdio.h>

nia_status_t nia_fw_orchestrator_init(nia_fw_orchestrator_t *orch,
                                       nia_policy_engine_t *policy,
                                       nia_fw_router_t *router)
{
    if (!orch) return NIA_ERR_INVALID;
    memset(orch, 0, sizeof(*orch));
    orch->policy = policy;
    orch->router = router;
    return nia_tool_registry_init(&orch->registry);
}

nia_status_t nia_fw_orchestrator_register_tool(nia_fw_orchestrator_t *orch,
                                                const nia_tool_entry_t *entry)
{
    if (!orch || !entry) return NIA_ERR_INVALID;
    return nia_tool_register(&orch->registry, entry);
}

nia_status_t nia_fw_orchestrator_dispatch(nia_fw_orchestrator_t *orch,
                                           const nia_event_t *ev,
                                           const nia_tool_call_t *call,
                                           nia_tool_result_t *result)
{
    if (!orch || !ev || !call || !result) return NIA_ERR_INVALID;

    /* Route classification */
    nia_route_priority_t pri = NIA_ROUTE_NORMAL;
    if (orch->router) {
        pri = nia_fw_router_classify(orch->router, ev);
    }

    /* Policy check */
    if (orch->policy) {
        nia_policy_verdict_t v = nia_policy_evaluate(orch->policy, call->tool);
        if (v == NIA_POLICY_DENY) {
            NIA_LOG_WARN("fw.orch", "policy denied: %s (priority=%d)", call->tool, pri);
            orch->denied_count++;
            result->status = NIA_ERR_POLICY_DENIED;
            return NIA_ERR_POLICY_DENIED;
        }
        if (v == NIA_POLICY_CONFIRM) {
            NIA_LOG_INFO("fw.orch", "confirmation required: %s", call->tool);
        }
    }

    nia_status_t st = nia_tool_exec(&orch->registry, call, result);
    if (st == NIA_OK) {
        orch->exec_count++;
    }
    return st;
}

void nia_fw_orchestrator_stats(const nia_fw_orchestrator_t *orch)
{
    if (!orch) return;
    printf("[orchestrator] executed=%llu denied=%llu tools=%d\n",
           (unsigned long long)orch->exec_count,
           (unsigned long long)orch->denied_count,
           orch->registry.count);
}
