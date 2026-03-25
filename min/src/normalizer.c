#include "nia_min/normalizer.h"
#include "nia/log.h"
#include <string.h>

nia_status_t nia_min_normalizer_init(nia_min_normalizer_t *norm, nia_mode_t mode)
{
    if (!norm) return NIA_ERR_INVALID;
    norm->mode = mode;
    return NIA_OK;
}

nia_status_t nia_min_normalizer_process(nia_min_normalizer_t *norm,
                                         const nia_event_t *raw,
                                         nia_event_t *normalized)
{
    if (!norm || !raw || !normalized) return NIA_ERR_INVALID;

    memcpy(normalized, raw, sizeof(*normalized));

    /* Clamp confidence to [0.0, 1.0] */
    if (normalized->type == NIA_EVENT_INTENT) {
        if (normalized->payload.intent.confidence < 0.0f)
            normalized->payload.intent.confidence = 0.0f;
        if (normalized->payload.intent.confidence > 1.0f)
            normalized->payload.intent.confidence = 1.0f;
    }

    /* Clamp feature values to [0.0, 1.0] */
    if (normalized->type == NIA_EVENT_FEATURES) {
        for (int i = 0; i < normalized->payload.features.count; i++) {
            float *v = &normalized->payload.features.features[i].value;
            if (*v < 0.0f) *v = 0.0f;
            if (*v > 1.0f) *v = 1.0f;
        }
    }

    /* Refresh timestamp */
    normalized->timestamp = nia_timestamp_now();

    return NIA_OK;
}
