#include "nia_min/mapper.h"
#include "nia/log.h"
#include <string.h>
#include <stdio.h>

nia_status_t nia_min_mapper_init(nia_min_mapper_t *mapper)
{
    if (!mapper) return NIA_ERR_INVALID;
    memset(mapper, 0, sizeof(*mapper));
    return NIA_OK;
}

nia_status_t nia_min_mapper_add(nia_min_mapper_t *mapper,
                                 const char *intent, const char *tool)
{
    if (!mapper || !intent || !tool) return NIA_ERR_INVALID;
    if (mapper->count >= NIA_MAPPER_MAX_ENTRIES) return NIA_ERR_OVERFLOW;

    nia_mapper_entry_t *e = &mapper->entries[mapper->count];

    size_t ilen = strlen(intent);
    if (ilen >= NIA_EVENT_INTENT_MAX) ilen = NIA_EVENT_INTENT_MAX - 1;
    memcpy(e->intent, intent, ilen);
    e->intent[ilen] = '\0';

    size_t tlen = strlen(tool);
    if (tlen >= NIA_TOOL_NAME_MAX) tlen = NIA_TOOL_NAME_MAX - 1;
    memcpy(e->tool, tool, tlen);
    e->tool[tlen] = '\0';

    mapper->count++;
    NIA_LOG_DEBUG("min.mapper", "mapped %s → %s", intent, tool);
    return NIA_OK;
}

nia_status_t nia_min_mapper_resolve(const nia_min_mapper_t *mapper,
                                     const nia_event_t *ev,
                                     nia_tool_call_t *call)
{
    if (!mapper || !ev || !call) return NIA_ERR_INVALID;
    if (ev->type != NIA_EVENT_INTENT) return NIA_ERR_INVALID;

    memset(call, 0, sizeof(*call));

    for (int i = 0; i < mapper->count; i++) {
        if (strcmp(mapper->entries[i].intent, ev->payload.intent.name) == 0) {
            size_t len = strlen(mapper->entries[i].tool);
            if (len >= NIA_TOOL_NAME_MAX) len = NIA_TOOL_NAME_MAX - 1;
            memcpy(call->tool, mapper->entries[i].tool, len);
            call->tool[len] = '\0';
            call->arg_count = 0;
            return NIA_OK;
        }
    }

    NIA_LOG_WARN("min.mapper", "no mapping for intent: %s", ev->payload.intent.name);
    return NIA_ERR_NOT_FOUND;
}

void nia_min_mapper_dump(const nia_min_mapper_t *mapper)
{
    if (!mapper) return;
    printf("[mapper] %d entries:\n", mapper->count);
    for (int i = 0; i < mapper->count; i++) {
        printf("  %s → %s\n", mapper->entries[i].intent, mapper->entries[i].tool);
    }
}
