#ifndef NIA_TYPES_H
#define NIA_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    NIA_OK = 0,
    NIA_ERR_NOMEM,
    NIA_ERR_INVALID,
    NIA_ERR_NOT_FOUND,
    NIA_ERR_IO,
    NIA_ERR_TIMEOUT,
    NIA_ERR_PERMISSION,
    NIA_ERR_RUNTIME,
    NIA_ERR_CONNECT,
    NIA_ERR_PROTOCOL,
    NIA_ERR_CONFIG,
    NIA_ERR_UNSUPPORTED,
    NIA_ERR_POLICY_DENIED,
    NIA_ERR_PROVIDER,
    NIA_ERR_OVERFLOW,
} nia_status_t;

typedef enum {
    NIA_VARIANT_MIN,
    NIA_VARIANT_FRAMEWORK,
} nia_variant_t;

typedef enum {
    NIA_MODE_INTENT,
    NIA_MODE_FEATURES,
    NIA_MODE_RAW,
    NIA_MODE_FEATURES_INTENT,
} nia_mode_t;

typedef enum {
    NIA_LOG_TRACE,
    NIA_LOG_DEBUG,
    NIA_LOG_INFO,
    NIA_LOG_WARN,
    NIA_LOG_ERROR,
    NIA_LOG_FATAL,
} nia_log_level_t;

typedef enum {
    NIA_TRANSPORT_UNIX_SOCKET,
    NIA_TRANSPORT_TCP,
    NIA_TRANSPORT_GRPC,
    NIA_TRANSPORT_MQTT,
    NIA_TRANSPORT_SHARED_MEM,
} nia_transport_t;

typedef struct {
    const char *key;
    const char *value;
} nia_kv_t;

typedef struct {
    uint8_t *data;
    size_t   len;
    size_t   cap;
} nia_buffer_t;

typedef struct {
    uint64_t sec;
    uint32_t nsec;
} nia_timestamp_t;

const char     *nia_status_str(nia_status_t status);
nia_timestamp_t nia_timestamp_now(void);

#endif /* NIA_TYPES_H */
