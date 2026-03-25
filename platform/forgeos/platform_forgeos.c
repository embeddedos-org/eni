#include "nia_platform/platform.h"

#ifdef NIA_PLATFORM_FORGEOS_ENABLED

nia_status_t nia_platform_init(void)
{
    /* ForgeOS: initialize system services, GPIO, real-time scheduler */
    return NIA_OK;
}

nia_platform_info_t nia_platform_info(void)
{
    nia_platform_info_t info = {
        .os_name          = "forgeos",
#if defined(__aarch64__)
        .arch             = "arm64",
#elif defined(__x86_64__)
        .arch             = "x86_64",
#elif defined(__riscv)
        .arch             = "riscv64",
#else
        .arch             = "unknown",
#endif
        .realtime_capable = true,
        .hardware_access  = true,
    };
    return info;
}

void nia_platform_sleep_ms(uint32_t ms)
{
    /* ForgeOS: real-time sleep via system scheduler */
    (void)ms;
}

uint64_t nia_platform_monotonic_ms(void)
{
    /* ForgeOS: high-precision monotonic clock */
    return 0;
}

#endif /* NIA_PLATFORM_FORGEOS_ENABLED */
