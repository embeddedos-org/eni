#include "generic.h"
#include "nia/log.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    nia_transport_t transport;
    char            endpoint[256];
    uint32_t        timeout_ms;
} generic_ctx_t;

static nia_status_t generic_init(nia_provider_t *prov, const void *config)
{
    generic_ctx_t *ctx = (generic_ctx_t *)calloc(1, sizeof(generic_ctx_t));
    if (!ctx) return NIA_ERR_NOMEM;

    if (config) {
        const nia_generic_config_t *gcfg = (const nia_generic_config_t *)config;
        ctx->transport  = gcfg->transport;
        ctx->timeout_ms = gcfg->timeout_ms;
        if (gcfg->endpoint) {
            size_t len = strlen(gcfg->endpoint);
            if (len >= sizeof(ctx->endpoint)) len = sizeof(ctx->endpoint) - 1;
            memcpy(ctx->endpoint, gcfg->endpoint, len);
            ctx->endpoint[len] = '\0';
        }
    } else {
        ctx->transport  = NIA_TRANSPORT_TCP;
        ctx->timeout_ms = 1000;
    }

    prov->ctx = ctx;
    NIA_LOG_INFO("generic", "initialized (transport=%d endpoint=%s)",
                 ctx->transport, ctx->endpoint);
    return NIA_OK;
}

static nia_status_t generic_start(nia_provider_t *prov)
{
    generic_ctx_t *ctx = (generic_ctx_t *)prov->ctx;
    NIA_LOG_INFO("generic", "connecting to %s...", ctx->endpoint);
    /* TODO: actual socket/gRPC/MQTT connection */
    NIA_LOG_WARN("generic", "network transport not yet implemented — stub mode");
    return NIA_OK;
}

static nia_status_t generic_poll(nia_provider_t *prov, nia_event_t *ev)
{
    (void)prov;
    (void)ev;
    /* Stub — would read from network transport */
    return NIA_ERR_TIMEOUT;
}

static nia_status_t generic_stop(nia_provider_t *prov)
{
    NIA_LOG_INFO("generic", "disconnected");
    return NIA_OK;
}

static void generic_shutdown(nia_provider_t *prov)
{
    if (prov && prov->ctx) {
        free(prov->ctx);
        prov->ctx = NULL;
    }
    NIA_LOG_INFO("generic", "shutdown");
}

const nia_provider_ops_t nia_provider_generic_ops = {
    .name     = "generic",
    .init     = generic_init,
    .poll     = generic_poll,
    .start    = generic_start,
    .stop     = generic_stop,
    .shutdown = generic_shutdown,
};
