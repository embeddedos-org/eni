#include "nia_min/tool_bridge.h"
#include "nia/log.h"
#include <string.h>

nia_status_t nia_min_tool_bridge_init(nia_min_tool_bridge_t *bridge,
                                       nia_policy_engine_t *policy)
{
    if (!bridge) return NIA_ERR_INVALID;
    memset(bridge, 0, sizeof(*bridge));
    bridge->policy = policy;
    return nia_tool_registry_init(&bridge->registry);
}

nia_status_t nia_min_tool_bridge_register(nia_min_tool_bridge_t *bridge,
                                            const nia_tool_entry_t *entry)
{
    if (!bridge || !entry) return NIA_ERR_INVALID;
    return nia_tool_register(&bridge->registry, entry);
}

nia_status_t nia_min_tool_bridge_exec(nia_min_tool_bridge_t *bridge,
                                        const nia_tool_call_t *call,
                                        nia_tool_result_t *result)
{
    if (!bridge || !call || !result) return NIA_ERR_INVALID;

    /* Policy check */
    if (bridge->policy) {
        nia_policy_verdict_t v = nia_policy_evaluate(bridge->policy, call->tool);
        if (v == NIA_POLICY_DENY) {
            NIA_LOG_WARN("min.bridge", "policy denied: %s", call->tool);
            result->status = NIA_ERR_POLICY_DENIED;
            return NIA_ERR_POLICY_DENIED;
        }
        if (v == NIA_POLICY_CONFIRM) {
            NIA_LOG_INFO("min.bridge", "confirmation required for: %s (auto-skipping in min mode)",
                         call->tool);
        }
    }

    return nia_tool_exec(&bridge->registry, call, result);
}
