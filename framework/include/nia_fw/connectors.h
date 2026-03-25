#ifndef NIA_FW_CONNECTORS_H
#define NIA_FW_CONNECTORS_H

#include "nia/common.h"

#define NIA_FW_CONNECTOR_NAME_MAX 64
#define NIA_FW_MAX_CONNECTORS     16

typedef struct nia_fw_connector_s nia_fw_connector_t;

typedef struct {
    const char  *name;
    nia_status_t (*init)(nia_fw_connector_t *conn, const void *config);
    nia_status_t (*send)(nia_fw_connector_t *conn, const nia_event_t *ev);
    nia_status_t (*recv)(nia_fw_connector_t *conn, nia_event_t *ev);
    void         (*shutdown)(nia_fw_connector_t *conn);
} nia_fw_connector_ops_t;

struct nia_fw_connector_s {
    char                          name[NIA_FW_CONNECTOR_NAME_MAX];
    const nia_fw_connector_ops_t *ops;
    void                         *ctx;
    bool                          active;
};

typedef struct {
    nia_fw_connector_t connectors[NIA_FW_MAX_CONNECTORS];
    int                count;
} nia_fw_connector_manager_t;

nia_status_t nia_fw_connector_manager_init(nia_fw_connector_manager_t *mgr);
nia_status_t nia_fw_connector_manager_add(nia_fw_connector_manager_t *mgr,
                                            const nia_fw_connector_ops_t *ops,
                                            const char *name, const void *config);
nia_status_t nia_fw_connector_broadcast(nia_fw_connector_manager_t *mgr,
                                          const nia_event_t *ev);
void         nia_fw_connector_manager_shutdown(nia_fw_connector_manager_t *mgr);

#endif /* NIA_FW_CONNECTORS_H */
