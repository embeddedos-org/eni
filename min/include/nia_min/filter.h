#ifndef NIA_MIN_FILTER_H
#define NIA_MIN_FILTER_H

#include "nia/common.h"

typedef struct {
    float       min_confidence;
    uint32_t    debounce_ms;
    nia_timestamp_t last_event_time;
    char        last_intent[NIA_EVENT_INTENT_MAX];
} nia_min_filter_t;

nia_status_t nia_min_filter_init(nia_min_filter_t *filter,
                                  float min_confidence, uint32_t debounce_ms);
bool         nia_min_filter_accept(nia_min_filter_t *filter, const nia_event_t *ev);

#endif /* NIA_MIN_FILTER_H */
