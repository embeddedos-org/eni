#include "nia_fw/stream_bus.h"
#include "nia/log.h"
#include <string.h>
#include <stdio.h>

nia_status_t nia_fw_stream_bus_init(nia_fw_stream_bus_t *bus)
{
    if (!bus) return NIA_ERR_INVALID;
    memset(bus, 0, sizeof(*bus));
    return NIA_OK;
}

nia_status_t nia_fw_stream_bus_push(nia_fw_stream_bus_t *bus, const nia_event_t *ev)
{
    if (!bus || !ev) return NIA_ERR_INVALID;

    if (bus->count >= NIA_FW_STREAM_BUS_CAPACITY) {
        bus->total_dropped++;
        NIA_LOG_WARN("fw.bus", "event dropped (bus full, total_dropped=%llu)",
                     (unsigned long long)bus->total_dropped);
        return NIA_ERR_OVERFLOW;
    }

    memcpy(&bus->events[bus->tail], ev, sizeof(*ev));
    bus->tail = (bus->tail + 1) % NIA_FW_STREAM_BUS_CAPACITY;
    bus->count++;
    bus->total_enqueued++;

    return NIA_OK;
}

nia_status_t nia_fw_stream_bus_pop(nia_fw_stream_bus_t *bus, nia_event_t *ev)
{
    if (!bus || !ev) return NIA_ERR_INVALID;

    if (bus->count == 0) {
        return NIA_ERR_TIMEOUT;
    }

    memcpy(ev, &bus->events[bus->head], sizeof(*ev));
    bus->head = (bus->head + 1) % NIA_FW_STREAM_BUS_CAPACITY;
    bus->count--;

    return NIA_OK;
}

int nia_fw_stream_bus_pending(const nia_fw_stream_bus_t *bus)
{
    return bus ? bus->count : 0;
}

bool nia_fw_stream_bus_empty(const nia_fw_stream_bus_t *bus)
{
    return !bus || bus->count == 0;
}

void nia_fw_stream_bus_stats(const nia_fw_stream_bus_t *bus)
{
    if (!bus) return;
    printf("[stream_bus] pending=%d enqueued=%llu dropped=%llu\n",
           bus->count,
           (unsigned long long)bus->total_enqueued,
           (unsigned long long)bus->total_dropped);
}
