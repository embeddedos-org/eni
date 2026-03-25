#ifndef NIA_FW_STREAM_BUS_H
#define NIA_FW_STREAM_BUS_H

#include "nia/common.h"

#define NIA_FW_STREAM_BUS_CAPACITY 256

typedef struct {
    nia_event_t events[NIA_FW_STREAM_BUS_CAPACITY];
    int         head;
    int         tail;
    int         count;
    uint64_t    total_enqueued;
    uint64_t    total_dropped;
} nia_fw_stream_bus_t;

nia_status_t nia_fw_stream_bus_init(nia_fw_stream_bus_t *bus);
nia_status_t nia_fw_stream_bus_push(nia_fw_stream_bus_t *bus, const nia_event_t *ev);
nia_status_t nia_fw_stream_bus_pop(nia_fw_stream_bus_t *bus, nia_event_t *ev);
int          nia_fw_stream_bus_pending(const nia_fw_stream_bus_t *bus);
bool         nia_fw_stream_bus_empty(const nia_fw_stream_bus_t *bus);
void         nia_fw_stream_bus_stats(const nia_fw_stream_bus_t *bus);

#endif /* NIA_FW_STREAM_BUS_H */
