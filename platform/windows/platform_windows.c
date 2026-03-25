#include "nia_platform/platform.h"

#ifdef _WIN32

#include <windows.h>

nia_status_t nia_platform_init(void)
{
    return NIA_OK;
}

nia_platform_info_t nia_platform_info(void)
{
    nia_platform_info_t info = {
        .os_name          = "windows",
#if defined(_M_X64)
        .arch             = "x86_64",
#elif defined(_M_ARM64)
        .arch             = "arm64",
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
    Sleep(ms);
}

uint64_t nia_platform_monotonic_ms(void)
{
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (uint64_t)(count.QuadPart * 1000 / freq.QuadPart);
}

#endif /* _WIN32 */
