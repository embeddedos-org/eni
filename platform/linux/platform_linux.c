#include "nia_platform/platform.h"

#ifndef _WIN32

#include <time.h>
#include <unistd.h>

nia_status_t nia_platform_init(void)
{
    return NIA_OK;
}

nia_platform_info_t nia_platform_info(void)
{
    nia_platform_info_t info = {
        .os_name          = "linux",
#if defined(__aarch64__)
        .arch             = "arm64",
#elif defined(__x86_64__)
        .arch             = "x86_64",
#elif defined(__riscv)
        .arch             = "riscv64",
#else
        .arch             = "unknown",
#endif
        .realtime_capable = false,
        .hardware_access  = false,
    };
    return info;
}

void nia_platform_sleep_ms(uint32_t ms)
{
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

uint64_t nia_platform_monotonic_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

#endif /* !_WIN32 */
