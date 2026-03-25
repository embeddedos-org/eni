#include "nia_min/input.h"
#include "nia/log.h"
#include <string.h>

nia_status_t nia_min_input_init(nia_min_input_t *input, const nia_provider_ops_t *ops,
                                 const char *provider_name, nia_transport_t transport)
{
    if (!input || !ops || !provider_name) return NIA_ERR_INVALID;

    memset(input, 0, sizeof(*input));
    input->transport = transport;
    input->connected = false;

    return nia_provider_init(&input->provider, ops, provider_name, NULL);
}

nia_status_t nia_min_input_connect(nia_min_input_t *input)
{
    if (!input) return NIA_ERR_INVALID;

    nia_status_t st = nia_provider_start(&input->provider);
    if (st == NIA_OK) {
        input->connected = true;
        NIA_LOG_INFO("min.input", "connected to provider: %s", input->provider.name);
    }
    return st;
}

nia_status_t nia_min_input_poll(nia_min_input_t *input, nia_event_t *ev)
{
    if (!input || !ev) return NIA_ERR_INVALID;
    if (!input->connected) return NIA_ERR_RUNTIME;

    return nia_provider_poll(&input->provider, ev);
}

nia_status_t nia_min_input_disconnect(nia_min_input_t *input)
{
    if (!input) return NIA_ERR_INVALID;

    nia_status_t st = nia_provider_stop(&input->provider);
    input->connected = false;
    NIA_LOG_INFO("min.input", "disconnected from provider: %s", input->provider.name);
    return st;
}

void nia_min_input_shutdown(nia_min_input_t *input)
{
    if (!input) return;
    if (input->connected) {
        nia_min_input_disconnect(input);
    }
    nia_provider_shutdown(&input->provider);
}
