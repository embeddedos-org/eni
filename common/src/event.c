#include "nia/event.h"
#include "nia/log.h"
#include <string.h>
#include <stdio.h>

static uint32_t s_event_id_counter = 0;

nia_status_t nia_event_init(nia_event_t *ev, nia_event_type_t type, const char *source)
{
    if (!ev || !source) return NIA_ERR_INVALID;

    memset(ev, 0, sizeof(*ev));
    ev->id      = ++s_event_id_counter;
    ev->version = 1;
    ev->type    = type;
    ev->timestamp = nia_timestamp_now();

    size_t slen = strlen(source);
    if (slen >= NIA_EVENT_SOURCE_MAX) slen = NIA_EVENT_SOURCE_MAX - 1;
    memcpy(ev->source, source, slen);
    ev->source[slen] = '\0';

    return NIA_OK;
}

nia_status_t nia_event_set_intent(nia_event_t *ev, const char *intent, float confidence)
{
    if (!ev || !intent) return NIA_ERR_INVALID;
    if (ev->type != NIA_EVENT_INTENT) return NIA_ERR_INVALID;

    size_t len = strlen(intent);
    if (len >= NIA_EVENT_INTENT_MAX) len = NIA_EVENT_INTENT_MAX - 1;
    memcpy(ev->payload.intent.name, intent, len);
    ev->payload.intent.name[len] = '\0';
    ev->payload.intent.confidence = confidence;

    return NIA_OK;
}

nia_status_t nia_event_add_feature(nia_event_t *ev, const char *name, float value)
{
    if (!ev || !name) return NIA_ERR_INVALID;
    if (ev->type != NIA_EVENT_FEATURES) return NIA_ERR_INVALID;

    nia_feature_set_t *fs = &ev->payload.features;
    if (fs->count >= NIA_EVENT_FEATURES_MAX) return NIA_ERR_OVERFLOW;

    nia_feature_t *f = &fs->features[fs->count];
    size_t len = strlen(name);
    if (len >= sizeof(f->name)) len = sizeof(f->name) - 1;
    memcpy(f->name, name, len);
    f->name[len] = '\0';
    f->value = value;
    fs->count++;

    return NIA_OK;
}

nia_status_t nia_event_set_raw(nia_event_t *ev, const uint8_t *data, size_t len)
{
    if (!ev || !data) return NIA_ERR_INVALID;
    if (ev->type != NIA_EVENT_RAW) return NIA_ERR_INVALID;
    if (len > NIA_EVENT_PAYLOAD_MAX) return NIA_ERR_OVERFLOW;

    memcpy(ev->payload.raw.data, data, len);
    ev->payload.raw.len = len;

    return NIA_OK;
}

void nia_event_print(const nia_event_t *ev)
{
    if (!ev) return;

    const char *type_str = "unknown";
    switch (ev->type) {
    case NIA_EVENT_INTENT:   type_str = "intent";   break;
    case NIA_EVENT_FEATURES: type_str = "features"; break;
    case NIA_EVENT_RAW:      type_str = "raw";      break;
    case NIA_EVENT_CONTROL:  type_str = "control";  break;
    }

    printf("[event] id=%u v=%u type=%s source=%s\n",
           ev->id, ev->version, type_str, ev->source);

    if (ev->type == NIA_EVENT_INTENT) {
        printf("  intent: %s (confidence=%.2f)\n",
               ev->payload.intent.name, ev->payload.intent.confidence);
    } else if (ev->type == NIA_EVENT_FEATURES) {
        printf("  features (%d):\n", ev->payload.features.count);
        for (int i = 0; i < ev->payload.features.count; i++) {
            printf("    %s = %.4f\n",
                   ev->payload.features.features[i].name,
                   ev->payload.features.features[i].value);
        }
    } else if (ev->type == NIA_EVENT_RAW) {
        printf("  raw: %zu bytes\n", ev->payload.raw.len);
    }
}
