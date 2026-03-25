#ifndef NIA_PROVIDER_CONTRACT_H
#define NIA_PROVIDER_CONTRACT_H

#include "nia/types.h"
#include "nia/event.h"

#define NIA_PROVIDER_NAME_MAX 64

typedef struct nia_provider_s nia_provider_t;

typedef nia_status_t (*nia_provider_poll_fn)(nia_provider_t *prov, nia_event_t *ev);

typedef struct {
    const char          *name;
    nia_status_t       (*init)(nia_provider_t *prov, const void *config);
    nia_provider_poll_fn poll;
    nia_status_t       (*start)(nia_provider_t *prov);
    nia_status_t       (*stop)(nia_provider_t *prov);
    void               (*shutdown)(nia_provider_t *prov);
} nia_provider_ops_t;

struct nia_provider_s {
    char                     name[NIA_PROVIDER_NAME_MAX];
    const nia_provider_ops_t *ops;
    void                     *ctx;
    bool                      running;
    nia_transport_t           transport;
};

nia_status_t nia_provider_init(nia_provider_t *prov, const nia_provider_ops_t *ops,
                               const char *name, const void *config);
nia_status_t nia_provider_start(nia_provider_t *prov);
nia_status_t nia_provider_poll(nia_provider_t *prov, nia_event_t *ev);
nia_status_t nia_provider_stop(nia_provider_t *prov);
void         nia_provider_shutdown(nia_provider_t *prov);

#endif /* NIA_PROVIDER_CONTRACT_H */
