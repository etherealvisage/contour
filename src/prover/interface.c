#include "interface.h"
#include "rules.h"
#include "util.h"
#include "log/log.h"

struct proof *prove(struct tree_node *expression) {
    struct proof_sequent *initial_sequent = zalloc(sizeof(*initial_sequent));

    initial_sequent->left_count = 0;
    initial_sequent->right = expression;

    int count;
    struct prover_rule_application *results = prover_rules_find(initial_sequent, &count);
    contour_log_info("results: %p", results);
    for(int i = 0; i < count; i ++) {
        struct prover_rule_application *r = results + i;
        //contour_log_info("results[%i]: = {%p,%p}", i, r->left, r->right);
        contour_log_info("results[%i] = {%p, %i}", i, r->rule, r->index);

        #if 0
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
        #endif
    }

    return NULL;
}
