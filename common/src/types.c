#include "nia/types.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

const char *nia_status_str(nia_status_t status)
{
    switch (status) {
    case NIA_OK:               return "OK";
    case NIA_ERR_NOMEM:        return "ERR_NOMEM";
    case NIA_ERR_INVALID:      return "ERR_INVALID";
    case NIA_ERR_NOT_FOUND:    return "ERR_NOT_FOUND";
    case NIA_ERR_IO:           return "ERR_IO";
    case NIA_ERR_TIMEOUT:      return "ERR_TIMEOUT";
    case NIA_ERR_PERMISSION:   return "ERR_PERMISSION";
    case NIA_ERR_RUNTIME:      return "ERR_RUNTIME";
    case NIA_ERR_CONNECT:      return "ERR_CONNECT";
    case NIA_ERR_PROTOCOL:     return "ERR_PROTOCOL";
    case NIA_ERR_CONFIG:       return "ERR_CONFIG";
    case NIA_ERR_UNSUPPORTED:  return "ERR_UNSUPPORTED";
    case NIA_ERR_POLICY_DENIED:return "ERR_POLICY_DENIED";
    case NIA_ERR_PROVIDER:     return "ERR_PROVIDER";
    case NIA_ERR_OVERFLOW:     return "ERR_OVERFLOW";
    default:                   return "UNKNOWN";
    }
}

nia_timestamp_t nia_timestamp_now(void)
{
    nia_timestamp_t ts = {0, 0};
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    uint64_t ticks = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    ticks -= 116444736000000000ULL; /* Windows epoch → Unix epoch */
    ts.sec  = ticks / 10000000ULL;
    ts.nsec = (uint32_t)((ticks % 10000000ULL) * 100);
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    ts.sec  = (uint64_t)now.tv_sec;
    ts.nsec = (uint32_t)now.tv_nsec;
#endif
    return ts;
}
