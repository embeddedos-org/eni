// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// Mock provider for testing provider lifecycle

#ifndef ENI_MOCK_PROVIDER_H
#define ENI_MOCK_PROVIDER_H

#include "eni/provider_contract.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MOCK_PROVIDER_MAX_EVENTS 32

typedef struct {
    int  init_called;
    int  start_called;
    int  stop_called;
    int  shutdown_called;
    int  poll_count;
    int  fail_on_init;
    int  fail_on_start;
    int  fail_on_poll;
    int  events_to_generate;
    int  events_generated;

    eni_event_t event_queue[MOCK_PROVIDER_MAX_EVENTS];
    int         event_queue_count;
    int         event_queue_head;
} mock_provider_ctx_t;

extern const eni_provider_ops_t mock_provider_ops;
extern const eni_provider_ops_t mock_provider_failing_ops;

void mock_provider_ctx_init(mock_provider_ctx_t *ctx);

void mock_provider_ctx_reset(mock_provider_ctx_t *ctx);

void mock_provider_enqueue_event(mock_provider_ctx_t *ctx,
                                  const eni_event_t *ev);

void mock_provider_set_fail_init(mock_provider_ctx_t *ctx, int fail);
void mock_provider_set_fail_start(mock_provider_ctx_t *ctx, int fail);
void mock_provider_set_fail_poll(mock_provider_ctx_t *ctx, int fail);

#ifdef __cplusplus
}
#endif

#endif /* ENI_MOCK_PROVIDER_H */
