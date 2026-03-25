#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "nia/common.h"

static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    do { \
        tests_run++; \
        printf("  TEST %-40s ", #name); \
    } while (0)

#define PASS() \
    do { \
        tests_passed++; \
        printf("[PASS]\n"); \
    } while (0)

#define FAIL(msg) \
    do { \
        tests_failed++; \
        printf("[FAIL] %s\n", msg); \
    } while (0)

static void test_event_init(void)
{
    TEST(event_init);
    nia_event_t ev;
    nia_status_t st = nia_event_init(&ev, NIA_EVENT_INTENT, "test-source");
    if (st != NIA_OK) { FAIL("init returned error"); return; }
    if (ev.type != NIA_EVENT_INTENT) { FAIL("wrong type"); return; }
    if (strcmp(ev.source, "test-source") != 0) { FAIL("wrong source"); return; }
    if (ev.version != 1) { FAIL("wrong version"); return; }
    PASS();
}

static void test_event_set_intent(void)
{
    TEST(event_set_intent);
    nia_event_t ev;
    nia_event_init(&ev, NIA_EVENT_INTENT, "test");
    nia_status_t st = nia_event_set_intent(&ev, "move_left", 0.95f);
    if (st != NIA_OK) { FAIL("set_intent returned error"); return; }
    if (strcmp(ev.payload.intent.name, "move_left") != 0) { FAIL("wrong intent name"); return; }
    if (ev.payload.intent.confidence < 0.94f || ev.payload.intent.confidence > 0.96f) { FAIL("wrong confidence"); return; }
    PASS();
}

static void test_event_add_feature(void)
{
    TEST(event_add_feature);
    nia_event_t ev;
    nia_event_init(&ev, NIA_EVENT_FEATURES, "test");
    nia_status_t st = nia_event_add_feature(&ev, "attention", 0.72f);
    if (st != NIA_OK) { FAIL("add_feature returned error"); return; }
    if (ev.payload.features.count != 1) { FAIL("wrong count"); return; }
    if (strcmp(ev.payload.features.features[0].name, "attention") != 0) { FAIL("wrong feature name"); return; }
    PASS();
}

static void test_event_set_raw(void)
{
    TEST(event_set_raw);
    nia_event_t ev;
    nia_event_init(&ev, NIA_EVENT_RAW, "test");
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    nia_status_t st = nia_event_set_raw(&ev, data, sizeof(data));
    if (st != NIA_OK) { FAIL("set_raw returned error"); return; }
    if (ev.payload.raw.len != 4) { FAIL("wrong len"); return; }
    if (ev.payload.raw.data[0] != 0xDE) { FAIL("wrong data"); return; }
    PASS();
}

static void test_event_type_mismatch(void)
{
    TEST(event_type_mismatch);
    nia_event_t ev;
    nia_event_init(&ev, NIA_EVENT_RAW, "test");
    nia_status_t st = nia_event_set_intent(&ev, "move", 0.5f);
    if (st != NIA_ERR_INVALID) { FAIL("should reject intent on raw event"); return; }
    PASS();
}

static void test_policy_allow_deny(void)
{
    TEST(policy_allow_deny);
    nia_policy_engine_t pol;
    nia_policy_init(&pol);
    nia_policy_add_rule(&pol, "ui.cursor.move", NIA_POLICY_ALLOW, NIA_ACTION_SAFE);
    nia_policy_add_rule(&pol, "system.shutdown", NIA_POLICY_DENY, NIA_ACTION_RESTRICTED);

    if (nia_policy_evaluate(&pol, "ui.cursor.move") != NIA_POLICY_ALLOW) { FAIL("should allow"); return; }
    if (nia_policy_evaluate(&pol, "system.shutdown") != NIA_POLICY_DENY) { FAIL("should deny"); return; }
    if (nia_policy_evaluate(&pol, "unknown.action") != NIA_POLICY_ALLOW) { FAIL("default should allow"); return; }

    nia_policy_set_default_deny(&pol, true);
    if (nia_policy_evaluate(&pol, "unknown.action") != NIA_POLICY_DENY) { FAIL("default_deny should deny"); return; }
    PASS();
}

static nia_status_t dummy_tool_exec(const nia_tool_call_t *call, nia_tool_result_t *result)
{
    (void)call;
    result->status = NIA_OK;
    const char *msg = "ok";
    memcpy(result->data, msg, 2);
    result->len = 2;
    return NIA_OK;
}

static void test_tool_registry(void)
{
    TEST(tool_registry);
    nia_tool_registry_t reg;
    nia_tool_registry_init(&reg);

    nia_tool_entry_t entry = { .name = "test.tool", .description = "A test tool", .exec = dummy_tool_exec };
    nia_status_t st = nia_tool_register(&reg, &entry);
    if (st != NIA_OK) { FAIL("register failed"); return; }

    nia_tool_entry_t *found = nia_tool_find(&reg, "test.tool");
    if (!found) { FAIL("find returned NULL"); return; }
    if (strcmp(found->name, "test.tool") != 0) { FAIL("wrong name"); return; }

    nia_tool_call_t call = {0};
    memcpy(call.tool, "test.tool", 9);
    nia_tool_result_t result;
    st = nia_tool_exec(&reg, &call, &result);
    if (st != NIA_OK) { FAIL("exec failed"); return; }
    PASS();
}

static void test_config_defaults(void)
{
    TEST(config_defaults);
    nia_config_t cfg;
    nia_config_load_defaults(&cfg, NIA_VARIANT_MIN);
    if (cfg.variant != NIA_VARIANT_MIN) { FAIL("wrong variant"); return; }
    if (cfg.mode != NIA_MODE_INTENT) { FAIL("wrong mode"); return; }
    if (cfg.provider_count != 1) { FAIL("wrong provider count"); return; }

    nia_config_load_defaults(&cfg, NIA_VARIANT_FRAMEWORK);
    if (cfg.variant != NIA_VARIANT_FRAMEWORK) { FAIL("wrong variant"); return; }
    if (cfg.mode != NIA_MODE_FEATURES_INTENT) { FAIL("wrong mode"); return; }
    PASS();
}

static void test_status_strings(void)
{
    TEST(status_strings);
    if (strcmp(nia_status_str(NIA_OK), "OK") != 0) { FAIL("NIA_OK"); return; }
    if (strcmp(nia_status_str(NIA_ERR_POLICY_DENIED), "ERR_POLICY_DENIED") != 0) { FAIL("POLICY_DENIED"); return; }
    if (strcmp(nia_status_str(NIA_ERR_OVERFLOW), "ERR_OVERFLOW") != 0) { FAIL("OVERFLOW"); return; }
    PASS();
}

int main(void)
{
    printf("=== NIA Unit Tests ===\n\n");

    test_event_init();
    test_event_set_intent();
    test_event_add_feature();
    test_event_set_raw();
    test_event_type_mismatch();
    test_policy_allow_deny();
    test_tool_registry();
    test_config_defaults();
    test_status_strings();

    printf("\n=== Results: %d/%d passed, %d failed ===\n",
           tests_passed, tests_run, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
