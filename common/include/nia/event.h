#ifndef NIA_EVENT_H
#define NIA_EVENT_H

#include "nia/types.h"

#define NIA_EVENT_INTENT_MAX    64
#define NIA_EVENT_PAYLOAD_MAX   4096
#define NIA_EVENT_FEATURES_MAX  32
#define NIA_EVENT_SOURCE_MAX    64

typedef enum {
    NIA_EVENT_INTENT,
    NIA_EVENT_FEATURES,
    NIA_EVENT_RAW,
    NIA_EVENT_CONTROL,
} nia_event_type_t;

typedef struct {
    char  name[NIA_EVENT_INTENT_MAX];
    float confidence;
} nia_intent_t;

typedef struct {
    char  name[32];
    float value;
} nia_feature_t;

typedef struct {
    nia_feature_t features[NIA_EVENT_FEATURES_MAX];
    int           count;
} nia_feature_set_t;

typedef struct {
    uint8_t data[NIA_EVENT_PAYLOAD_MAX];
    size_t  len;
} nia_raw_payload_t;

typedef struct {
    uint32_t         id;
    uint32_t         version;
    nia_event_type_t type;
    nia_timestamp_t  timestamp;
    char             source[NIA_EVENT_SOURCE_MAX];

    union {
        nia_intent_t      intent;
        nia_feature_set_t features;
        nia_raw_payload_t raw;
    } payload;
} nia_event_t;

nia_status_t nia_event_init(nia_event_t *ev, nia_event_type_t type, const char *source);
nia_status_t nia_event_set_intent(nia_event_t *ev, const char *intent, float confidence);
nia_status_t nia_event_add_feature(nia_event_t *ev, const char *name, float value);
nia_status_t nia_event_set_raw(nia_event_t *ev, const uint8_t *data, size_t len);
void         nia_event_print(const nia_event_t *ev);

#endif /* NIA_EVENT_H */
