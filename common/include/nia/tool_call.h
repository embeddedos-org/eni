#ifndef NIA_TOOL_CALL_H
#define NIA_TOOL_CALL_H

#include "nia/types.h"

#define NIA_TOOL_NAME_MAX   64
#define NIA_TOOL_MAX_ARGS   16
#define NIA_TOOL_RESULT_MAX 4096

typedef struct {
    char     tool[NIA_TOOL_NAME_MAX];
    nia_kv_t args[NIA_TOOL_MAX_ARGS];
    int      arg_count;
} nia_tool_call_t;

typedef struct {
    char         data[NIA_TOOL_RESULT_MAX];
    size_t       len;
    nia_status_t status;
    uint32_t     latency_ms;
} nia_tool_result_t;

typedef nia_status_t (*nia_tool_exec_fn)(
    const nia_tool_call_t *call,
    nia_tool_result_t     *result
);

typedef struct {
    char            name[NIA_TOOL_NAME_MAX];
    const char     *description;
    nia_tool_exec_fn exec;
} nia_tool_entry_t;

#define NIA_TOOL_REGISTRY_MAX 64

typedef struct {
    nia_tool_entry_t tools[NIA_TOOL_REGISTRY_MAX];
    int              count;
} nia_tool_registry_t;

nia_status_t     nia_tool_registry_init(nia_tool_registry_t *reg);
nia_status_t     nia_tool_register(nia_tool_registry_t *reg, const nia_tool_entry_t *entry);
nia_tool_entry_t *nia_tool_find(nia_tool_registry_t *reg, const char *name);
nia_status_t     nia_tool_exec(nia_tool_registry_t *reg, const nia_tool_call_t *call,
                               nia_tool_result_t *result);
void             nia_tool_registry_list(const nia_tool_registry_t *reg);

#endif /* NIA_TOOL_CALL_H */
