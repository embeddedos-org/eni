// SPDX-License-Identifier: MIT
// Copyright (c) 2026 EoS Project
// Provider lifecycle tests using mock provider

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "eni/provider_contract.h"
#include "mocks/mock_provider.h"

static int passed = 0;
#define PASS(name) do { printf("[PASS] %s\n", name); passed++; } while(0)

static void test_provider_init_basic(void) {
    eni_provider_t prov;
    mock_provider_ctx_t ctx;
    mock_provider_ctx_init(&ctx);
    prov.ctx = &ctx;
    eni_status_t rc = eni_provider_init(&prov, &mock_provider_ops, "test0", NULL);
    assert(rc == ENI_OK);
    assert(strcmp(prov.name, "test0") == 0);
    assert(prov.ops == &mock_provider_ops);
    assert(ctx.init_called == 1);
    eni_provider_shutdown(&prov);
    assert(ctx.shutdown_called == 1);
    PASS("provider_init_basic");
}

static void test_provider_start_stop(void) {
    eni_provider_t prov;
    mock_provider_ctx_t ctx;
    mock_provider_ctx_init(&ctx);
    prov.ctx = &ctx;
    eni_provider_init(&prov, &mock_provider_ops, "test1", NULL);
    assert(eni_provider_start(&prov) == ENI_OK);
    assert(ctx.start_called == 1);
    assert(prov.running == true);
    assert(eni_provider_stop(&prov) == ENI_OK);
    assert(ctx.stop_called == 1);
    assert(prov.running == false);
    eni_provider_shutdown(&prov);
    PASS("provider_start_stop");
}

static void test_provider_poll_events(void) {
    eni_provider_t prov;
    mock_provider_ctx_t ctx;
    mock_provider_ctx_init(&ctx);
    ctx.events_to_generate = 3;
    prov.ctx = &ctx;
    eni_provider_init(&prov, &mock_provider_ops, "test2", NULL);
    eni_provider_start(&prov);

    for (int i = 0; i < 3; i++) {
        eni_event_t ev;
        eni_status_t rc = eni_provider_poll(&prov, &ev);
        assert(rc == ENI_OK);
        assert(ev.type == ENI_EVENT_INTENT);
        assert(ev.id == (uint32_t)i);
    }
    eni_event_t ev;
    assert(eni_provider_poll(&prov, &ev) == ENI_ERR_NOT_FOUND);
    assert(ctx.poll_count == 4);

    eni_provider_stop(&prov);
    eni_provider_shutdown(&prov);
    PASS("provider_poll_events");
}

static void test_provider_enqueued_events(void) {
    eni_provider_t prov;
    mock_provider_ctx_t ctx;
    mock_provider_ctx_init(&ctx);
    prov.ctx = &ctx;

    eni_event_t custom_ev;
    memset(&custom_ev, 0, sizeof(custom_ev));
    custom_ev.type = ENI_EVENT_FEATURES;
    custom_ev.id = 42;
    strncpy(custom_ev.source, "custom", ENI_EVENT_SOURCE_MAX - 1);
    mock_provider_enqueue_event(&ctx, &custom_ev);

    eni_provider_init(&prov, &mock_provider_ops, "test3", NULL);
    eni_provider_start(&prov);

    eni_event_t ev;
    assert(eni_provider_poll(&prov, &ev) == ENI_OK);
    assert(ev.type == ENI_EVENT_FEATURES);
    assert(ev.id == 42);
    assert(strcmp(ev.source, "custom") == 0);

    eni_provider_stop(&prov);
    eni_provider_shutdown(&prov);
    PASS("provider_enqueued_events");
}

static void test_provider_init_failure(void) {
    eni_provider_t prov;
    mock_provider_ctx_t ctx;
    mock_provider_ctx_init(&ctx);
    ctx.fail_on_init = 1;
    prov.ctx = &ctx;
    eni_status_t rc = eni_provider_init(&prov, &mock_provider_ops, "fail0", NULL);
    assert(rc == ENI_ERR_RUNTIME);
    PASS("provider_init_failure");
}

static void test_provider_start_failure(void) {
    eni_provider_t prov;
    mock_provider_ctx_t ctx;
    mock_provider_ctx_init(&ctx);
    prov.ctx = &ctx;
    eni_provider_init(&prov, &mock_provider_ops, "fail1", NULL);
    ctx.fail_on_start = 1;
    eni_status_t rc = eni_provider_start(&prov);
    assert(rc == ENI_ERR_RUNTIME);
    assert(prov.running == false);
    eni_provider_shutdown(&prov);
    PASS("provider_start_failure");
}

static void test_provider_poll_failure(void) {
    eni_provider_t prov;
    mock_provider_ctx_t ctx;
    mock_provider_ctx_init(&ctx);
    prov.ctx = &ctx;
    eni_provider_init(&prov, &mock_provider_ops, "fail2", NULL);
    eni_provider_start(&prov);
    ctx.fail_on_poll = 1;
    eni_event_t ev;
    assert(eni_provider_poll(&prov, &ev) == ENI_ERR_RUNTIME);
    eni_provider_stop(&prov);
    eni_provider_shutdown(&prov);
    PASS("provider_poll_failure");
}

static void test_provider_failing_ops(void) {
    eni_provider_t prov;
    eni_status_t rc = eni_provider_init(&prov, &mock_provider_failing_ops, "always_fail", NULL);
    assert(rc == ENI_ERR_RUNTIME);
    PASS("provider_failing_ops");
}

static void test_provider_multiple_start_stop(void) {
    eni_provider_t prov;
    mock_provider_ctx_t ctx;
    mock_provider_ctx_init(&ctx);
    prov.ctx = &ctx;
    eni_provider_init(&prov, &mock_provider_ops, "multi", NULL);
    for (int i = 0; i < 5; i++) {
        assert(eni_provider_start(&prov) == ENI_OK);
        assert(prov.running == true);
        assert(eni_provider_stop(&prov) == ENI_OK);
        assert(prov.running == false);
    }
    assert(ctx.start_called == 5);
    assert(ctx.stop_called == 5);
    eni_provider_shutdown(&prov);
    PASS("provider_multiple_start_stop");
}

static void test_provider_name_truncation(void) {
    eni_provider_t prov;
    mock_provider_ctx_t ctx;
    mock_provider_ctx_init(&ctx);
    prov.ctx = &ctx;
    char long_name[128];
    memset(long_name, 'A', sizeof(long_name) - 1);
    long_name[127] = '\0';
    eni_provider_init(&prov, &mock_provider_ops, long_name, NULL);
    assert(strlen(prov.name) < ENI_PROVIDER_NAME_MAX);
    eni_provider_shutdown(&prov);
    PASS("provider_name_truncation");
}

static void test_provider_ctx_reset(void) {
    mock_provider_ctx_t ctx;
    mock_provider_ctx_init(&ctx);
    ctx.init_called = 5;
    ctx.poll_count = 10;
    ctx.events_generated = 3;
    mock_provider_ctx_reset(&ctx);
    assert(ctx.init_called == 0);
    assert(ctx.poll_count == 0);
    assert(ctx.events_generated == 0);
    PASS("provider_ctx_reset");
}

int main(void) {
    printf("=== ENI Provider Lifecycle Tests ===\n");
    test_provider_init_basic();
    test_provider_start_stop();
    test_provider_poll_events();
    test_provider_enqueued_events();
    test_provider_init_failure();
    test_provider_start_failure();
    test_provider_poll_failure();
    test_provider_failing_ops();
    test_provider_multiple_start_stop();
    test_provider_name_truncation();
    test_provider_ctx_reset();
    printf("\n=== ALL %d TESTS PASSED ===\n", passed);
    return 0;
}
