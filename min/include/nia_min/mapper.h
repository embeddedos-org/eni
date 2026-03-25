#ifndef NIA_MIN_MAPPER_H
#define NIA_MIN_MAPPER_H

#include "nia/common.h"

#define NIA_MAPPER_MAX_ENTRIES 64

typedef struct {
    char intent[NIA_EVENT_INTENT_MAX];
    char tool[NIA_TOOL_NAME_MAX];
} nia_mapper_entry_t;

typedef struct {
    nia_mapper_entry_t entries[NIA_MAPPER_MAX_ENTRIES];
    int                count;
} nia_min_mapper_t;

nia_status_t nia_min_mapper_init(nia_min_mapper_t *mapper);
nia_status_t nia_min_mapper_add(nia_min_mapper_t *mapper,
                                 const char *intent, const char *tool);
nia_status_t nia_min_mapper_resolve(const nia_min_mapper_t *mapper,
                                     const nia_event_t *ev,
                                     nia_tool_call_t *call);
void         nia_min_mapper_dump(const nia_min_mapper_t *mapper);

#endif /* NIA_MIN_MAPPER_H */
