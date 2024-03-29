#include <stdio.h>

#include "rules.h"
#include "util.h"

struct lrule {
    prover_rule_function rule;
    bool (*is)(struct proof_sequent *sequent, int index);
    const char *name;
    const char *latex_name;
    bool invertible;
};

struct rrule {
    prover_rule_function rule;
    bool (*is)(struct proof_sequent *sequent);
    const char *name;
    const char *latex_name;
    bool invertible;
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
    {rule_axiom, is_rule_axiom, "Axiom", "Axiom", true},
    {rule_absurd, is_rule_absurd, "Absurdity", "Absurd", true},
    {rule_conj_l, is_rule_conj_l, "&L", "$\\wedge \\Rightarrow$", true},
    {rule_disj_l, is_rule_disj_l, "|L", "$\\vee \\Rightarrow$", true},

    {rule_cond1_l, is_rule_cond1_l, ">1L", "$\\supset \\Rightarrow_1$", true},
    {rule_cond2_l, is_rule_cond2_l, ">2L", "$\\supset \\Rightarrow_2$", true},
    {rule_cond3_l, is_rule_cond3_l, ">3L", "$\\supset \\Rightarrow_3$", true},
    {rule_cond4_l, is_rule_cond4_l, ">4L", "$\\supset \\Rightarrow_4$", false}
};
const int lrules_count = 8;

struct rrule rrules[] = {
    {rule_conj_r, is_rule_conj_r, "R&", "$\\Rightarrow\\wedge$", true},
    {rule_disjl_r, is_rule_disj_r, "R|1", "$\\Rightarrow\\vee$", false},
    {rule_disjr_r, is_rule_disj_r, "R|2", "$\\Rightarrow\\vee$", false},
    {rule_impl_r, is_rule_impl_r, "R>", "$\\Rightarrow\\supset$", false}
};
const int rrules_count = 4;

struct prover_rule_application *prover_rules_find(
    struct proof_sequent *sequent, int *count) {

    struct prover_rule_application *applications = NULL;
    *count = 0;

    for(int j = 0; j < lrules_count; j ++) {
        for(int index = 0; index < sequent->left_count; index ++) {
            if(!lrules[j].is(sequent, index)) continue;

            applications = realloc(applications,
                sizeof(*applications) * (*count + 1));
            applications[*count].rule = lrules[j].rule;
            applications[*count].index = index;
            applications[*count].name = lrules[j].name;
            applications[*count].latex_name = lrules[j].latex_name;
            applications[*count].invertible = lrules[j].invertible;
            (*count) ++;
        }
    }

    for(int j = 0; j < rrules_count; j ++) {
        if(!rrules[j].is(sequent)) continue;

        applications = realloc(applications,
            sizeof(*applications) * (*count + 1));
        applications[*count].rule = rrules[j].rule;
        applications[*count].index = -1; // unused
        applications[*count].name = rrules[j].name;
        applications[*count].latex_name = rrules[j].latex_name;
        applications[*count].invertible = rrules[j].invertible;
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

    struct proof_sequent *nseq = sequent_make(sequent->left_count + 1);

    nseq->right = tree_node_inc(sequent->right);

    for(int i = 0; i < sequent->left_count; i ++) {
        if(i != which) {
            nseq->left[i] = tree_node_inc(sequent->left[i]);
        }
        else {
            // add left child in place
            nseq->left[i] = tree_node_inc(sequent->left[i]->child[0]);
            // add right child onto end of left sequent
            nseq->left[sequent->left_count] =
                tree_node_inc(sequent->left[i]->child[1]);
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

    struct proof_sequent *lseq = sequent_make(sequent->left_count);
    struct proof_sequent *rseq = sequent_make(sequent->left_count);

    lseq->right = tree_node_inc(sequent->right);
    rseq->right = tree_node_inc(sequent->right);

    for(int i = 0; i < sequent->left_count; i ++) {
        if(i != which) {
            lseq->left[i] = tree_node_inc(sequent->left[i]);
            rseq->left[i] = tree_node_inc(sequent->left[i]);
        }
        else {
            // add left child in place
            lseq->left[i] = tree_node_inc(sequent->left[i]->child[0]);
            rseq->left[i] = tree_node_inc(sequent->left[i]->child[1]);
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

    struct proof_sequent *lseq = sequent_make(sequent->left_count);
    struct proof_sequent *rseq = sequent_make(sequent->left_count);

    lseq->right = tree_node_inc(sequent->right->child[0]);
    rseq->right = tree_node_inc(sequent->right->child[1]);

    // copy old sequent left children over
    for(int i = 0; i < sequent->left_count; i ++) {
        lseq->left[i] = tree_node_inc(sequent->left[i]);
        rseq->left[i] = tree_node_inc(sequent->left[i]);
    }

    return (struct prover_rule_result){lseq, rseq};
}

static bool is_rule_conj_r(struct proof_sequent *sequent) {
    return sequent->right->type == TREE_NODE_CONJUNCTION;
}

// disjunction (left disjunct), right
static struct prover_rule_result rule_disjl_r(struct proof_sequent *sequent,
    int __attribute__((unused)) which) {

    struct proof_sequent *nseq = sequent_make(sequent->left_count);

    nseq->right = tree_node_inc(sequent->right->child[0]);

    // copy old sequent left children over
    for(int i = 0; i < sequent->left_count; i ++) {
        nseq->left[i] = tree_node_inc(sequent->left[i]);
    }

    return (struct prover_rule_result){nseq, NULL};
}

// disjunction (right disjunct), right
static struct prover_rule_result rule_disjr_r(struct proof_sequent *sequent,
    int __attribute__((unused)) which) {

    struct proof_sequent *nseq = sequent_make(sequent->left_count);

    nseq->right = tree_node_inc(sequent->right->child[1]);

    // copy old sequent left children over
    for(int i = 0; i < sequent->left_count; i ++) {
        nseq->left[i] = tree_node_inc(sequent->left[i]);
    }

    return (struct prover_rule_result){nseq, NULL};
}

static bool is_rule_disj_r(struct proof_sequent *sequent) {
    return sequent->right->type == TREE_NODE_DISJUNCTION;
}

// implication, right
static struct prover_rule_result rule_impl_r(struct proof_sequent *sequent,
    int __attribute__((unused)) which) {

    struct proof_sequent *nseq = sequent_make(sequent->left_count + 1);

    nseq->right = tree_node_inc(sequent->right->child[1]);

    // copy old sequent left children over
    for(int i = 0; i < sequent->left_count; i ++) {
        nseq->left[i] = tree_node_inc(sequent->left[i]);
    }
    // add new left child

    nseq->left[nseq->left_count-1] = tree_node_inc(sequent->right->child[0]);

    return (struct prover_rule_result){nseq, NULL};
}

static bool is_rule_impl_r(struct proof_sequent *sequent) {
    return sequent->right->type == TREE_NODE_IMPLICATION;
}

// conditional 1, left
static struct prover_rule_result rule_cond1_l(struct proof_sequent *sequent,
    int which) {

    struct proof_sequent *nseq = sequent_make(sequent->left_count);

    nseq->right = tree_node_inc(sequent->right);

    for(int i = 0; i < sequent->left_count; i ++) {
        if(i != which) {
            nseq->left[i] = tree_node_inc(sequent->left[i]);
        }
        else {
            // add right child (the non-atomic one)
            nseq->left[i] = tree_node_inc(sequent->left[i]->child[1]);
        }
    }

    return (struct prover_rule_result){nseq, NULL};
}

static bool is_rule_cond1_l(struct proof_sequent *sequent, int index) {
    if(sequent->left[index]->type != TREE_NODE_IMPLICATION) return false;

    struct tree_node *impl = sequent->left[index];

    if(impl->child[0]->type != TREE_NODE_PREDICATE) return false;

    for(int i = 0; i < sequent->left_count; i ++) {
        if(!tree_node_cmp(impl->child[0], sequent->left[i])) continue;

        return true;
    }

    return false;
}

// conditional 2, left
static struct prover_rule_result rule_cond2_l(struct proof_sequent *sequent,
    int which) {

    struct proof_sequent *nseq = sequent_make(sequent->left_count);

    nseq->right = tree_node_inc(sequent->right);

    for(int i = 0; i < sequent->left_count; i ++) {
        if(i != which) {
            nseq->left[i] = tree_node_inc(sequent->left[i]);
        }
        else {
            // turn (A&B)>C into A>(B>C)

            struct tree_node *r = tree_node_make(TREE_NODE_IMPLICATION, 2);
            // A
            r->child[0] = tree_node_inc(sequent->left[i]->child[0]->child[0]);
            // B>C
            struct tree_node *s = tree_node_make(TREE_NODE_IMPLICATION, 2);
            r->child[1] = s;

            s->child[0] = tree_node_inc(sequent->left[i]->child[0]->child[1]);
            s->child[1] = tree_node_inc(sequent->left[i]->child[1]);

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

    struct proof_sequent *nseq = sequent_make(sequent->left_count + 1);

    nseq->right = tree_node_inc(sequent->right);

    for(int i = 0; i < sequent->left_count; i ++) {
        if(i != which) {
            nseq->left[i] = tree_node_inc(sequent->left[i]);
        }
        else {
            // turn (A | B) > C into A>C,B>C

            // put A>C in place, append B>C
            struct tree_node *f = tree_node_make(TREE_NODE_IMPLICATION, 2);

            f->child[0] = tree_node_inc(sequent->left[i]->child[0]->child[0]);
            f->child[1] = tree_node_inc(sequent->left[i]->child[1]);
            nseq->left[i] = f;

            struct tree_node *s = tree_node_make(TREE_NODE_IMPLICATION, 2);

            s->child[0] = tree_node_inc(sequent->left[i]->child[0]->child[1]);
            s->child[1] = tree_node_inc(sequent->left[i]->child[1]);
            nseq->left[sequent->left_count] = s;
        }
    }

    return (struct prover_rule_result){nseq, NULL};
}

static bool is_rule_cond3_l(struct proof_sequent *sequent, int index) {
    if(sequent->left[index]->type != TREE_NODE_IMPLICATION) return false;

    struct tree_node *impl = sequent->left[index];

    if(impl->child[0]->type != TREE_NODE_DISJUNCTION) return false;

    return true;
}

// conditional 4, left
static struct prover_rule_result rule_cond4_l(struct proof_sequent *sequent,
    int which) {

    struct proof_sequent *lseq = sequent_make(sequent->left_count);
    struct proof_sequent *rseq = sequent_make(sequent->left_count);

    lseq->right = tree_node_make(TREE_NODE_IMPLICATION, 2);

    lseq->right->child[0] =
        tree_node_inc(sequent->left[which]->child[0]->child[0]);
    lseq->right->child[1] =
        tree_node_inc(sequent->left[which]->child[0]->child[1]);

    rseq->right = tree_node_inc(sequent->right);

    // fill lseq->left
    for(int i = 0; i < sequent->left_count; i ++) {
        if(i != which) {
            lseq->left[i] = tree_node_inc(sequent->left[i]);

            rseq->left[i] = tree_node_inc(sequent->left[i]);
        }
        else {
            // (left) replace (A>B)>C with B>C
            struct tree_node *n = tree_node_make(TREE_NODE_IMPLICATION, 2);
            n->child[0] = tree_node_inc(sequent->left[i]->child[0]->child[1]);
            n->child[1] = tree_node_inc(sequent->left[i]->child[1]);

            lseq->left[i] = n;

            // (right) pull out C from (A>B)>C
            rseq->left[i] = tree_node_inc(sequent->left[i]->child[1]);
        }
    }

    return (struct prover_rule_result){lseq, rseq};
}

static bool is_rule_cond4_l(struct proof_sequent *sequent, int index) {
    if(sequent->left[index]->type != TREE_NODE_IMPLICATION) return false;

    struct tree_node *impl = sequent->left[index];

    if(impl->child[0]->type != TREE_NODE_IMPLICATION) return false;

    return true;
}
