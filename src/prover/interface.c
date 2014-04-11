#include "interface.h"
#include "rules.h"
#include "util.h"
#include "log/log.h"

static bool prove_dfs(struct proof_sequent *sequent);

struct proof_sequent *prove(struct tree_node *expression) {
    struct proof_sequent *initial_sequent = zalloc(sizeof(*initial_sequent));

    initial_sequent->left_count = 0;
    initial_sequent->right = expression;

    if(prove_dfs(initial_sequent)) return initial_sequent;
    // XXX: leak
    else return NULL;

    #if 0
    int count;
    struct prover_rule_application *results = prover_rules_find(initial_sequent,
        &count);
    contour_log_info("results: %p", results);
    for(int i = 0; i < count; i ++) {
        struct prover_rule_application *r = results + i;
        //contour_log_info("results[%i]: = {%p,%p}", i, r->left, r->right);
        contour_log_info("results[%i] = {%p, %i}", i, r->rule, r->index);

        char buffer[1024];
        if(r->left) {
            struct proof_sequent *left = r->left;
            contour_log_info("left sequent:");
            tree_node_dump(buffer, left->right);
            contour_log_info("right: %s", buffer);

            for(int j = 0; j < left->left_count; j ++) {
                tree_node_dump(buffer, left->left[j]);
                contour_log_info("left %i: %s", j, buffer);
            }
        }
        if(r->right) {
            struct proof_sequent *right = r->right;
            contour_log_info("right sequent:");
            tree_node_dump(buffer, right->right);
            contour_log_info("right: %s", buffer);

            for(int j = 0; j < right->left_count; j ++) {
                tree_node_dump(buffer, right->left[j]);
                contour_log_info("left %i: %s", j, buffer);
            }
        }
    }
    #endif
}

static bool prove_dfs(struct proof_sequent *sequent) {
    if(!sequent) return true;

    int count;
    struct prover_rule_application *results = prover_rules_find(sequent,
        &count);

    if(count == 0) {
        free(results);
        return false;
    }

    for(int i = 0; i < count; i ++) {
        struct prover_rule_application *app = results + i;

        struct prover_rule_result result = app->rule(sequent, app->index);

        // do the right first, it can be smaller.
        if(prove_dfs(result.right) && prove_dfs(result.left)) {
            sequent->sleft = result.left;
            sequent->sright = result.right;
            return true;
        }
        else {
            // XXX: free memory + do refcounting etc.
        }
    }

    return false;
}
