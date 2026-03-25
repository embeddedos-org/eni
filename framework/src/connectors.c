#include "nia_fw/connectors.h"
#include "nia/log.h"
#include <string.h>

nia_status_t nia_fw_connector_manager_init(nia_fw_connector_manager_t *mgr)
{
    if (!mgr) return NIA_ERR_INVALID;
    memset(mgr, 0, sizeof(*mgr));
    return NIA_OK;
}

nia_status_t nia_fw_connector_manager_add(nia_fw_connector_manager_t *mgr,
                                            const nia_fw_connector_ops_t *ops,
                                            const char *name, const void *config)
{
    if (!mgr || !ops || !name) return NIA_ERR_INVALID;
    if (mgr->count >= NIA_FW_MAX_CONNECTORS) return NIA_ERR_OVERFLOW;

    nia_fw_connector_t *conn = &mgr->connectors[mgr->count];
    size_t len = strlen(name);
    if (len >= NIA_FW_CONNECTOR_NAME_MAX) len = NIA_FW_CONNECTOR_NAME_MAX - 1;
    memcpy(conn->name, name, len);
    conn->name[len] = '\0';
    conn->ops    = ops;
    conn->ctx    = NULL;
    conn->active = false;

    if (ops->init) {
        nia_status_t st = ops->init(conn, config);
        if (st != NIA_OK) return st;
    }

    conn->active = true;
    mgr->count++;
    NIA_LOG_INFO("fw.connectors", "added connector: %s", name);
    return NIA_OK;
}

nia_status_t nia_fw_connector_broadcast(nia_fw_connector_manager_t *mgr,
                                          const nia_event_t *ev)
{
    if (!mgr || !ev) return NIA_ERR_INVALID;

    for (int i = 0; i < mgr->count; i++) {
        nia_fw_connector_t *conn = &mgr->connectors[i];
        if (!conn->active || !conn->ops || !conn->ops->send) continue;

        nia_status_t st = conn->ops->send(conn, ev);
        if (st != NIA_OK) {
            NIA_LOG_WARN("fw.connectors", "send failed on connector: %s", conn->name);
        }
    }

    return NIA_OK;
}

void nia_fw_connector_manager_shutdown(nia_fw_connector_manager_t *mgr)
{
    if (!mgr) return;

    for (int i = 0; i < mgr->count; i++) {
        nia_fw_connector_t *conn = &mgr->connectors[i];
        if (conn->ops && conn->ops->shutdown) {
            conn->ops->shutdown(conn);
        }
        conn->active = false;
    }
    mgr->count = 0;
    NIA_LOG_INFO("fw.connectors", "all connectors shutdown");
}
