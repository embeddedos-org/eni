#include "simulator.h"
#include "nia/log.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    uint32_t event_interval_ms;
    uint32_t tick_count;
    uint32_t cycle;
} sim_ctx_t;

static const char *s_sim_intents[] = {
    "move_left",
    "move_right",
    "select",
    "scroll_up",
    "scroll_down",
    "activate",
    "deactivate",
};
static const int s_sim_intent_count = sizeof(s_sim_intents) / sizeof(s_sim_intents[0]);

static nia_status_t sim_init(nia_provider_t *prov, const void *config)
{
    sim_ctx_t *ctx = (sim_ctx_t *)calloc(1, sizeof(sim_ctx_t));
    if (!ctx) return NIA_ERR_NOMEM;

    if (config) {
        const nia_simulator_config_t *scfg = (const nia_simulator_config_t *)config;
        ctx->event_interval_ms = scfg->event_interval_ms;
    } else {
        ctx->event_interval_ms = 500;
    }

    ctx->tick_count = 0;
    ctx->cycle = 0;
    prov->ctx = ctx;

    NIA_LOG_INFO("simulator", "initialized (interval=%u ms)", ctx->event_interval_ms);
    return NIA_OK;
}

static nia_status_t sim_start(nia_provider_t *prov)
{
    NIA_LOG_INFO("simulator", "started");
    return NIA_OK;
}

static nia_status_t sim_poll(nia_provider_t *prov, nia_event_t *ev)
{
    if (!prov || !ev) return NIA_ERR_INVALID;

    sim_ctx_t *ctx = (sim_ctx_t *)prov->ctx;
    if (!ctx) return NIA_ERR_RUNTIME;

    ctx->tick_count++;

    /* Generate an event every N ticks to simulate intermittent signals */
    if (ctx->tick_count % 3 != 0) {
        return NIA_ERR_TIMEOUT;
    }

    nia_status_t st = nia_event_init(ev, NIA_EVENT_INTENT, "simulator");
    if (st != NIA_OK) return st;

    int idx = ctx->cycle % s_sim_intent_count;
    float confidence = 0.75f + (float)(ctx->cycle % 25) / 100.0f;
    if (confidence > 1.0f) confidence = 1.0f;

    st = nia_event_set_intent(ev, s_sim_intents[idx], confidence);
    ctx->cycle++;

    return st;
}

static nia_status_t sim_stop(nia_provider_t *prov)
{
    NIA_LOG_INFO("simulator", "stopped");
    return NIA_OK;
}

static void sim_shutdown(nia_provider_t *prov)
{
    if (prov && prov->ctx) {
        free(prov->ctx);
        prov->ctx = NULL;
    }
    NIA_LOG_INFO("simulator", "shutdown");
}

const nia_provider_ops_t nia_provider_simulator_ops = {
    .name     = "simulator",
    .init     = sim_init,
    .poll     = sim_poll,
    .start    = sim_start,
    .stop     = sim_stop,
    .shutdown = sim_shutdown,
};
