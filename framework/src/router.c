#include "nia_fw/router.h"
#include "nia/log.h"
#include <string.h>

nia_status_t nia_fw_router_init(nia_fw_router_t *router)
{
    if (!router) return NIA_ERR_INVALID;
    memset(router, 0, sizeof(*router));
    return NIA_OK;
}

nia_status_t nia_fw_router_add_rule(nia_fw_router_t *router,
                                     nia_event_type_t type,
                                     nia_route_priority_t priority,
                                     uint32_t max_latency_ms,
                                     bool local_only)
{
    if (!router) return NIA_ERR_INVALID;
    if (router->count >= NIA_FW_MAX_ROUTES) return NIA_ERR_OVERFLOW;

    nia_route_rule_t *rule = &router->rules[router->count];
    rule->type_filter    = type;
    rule->priority       = priority;
    rule->max_latency_ms = max_latency_ms;
    rule->local_only     = local_only;
    router->count++;

    return NIA_OK;
}

nia_route_priority_t nia_fw_router_classify(const nia_fw_router_t *router,
                                             const nia_event_t *ev)
{
    if (!router || !ev) return NIA_ROUTE_NORMAL;

    for (int i = 0; i < router->count; i++) {
        if (router->rules[i].type_filter == ev->type) {
            return router->rules[i].priority;
        }
    }

    return NIA_ROUTE_NORMAL;
}
