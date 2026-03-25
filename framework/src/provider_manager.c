#include "nia_fw/provider_manager.h"
#include "nia/log.h"
#include <string.h>

nia_status_t nia_fw_provider_manager_init(nia_fw_provider_manager_t *mgr)
{
    if (!mgr) return NIA_ERR_INVALID;
    memset(mgr, 0, sizeof(*mgr));
    return NIA_OK;
}

nia_status_t nia_fw_provider_manager_add(nia_fw_provider_manager_t *mgr,
                                          const nia_provider_ops_t *ops,
                                          const char *name,
                                          const void *config)
{
    if (!mgr || !ops || !name) return NIA_ERR_INVALID;
    if (mgr->count >= NIA_FW_MAX_PROVIDERS) return NIA_ERR_OVERFLOW;

    nia_status_t st = nia_provider_init(&mgr->providers[mgr->count], ops, name, config);
    if (st == NIA_OK) {
        mgr->count++;
        NIA_LOG_INFO("fw.providers", "added provider: %s (total=%d)", name, mgr->count);
    }
    return st;
}

nia_status_t nia_fw_provider_manager_start_all(nia_fw_provider_manager_t *mgr)
{
    if (!mgr) return NIA_ERR_INVALID;

    mgr->active_count = 0;
    for (int i = 0; i < mgr->count; i++) {
        nia_status_t st = nia_provider_start(&mgr->providers[i]);
        if (st == NIA_OK) {
            mgr->active_count++;
        } else {
            NIA_LOG_WARN("fw.providers", "failed to start provider: %s",
                         mgr->providers[i].name);
        }
    }

    NIA_LOG_INFO("fw.providers", "started %d/%d providers",
                 mgr->active_count, mgr->count);
    return mgr->active_count > 0 ? NIA_OK : NIA_ERR_PROVIDER;
}

nia_status_t nia_fw_provider_manager_stop_all(nia_fw_provider_manager_t *mgr)
{
    if (!mgr) return NIA_ERR_INVALID;

    for (int i = 0; i < mgr->count; i++) {
        nia_provider_stop(&mgr->providers[i]);
    }
    mgr->active_count = 0;
    NIA_LOG_INFO("fw.providers", "all providers stopped");
    return NIA_OK;
}

nia_status_t nia_fw_provider_manager_poll_any(nia_fw_provider_manager_t *mgr,
                                               nia_event_t *ev, int *provider_index)
{
    if (!mgr || !ev) return NIA_ERR_INVALID;

    for (int i = 0; i < mgr->count; i++) {
        if (!mgr->providers[i].running) continue;

        nia_status_t st = nia_provider_poll(&mgr->providers[i], ev);
        if (st == NIA_OK) {
            if (provider_index) *provider_index = i;
            return NIA_OK;
        }
    }

    return NIA_ERR_TIMEOUT;
}

void nia_fw_provider_manager_shutdown(nia_fw_provider_manager_t *mgr)
{
    if (!mgr) return;

    for (int i = 0; i < mgr->count; i++) {
        nia_provider_shutdown(&mgr->providers[i]);
    }
    mgr->count = 0;
    mgr->active_count = 0;
}
