#ifndef CONTOUR_PROVER_RULES_H
#define CONTOUR_PROVER_RULES_H

#include "proof.h"

struct prover_rule_result {
    struct proof_sequent *left, *right;
};

// rule application, generates new sequent(s). which is ignored for non-left
// rules.
typedef struct prover_rule_result (*prover_rule_function)(
    struct proof_sequent *sequent, int which);

struct prover_rule_application {
    prover_rule_function rule;
    int index;
    const char *name;
    const char *latex_name;
};

struct prover_rule_application *prover_rules_find(
    struct proof_sequent *sequent, int *count);

#endif
