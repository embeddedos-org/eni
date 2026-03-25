#ifndef NIA_FW_ROUTER_H
#define NIA_FW_ROUTER_H

#include "nia/common.h"

typedef enum {
    NIA_ROUTE_CRITICAL,
    NIA_ROUTE_NORMAL,
    NIA_ROUTE_LOW,
} nia_route_priority_t;

typedef struct {
    nia_event_type_t     type_filter;
    nia_route_priority_t priority;
    uint32_t             max_latency_ms;
    bool                 local_only;
} nia_route_rule_t;

#define NIA_FW_MAX_ROUTES 32

typedef struct {
    nia_route_rule_t rules[NIA_FW_MAX_ROUTES];
    int              count;
} nia_fw_router_t;

nia_status_t         nia_fw_router_init(nia_fw_router_t *router);
nia_status_t         nia_fw_router_add_rule(nia_fw_router_t *router,
                                             nia_event_type_t type,
                                             nia_route_priority_t priority,
                                             uint32_t max_latency_ms,
                                             bool local_only);
nia_route_priority_t nia_fw_router_classify(const nia_fw_router_t *router,
                                             const nia_event_t *ev);

#endif /* NIA_FW_ROUTER_H */
