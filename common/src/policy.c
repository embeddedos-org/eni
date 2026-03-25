#include "nia/policy.h"
#include "nia/log.h"
#include <string.h>
#include <stdio.h>

nia_status_t nia_policy_init(nia_policy_engine_t *engine)
{
    if (!engine) return NIA_ERR_INVALID;
    memset(engine, 0, sizeof(*engine));
    engine->default_deny = false;
    return NIA_OK;
}

nia_status_t nia_policy_add_rule(nia_policy_engine_t *engine,
                                  const char *action,
                                  nia_policy_verdict_t verdict,
                                  nia_action_class_t action_class)
{
    if (!engine || !action) return NIA_ERR_INVALID;
    if (engine->count >= NIA_POLICY_MAX_RULES) return NIA_ERR_OVERFLOW;

    nia_policy_rule_t *rule = &engine->rules[engine->count];
    size_t len = strlen(action);
    if (len >= NIA_POLICY_ACTION_MAX) len = NIA_POLICY_ACTION_MAX - 1;
    memcpy(rule->action, action, len);
    rule->action[len] = '\0';
    rule->verdict      = verdict;
    rule->action_class = action_class;
    engine->count++;

    NIA_LOG_DEBUG("policy", "added rule: %s → %s",
                  action,
                  verdict == NIA_POLICY_ALLOW ? "allow" :
                  verdict == NIA_POLICY_DENY  ? "deny"  : "confirm");

    return NIA_OK;
}

nia_policy_verdict_t nia_policy_evaluate(const nia_policy_engine_t *engine,
                                          const char *action)
{
    if (!engine || !action) return NIA_POLICY_DENY;

    for (int i = 0; i < engine->count; i++) {
        if (strcmp(engine->rules[i].action, action) == 0) {
            return engine->rules[i].verdict;
        }
    }

    return engine->default_deny ? NIA_POLICY_DENY : NIA_POLICY_ALLOW;
}

nia_status_t nia_policy_set_default_deny(nia_policy_engine_t *engine, bool deny)
{
    if (!engine) return NIA_ERR_INVALID;
    engine->default_deny = deny;
    return NIA_OK;
}

void nia_policy_dump(const nia_policy_engine_t *engine)
{
    if (!engine) return;

    printf("[policy] %d rules (default=%s)\n",
           engine->count, engine->default_deny ? "deny" : "allow");

    for (int i = 0; i < engine->count; i++) {
        const nia_policy_rule_t *r = &engine->rules[i];
        const char *verdict_str =
            r->verdict == NIA_POLICY_ALLOW   ? "allow" :
            r->verdict == NIA_POLICY_DENY    ? "deny"  : "confirm";
        const char *class_str =
            r->action_class == NIA_ACTION_SAFE       ? "safe" :
            r->action_class == NIA_ACTION_CONTROLLED ? "controlled" : "restricted";
        printf("  [%d] %s → %s (%s)\n", i, r->action, verdict_str, class_str);
    }
}
