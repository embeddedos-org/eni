#ifndef NIA_LOG_H
#define NIA_LOG_H

#include "nia/types.h"
#include <stdio.h>

void nia_log_set_level(nia_log_level_t level);
void nia_log_set_output(FILE *fp);
void nia_log_write(nia_log_level_t level, const char *module, const char *fmt, ...);

#define NIA_LOG_TRACE(mod, ...) nia_log_write(NIA_LOG_TRACE, mod, __VA_ARGS__)
#define NIA_LOG_DEBUG(mod, ...) nia_log_write(NIA_LOG_DEBUG, mod, __VA_ARGS__)
#define NIA_LOG_INFO(mod, ...)  nia_log_write(NIA_LOG_INFO,  mod, __VA_ARGS__)
#define NIA_LOG_WARN(mod, ...)  nia_log_write(NIA_LOG_WARN,  mod, __VA_ARGS__)
#define NIA_LOG_ERROR(mod, ...) nia_log_write(NIA_LOG_ERROR, mod, __VA_ARGS__)
#define NIA_LOG_FATAL(mod, ...) nia_log_write(NIA_LOG_FATAL, mod, __VA_ARGS__)

#endif /* NIA_LOG_H */
