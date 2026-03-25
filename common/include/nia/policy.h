#ifndef NIA_POLICY_H
#define NIA_POLICY_H

#include "nia/types.h"

#define NIA_POLICY_MAX_RULES 64
#define NIA_POLICY_ACTION_MAX 64

typedef enum {
    NIA_ACTION_SAFE,
    NIA_ACTION_CONTROLLED,
    NIA_ACTION_RESTRICTED,
} nia_action_class_t;

typedef enum {
    NIA_POLICY_ALLOW,
    NIA_POLICY_DENY,
    NIA_POLICY_CONFIRM,
} nia_policy_verdict_t;

typedef struct {
    char                action[NIA_POLICY_ACTION_MAX];
    nia_policy_verdict_t verdict;
    nia_action_class_t  action_class;
} nia_policy_rule_t;

typedef struct {
    nia_policy_rule_t rules[NIA_POLICY_MAX_RULES];
    int               count;
    bool              default_deny;
} nia_policy_engine_t;

nia_status_t        nia_policy_init(nia_policy_engine_t *engine);
nia_status_t        nia_policy_add_rule(nia_policy_engine_t *engine,
                                        const char *action,
                                        nia_policy_verdict_t verdict,
                                        nia_action_class_t action_class);
nia_policy_verdict_t nia_policy_evaluate(const nia_policy_engine_t *engine,
                                         const char *action);
nia_status_t        nia_policy_set_default_deny(nia_policy_engine_t *engine, bool deny);
void                nia_policy_dump(const nia_policy_engine_t *engine);

#endif /* NIA_POLICY_H */
