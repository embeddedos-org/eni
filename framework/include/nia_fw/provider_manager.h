#ifndef NIA_FW_PROVIDER_MANAGER_H
#define NIA_FW_PROVIDER_MANAGER_H

#include "nia/common.h"

#define NIA_FW_MAX_PROVIDERS 16

typedef struct {
    nia_provider_t providers[NIA_FW_MAX_PROVIDERS];
    int            count;
    int            active_count;
} nia_fw_provider_manager_t;

nia_status_t nia_fw_provider_manager_init(nia_fw_provider_manager_t *mgr);
nia_status_t nia_fw_provider_manager_add(nia_fw_provider_manager_t *mgr,
                                          const nia_provider_ops_t *ops,
                                          const char *name,
                                          const void *config);
nia_status_t nia_fw_provider_manager_start_all(nia_fw_provider_manager_t *mgr);
nia_status_t nia_fw_provider_manager_stop_all(nia_fw_provider_manager_t *mgr);
nia_status_t nia_fw_provider_manager_poll_any(nia_fw_provider_manager_t *mgr,
                                               nia_event_t *ev, int *provider_index);
void         nia_fw_provider_manager_shutdown(nia_fw_provider_manager_t *mgr);

#endif /* NIA_FW_PROVIDER_MANAGER_H */
