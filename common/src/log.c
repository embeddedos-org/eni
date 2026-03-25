#include "nia/log.h"
#include <stdarg.h>
#include <time.h>

static nia_log_level_t s_log_level = NIA_LOG_INFO;
static FILE           *s_log_out   = NULL;

void nia_log_set_level(nia_log_level_t level)
{
    s_log_level = level;
}

void nia_log_set_output(FILE *fp)
{
    s_log_out = fp;
}

void nia_log_write(nia_log_level_t level, const char *module, const char *fmt, ...)
{
    if (level < s_log_level) return;

    FILE *out = s_log_out ? s_log_out : stderr;

    const char *level_str;
    switch (level) {
    case NIA_LOG_TRACE: level_str = "TRACE"; break;
    case NIA_LOG_DEBUG: level_str = "DEBUG"; break;
    case NIA_LOG_INFO:  level_str = "INFO";  break;
    case NIA_LOG_WARN:  level_str = "WARN";  break;
    case NIA_LOG_ERROR: level_str = "ERROR"; break;
    case NIA_LOG_FATAL: level_str = "FATAL"; break;
    default:            level_str = "???";   break;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(out, "%04d-%02d-%02d %02d:%02d:%02d [%s] [%s] ",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec,
            level_str, module ? module : "nia");

    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);

    fprintf(out, "\n");
    fflush(out);
}
