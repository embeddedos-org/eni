#ifndef NIA_MIN_TOOL_BRIDGE_H
#define NIA_MIN_TOOL_BRIDGE_H

#include "nia/common.h"

typedef struct {
    nia_tool_registry_t  registry;
    nia_policy_engine_t *policy;
} nia_min_tool_bridge_t;

nia_status_t nia_min_tool_bridge_init(nia_min_tool_bridge_t *bridge,
                                       nia_policy_engine_t *policy);
nia_status_t nia_min_tool_bridge_register(nia_min_tool_bridge_t *bridge,
                                            const nia_tool_entry_t *entry);
nia_status_t nia_min_tool_bridge_exec(nia_min_tool_bridge_t *bridge,
                                        const nia_tool_call_t *call,
                                        nia_tool_result_t *result);

#endif /* NIA_MIN_TOOL_BRIDGE_H */
