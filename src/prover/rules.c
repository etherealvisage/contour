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
// absurdity
static struct prover_rule_result rule_absurd(struct proof_sequent *sequent,
    int which);
static bool is_rule_absurd(struct proof_sequent *sequent, int index);
// conjunction, left
static struct prover_rule_result rule_conj_l(struct proof_sequent *sequent,
    int which);
static bool is_rule_conj_l(struct proof_sequent *sequent, int index);
// disjunction, left
static struct prover_rule_result rule_disj_l(struct proof_sequent *sequent,
    int which);
static bool is_rule_disj_l(struct proof_sequent *sequent, int index);
// conjunction, right
static struct prover_rule_result rule_conj_r(struct proof_sequent *sequent,
    int which);
static bool is_rule_conj_r(struct proof_sequent *sequent);
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

/* left implication rules */

// implication, left, 1
static struct prover_rule_result rule_cond1_l(struct proof_sequent *sequent,
    int which);
static bool is_rule_cond1_l(struct proof_sequent *sequent, int index);
// implication, left, 2
static struct prover_rule_result rule_cond2_l(struct proof_sequent *sequent,
    int which);
static bool is_rule_cond2_l(struct proof_sequent *sequent, int index);
// implication, left, 3
static struct prover_rule_result rule_cond3_l(struct proof_sequent *sequent,
    int which);
static bool is_rule_cond3_l(struct proof_sequent *sequent, int index);
// implication, left, 4
static struct prover_rule_result rule_cond4_l(struct proof_sequent *sequent,
    int which);
static bool is_rule_cond4_l(struct proof_sequent *sequent, int index);

struct lrule lrules[] = {
    {rule_axiom, is_rule_axiom},
    {rule_absurd, is_rule_absurd},
    {rule_conj_l, is_rule_conj_l},
    {rule_disj_l, is_rule_disj_l},

    {rule_cond1_l, is_rule_cond1_l},
    {rule_cond2_l, is_rule_cond2_l},
    {rule_cond3_l, is_rule_cond3_l},
    {rule_cond4_l, is_rule_cond4_l}
};

struct rrule rrules[] = {
    {rule_conj_r, is_rule_conj_r},
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

// absurdity
static struct prover_rule_result rule_absurd(
    struct proof_sequent __attribute__((unused)) * sequent,
    int __attribute__((unused)) which) {

    // empty sequent as result
    return (struct prover_rule_result){NULL, NULL};
}

static bool is_rule_absurd(struct proof_sequent *sequent, int index) {
    //if(tree_node_cmp(sequent->left[index], sequent->right)) return true;
    if(sequent->left[index]->type == TREE_NODE_ABSURDITY) return true;
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

// conjunction, right
static struct prover_rule_result rule_conj_r(struct proof_sequent *sequent,
    int __attribute__((unused)) which) {

    struct proof_sequent *lseq = zalloc(sizeof(*lseq));
    struct proof_sequent *rseq = zalloc(sizeof(*rseq));

    lseq->right = sequent->right->child[0];
    tree_node_inc(lseq->right);
    rseq->right = sequent->right->child[1];
    tree_node_inc(rseq->right);

    lseq->left_count = rseq->left_count = sequent->left_count;
    lseq->left = zalloc(sizeof(*lseq->left) * lseq->left_count);
    rseq->left = zalloc(sizeof(*rseq->left) * rseq->left_count);

    // copy old sequent left children over
    for(int i = 0; i < sequent->left_count; i ++) {
        lseq->left[i] = sequent->left[i];
        tree_node_inc(lseq->left[i]);
        rseq->left[i] = sequent->left[i];
        tree_node_inc(rseq->left[i]);
    }

    return (struct prover_rule_result){lseq, rseq};
}

static bool is_rule_conj_r(struct proof_sequent *sequent) {
    return sequent->right->type == TREE_NODE_CONJUNCTION;
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

// conditional 1, left
static struct prover_rule_result rule_cond1_l(struct proof_sequent *sequent,
    int which) {

    struct proof_sequent *nseq = zalloc(sizeof(*nseq));

    nseq->right = sequent->right;
    tree_node_inc(nseq->right);

    nseq->left_count = sequent->left_count;
    nseq->left = zalloc(sizeof(*nseq->left) * nseq->left_count);

    for(int i = 0; i < sequent->left_count; i ++) {
        if(i != which) {
            nseq->left[i] = sequent->left[i];
            tree_node_inc(nseq->left[i]);
        }
        else {
            // add right child (the non-atomic one)
            nseq->left[i] = sequent->left[i]->child[1];
            tree_node_inc(nseq->left[i]);
        }
    }

    return (struct prover_rule_result){nseq, NULL};
}

static bool is_rule_cond1_l(struct proof_sequent *sequent, int index) {
    if(sequent->left[index]->type != TREE_NODE_IMPLICATION) return false;

    struct tree_node *impl = sequent->left[index];

    if(impl->child[1]->type != TREE_NODE_PREDICATE) return false;

    for(int i = 0; i < sequent->left_count; i ++) {
        if(!tree_node_cmp(impl->child[1], sequent->left[i])) continue;

        return true;
    }

    return false;
}

// conditional 2, left
static struct prover_rule_result rule_cond2_l(struct proof_sequent *sequent,
    int which) {

    struct proof_sequent *nseq = zalloc(sizeof(*nseq));

    nseq->right = sequent->right;
    tree_node_inc(nseq->right);

    nseq->left_count = sequent->left_count;
    nseq->left = zalloc(sizeof(*nseq->left) * nseq->left_count);

    for(int i = 0; i < sequent->left_count; i ++) {
        if(i != which) {
            nseq->left[i] = sequent->left[i];
            tree_node_inc(nseq->left[i]);
        }
        else {
            // turn (A&B)>C into A>(B>C)

            struct tree_node *r = zalloc(sizeof(*r));
            r->type = TREE_NODE_IMPLICATION;
            r->child_count = 2;
            r->refcount = 1;
            r->child = zalloc(sizeof(struct tree_node *) * r->child_count);

            // A
            r->child[0] = sequent->left[i]->child[0]->child[0];
            tree_node_inc(r->child[0]);

            // B>C
            struct tree_node *s = zalloc(sizeof(*s));
            r->child[1] = s;

            s->type = TREE_NODE_IMPLICATION;
            s->child_count = 2;
            s->refcount = 1;
            s->child = zalloc(sizeof(struct tree_node *) * s->child_count);

            s->child[0] = sequent->left[i]->child[0]->child[1];
            tree_node_inc(s->child[0]);
            s->child[1] = sequent->left[i]->child[1];
            tree_node_inc(s->child[1]);

            nseq->left[i] = r; // refcount = 1 already
        }
    }

    return (struct prover_rule_result){nseq, NULL};
}

static bool is_rule_cond2_l(struct proof_sequent *sequent, int index) {
    if(sequent->left[index]->type != TREE_NODE_IMPLICATION) return false;

    struct tree_node *impl = sequent->left[index];

    if(impl->child[0]->type != TREE_NODE_CONJUNCTION) return false;

    return true;
}

// conditional 3, left
static struct prover_rule_result rule_cond3_l(struct proof_sequent *sequent,
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
            // turn (A | B) > C into A>C,B>C

            // put A>C in place, append B>C
            struct tree_node *f = zalloc(sizeof(*f));
            f->type = TREE_NODE_IMPLICATION;
            f->child_count = 2;
            f->refcount = 1;
            f->child = zalloc(sizeof(struct tree_node *) * f->child_count);

            f->child[0] = sequent->left[i]->child[0]->child[0];
            f->child[1] = sequent->left[i]->child[1];
            tree_node_inc(f->child[0]);
            tree_node_inc(f->child[1]);
            nseq->left[i] = f;

            struct tree_node *s = zalloc(sizeof(*s));
            s->type = TREE_NODE_IMPLICATION;
            s->child_count = 2;
            s->refcount = 1;
            s->child = zalloc(sizeof(struct tree_node *) * s->child_count);

            s->child[0] = sequent->left[i]->child[0]->child[1];
            s->child[1] = sequent->left[i]->child[1];
            tree_node_inc(s->child[0]);
            tree_node_inc(s->child[1]);
            nseq->left[sequent->left_count] = s;
        }
    }

    return (struct prover_rule_result){nseq, NULL};
}

static bool is_rule_cond3_l(struct proof_sequent *sequent, int index) {
    if(sequent->left[index]->type != TREE_NODE_IMPLICATION) return false;

    struct tree_node *impl = sequent->left[index];

    if(impl->child[1]->type != TREE_NODE_DISJUNCTION) return false;

    return true;
}

// conditional 4, left
static struct prover_rule_result rule_cond4_l(struct proof_sequent *sequent,
    int which) {

    struct proof_sequent *lseq = zalloc(sizeof(*lseq));
    struct proof_sequent *rseq = zalloc(sizeof(*rseq));

    lseq->right = sequent->right;
    tree_node_inc(lseq->right);

    rseq->right = sequent->left[which]->child[1];
    tree_node_inc(rseq->right);

    lseq->left_count = sequent->left_count;
    lseq->left = zalloc(sizeof(*lseq->left) * lseq->left_count);
    rseq->left_count = sequent->left_count - 1;
    rseq->left = zalloc(sizeof(*lseq->left) * rseq->left_count);

    // fill lseq->left
    for(int i = 0; i < sequent->left_count; i ++) {
        if(i != which) {
            lseq->left[i] = sequent->left[i];
            tree_node_inc(lseq->left[i]);
        }
        else {
            // replace (A>B)>C with B>C
            struct tree_node *n = zalloc(sizeof(*n));
            n->child_count = 2;
            n->child = zalloc(sizeof(struct tree_node *) * n->child_count);

            n->child[0] = lseq->left[i]->child[0]->child[1];
            n->child[1] = lseq->left[i]->child[1];

            n->refcount = 1;
            lseq->left[i] = n;
        }
    }

    // fill rseq->left
    for(int i = 0; i < sequent->left_count; i ++) {
        if(i < which) {
            rseq->left[i] = sequent->left[i];
            tree_node_inc(rseq->left[i]);
        }
        else if(i > which) {
            rseq->left[i-1] = sequent->left[i];
            tree_node_inc(rseq->left[i-1]);
        }
    }

    return (struct prover_rule_result){lseq, rseq};
}

static bool is_rule_cond4_l(struct proof_sequent *sequent, int index) {
    if(sequent->left[index]->type != TREE_NODE_IMPLICATION) return false;

    struct tree_node *impl = sequent->left[index];

    if(impl->child[1]->type != TREE_NODE_IMPLICATION) return false;

    return true;
}
