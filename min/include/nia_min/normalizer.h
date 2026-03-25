#ifndef NIA_MIN_NORMALIZER_H
#define NIA_MIN_NORMALIZER_H

#include "nia/common.h"

typedef struct {
    nia_mode_t mode;
} nia_min_normalizer_t;

nia_status_t nia_min_normalizer_init(nia_min_normalizer_t *norm, nia_mode_t mode);
nia_status_t nia_min_normalizer_process(nia_min_normalizer_t *norm,
                                         const nia_event_t *raw,
                                         nia_event_t *normalized);

#endif /* NIA_MIN_NORMALIZER_H */
