#include "nia/tool_call.h"
#include "nia/log.h"
#include <string.h>
#include <stdio.h>

nia_status_t nia_tool_registry_init(nia_tool_registry_t *reg)
{
    if (!reg) return NIA_ERR_INVALID;
    memset(reg, 0, sizeof(*reg));
    return NIA_OK;
}

nia_status_t nia_tool_register(nia_tool_registry_t *reg, const nia_tool_entry_t *entry)
{
    if (!reg || !entry || !entry->exec) return NIA_ERR_INVALID;
    if (reg->count >= NIA_TOOL_REGISTRY_MAX) return NIA_ERR_OVERFLOW;

    nia_tool_entry_t *dst = &reg->tools[reg->count];
    size_t len = strlen(entry->name);
    if (len >= NIA_TOOL_NAME_MAX) len = NIA_TOOL_NAME_MAX - 1;
    memcpy(dst->name, entry->name, len);
    dst->name[len] = '\0';
    dst->description = entry->description;
    dst->exec        = entry->exec;
    reg->count++;

    NIA_LOG_DEBUG("tool_reg", "registered tool: %s", dst->name);
    return NIA_OK;
}

nia_tool_entry_t *nia_tool_find(nia_tool_registry_t *reg, const char *name)
{
    if (!reg || !name) return NULL;
    for (int i = 0; i < reg->count; i++) {
        if (strcmp(reg->tools[i].name, name) == 0) {
            return &reg->tools[i];
        }
    }
    return NULL;
}

nia_status_t nia_tool_exec(nia_tool_registry_t *reg, const nia_tool_call_t *call,
                            nia_tool_result_t *result)
{
    if (!reg || !call || !result) return NIA_ERR_INVALID;

    nia_tool_entry_t *entry = nia_tool_find(reg, call->tool);
    if (!entry) {
        NIA_LOG_WARN("tool_reg", "tool not found: %s", call->tool);
        return NIA_ERR_NOT_FOUND;
    }

    memset(result, 0, sizeof(*result));
    nia_timestamp_t start = nia_timestamp_now();

    nia_status_t st = entry->exec(call, result);
    result->status = st;

    nia_timestamp_t end = nia_timestamp_now();
    uint64_t elapsed_ms = (end.sec - start.sec) * 1000 +
                          (end.nsec > start.nsec
                               ? (end.nsec - start.nsec) / 1000000
                               : 0);
    result->latency_ms = (uint32_t)elapsed_ms;

    NIA_LOG_DEBUG("tool_reg", "exec %s → %s (%u ms)",
                  call->tool, nia_status_str(st), result->latency_ms);
    return st;
}

void nia_tool_registry_list(const nia_tool_registry_t *reg)
{
    if (!reg) return;
    printf("[tool_registry] %d tools:\n", reg->count);
    for (int i = 0; i < reg->count; i++) {
        printf("  [%d] %s — %s\n", i, reg->tools[i].name,
               reg->tools[i].description ? reg->tools[i].description : "(no description)");
    }
}
