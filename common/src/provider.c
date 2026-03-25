#include "nia/provider_contract.h"
#include "nia/log.h"
#include <string.h>

nia_status_t nia_provider_init(nia_provider_t *prov, const nia_provider_ops_t *ops,
                                const char *name, const void *config)
{
    if (!prov || !ops || !name) return NIA_ERR_INVALID;

    memset(prov, 0, sizeof(*prov));
    size_t len = strlen(name);
    if (len >= NIA_PROVIDER_NAME_MAX) len = NIA_PROVIDER_NAME_MAX - 1;
    memcpy(prov->name, name, len);
    prov->name[len] = '\0';
    prov->ops     = ops;
    prov->running = false;

    if (ops->init) {
        return ops->init(prov, config);
    }
    return NIA_OK;
}

nia_status_t nia_provider_start(nia_provider_t *prov)
{
    if (!prov || !prov->ops) return NIA_ERR_INVALID;
    if (prov->running) return NIA_OK;

    nia_status_t st = NIA_OK;
    if (prov->ops->start) {
        st = prov->ops->start(prov);
    }
    if (st == NIA_OK) {
        prov->running = true;
        NIA_LOG_INFO("provider", "started: %s", prov->name);
    }
    return st;
}

nia_status_t nia_provider_poll(nia_provider_t *prov, nia_event_t *ev)
{
    if (!prov || !ev) return NIA_ERR_INVALID;
    if (!prov->running) return NIA_ERR_RUNTIME;
    if (!prov->ops || !prov->ops->poll) return NIA_ERR_UNSUPPORTED;

    return prov->ops->poll(prov, ev);
}

nia_status_t nia_provider_stop(nia_provider_t *prov)
{
    if (!prov || !prov->ops) return NIA_ERR_INVALID;
    if (!prov->running) return NIA_OK;

    nia_status_t st = NIA_OK;
    if (prov->ops->stop) {
        st = prov->ops->stop(prov);
    }
    prov->running = false;
    NIA_LOG_INFO("provider", "stopped: %s", prov->name);
    return st;
}

void nia_provider_shutdown(nia_provider_t *prov)
{
    if (!prov || !prov->ops) return;

    if (prov->running) {
        nia_provider_stop(prov);
    }
    if (prov->ops->shutdown) {
        prov->ops->shutdown(prov);
    }
    NIA_LOG_INFO("provider", "shutdown: %s", prov->name);
}
