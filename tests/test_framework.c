#include <stdio.h>
#include <string.h>
#include "eni/common.h"
#include "eni_fw/stream_bus.h"
#include "eni_fw/router.h"

static int tests_run = 0, tests_passed = 0, tests_failed = 0;
#define TEST(name) do { tests_run++; printf("  TEST %-40s ", #name); } while(0)
#define PASS() do { tests_passed++; printf("[PASS]\n"); } while(0)
#define FAIL(msg) do { tests_failed++; printf("[FAIL] %s\n", msg); } while(0)

static void test_stream_bus_empty(void)
{
    TEST(stream_bus_empty);
    eni_fw_stream_bus_t bus;
    eni_fw_stream_bus_init(&bus);
    if (!eni_fw_stream_bus_empty(&bus)) { FAIL("should be empty on init"); return; }
    if (eni_fw_stream_bus_pending(&bus) != 0) { FAIL("pending should be 0"); return; }
    PASS();
}

static void test_stream_bus_push_pop(void)
{
    TEST(stream_bus_push_pop);
    eni_fw_stream_bus_t bus;
    eni_fw_stream_bus_init(&bus);

    eni_event_t ev;
    eni_event_init(&ev, ENI_EVENT_INTENT, "test");
    eni_event_set_intent(&ev, "move_left", 0.9f);

    eni_status_t st = eni_fw_stream_bus_push(&bus, &ev);
    if (st != ENI_OK) { FAIL("push failed"); return; }
    if (eni_fw_stream_bus_pending(&bus) != 1) { FAIL("pending should be 1"); return; }

    eni_event_t popped;
    st = eni_fw_stream_bus_pop(&bus, &popped);
    if (st != ENI_OK) { FAIL("pop failed"); return; }
    if (popped.type != ENI_EVENT_INTENT) { FAIL("wrong type"); return; }
    if (eni_fw_stream_bus_empty(&bus) != true) { FAIL("should be empty after pop"); return; }
    PASS();
}

static void test_stream_bus_overflow(void)
{
    TEST(stream_bus_overflow);
    eni_fw_stream_bus_t bus;
    eni_fw_stream_bus_init(&bus);

    eni_event_t ev;
    eni_event_init(&ev, ENI_EVENT_RAW, "test");

    for (int i = 0; i < ENI_FW_STREAM_BUS_CAPACITY; i++) {
        eni_fw_stream_bus_push(&bus, &ev);
    }

    eni_status_t st = eni_fw_stream_bus_push(&bus, &ev);
    if (st == ENI_OK) { FAIL("should fail on overflow"); return; }
    PASS();
}

static void test_router_classify(void)
{
    TEST(router_classify);
    eni_fw_router_t router;
    eni_fw_router_init(&router);

    eni_fw_router_add_rule(&router, ENI_EVENT_CONTROL, ENI_ROUTE_CRITICAL, 10, true);
    eni_fw_router_add_rule(&router, ENI_EVENT_INTENT, ENI_ROUTE_NORMAL, 50, false);

    eni_event_t ev;
    eni_event_init(&ev, ENI_EVENT_CONTROL, "test");
    eni_route_priority_t pri = eni_fw_router_classify(&router, &ev);
    if (pri != ENI_ROUTE_CRITICAL) { FAIL("control should be critical"); return; }

    eni_event_init(&ev, ENI_EVENT_INTENT, "test");
    pri = eni_fw_router_classify(&router, &ev);
    if (pri != ENI_ROUTE_NORMAL) { FAIL("intent should be normal"); return; }
    PASS();
}

int main(void)
{
    printf("=== ENI Framework Tests ===\n\n");

    test_stream_bus_empty();
    test_stream_bus_push_pop();
    test_stream_bus_overflow();
    test_router_classify();

    printf("\n=== Results: %d/%d passed, %d failed ===\n",
           tests_passed, tests_run, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
#include <stdio.h>
#include <string.h>
#include "eni/common.h"
#include "eni_fw/stream_bus.h"
#include "eni_fw/router.h"

static int tests_run    = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    tests_run++; \
    printf("  [TEST] %s ... ", #name); \
} while (0)

#define PASS() do { \
    tests_passed++; \
    printf("PASS\n"); \
} while (0)

#define FAIL(msg) do { \
    printf("FAIL: %s\n", msg); \
} while (0)

/* ── Stream Bus tests ───────────────────────────────────────── */

static void test_stream_bus_empty(void)
{
    TEST(stream_bus_empty);
    eni_fw_stream_bus_t bus;
    eni_fw_stream_bus_init(&bus);

    if (!eni_fw_stream_bus_empty(&bus)) { FAIL("bus should be empty after init"); return; }
    if (eni_fw_stream_bus_pending(&bus) != 0) { FAIL("pending should be 0"); return; }
    PASS();
}

static void test_stream_bus_push_pop(void)
{
    TEST(stream_bus_push_pop);
    eni_fw_stream_bus_t bus;
    eni_fw_stream_bus_init(&bus);

    eni_event_t ev_in;
    memset(&ev_in, 0, sizeof(ev_in));
    ev_in.type = ENI_EVENT_INTENT;
    ev_in.payload.intent.name = "test_intent";
    ev_in.payload.intent.confidence = 0.90f;

    eni_status_t st = eni_fw_stream_bus_push(&bus, &ev_in);
    if (st != ENI_OK) { FAIL("push failed"); return; }
    if (eni_fw_stream_bus_empty(&bus)) { FAIL("bus should not be empty after push"); return; }
    if (eni_fw_stream_bus_pending(&bus) != 1) { FAIL("pending should be 1"); return; }

    eni_event_t ev_out;
    memset(&ev_out, 0, sizeof(ev_out));
    st = eni_fw_stream_bus_pop(&bus, &ev_out);
    if (st != ENI_OK) { FAIL("pop failed"); return; }
    if (ev_out.type != ENI_EVENT_INTENT) { FAIL("event type mismatch"); return; }
    if (strcmp(ev_out.payload.intent.name, "test_intent") != 0) { FAIL("intent name mismatch"); return; }
    if (eni_fw_stream_bus_pending(&bus) != 0) { FAIL("pending should be 0 after pop"); return; }
    PASS();
}

static void test_stream_bus_overflow(void)
{
    TEST(stream_bus_overflow);
    eni_fw_stream_bus_t bus;
    eni_fw_stream_bus_init(&bus);

    eni_event_t ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = ENI_EVENT_INTENT;
    ev.payload.intent.name = "overflow_test";
    ev.payload.intent.confidence = 0.50f;

    /* Fill to capacity */
    eni_status_t st;
    for (int i = 0; i < ENI_FW_STREAM_BUS_CAPACITY; i++) {
        st = eni_fw_stream_bus_push(&bus, &ev);
        if (st != ENI_OK) { FAIL("push should succeed before capacity"); return; }
    }

    /* Next push should fail or indicate overflow */
    st = eni_fw_stream_bus_push(&bus, &ev);
    if (st == ENI_OK) { FAIL("push should fail at capacity"); return; }
    PASS();
}

/* ── Router tests ───────────────────────────────────────────── */

static void test_router_classify(void)
{
    TEST(router_classify);
    eni_fw_router_t router;
    eni_fw_router_init(&router);

    eni_fw_router_add_rule(&router, ENI_EVENT_INTENT, ENI_ROUTE_CRITICAL, 10, true);

    eni_event_t ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = ENI_EVENT_INTENT;
    ev.payload.intent.name = "emergency_stop";
    ev.payload.intent.confidence = 0.99f;

    eni_route_priority_t pri = eni_fw_router_classify(&router, &ev);
    if (pri != ENI_ROUTE_CRITICAL) { FAIL("should classify as CRITICAL"); return; }
    PASS();
}

/* ── Main ───────────────────────────────────────────────────── */

int main(void)
{
    printf("=== ENI Framework Tests ===\n");

    test_stream_bus_empty();
    test_stream_bus_push_pop();
    test_stream_bus_overflow();
    test_router_classify();

    printf("\nResults: %d/%d passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
