#ifndef NIA_MIN_INPUT_H
#define NIA_MIN_INPUT_H

#include "nia/common.h"

typedef struct {
    nia_provider_t   provider;
    nia_transport_t  transport;
    bool             connected;
} nia_min_input_t;

nia_status_t nia_min_input_init(nia_min_input_t *input, const nia_provider_ops_t *ops,
                                 const char *provider_name, nia_transport_t transport);
nia_status_t nia_min_input_connect(nia_min_input_t *input);
nia_status_t nia_min_input_poll(nia_min_input_t *input, nia_event_t *ev);
nia_status_t nia_min_input_disconnect(nia_min_input_t *input);
void         nia_min_input_shutdown(nia_min_input_t *input);

#endif /* NIA_MIN_INPUT_H */
