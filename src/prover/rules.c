#include <stdio.h>

#include "rules.h"
#include "util.h"

struct lrule {
    prover_rule_function rule;
    bool (*is)(struct proof_sequent *sequent, int index);
};

struct rrule {
    prover_rule_function rule;
    bool (*is)(struct proof_sequent *sequent);
};

// axiom
static struct prover_rule_result rule_axiom(struct proof_sequent *sequent,
    int which);
static bool is_rule_axiom(struct proof_sequent *sequent, int index);
// conjunction, left
static struct prover_rule_result rule_conj_l(struct proof_sequent *sequent,
    int which);
static bool is_rule_conj_l(struct proof_sequent *sequent, int index);
// disjunction, left
static struct prover_rule_result rule_disj_l(struct proof_sequent *sequent,
    int which);
static bool is_rule_disj_l(struct proof_sequent *sequent, int index);
// disjunction (left/right disjunct), right
static struct prover_rule_result rule_disjl_r(struct proof_sequent *sequent,
    int which);
static struct prover_rule_result rule_disjr_r(struct proof_sequent *sequent,
    int which);
static bool is_rule_disj_r(struct proof_sequent *sequent);
// implication, right
static struct prover_rule_result rule_impl_r(struct proof_sequent *sequent,
    int which);
static bool is_rule_impl_r(struct proof_sequent *sequent);

struct lrule lrules[] = {
    {rule_axiom, is_rule_axiom},
    {rule_conj_l, is_rule_conj_l},
    {rule_disj_l, is_rule_disj_l}
};

struct rrule rrules[] = {
    {rule_disjl_r, is_rule_disj_r},
    {rule_disjr_r, is_rule_disj_r},
    {rule_impl_r, is_rule_impl_r}
};

struct prover_rule_application *prover_rules_find(
    struct proof_sequent *sequent, int *count) {

    struct prover_rule_application *applications = NULL;
    *count = 0;

    for(int index = 0; index < sequent->left_count; index ++) {
        for(int j = 0; j < (int)(sizeof(lrules)/sizeof(lrules[0])); j ++) {
            if(!lrules[j].is(sequent, index)) continue;

            applications = realloc(applications, sizeof(*applications) * (*count + 1));
            applications[*count].rule = lrules[j].rule;
            applications[*count].index = index;
            (*count) ++;
        }
    }

    for(int j = 0; j < (int)(sizeof(rrules)/sizeof(rrules[0])); j ++) {
        if(!rrules[j].is(sequent)) continue;

        applications = realloc(applications, sizeof(*applications) * (*count + 1));
        applications[*count].rule = rrules[j].rule;
        applications[*count].index = -1; // unused
        (*count) ++;
    }

    return applications;
}

// axiom
static struct prover_rule_result rule_axiom(
    struct proof_sequent __attribute__((unused)) * sequent,
    int __attribute__((unused)) which) {

    // empty sequent as result
    return (struct prover_rule_result){NULL, NULL};
}

static bool is_rule_axiom(struct proof_sequent *sequent, int index) {
    if(tree_node_cmp(sequent->left[index], sequent->right)) return true;
    return false;
}

// conjunction, left
static struct prover_rule_result rule_conj_l(struct proof_sequent *sequent,
    int which) {

    struct proof_sequent *nseq = zalloc(sizeof(*nseq));

    nseq->right = sequent->right;
    tree_node_inc(nseq->right);

    nseq->left_count = sequent->left_count + 1;
    nseq->left = zalloc(sizeof(*nseq->left) * nseq->left_count);

    for(int i = 0; i < sequent->left_count; i ++) {
        if(i != which) {
            nseq->left[i] = sequent->left[i];
            tree_node_inc(nseq->left[i]);
        }
        else {
            // add left child in place
            nseq->left[i] = sequent->left[i]->child[0];
            tree_node_inc(nseq->left[i]);
            // add right child onto end of left sequent
            nseq->left[sequent->left_count-1] = sequent->left[i]->child[1];
            tree_node_inc(nseq->left[sequent->left_count-1]);
        }
    }

    return (struct prover_rule_result){nseq, NULL};
}

static bool is_rule_conj_l(struct proof_sequent *sequent, int index) {
    return sequent->left[index]->type == TREE_NODE_CONJUNCTION;
}

// disjunction, left
static struct prover_rule_result rule_disj_l(struct proof_sequent *sequent,
    int which) {

    struct proof_sequent *lseq = zalloc(sizeof(*lseq));
    struct proof_sequent *rseq = zalloc(sizeof(*rseq));

    lseq->right = sequent->right;
    tree_node_inc(lseq->right);
    rseq->right = sequent->right;
    tree_node_inc(rseq->right);

    lseq->left_count = sequent->left_count;
    lseq->left = zalloc(sizeof(*lseq->left) * lseq->left_count);
    rseq->left_count = sequent->left_count;
    rseq->left = zalloc(sizeof(*lseq->left) * lseq->left_count);

    for(int i = 0; i < sequent->left_count; i ++) {
        if(i != which) {
            lseq->left[i] = sequent->left[i];
            tree_node_inc(lseq->left[i]);
            rseq->left[i] = sequent->left[i];
            tree_node_inc(rseq->left[i]);
        }
        else {
            // add left child in place
            lseq->left[i] = sequent->left[i]->child[0];
            tree_node_inc(lseq->left[i]);
            rseq->left[i] = sequent->left[i]->child[0];
            tree_node_inc(rseq->left[i]);
        }
    }

    return (struct prover_rule_result){lseq, rseq};
}

static bool is_rule_disj_l(struct proof_sequent *sequent, int index) {
    return sequent->left[index]->type == TREE_NODE_DISJUNCTION;
}

// disjunction (left disjunct), right
static struct prover_rule_result rule_disjl_r(struct proof_sequent *sequent,
    int __attribute__((unused)) which) {

    struct proof_sequent *nseq = zalloc(sizeof(*nseq));

    nseq->right = sequent->right->child[0];
    tree_node_inc(nseq->right);

    nseq->left_count = sequent->left_count;
    nseq->left = zalloc(sizeof(*nseq->left) * nseq->left_count);

    // copy old sequent left children over
    for(int i = 0; i < sequent->left_count; i ++) {
        nseq->left[i] = sequent->left[i];
        tree_node_inc(nseq->left[i]);
    }

    return (struct prover_rule_result){nseq, NULL};
}

// disjunction (right disjunct), right
static struct prover_rule_result rule_disjr_r(struct proof_sequent *sequent,
    int __attribute__((unused)) which) {

    struct proof_sequent *nseq = zalloc(sizeof(*nseq));

    nseq->right = sequent->right->child[1];
    tree_node_inc(nseq->right);

    nseq->left_count = sequent->left_count;
    nseq->left = zalloc(sizeof(*nseq->left) * nseq->left_count);

    // copy old sequent left children over
    for(int i = 0; i < sequent->left_count; i ++) {
        nseq->left[i] = sequent->left[i];
        tree_node_inc(nseq->left[i]);
    }

    return (struct prover_rule_result){nseq, NULL};
}

static bool is_rule_disj_r(struct proof_sequent *sequent) {
    return sequent->right->type == TREE_NODE_DISJUNCTION;
}

// implication, right
static struct prover_rule_result rule_impl_r(struct proof_sequent *sequent,
    int __attribute__((unused)) which) {

    struct proof_sequent *nseq = zalloc(sizeof(*nseq));

    nseq->right = sequent->right->child[1];
    tree_node_inc(nseq->right);

    nseq->left_count = sequent->left_count + 1;
    nseq->left = zalloc(sizeof(*nseq->left) * nseq->left_count);

    // copy old sequent left children over
    for(int i = 0; i < sequent->left_count; i ++) {
        nseq->left[i] = sequent->left[i];
        tree_node_inc(nseq->left[i]);
    }
    // add new left child

    nseq->left[nseq->left_count-1] = sequent->right->child[0];
    tree_node_inc(nseq->left[nseq->left_count-1]);

    return (struct prover_rule_result){nseq, NULL};
}

static bool is_rule_impl_r(struct proof_sequent *sequent) {
    return sequent->right->type == TREE_NODE_IMPLICATION;
}

