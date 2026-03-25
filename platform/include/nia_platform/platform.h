#ifndef NIA_PLATFORM_H
#define NIA_PLATFORM_H

#include "nia/types.h"

typedef struct {
    const char *os_name;
    const char *arch;
    bool        realtime_capable;
    bool        hardware_access;
} nia_platform_info_t;

nia_status_t        nia_platform_init(void);
nia_platform_info_t nia_platform_info(void);
void                nia_platform_sleep_ms(uint32_t ms);
uint64_t            nia_platform_monotonic_ms(void);

#endif /* NIA_PLATFORM_H */
