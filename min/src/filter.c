#include "nia_min/filter.h"
#include "nia/log.h"
#include <string.h>

nia_status_t nia_min_filter_init(nia_min_filter_t *filter,
                                  float min_confidence, uint32_t debounce_ms)
{
    if (!filter) return NIA_ERR_INVALID;
    memset(filter, 0, sizeof(*filter));
    filter->min_confidence = min_confidence;
    filter->debounce_ms    = debounce_ms;
    return NIA_OK;
}

bool nia_min_filter_accept(nia_min_filter_t *filter, const nia_event_t *ev)
{
    if (!filter || !ev) return false;

    /* Only filter intent events — pass everything else through */
    if (ev->type != NIA_EVENT_INTENT) return true;

    /* Confidence threshold */
    if (ev->payload.intent.confidence < filter->min_confidence) {
        NIA_LOG_TRACE("min.filter", "rejected %s (confidence=%.2f < %.2f)",
                      ev->payload.intent.name,
                      ev->payload.intent.confidence,
                      filter->min_confidence);
        return false;
    }

    /* Debounce: reject duplicate intents within debounce window */
    if (filter->debounce_ms > 0 &&
        strcmp(ev->payload.intent.name, filter->last_intent) == 0) {
        uint64_t last_ms = filter->last_event_time.sec * 1000 +
                           filter->last_event_time.nsec / 1000000;
        uint64_t now_ms  = ev->timestamp.sec * 1000 +
                           ev->timestamp.nsec / 1000000;
        if (now_ms - last_ms < filter->debounce_ms) {
            NIA_LOG_TRACE("min.filter", "debounced %s", ev->payload.intent.name);
            return false;
        }
    }

    /* Accept — update state */
    filter->last_event_time = ev->timestamp;
    size_t len = strlen(ev->payload.intent.name);
    if (len >= NIA_EVENT_INTENT_MAX) len = NIA_EVENT_INTENT_MAX - 1;
    memcpy(filter->last_intent, ev->payload.intent.name, len);
    filter->last_intent[len] = '\0';

    return true;
}
